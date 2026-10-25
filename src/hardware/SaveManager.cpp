#include "SaveManager.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <new>
namespace {
const char *slots[] = {"/wilds-a.sav", "/wilds-b.sav"};
}
bool SaveManager::begin(wilds::Game &g) {
  auto start = micros();
  // Never auto-format: mount failure must not destroy a possibly recoverable farm.
  if (!LittleFS.begin(false, "/littlefs", 10, "littlefs")) {
    g.saveStatus = "Storage unavailable";
    g.message("Storage unavailable. Your farm cannot be saved.", 15);
    Serial.println("SAVE mount failed: no autoformat");
    return false;
  }
  buffer =
      (uint8_t *)heap_caps_malloc(wilds::SaveCodec::Capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  auto *mem = heap_caps_malloc(sizeof(wilds::SaveState), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (mem)
    snapshot = new (mem) wilds::SaveState();
  if (!buffer || !snapshot) {
    g.saveStatus = "Save memory unavailable";
    return false;
  }
  bool found = false, damaged = false;
  for (int slot = 0; slot < 2; slot++) {
    if (!LittleFS.exists(slots[slot]))
      continue;
    auto file = LittleFS.open(slots[slot], "r");
    size_t n = file ? file.size() : 0;
    bool valid = file && n <= wilds::SaveCodec::Capacity && file.read(buffer, n) == n;
    file.close();
    wilds::SaveInfo info;
    valid = valid && wilds::SaveCodec::decode(buffer, n, *snapshot, info);
    Serial.printf("SAVE slot=%d valid=%d sequence=%u bytes=%u\n", slot, valid,
                  valid ? info.sequence : 0, unsigned(n));
    if (!valid) {
      damaged = true;
      continue;
    }
    if (!found || int32_t(info.sequence - sequence) > 0) {
      snapshot->apply(g);
      sequence = info.sequence;
      currentSlot = slot;
      found = true;
    }
  }
  queue = xQueueCreate(1, sizeof(uint8_t));
  if (!queue ||
      xTaskCreatePinnedToCore(worker, "wilds-save", 4096, this, 1, nullptr, 0) != pdPASS) {
    g.saveStatus = "Save worker unavailable";
    return false;
  }
  available = true;
  lastSave = millis();
  g.saveStatus = found ? "Saved" : "New garden";
  if (damaged)
    g.message(found ? "Recovered your previous healthy save"
                    : "Saves unreadable. Started a fresh garden.",
              12);
  else if (found)
    g.message("Welcome back. Your garden is safe.", 6);
  Serial.printf("SAVE load_us=%u sequence=%u loaded=%d damaged=%d\n", micros() - start, sequence,
                found, damaged);
  if (!found) {
    g.saveRequested = true;
    retryNeeded = true;
  }
  return true;
}
void SaveManager::request(wilds::Game &g, uint32_t now, bool exit) {
  uint32_t encodeStart = micros();
  snapshot->capture(g);
  pendingRevision = g.revision;
  length = wilds::SaveCodec::encode(*snapshot, buffer, wilds::SaveCodec::Capacity, sequence + 1);
  if (!length) {
    g.saveStatus = "Save failed";
    return;
  }
  Serial.printf("SAVE encode_us=%u bytes=%u revision=%u\n", micros() - encodeStart,
                unsigned(length), pendingRevision);
  busy = true;
  exitAfterSave = exit;
  g.saveStatus = "Saving...";
  result.store(0);
  uint8_t slot = currentSlot == 0 ? 1 : 0;
  if (xQueueSend(queue, &slot, 0) != pdTRUE) {
    busy = false;
    g.saveStatus = "Save queue failed";
    return;
  }
  lastChange = now;
}
void SaveManager::worker(void *context) {
  auto *self = static_cast<SaveManager *>(context);
  uint8_t slot;
  for (;;) {
    xQueueReceive(self->queue, &slot, portMAX_DELAY);
    auto start = micros();
    const char *temp = "/wilds.tmp";
    auto file = LittleFS.open(temp, "w");
    bool ok = bool(file);
    for (size_t offset = 0; ok && offset < self->length; offset += 512) {
      size_t n = std::min(size_t(512), self->length - offset);
      ok = file.write(self->buffer + offset, n) == n;
      vTaskDelay(1);
    }
    if (file) {
      file.flush();
      file.close();
    }
    // Byte-for-byte readback includes header and payload checksums prepared by the codec.
    if (ok) {
      file = LittleFS.open(temp, "r");
      ok = file && file.size() == self->length;
      uint8_t check[512];
      for (size_t pos = 0; ok && pos < self->length; pos += 512) {
        size_t n = std::min(size_t(512), self->length - pos);
        ok = file.read(check, n) == n && memcmp(check, self->buffer + pos, n) == 0;
        vTaskDelay(1);
      }
      file.close();
    }
    // Only remove the older slot. The current good slot survives every failure point.
    if (ok && LittleFS.exists(slots[slot]))
      ok = LittleFS.remove(slots[slot]);
    if (ok)
      ok = LittleFS.rename(temp, slots[slot]);
    Serial.printf("SAVE write_us=%u bytes=%u target=%d sequence=%u ok=%d\n", micros() - start,
                  unsigned(self->length), slot, self->sequence + 1, ok);
    self->result.store(ok ? int(slot) + 1 : -1, std::memory_order_release);
  }
}
void SaveManager::update(wilds::Game &g, uint32_t now) {
  if (!available) {
    g.saveRequested = false;
    g.exitRequested = false;
    return;
  }
  if (busy) {
    int done = result.load(std::memory_order_acquire);
    if (!done)
      return;
    busy = false;
    if (done > 0) {
      currentSlot = done - 1;
      ++sequence;
      savedRevision = pendingRevision;
      lastSave = now;
      retryNeeded = false;
      g.saveStatus = "Saved";
      if (exitAfterSave && g.exitRequested && g.revision == pendingRevision) {
        g.rest();
      }
    } else {
      g.saveStatus = "Save failed - retrying";
      retryNeeded = true;
      lastChange = now;
      g.message("Save failed. Keeping the previous farm safe.", 8);
    }
  }
  if (g.revision != observedRevision) {
    observedRevision = g.revision;
    lastChange = now;
    g.saveStatus = "Unsaved changes";
  }
  bool explicitSave = g.saveRequested;
  bool exiting = g.exitRequested;
  if (explicitSave || ((g.revision != savedRevision || retryNeeded) && now - lastChange >= 2500) ||
      (!g.resting && now - lastSave >= 30000)) {
    g.saveRequested = false;
    request(g, now, exiting);
  }
}

#ifdef WILDS_SAVE_TESTING
bool SaveManager::testFault(int mode) {
  if (!available || busy || currentSlot < 0) {
    Serial.println("FAULT refused: save busy or absent");
    return false;
  }
  const char *backup = "/wilds-test-backup.sav";
  auto readFile = [&](const char *path) -> size_t {
    auto f = LittleFS.open(path, "r");
    size_t n = f ? f.size() : 0;
    if (!f || n > wilds::SaveCodec::Capacity || f.read(buffer, n) != n)
      n = 0;
    f.close();
    return n;
  };
  wilds::SaveInfo info;
  if (mode == 1) {
    if (LittleFS.exists(backup)) {
      Serial.println("FAULT refused: restore the existing backup first");
      return false;
    }
    size_t n = readFile(slots[currentSlot]);
    if (!n || !wilds::SaveCodec::decode(buffer, n, *snapshot, info))
      return false;
    auto f = LittleFS.open(backup, "w");
    bool ok = f && f.write(buffer, n) == n;
    if (f) {
      f.flush();
      f.close();
    }
    if (!ok)
      return false;
    size_t check = readFile(backup);
    if (check != n || !wilds::SaveCodec::decode(buffer, check, *snapshot, info))
      return false;
    f = LittleFS.open(slots[currentSlot], "r+");
    if (!f)
      return false;
    f.seek(wilds::SaveCodec::HeaderSize + 2);
    int value = f.read();
    f.seek(wilds::SaveCodec::HeaderSize + 2);
    ok = value >= 0 && f.write(uint8_t(value ^ 0x80)) == 1;
    f.flush();
    f.close();
    Serial.printf("FAULT corrupt_latest slot=%d sequence=%u backup_valid=1 ok=%d\n", currentSlot,
                  sequence, ok);
    if (ok) {
      delay(100);
      ESP.restart();
    }
    return ok;
  }
  if (mode == 2) {
    size_t n = readFile(slots[currentSlot]);
    if (!n || !wilds::SaveCodec::decode(buffer, n, *snapshot, info))
      return false;
    auto f = LittleFS.open("/wilds.tmp", "w");
    bool ok = f && f.write(buffer, n / 2) == n / 2;
    if (f) {
      f.flush();
      f.close();
    }
    Serial.printf("FAULT interrupted_temp bytes=%u good_slot=%d sequence=%u ok=%d\n",
                  unsigned(n / 2), currentSlot, sequence, ok);
    if (ok) {
      delay(100);
      ESP.restart();
    }
    return ok;
  }
  if (mode == 3) {
    size_t n = readFile(backup);
    if (!n || !wilds::SaveCodec::decode(buffer, n, *snapshot, info))
      return false;
    // Restore to the slot opposite the fallback selected on boot.
    int target = currentSlot == 0 ? 1 : 0;
    if (LittleFS.exists(slots[target]) && !LittleFS.remove(slots[target]))
      return false;
    if (!LittleFS.rename(backup, slots[target]))
      return false;
    Serial.printf("FAULT restored_backup sequence=%u target=%d\n", info.sequence, target);
    delay(100);
    ESP.restart();
    return true;
  }
  return false;
}
#endif

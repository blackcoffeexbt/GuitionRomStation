#include "RomStorage.h"
#include "../RomReader.h"
#include "BoardConfig.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_heap_caps.h>
#include <algorithm>
#include <strings.h>
bool RomStorage::begin() {
  if (!entries) {
    entries = (port::RomEntry *)heap_caps_calloc(port::LibraryCapacity, sizeof(port::RomEntry),
                                                 MALLOC_CAP_SPIRAM);
    if (!entries) {
      snprintf(error, sizeof(error), "Not enough memory for the ROM library");
      return false;
    }
  }
  return scan();
}
bool RomStorage::scan(Progress progress) {
  total = skipped = 0;
  if (!entries)
    return false;
  if (mounted) {
    SD_MMC.end();
    mounted = false;
  }
  // DAT3 must remain high for native SD mode; it is not used as an SPI CS here.
  pinMode(board::SdData3, INPUT_PULLUP);
  if (!SD_MMC.setPins(board::SdClock, board::SdCommand, board::SdData0) ||
      !SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 4)) {
    snprintf(error, sizeof(error), "No readable FAT32 card. Insert card and rescan.");
    return false;
  }
  mounted = true;
  for (const char *candidate : {"/roms/sms", "/"}) {
    snprintf(directory, sizeof(directory), "%s", candidate);
    File folder = SD_MMC.open(directory, FILE_READ);
    if (!folder || !folder.isDirectory()) {
      folder.close();
      continue;
    }
    for (File f = folder.openNextFile(); f; f = folder.openNextFile()) {
      const char *name = strrchr(f.name(), '/');
      name = name ? name + 1 : f.name();
      if (!f.isDirectory() && port::smsFilename(name)) {
        if (total == port::LibraryCapacity || strlen(name) >= port::LibraryNameBytes)
          ++skipped;
        else {
          auto &entry = entries[total++];
          snprintf(entry.name, sizeof(entry.name), "%s", name);
          entry.bytes = f.size();
        }
      }
      f.close();
      if (progress)
        progress(0, 0);
      delay(1);
    }
    folder.close();
    if (total)
      break;
  }
  std::sort(entries, entries + total,
            [](const auto &a, const auto &b) { return strcasecmp(a.name, b.name) < 0; });
  if (skipped)
    snprintf(error, sizeof(error), "%d games shown; %d names/entries skipped", total, skipped);
  else
    snprintf(error, sizeof(error),
             total ? "A PLAY / B BACK / X RESCAN" : "Put .sms files in /roms/sms or card root");
  Serial.printf("SD library mounted=%d count=%d skipped=%d folder=%s\n", mounted, total, skipped,
                directory);
  return true;
}
uint8_t *RomStorage::load(int index, size_t &mapped, Progress progress) {
  mapped = 0;
  if (index < 0 || index >= total || !mounted) {
    snprintf(error, sizeof(error), "Select a game first");
    return nullptr;
  }
  char path[224];
  snprintf(path, sizeof(path), "%s%s%s", directory, strcmp(directory, "/") ? "/" : "",
           entries[index].name);
  File file = SD_MMC.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    snprintf(error, sizeof(error), "Cannot open game. Reinsert card and rescan.");
    return nullptr;
  }
  auto image = port::readRom(
      file, file.size(),
      [](size_t bytes) { return heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); },
      [](void *data) { free(data); }, progress);
  file.close();
  if (image.error) {
    snprintf(error, sizeof(error), "%s", image.error);
    return nullptr;
  }
  mapped = image.bytes;
  snprintf(error, sizeof(error), "Loaded %s", entries[index].name);
  return image.data;
}
void RomStorage::view(port::LibraryView &out, const port::LibraryModel &model, bool hasGame) const {
  out = {};
  out.first = model.first();
  out.selected = model.selected;
  out.count = total;
  out.hasGame = hasGame;
  snprintf(out.folder, sizeof(out.folder), "%s", directory);
  snprintf(out.message, sizeof(out.message), "%s", error);
  for (int row = 0; row < port::LibraryRows && out.first + row < total; row++) {
    const auto &entry = entries[out.first + row];
    snprintf(out.rows[row], sizeof(out.rows[row]), "%.49s", entry.name);
    snprintf(out.sizes[row], sizeof(out.sizes[row]), "%uK", unsigned(entry.bytes / 1024));
  }
}

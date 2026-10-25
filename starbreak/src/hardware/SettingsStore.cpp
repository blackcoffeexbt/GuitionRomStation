#include "SettingsStore.h"
#include <Arduino.h>
namespace sb {
bool SettingsStore::begin(Game &g) {
  ready = prefs.begin("starbreak", false);
  if (!ready) {
    g.message("SETTINGS STORAGE UNAVAILABLE", 8);
    return false;
  }
  bool damaged = false;
  for (int i = 0; i < 2; i++) {
    const char *key = i ? "cfg1" : "cfg0";
    size_t size = prefs.getBytesLength(key);
    if (!size)
      continue;
    SettingsBytes bytes{};
    Settings loaded;
    bool valid = size == bytes.size() &&
                 prefs.getBytes(key, bytes.data(), bytes.size()) == bytes.size() &&
                 decodeSettings(bytes, loaded);
    Serial.printf("SETTINGS slot=%d valid=%d\n", i, valid);
    if (!valid) {
      damaged = true;
      continue;
    }
    if (slot < 0 || int32_t(loaded.sequence - current.sequence) > 0) {
      slot = i;
      current = loaded;
    }
  }
  g.best = current.best;
  g.muted = current.muted;
  if (damaged)
    g.message(slot < 0 ? "SETTINGS RESET SAFELY" : "PREVIOUS SETTINGS RECOVERED", 6);
  Serial.printf("SETTINGS namespace=starbreak best=%u mute=%d seq=%u recovered=%d\n", g.best,
                g.muted, current.sequence, damaged);
  return true;
}
void SettingsStore::update(Game &g) {
  if (!ready || !g.needsSave || int32_t(millis() - nextRetry) < 0)
    return;
  Settings next{g.best, current.sequence + 1, g.muted};
  auto bytes = encodeSettings(next);
  int target = slot == 0 ? 1 : 0;
  const char *key = target ? "cfg1" : "cfg0";
  auto start = micros();
  bool ok = prefs.putBytes(key, bytes.data(), bytes.size()) == bytes.size();
  SettingsBytes check{};
  Settings decoded;
  ok = ok && prefs.getBytes(key, check.data(), check.size()) == check.size() && check == bytes &&
       decodeSettings(check, decoded);
  if (ok) {
    slot = target;
    current = next;
    g.needsSave = false;
  } else {
    nextRetry = millis() + 5000;
    g.message("SETTINGS SAVE FAILED - RETRYING", 4);
  }
  Serial.printf("SETTINGS save_us=%u best=%u seq=%u ok=%d\n", micros() - start, g.best,
                next.sequence, ok);
}
#ifdef STARBREAK_VALIDATION
bool SettingsStore::corruptLatest() {
  if (slot < 0)
    return false;
  auto bytes = encodeSettings(current);
  bytes[9] ^= 0x80;
  return prefs.putBytes(slot ? "cfg1" : "cfg0", bytes.data(), bytes.size()) == bytes.size();
}
#endif
} // namespace sb

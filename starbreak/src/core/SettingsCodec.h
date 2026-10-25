#pragma once
#include <array>
#include <cstdint>
namespace sb {
struct Settings {
  uint32_t best = 0, sequence = 0;
  bool muted = false;
};
using SettingsBytes = std::array<uint8_t, 20>;
inline uint32_t settingsCrc(const uint8_t *data, int n) {
  uint32_t c = ~0u;
  for (int i = 0; i < n; i++) {
    c ^= data[i];
    for (int j = 0; j < 8; j++)
      c = (c >> 1) ^ (0xedb88320u & uint32_t(-int(c & 1)));
  }
  return ~c;
}
inline SettingsBytes encodeSettings(Settings s) {
  SettingsBytes b{};
  auto put = [&](int offset, uint32_t n) {
    for (int i = 0; i < 4; i++)
      b[offset + i] = n >> (i * 8);
  };
  put(0, 0x52425453);
  b[4] = 1;
  b[5] = s.muted;
  put(8, s.best);
  put(12, s.sequence);
  put(16, settingsCrc(b.data(), 16));
  return b;
}
inline bool decodeSettings(SettingsBytes b, Settings &s) {
  auto get = [&](int offset) {
    uint32_t n = 0;
    for (int i = 0; i < 4; i++)
      n |= uint32_t(b[offset + i]) << (i * 8);
    return n;
  };
  if (get(0) != 0x52425453 || b[4] != 1 || b[5] > 1 || b[6] || b[7] ||
      settingsCrc(b.data(), 16) != get(16) || get(8) > 99999999)
    return false;
  s = {get(8), get(12), bool(b[5])};
  return true;
}
} // namespace sb

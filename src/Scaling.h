#pragma once
#include <algorithm>
#include <cstdint>
namespace port {
inline void scaleSMS(const uint8_t *source, uint8_t *destination) {
  // Native LCD order is column-major landscape. Small tiles prevent PSRAM thrashing.
  static uint16_t rows[320];
  static bool mapped = false;
  if (!mapped) {
    for (int y = 0; y < 320; y++)
      rows[y] = (y * 192 / 320) * 256;
    mapped = true;
  }
  for (int bx = 0; bx < 426; bx += 8)
    for (int by = 0; by < 320; by += 32)
      for (int x = bx; x < std::min(bx + 8, 426); x++) {
        int sx = x * 256 / 426;
        uint8_t *column = destination + (x + 27) * 320;
        auto *words = reinterpret_cast<uint32_t *>(column);
        for (int y = by; y < by + 32; y += 4) {
          uint32_t packed =
              uint32_t(source[rows[y + 3] + sx]) | (uint32_t(source[rows[y + 2] + sx]) << 8) |
              (uint32_t(source[rows[y + 1] + sx]) << 16) | (uint32_t(source[rows[y] + sx]) << 24);
          words[(316 - y) / 4] = packed & 0x1f1f1f1fu;
        }
      }
}
} // namespace port

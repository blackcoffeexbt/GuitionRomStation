#include "Scaling.h"
#include <array>
#include <cassert>
#include <cstdio>
int main() {
  std::array<uint8_t, 256 * 192> source{};
  for (int y = 0; y < 192; y++)
    for (int x = 0; x < 256; x++)
      source[y * 256 + x] = uint8_t((x * 7 + y * 13) | 0x80);
  alignas(4) std::array<uint8_t, 480 * 320 + 64> guarded;
  guarded.fill(252);
  auto *output = guarded.data() + 32;
  port::scaleSMS(source.data(), output);
  for (int x = 0; x < 480; x++)
    for (int y = 0; y < 320; y++) {
      uint8_t expected =
          (x >= 27 && x < 453) ? source[(y * 192 / 320) * 256 + (x - 27) * 256 / 426] & 31 : 252;
      assert(output[x * 320 + 319 - y] == expected);
    }
  for (int i = 0; i < 32; i++)
    assert(guarded[i] == 252 && guarded[guarded.size() - 1 - i] == 252);
  puts("PASS exact 4:3 scaling, rotation, attribute mask, side borders and framebuffer bounds");
}

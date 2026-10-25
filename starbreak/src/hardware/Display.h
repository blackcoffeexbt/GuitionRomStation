#pragma once
#include <Arduino_GFX_Library.h>
namespace sb {
constexpr uint8_t color(uint32_t c) {
  return uint8_t(((c >> 16) & 0xe0) | ((c >> 11) & 0x1c) | ((c >> 6) & 3));
}
class Display {
  Arduino_ESP32QSPI *bus = nullptr;
  Arduino_AXS15231B *panel = nullptr;
  uint8_t *pixels = nullptr;
  uint16_t palette[256]{};

public:
  uint32_t transferUs = 0;
  bool internal = false;
  bool begin();
  void present();
  void clear(uint8_t c);
  void pixel(int x, int y, uint8_t c);
  void rect(int x, int y, int w, int h, uint8_t c);
  void text(int x, int y, const char *str, uint8_t c, int scale = 1);
  void ship(int x, int y, bool firing);
};
} // namespace sb

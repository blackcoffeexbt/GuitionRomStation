#pragma once
#include <Arduino_GFX_Library.h>
class Display {
  Arduino_ESP32QSPI *bus = nullptr;
  Arduino_AXS15231B *panel = nullptr;
  uint16_t *logical = nullptr;
  uint16_t *physical = nullptr;

public:
  bool begin();
  uint16_t *framebuffer() { return logical; }
  uint32_t transferUs = 0, rotationUs = 0;
  // Consumes framebuffer contents; the next frame must redraw every pixel.
  void present();
  void benchmark();
};

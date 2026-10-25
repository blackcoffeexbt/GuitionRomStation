#include "Display.h"
#include "../Scaling.h"
#include "BoardConfig.h"
#include "Font.h"
#include <esp_heap_caps.h>
#include <cstring>
namespace sb {
bool Display::begin() {
  if (!psramFound())
    return false;
  // Budget Bluetooth before reserving the framebuffer. Keep 70 KB internal free.
  size_t available = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (available > 153600 + 70000)
    pixels = (uint8_t *)heap_caps_malloc(153600, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  internal = pixels != nullptr;
  if (!pixels)
    pixels = (uint8_t *)heap_caps_malloc(153600, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!pixels)
    return false;
  sourceCache = (uint8_t *)heap_caps_malloc(256 * 192, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!sourceCache)
    return false;
  for (int i = 0; i < 256; i++) {
    int r = ((i >> 5) & 7) * 255 / 7, g = ((i >> 2) & 7) * 255 / 7, b = (i & 3) * 255 / 3;
    palette[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }
  palette[250] = 0x1084;
  palette[251] = 0xfea0;
  palette[252] = 0;
  palette[253] = 0x2aef;
  palette[254] = 0x6fff;
  palette[255] = 0xffff;
  bus = new Arduino_ESP32QSPI(board::LcdCs, board::LcdClock, board::LcdD0, board::LcdD1,
                              board::LcdD2, board::LcdD3);
  panel = new Arduino_AXS15231B(bus, -1, 0, false, 320, 480);
  pinMode(board::Backlight, OUTPUT);
  digitalWrite(board::Backlight, LOW);
  if (!panel->begin(board::DisplayClockHz))
    return false;
  clear(color(0x050919));
  present();
  digitalWrite(board::Backlight, HIGH);
  Serial.printf("DISPLAY indexed8 native-layout buffer=%s bytes=153600 heap=%u largest=%u\n",
                internal ? "internal" : "PSRAM", ESP.getFreeHeap(),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  return true;
}
void Display::present() {
  auto start = micros();
  panel->drawIndexedBitmap(0, 0, pixels, palette, 320, 480);
  transferUs = micros() - start;
}
void Display::clear(uint8_t c) {
  memset(pixels, c, 153600);
}
void Display::pixel(int x, int y, uint8_t c) {
  if (x >= 0 && x < 480 && y >= 0 && y < 320)
    pixels[x * 320 + 319 - y] = c;
}
void Display::rect(int x, int y, int w, int h, uint8_t c) {
  int endX = std::min(480, x + w), endY = std::min(320, y + h);
  x = std::max(0, x);
  y = std::max(0, y);
  if (endX <= x || endY <= y)
    return;
  for (int i = x; i < endX; i++)
    memset(pixels + i * 320 + 320 - endY, c, endY - y);
}
void Display::text(int x, int y, const char *str, uint8_t c, int scale) {
  while (*str) {
    int ch = uint8_t(*str++);
    if (ch < 32 || ch > 127)
      ch = '?';
    int n = ch - 32;
    auto *mask = wilds::glyphPixels + n * 192;
    for (int k = 0; k < wilds::glyphWidths[n]; k++)
      for (int j = 0; j < 16; j++)
        if (mask[j * 12 + k] >= 100)
          rect(x + k * scale, y + j * scale, scale, scale, c);
    x += (wilds::glyphWidths[n] + 1) * scale;
  }
}
void Display::frame(const uint8_t *data, const uint16_t *colors) {
  for (int i = 0; i < 32; i++)
    palette[i] = colors[i];
  palette[250] = 0x1084;
  palette[251] = 0xfea0;
  palette[252] = 0;
  palette[253] = 0x2aef;
  palette[254] = 0x6fff;
  palette[255] = 0xffff;
  memset(pixels, 252, 27 * 320);
  memset(pixels + 453 * 320, 252, 27 * 320);
  memcpy(sourceCache, data, 256 * 192);
  // 256x192 -> 426x320, preserving 4:3 aspect with narrow pillarboxes.
  port::scaleSMS(sourceCache, pixels);
}
} // namespace sb

#include "Display.h"
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
  for (int i = 0; i < 256; i++) {
    int r = ((i >> 5) & 7) * 255 / 7, g = ((i >> 2) & 7) * 255 / 7, b = (i & 3) * 255 / 3;
    palette[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }
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
void Display::sprite(int cx, int cy, const Sprite &s) {
  int x = cx - s.w / 2, y = cy - s.h / 2;
  int left = std::max(0, -x), right = std::min(s.w, 480 - x), top = std::max(0, -y),
      bottom = std::min(s.h, 320 - y);
  if (left >= right || top >= bottom)
    return;
  for (int col = left; col < right; col++)
    for (int row = bottom - 1; row >= top; row--) {
      auto c = s.data[col * s.h + s.h - 1 - row];
      if (c)
        pixels[(x + col) * 320 + 319 - y - row] = c;
    }
}
void Display::ship(int x, int y, bool firing) {
  auto dark = color(0x285b9e), light = color(0x65cde6), white = color(0xecffff);
  rect(x - 18, y - 12, 8, 4, dark);
  rect(x - 18, y + 8, 8, 4, dark);
  rect(x - 12, y - 8, 22, 16, dark);
  rect(x - 8, y - 6, 22, 12, light);
  rect(x + 12, y - 3, 10, 6, white);
  rect(x, y - 3, 8, 6, color(0x185ba5));
  rect(x - 20, y - 4, 8, 8, color(firing ? 0xffc964 : 0xff7954));
  rect(x - 26, y - 2, 6, 4, color(0xffe9a9));
}
} // namespace sb

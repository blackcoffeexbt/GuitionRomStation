#pragma once
#include "../core/Types.h"
#include "Sprites.h"
namespace wilds {
constexpr uint16_t rgb(uint32_t c) {
  return ((c >> 8) & 0xf800) | ((c >> 5) & 0x07e0) | ((c >> 3) & 0x001f);
}
class Surface {
public:
  uint16_t *pixels;
  int width = Width, height = Height;
  explicit Surface(uint16_t *data) : pixels(data) {}
  void pixel(int x, int y, uint16_t c) {
    if (x >= 0 && y >= 0 && x < width && y < height)
      pixels[y * width + x] = c;
  }
  void blend(int x, int y, uint16_t c, uint8_t alpha);
  void clear(uint16_t c);
  void rect(int x, int y, int w, int h, uint16_t c);
  void rounded(int x, int y, int w, int h, int r, uint16_t c);
  void line(int x0, int y0, int x1, int y1, uint16_t c);
  void ellipse(int cx, int cy, int rx, int ry, uint16_t c);
  void diamond(int x, int y, uint16_t c);
  void diamondOutline(int x, int y, uint16_t c);
  void sprite(int x, int y, const Sprite &s);
  int textWidth(const char *text, int scale = 1) const;
  void text(int x, int y, const char *str, uint16_t c, int scale = 1);
};
} // namespace wilds

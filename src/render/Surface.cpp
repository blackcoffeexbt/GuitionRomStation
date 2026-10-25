#include "Surface.h"
#include "Font.h"
#include <cstring>
namespace wilds {
namespace {
// PSRAM favours aligned 32-bit writes over individual halfword stores.
void fillPixels(uint16_t *p, int count, uint16_t c) {
  if (!count)
    return;
  if (reinterpret_cast<uintptr_t>(p) & 2) {
    *p++ = c;
    --count;
  }
  auto *words = reinterpret_cast<uint32_t *>(p);
  uint32_t pair = uint32_t(c) | (uint32_t(c) << 16);
  int n = count / 2;
  for (int i = 0; i < n; i++)
    words[i] = pair;
  if (count & 1)
    p[count - 1] = c;
}
} // namespace
void Surface::clear(uint16_t c) {
  fillPixels(pixels, width * height, c);
}
void Surface::blend(int x, int y, uint16_t c, uint8_t a) {
  if (x < 0 || y < 0 || x >= width || y >= height || !a)
    return;
  if (a > 245) {
    pixel(x, y, c);
    return;
  }
  uint16_t b = pixels[y * width + x];
  int r = (((c >> 11) * a + (b >> 11) * (255 - a)) / 255),
      g = ((((c >> 5) & 63) * a + ((b >> 5) & 63) * (255 - a)) / 255),
      bl = (((c & 31) * a + (b & 31) * (255 - a)) / 255);
  pixel(x, y, (r << 11) | (g << 5) | bl);
}
void Surface::rect(int x, int y, int w, int h, uint16_t c) {
  int right = std::min(width, x + w), bottom = std::min(height, y + h);
  x = std::max(x, 0);
  y = std::max(y, 0);
  if (right <= x || bottom <= y)
    return;
  for (int j = y; j < bottom; j++)
    fillPixels(pixels + j * width + x, right - x, c);
}
void Surface::rounded(int x, int y, int w, int h, int r, uint16_t c) {
  rect(x + r, y, w - 2 * r, h, c);
  rect(x, y + r, w, h - 2 * r, c);
  ellipse(x + r, y + r, r, r, c);
  ellipse(x + w - r - 1, y + r, r, r, c);
  ellipse(x + r, y + h - r - 1, r, r, c);
  ellipse(x + w - r - 1, y + h - r - 1, r, r, c);
}
void Surface::line(int x0, int y0, int x1, int y1, uint16_t c) {
  int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1,
      err = dx + dy;
  while (true) {
    pixel(x0, y0, c);
    if (x0 == x1 && y0 == y1)
      break;
    int e = 2 * err;
    if (e >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}
void Surface::ellipse(int x, int y, int rx, int ry, uint16_t c) {
  if (rx <= 0 || ry <= 0)
    return;
  for (int j = -ry; j <= ry; j++) {
    int half = int(rx * std::sqrt(std::max(0.f, 1.f - float(j * j) / float(ry * ry))));
    rect(x - half, y + j, half * 2 + 1, 1, c);
  }
}
void Surface::diamond(int x, int y, uint16_t c) {
  for (int j = -16; j < 16; j++) {
    int half = (16 - std::abs(j)) * 2;
    rect(x - half, y + j, half * 2 + 1, 1, c);
  }
}
void Surface::diamondOutline(int x, int y, uint16_t c) {
  line(x, y - 16, x + 32, y, c);
  line(x + 32, y, x, y + 16, c);
  line(x, y + 16, x - 32, y, c);
  line(x - 32, y, x, y - 16, c);
}
void Surface::sprite(int x, int y, const Sprite &s) {
  x -= s.anchorX;
  y -= s.anchorY;
  int left = std::max(0, -x), right = std::min(int(s.w), width - x), top = std::max(0, -y),
      bottom = std::min(int(s.h), height - y);
  if (right <= left || bottom <= top)
    return;
  for (int j = top; j < bottom; j++) {
    const auto *src = s.pixels + j * s.w;
    auto *dest = pixels + (y + j) * width + x + left;
    int i = left;
    while (i < right) {
      while (i < right && src[i] == 0xf81f)
        ++i;
      int start = i;
      while (i < right && src[i] != 0xf81f)
        ++i;
      if (i > start)
        std::memcpy(dest + start - left, src + start, (i - start) * 2);
    }
  }
}
int Surface::textWidth(const char *s, int scale) const {
  int w = 0;
  while (*s) {
    unsigned c = *s++;
    if (c < 32 || c > 127)
      c = '?';
    w += (glyphWidths[c - 32] + 1) * scale;
  }
  return w;
}
void Surface::text(int x, int y, const char *s, uint16_t c, int scale) {
  while (*s) {
    unsigned ch = *s++;
    if (ch < 32 || ch > 127)
      ch = '?';
    int i = ch - 32;
    const auto *mask = glyphPixels + i * 192;
    for (int j = 0; j < 16; j++)
      for (int k = 0; k < glyphWidths[i]; k++) {
        uint8_t a = mask[j * 12 + k];
        if (a > 20)
          for (int dy = 0; dy < scale; dy++)
            for (int dx = 0; dx < scale; dx++)
              blend(x + k * scale + dx, y + j * scale + dy, c, a);
      }
    x += (glyphWidths[i] + 1) * scale;
  }
}
} // namespace wilds

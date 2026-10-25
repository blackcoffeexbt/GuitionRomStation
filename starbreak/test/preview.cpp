// Host preview of actual Renderer + portable simulation; not hardware evidence.
#include "Renderer.h"
#include "core/Pilot.h"
#include "Font.h"
#include <array>
#include <cstdio>
#include <cstring>
namespace {
std::array<uint8_t, 480 * 320> framebuffer;
}
namespace sb {
void Display::clear(uint8_t c) {
  framebuffer.fill(c);
}
void Display::rect(int x, int y, int w, int h, uint8_t c) {
  int right = std::min(480, x + w), bottom = std::min(320, y + h);
  for (int py = std::max(0, y); py < bottom; ++py)
    for (int px = std::max(0, x); px < right; ++px)
      framebuffer[py * 480 + px] = c;
}
void Display::text(int x, int y, const char *s, uint8_t c, int scale) {
  while (*s) {
    int ch = uint8_t(*s++);
    if (ch < 32 || ch > 127)
      ch = '?';
    int n = ch - 32;
    auto *mask = wilds::glyphPixels + n * 192;
    for (int k = 0; k < wilds::glyphWidths[n]; ++k)
      for (int j = 0; j < 16; ++j)
        if (mask[j * 12 + k] >= 100)
          rect(x + k * scale, y + j * scale, scale, scale, c);
    x += (wilds::glyphWidths[n] + 1) * scale;
  }
}
void Display::sprite(int cx, int cy, const Sprite &s) {
  for (int x = 0; x < s.w; x++)
    for (int y = 0; y < s.h; y++) {
      uint8_t c = s.data[x * s.h + s.h - 1 - y];
      if (c)
        rect(cx - s.w / 2 + x, cy - s.h / 2 + y, 1, 1, c);
    }
}
} // namespace sb
void save(const char *name) {
  char path[200];
  snprintf(path, sizeof(path), ".build/%s.ppm", name);
  auto *f = fopen(path, "wb");
  fprintf(f, "P6\n480 320\n255\n");
  for (auto c : framebuffer) {
    uint8_t rgb[] = {uint8_t((c >> 5) * 255 / 7), uint8_t(((c >> 2) & 7) * 255 / 7),
                     uint8_t((c & 3) * 255 / 3)};
    fwrite(rgb, 1, 3, f);
  }
  fclose(f);
}
int main() {
  sb::Game g;
  sb::Display display;
  sb::Renderer renderer;
  g.connected = true;
  g.start();
  bool seen[3][3]{};
  for (int tick = 0; tick < 39000 && g.screen == sb::Screen::Playing; ++tick) {
    g.receive(sb::pilot(g));
    g.update(1.f / 60);
    int index = g.boss.active ? g.boss.phase : 0;
    if ((index > 0 || g.levelTime > 30) && !seen[g.level - 1][index]) {
      seen[g.level - 1][index] = true;
      renderer.draw(display, g);
      char name[40];
      snprintf(name, sizeof(name), "sector-%d-%d", g.level, index);
      save(name);
    }
  }
  renderer.draw(display, g);
  save("campaign-result");
  return g.screen == sb::Screen::Victory ? 0 : 1;
}

#include "Renderer.h"
#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
namespace {
struct Text {
  int x, y;
  std::string label;
};
std::vector<Text> labels;
} // namespace
namespace sb {
void Display::clear(uint8_t) {
  labels.clear();
}
void Display::rect(int, int, int, int, uint8_t) {}
void Display::text(int x, int y, const char *text, uint8_t, int) {
  labels.push_back({x, y, text});
}
void Display::sprite(int, int, const Sprite &) {}
} // namespace sb
int main() {
  sb::Game game;
  sb::Display display;
  sb::Renderer renderer;
  for (auto screen : {sb::Screen::Pause, sb::Screen::Victory, sb::Screen::GameOver}) {
    game.screen = screen;
    game.boss.active = true;
    renderer.draw(display, game);
    int footer = 0;
    for (const auto &t : labels)
      if (t.y >= 280) {
        ++footer;
        assert(t.label.find("SOUND ") == 0);
      }
    assert(footer == 1);
  }
  game.screen = sb::Screen::Playing;
  game.boss.phase = 2;
  renderer.draw(display, game);
  bool bossHealthLabel = false;
  for (const auto &t : labels) {
    bossHealthLabel |= t.label == "EXPOSED CORE";
    assert(t.label.find("SOUND ") != 0);
  }
  assert(bossHealthLabel);
  std::puts("PASS result/pause footers never overlap flight hints or boss health; gameplay retains "
            "boss HUD");
}

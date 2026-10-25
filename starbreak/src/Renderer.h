#pragma once
#include "core/Game.h"
#include "hardware/Display.h"
namespace sb {
class Renderer {
  void button(Display &d, int y, const char *label, bool enabled = true);

public:
  void draw(Display &d, const Game &g);
};
} // namespace sb

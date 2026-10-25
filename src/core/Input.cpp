#include "Input.h"
namespace wilds {
Gesture Input::sample(bool touch, Vec2 p, uint32_t now) {
  if (touch && !down) {
    down = true;
    dragging = false;
    start = last = p;
    pressed = now;
    return {};
  }
  if (touch) {
    if (!dragging && distance(start, p) > 10) {
      dragging = true;
      Vec2 delta{p.x - start.x, p.y - start.y};
      last = p;
      return {GestureKind::Drag, p, delta};
    }
    if (dragging) {
      Vec2 delta{p.x - last.x, p.y - last.y};
      last = p;
      return {GestureKind::Drag, p, delta};
    }
    last = p;
    return {};
  }
  if (down) {
    down = false;
    if (!dragging && now - pressed < 1000)
      return {GestureKind::Tap, last, {}};
  }
  return {};
}
} // namespace wilds

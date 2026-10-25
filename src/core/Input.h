#pragma once
#include "Types.h"
namespace wilds {
enum class GestureKind { None, Tap, Drag };
struct Gesture {
  GestureKind kind = GestureKind::None;
  Vec2 point{}, delta{};
};
class Input {
  bool down = false, dragging = false;
  Vec2 start{}, last{};
  uint32_t pressed = 0;

public:
  Gesture sample(bool touched, Vec2 point, uint32_t now);
};
} // namespace wilds

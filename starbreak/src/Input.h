#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace sb {
struct Vec2 {
  float x = 0, y = 0;
};
struct Input {
  bool fresh = false, connected = false, fire = false, bomb = false, menu = false;
  bool nextWeapon = false, previousWeapon = false;
  float x = 0, y = 0;
  uint32_t sampledAt = 0;
};
inline Vec2 normalizeMove(float x, float y) {
  float m = std::hypot(x, y);
  if (m > 1) {
    x /= m;
    y /= m;
  }
  return {x, y};
}
inline Vec2 stick(int x, int y) {
  float m = std::hypot(float(x), float(y));
  if (m < 80)
    return {};
  float scale = std::min(1.f, (m - 80) / 432) / m;
  return {x * scale, y * scale};
}
} // namespace sb

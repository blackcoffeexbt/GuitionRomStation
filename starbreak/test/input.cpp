#include "Input.h"
#include <cassert>
#include <cstdio>
int main() {
  for (int degrees = 0; degrees < 360; degrees++) {
    float a = degrees * 3.14159265f / 180;
    auto v = sb::normalizeMove(2 * std::cos(a), 2 * std::sin(a));
    assert(std::abs(std::hypot(v.x, v.y) - 1) < .0001);
  }
  auto diagonal = sb::normalizeMove(1, 1);
  assert(std::abs(diagonal.x - .70710678f) < .0001);
  auto dead = sb::stick(25, -30);
  assert(dead.x == 0 && dead.y == 0);
  auto centre = sb::stick(0, 0);
  assert(centre.x == 0 && centre.y == 0);
  auto right = sb::stick(512, 0);
  assert(right.x == 1 && right.y == 0);
  auto stickDiagonal = sb::stick(512, 512);
  assert(std::abs(std::hypot(stickDiagonal.x, stickDiagonal.y) - 1) < .0001);
  std::puts("PASS stick dead zone, radial speed, D-pad diagonal normalization");
}

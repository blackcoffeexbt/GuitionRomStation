#include "core/Collision.h"
#include <cassert>
#include <cstdio>
int main() {
  sb::Box target{100, 100, 10, 10};
  assert(sb::swept({0, 100}, {200, 100}, 1, target));
  assert(!sb::swept({0, 70}, {200, 70}, 1, target));
  assert(sb::swept({100, 0}, {100, 200}, 1, target));
  assert(sb::swept({0, 0}, {200, 200}, 0, target));
  assert(sb::swept({100, 100}, {100, 100}, 0, target));
  assert(!sb::swept({20, 20}, {20, 20}, 5, target));
  assert(sb::swept({0, 89}, {200, 89}, 1, target));
  assert(!sb::swept({0, 88}, {200, 88}, 1, target));
  assert(!sb::swept({150, 100}, {200, 100}, 1, target));
  std::puts(
      "PASS swept collision: tunnelling, misses, diagonals, zero motion, radius, segment bounds");
}

#pragma once
#include "../Input.h"
namespace sb {
struct Box {
  float x, y, halfW, halfH;
};
// Segment against an expanded AABB: moving circles cannot tunnel between frames.
inline bool swept(Vec2 from, Vec2 to, float radius, Box box) {
  float lo = 0, hi = 1;
  float start[] = {from.x, from.y}, delta[] = {to.x - from.x, to.y - from.y};
  float min[] = {box.x - box.halfW - radius, box.y - box.halfH - radius},
        max[] = {box.x + box.halfW + radius, box.y + box.halfH + radius};
  for (int axis = 0; axis < 2; axis++) {
    if (std::abs(delta[axis]) < 1e-6f) {
      if (start[axis] < min[axis] || start[axis] > max[axis])
        return false;
      continue;
    }
    float a = (min[axis] - start[axis]) / delta[axis], b = (max[axis] - start[axis]) / delta[axis];
    if (a > b)
      std::swap(a, b);
    lo = std::max(lo, a);
    hi = std::min(hi, b);
    if (lo > hi)
      return false;
  }
  return true;
}
inline bool overlaps(Box a, Box b) {
  return std::abs(a.x - b.x) < a.halfW + b.halfW && std::abs(a.y - b.y) < a.halfH + b.halfH;
}
} // namespace sb

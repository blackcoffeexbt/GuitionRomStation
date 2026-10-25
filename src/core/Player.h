#pragma once
#include "Pathfinding.h"
namespace wilds {
class Player {
public:
  Vec2 position{34, 34};
  TilePos settled{34, 34};
  Path path{};
  int next = 0;
  float animation = 0;
  uint8_t direction = 0;
  bool moving() const { return next < path.count; }
  void update(float dt);
  // Retarget only at tile centres, so a new route never cuts an obstacle corner.
  bool hasPending = false;
  TilePos pending{};
};
} // namespace wilds

#include "Assets.h"
// Generated from the original PNG assets; compiled once into flash.
#include "../../assets/generated/Sprites.cpp"
namespace wilds {
const Sprite &objectSprite(ObjectKind k, int v) {
  static const Sprite *oaks[] = {&oak0, &oak1, &oak2};
  static const Sprite *bushes[] = {&bush0, &bush1, &bush2};
  static const Sprite *flowers[] = {&flowers0, &flowers1, &flowers2};
  switch (k) {
  case ObjectKind::Oak:
    return *oaks[v % 3];
  case ObjectKind::Pine:
    return pine;
  case ObjectKind::Rock:
    return v % 2 ? rock1 : rock0;
  case ObjectKind::Bush:
    return *bushes[v % 3];
  case ObjectKind::Flowers:
    return *flowers[v % 3];
  case ObjectKind::Cottage:
    return cottage;
  case ObjectKind::Log:
    return log;
  default:
    return flowers0;
  }
}
const Sprite &playerSprite(int dir, int frame) {
  static const Sprite *frames[4][4] = {{&player0_0, &player0_1, &player0_2, &player0_3},
                                       {&player1_0, &player1_1, &player1_2, &player1_3},
                                       {&player2_0, &player2_1, &player2_2, &player2_3},
                                       {&player3_0, &player3_1, &player3_2, &player3_3}};
  return *frames[dir % 4][frame % 4];
}
} // namespace wilds

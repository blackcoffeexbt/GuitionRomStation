#include "Assets.h"
// Generated from the original PNG assets; compiled once into flash.
#include "../../assets/generated/Sprites.cpp"
namespace wilds {
const Sprite &objectSprite(ObjectKind k, int v) {
  static const Sprite *oaks[] = {&oak0, &oak1, &oak2};
  static const Sprite *bushes[] = {&bush0, &bush1, &bush2};
  static const Sprite *flowers[] = {&flowers0, &flowers1, &flowers2};
  static const Sprite *resources[] = {&resource0, &resource1, &resource2,
                                      &resource3, &resource4, &resource5};
  if (int(k) >= 8)
    return *resources[(int(k) - 8) % 6];
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
const Sprite &cropSprite(int type, int stage) {
  static const Sprite *frames[8][4] = {
      {&crop0_0, &crop0_1, &crop0_2, &crop0_3}, {&crop1_0, &crop1_1, &crop1_2, &crop1_3},
      {&crop2_0, &crop2_1, &crop2_2, &crop2_3}, {&crop3_0, &crop3_1, &crop3_2, &crop3_3},
      {&crop4_0, &crop4_1, &crop4_2, &crop4_3}, {&crop5_0, &crop5_1, &crop5_2, &crop5_3},
      {&crop6_0, &crop6_1, &crop6_2, &crop6_3}, {&crop7_0, &crop7_1, &crop7_2, &crop7_3}};
  return *frames[std::clamp(type, 0, 7)][std::clamp(stage, 0, 3)];
}
const Sprite &playerSprite(int dir, int frame) {
  static const Sprite *frames[4][4] = {{&player0_0, &player0_1, &player0_2, &player0_3},
                                       {&player1_0, &player1_1, &player1_2, &player1_3},
                                       {&player2_0, &player2_1, &player2_2, &player2_3},
                                       {&player3_0, &player3_1, &player3_2, &player3_3}};
  return *frames[dir % 4][frame % 4];
}
} // namespace wilds

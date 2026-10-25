#pragma once
#include "Types.h"
#include <array>
namespace wilds {
enum class Item : uint8_t {
  GrassSeed,
  FlowerSeed,
  CarrotSeed,
  PumpkinSeed,
  BerrySeed,
  SaplingSeed,
  AppleSeed,
  OakSeed,
  Wood,
  Stone,
  Berry,
  Mushroom,
  Flower,
  Carrot,
  Pumpkin,
  Apple,
  Grass,
  Count
};
constexpr int ItemCount = int(Item::Count), SeedCount = 8, MaxStack = 9999;
struct ItemDefinition {
  const char *name;
  uint32_t color;
};
inline constexpr ItemDefinition ItemDefs[] = {
    {"Grass seed", 0x99b879},   {"Flower seed", 0xdfb1c2}, {"Carrot seed", 0xe1a267},
    {"Pumpkin seed", 0xd7a851}, {"Berry seed", 0xb88ba5},  {"Sapling seed", 0x7fac86},
    {"Apple seed", 0xbc8c79},   {"Oak seed", 0xa3b17a},    {"Wood", 0xa38565},
    {"Stone", 0x99b2ac},        {"Berries", 0xb9788c},     {"Mushrooms", 0xc8a58a},
    {"Flowers", 0xdfb1c2},      {"Carrots", 0xe1a267},     {"Pumpkins", 0xd7a851},
    {"Apples", 0xbc8c79},       {"Grass", 0x99b879}};
class Inventory {
public:
  std::array<uint16_t, ItemCount> counts{};
  bool add(Item item, int count = 1) {
    int i = int(item);
    if (i < 0 || i >= ItemCount || count < 0 || counts[i] + count > MaxStack)
      return false;
    counts[i] += count;
    return true;
  }
  bool take(Item item, int count = 1) {
    int i = int(item);
    if (i < 0 || i >= ItemCount || count < 0 || counts[i] < count)
      return false;
    counts[i] -= count;
    return true;
  }
  void starter() {
    counts.fill(0);
    for (int i = 0; i < SeedCount; i++)
      counts[i] = i == 2 ? 8 : 3;
  }
};
} // namespace wilds

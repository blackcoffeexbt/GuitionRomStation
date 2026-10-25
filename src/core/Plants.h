#pragma once
#include "Inventory.h"
#include "World.h"
namespace wilds {
struct PlantDefinition {
  const char *name;
  float growSeconds;
  Item yield;
  uint8_t quantity;
  bool regrows;
};
inline constexpr PlantDefinition PlantDefs[] = {
    {"Grass", 60, Item::Grass, 2, false},      {"Wildflowers", 90, Item::Flower, 2, true},
    {"Carrots", 120, Item::Carrot, 3, false},  {"Pumpkins", 240, Item::Pumpkin, 2, false},
    {"Berry bush", 300, Item::Berry, 3, true}, {"Small tree", 360, Item::Wood, 3, true},
    {"Apple tree", 480, Item::Apple, 3, true}, {"Oak tree", 600, Item::Wood, 4, true}};
struct Plant {
  TilePos tile{};
  uint8_t type = 0;
  bool active = false;
  float age = 0;
  float water = 0;
};
class Plants {
public:
  static constexpr int Capacity = 128;
  std::array<Plant, Capacity> entries{};
  std::array<int16_t, TileCount> atTile{};
  void reset() {
    for (auto &p : entries)
      p = {};
    atTile.fill(-1);
  }
  int at(TilePos p) const { return inBounds(p) ? atTile[index(p)] : -1; }
  int count() const {
    int n = 0;
    for (const auto &p : entries)
      n += p.active;
    return n;
  }
  int plant(TilePos tile, int type);
  void update(float dt);
  int stage(int id) const;
  bool mature(int id) const { return entries[id].age >= PlantDefs[entries[id].type].growSeconds; }
  bool harvest(int id, Inventory &inventory);
};
} // namespace wilds

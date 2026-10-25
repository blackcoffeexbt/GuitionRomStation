#include "World.h"
#include <cstdlib>
namespace wilds {
bool World::walkable(TilePos p) const {
  if (!inBounds(p) || at(p).terrain == Terrain::Water)
    return false;
  const int obj = at(p).object;
  if (obj < 0)
    return true;
  const auto k = objects[obj].kind;
  return k == ObjectKind::Flowers || k == ObjectKind::Reeds || int(k) >= 8;
}
bool World::addObject(TilePos p, ObjectKind k, uint8_t v) {
  if (!inBounds(p) || objectCount == MaxObjects || at(p).object >= 0 ||
      at(p).terrain == Terrain::Water)
    return false;
  tiles[index(p)].object = objectCount;
  objects[objectCount++] = {p, k, v};
  return true;
}
void World::generate(uint32_t seed) {
  objectCount = 0;
  for (int y = 0; y < MapSize; y++)
    for (int x = 0; x < MapSize; x++) {
      TilePos p{int16_t(x), int16_t(y)};
      auto h = hash(index(p) + seed);
      auto &t = tiles[index(p)];
      t = {Terrain::Grass, uint8_t(h % 4), -1};
      // A winding stream and a small pond east of the home clearing.
      float bend = 43.f + 2.f * std::sin(y * .18f);
      if (std::abs(x - bend) < 1.5f || ((x - 39) * (x - 39) * .65f + (y - 31) * (y - 31) < 10))
        t.terrain = Terrain::Water;
      if ((y == 34 && x >= 28 && x <= 40) || (x == 34 && y >= 32 && y <= 40))
        t.terrain = Terrain::Path;
      if (x >= 31 && x <= 33 && y >= 35 && y <= 36)
        t.terrain = Terrain::Soil;
      bool clearing = std::abs(x - 33) <= 5 && std::abs(y - 33) <= 5;
      if (!clearing && t.terrain == Terrain::Grass && h % 100 < 12) {
        auto k = (h % 7 < 4) ? ObjectKind::Oak
                             : (h % 7 == 4 ? ObjectKind::Pine
                                           : (h % 7 == 5 ? ObjectKind::Rock : ObjectKind::Bush));
        addObject(p, k, h % 3);
      } else if (t.terrain == Terrain::Grass && h % 100 < 8)
        addObject(p, ObjectKind::Flowers, h % 3);
    }
  // Cottage footprint; all occupied tiles refer to the same entity.
  addObject({32, 32}, ObjectKind::Cottage);
  int cottage = at({32, 32}).object;
  for (int y = 31; y <= 32; y++)
    for (int x = 31; x <= 32; x++)
      tiles[index({int16_t(x), int16_t(y)})].object = cottage;
  addObject({29, 33}, ObjectKind::Oak, 0);
  addObject({36, 32}, ObjectKind::Oak, 1);
  addObject({32, 38}, ObjectKind::Pine);
  addObject({37, 36}, ObjectKind::Rock);
  addObject({30, 36}, ObjectKind::Bush);
  addObject({35, 30}, ObjectKind::Flowers, 2);
  addObject({28, 35}, ObjectKind::Log);
  addObject({38, 33}, ObjectKind::Bush, 1);
  for (int y = 8; y < 56; y++)
    for (int x = 8; x < 56; x++) {
      TilePos p{int16_t(x), int16_t(y)};
      auto h = hash(index(p) + seed + 901);
      if (at(p).object < 0 && at(p).terrain == Terrain::Grass && h % 100 < 2)
        addObject(p, ObjectKind(8 + h % 6), h % 3);
    }
  constexpr TilePos gifts[] = {{35, 33}, {35, 35}, {36, 34}, {33, 37}, {34, 31}, {36, 36}};
  for (int i = 0; i < 6; i++) {
    auto p = gifts[i];
    int old = at(p).object;
    if (old >= 0 && objects[old].kind == ObjectKind::Flowers) {
      objects[old].active = false;
      objects[old].respawnAt = 600;
      tiles[index(p)].object = -1;
    }
    addObject(p, ObjectKind(8 + i));
  }
  // Ensure the spawn and a broad connected starting path are open.
  tiles[index({34, 34})].object = -1;
}
} // namespace wilds

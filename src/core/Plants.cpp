#include "Plants.h"
namespace wilds {
int Plants::plant(TilePos tile, int type) {
  if (!inBounds(tile) || at(tile) >= 0 || type < 0 || type >= SeedCount)
    return -1;
  for (int i = 0; i < Capacity; i++)
    if (!entries[i].active) {
      entries[i] = {tile, uint8_t(type), true, 0, 0};
      atTile[index(tile)] = i;
      return i;
    }
  return -1;
}
void Plants::update(float dt) {
  if (dt <= 0 || !std::isfinite(dt))
    return;
  for (auto &p : entries)
    if (p.active) {
      float boost = std::min(dt, p.water);
      p.age = std::min(PlantDefs[p.type].growSeconds, p.age + dt + boost);
      p.water = std::max(0.f, p.water - dt);
    }
}
int Plants::stage(int id) const {
  auto &p = entries[id];
  float f = p.age / PlantDefs[p.type].growSeconds;
  return f >= 1 ? 3 : f >= .66f ? 2 : f >= .33f ? 1 : 0;
}
bool Plants::harvest(int id, Inventory &inv) {
  if (id < 0 || id >= Capacity || !entries[id].active || !mature(id))
    return false;
  auto &p = entries[id];
  const auto &d = PlantDefs[p.type];
  if (inv.counts[int(d.yield)] > MaxStack - d.quantity || inv.counts[p.type] > MaxStack - 2)
    return false;
  inv.add(d.yield, d.quantity);
  inv.add(Item(p.type), 2);
  if (d.regrows) {
    p.age = d.growSeconds * .35f;
    p.water = 0;
  } else {
    atTile[index(p.tile)] = -1;
    p.active = false;
  }
  return true;
}
} // namespace wilds

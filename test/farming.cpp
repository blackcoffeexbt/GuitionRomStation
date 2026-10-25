#include "core/SaveCodec.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <vector>
using namespace wilds;
static void settle(Game &g) {
  for (int i = 0; i < 2400 && (g.player.moving() || g.hasAction()); i++)
    g.update(1.f / 60);
  assert(!g.player.moving());
  assert(!g.hasAction());
}
int main() {
  const uint8_t crcVector[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  assert(SaveCodec::crc(crcVector, sizeof(crcVector)) == 0xcbf43926u);
  auto game = std::make_unique<Game>();
  auto &g = *game;
  g.begin();
  assert(g.inventory.counts[int(Item::CarrotSeed)] == 8);
  // Gather the starter seed pouch by actually routing and interacting.
  int pouch = g.world.at({35, 33}).object;
  assert(pouch >= 0 && g.world.objects[pouch].kind == ObjectKind::WildSeeds);
  g.act({35, 33}, Tool::Hand);
  settle(g);
  assert(g.inventory.counts[2] == 11);
  assert(!g.world.objects[pouch].active);
  assert(g.world.at({35, 33}).object < 0);
  // Cultivate, plant, reject occupied planting without spending a second seed.
  TilePos soil{35, 33};
  g.act(soil, Tool::Hoe);
  settle(g);
  assert(g.world.at(soil).terrain == Terrain::Soil);
  g.act(soil, Tool::Seeds);
  settle(g);
  assert(g.plants.at(soil) >= 0);
  assert(g.inventory.counts[2] == 10);
  g.act(soil, Tool::Seeds);
  settle(g);
  assert(g.inventory.counts[2] == 10);
  g.act(soil, Tool::Water);
  settle(g);
  int id = g.plants.at(soil);
  assert(g.plants.entries[id].water == 60);
  // Exact simulation growth, watering boost and harvest yield.
  g.plants.update(60);
  assert(g.plants.mature(id));
  g.act(soil, Tool::Hand);
  settle(g);
  assert(g.plants.at(soil) < 0);
  assert(g.inventory.counts[int(Item::Carrot)] == 3);
  assert(g.inventory.counts[2] == 12);
  // All species, all four stages, regrow vs replant; inventory overflow is atomic.
  Plants p;
  p.reset();
  Inventory inv;
  for (int i = 0; i < SeedCount; i++) {
    int n = p.plant({int16_t(i), 1}, i);
    assert(n >= 0);
    assert(p.stage(n) == 0);
    p.entries[n].age = PlantDefs[i].growSeconds * .4f;
    assert(p.stage(n) == 1);
    p.entries[n].age = PlantDefs[i].growSeconds * .7f;
    assert(p.stage(n) == 2);
    p.entries[n].age = PlantDefs[i].growSeconds;
    assert(p.stage(n) == 3);
    assert(p.harvest(n, inv));
    assert(p.entries[n].active == PlantDefs[i].regrows);
  }
  int n = p.plant({10, 1}, 2);
  p.entries[n].age = 120;
  inv.counts[int(Item::Carrot)] = MaxStack;
  auto before = inv.counts;
  assert(!p.harvest(n, inv));
  assert(p.entries[n].active);
  assert(inv.counts == before);
  // A real planted crop plus changed terrain/resource removal survives serialization.
  g.selectedSeed = 3;
  g.act(soil, Tool::Seeds);
  settle(g);
  id = g.plants.at(soil);
  g.plants.entries[id].age = 77;
  g.clock = 456.25;
  g.muted = true;
  auto state = std::make_unique<SaveState>();
  state->capture(g);
  std::vector<uint8_t> buffer(SaveCodec::Capacity);
  size_t bytes = SaveCodec::encode(*state, buffer.data(), buffer.size(), 7);
  assert(bytes > 0);
  auto restored = std::make_unique<SaveState>();
  SaveInfo info;
  assert(SaveCodec::decode(buffer.data(), bytes, *restored, info));
  assert(info.sequence == 7);
  assert(restored->plants.count() == 1);
  assert(restored->plants.entries[restored->plants.at(soil)].age == 77);
  assert(restored->inventory.counts == g.inventory.counts);
  assert(!restored->world.objects[pouch].active);
  assert(restored->world.at(soil).terrain == Terrain::Soil);
  auto loaded = std::make_unique<Game>();
  loaded->begin();
  restored->apply(*loaded);
  assert(loaded->clock == 456.25);
  assert(loaded->muted);
  assert(loaded->player.settled == g.player.settled);
  // Truncation, payload corruption, header/sequence corruption and unsupported versions.
  for (size_t size : std::vector<size_t>{0, 10, 23, bytes - 1})
    assert(!SaveCodec::decode(buffer.data(), size, *restored, info));
  for (size_t offset : std::vector<size_t>{4, 12, 20, 24, bytes - 1}) {
    buffer[offset] ^= 0x80;
    assert(!SaveCodec::decode(buffer.data(), bytes, *restored, info));
    buffer[offset] ^= 0x80;
  }
  // Valid checksum is insufficient for invalid semantics: reject out-of-bounds plants.
  state->plants.entries[id].tile = {-1, 5};
  bytes = SaveCodec::encode(*state, buffer.data(), buffer.size(), 8);
  assert(!SaveCodec::decode(buffer.data(), bytes, *restored, info));
  state->capture(g);
  state->inventory.counts[0] = 10000;
  bytes = SaveCodec::encode(*state, buffer.data(), buffer.size(), 8);
  assert(!SaveCodec::decode(buffer.data(), bytes, *restored, info));
  state->capture(g);
  // No trusted wall time => no invented offline progression. Valid UTC is capped at 72h.
  restored->capture(g);
  restored->utc = 0;
  restored->apply(*loaded, 2000000000);
  assert(loaded->clock == g.clock);
  restored->utc = 1700000000;
  restored->apply(*loaded, 1700000000 + 1000000);
  assert(loaded->clock == g.clock + 72 * 3600);
  assert(loaded->plants.mature(loaded->plants.at(soil)));
  // Regeneration cannot overwrite a planted or cultivated tile.
  g.clock = 1000;
  g.update(5);
  assert(g.world.at(soil).object < 0);
  assert(!g.world.objects[pouch].active);
  // Tool/seed changes while walking must not alter the already queued planting action.
  g.selectedSeed = 1;
  g.act({31, 35}, Tool::Seeds);
  g.selectedSeed = 7;
  settle(g);
  assert(g.plants.entries[g.plants.at({31, 35})].type == 1);
  g.walkTo({34, 34});
  g.update(.1f);
  g.selectedSeed = 2;
  g.act({32, 35}, Tool::Seeds);
  g.selectedSeed = 4;
  settle(g);
  assert(g.plants.entries[g.plants.at({32, 35})].type == 2);
  // Rest cancels transient actions and resumes without an unexpected late interaction.
  g.act({33, 35}, Tool::Seeds);
  g.rest();
  assert(!g.hasAction() && !g.player.moving());
  double clock = g.clock;
  g.update(10);
  assert(g.clock == clock);
  std::puts("PASS farming loop, eight species, watering, harvest/regrow, overflow, save roundtrip, "
            "corruption, semantics, offline time, regeneration");
}

#include "SaveCodec.h"
#include <cstring>
namespace wilds {
namespace {
constexpr std::array<uint32_t, 256> makeCrcTable() {
  std::array<uint32_t, 256> table{};
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t c = i;
    for (int b = 0; b < 8; b++)
      c = (c >> 1) ^ (0xedb88320u & uint32_t(-int(c & 1)));
    table[i] = c;
  }
  return table;
}
constexpr auto CrcTable = makeCrcTable();
struct Writer {
  uint8_t *data;
  size_t capacity, pos = 0;
  bool ok = true;
  void u(uint64_t v, int bytes) {
    if (pos + bytes > capacity) {
      ok = false;
      return;
    }
    for (int i = 0; i < bytes; i++)
      data[pos++] = uint8_t(v >> (i * 8));
  }
  void f(float v) {
    uint32_t n;
    memcpy(&n, &v, 4);
    u(n, 4);
  }
  void d(double v) {
    uint64_t n;
    memcpy(&n, &v, 8);
    u(n, 8);
  }
};
struct Reader {
  const uint8_t *data;
  size_t size, pos = 0;
  bool ok = true;
  uint64_t u(int bytes) {
    if (pos + bytes > size) {
      ok = false;
      return 0;
    }
    uint64_t v = 0;
    for (int i = 0; i < bytes; i++)
      v |= uint64_t(data[pos++]) << (i * 8);
    return v;
  }
  float f() {
    uint32_t n = u(4);
    float v;
    memcpy(&v, &n, 4);
    return v;
  }
  double d() {
    uint64_t n = u(8);
    double v;
    memcpy(&v, &n, 8);
    return v;
  }
};
bool validTime(double t) {
  return std::isfinite(t) && t >= 0 && t < 1e12;
}
} // namespace
uint32_t SaveCodec::crc(const uint8_t *data, size_t n) {
  uint32_t c = ~0u;
  for (size_t i = 0; i < n; i++)
    c = (c >> 8) ^ CrcTable[(c ^ data[i]) & 255];
  return ~c;
}

void SaveState::capture(const Game &g, int64_t trustedUtc) {
  world = g.world;
  plants = g.plants;
  inventory = g.inventory;
  player = g.player.settled;
  clock = g.clock;
  dayLength = g.dayLength;
  unlocks = g.unlocks;
  direction = g.player.direction;
  seed = g.selectedSeed;
  muted = g.muted;
  utc = trustedUtc;
}
void SaveState::apply(Game &g, int64_t trustedUtc) const {
  g.world = world;
  g.plants = plants;
  g.inventory = inventory;
  g.clock = clock;
  g.dayLength = dayLength;
  g.unlocks = unlocks;
  g.selectedSeed = seed;
  g.muted = muted;
  g.player = Player{};
  g.player.settled = player;
  g.player.position = {float(player.x), float(player.y)};
  g.player.direction = direction;
  // No network or RTC is assumed. Only callers with a trusted clock may advance offline.
  if (utc >= 1577836800 && trustedUtc >= utc) {
    double elapsed = std::min(double(trustedUtc - utc), 72. * 3600);
    g.clock += elapsed;
    g.plants.update(float(elapsed));
  }
  g.camera.following = true;
  g.camera.center(g.player.position);
  g.revision = 0;
}
size_t SaveCodec::encode(const SaveState &s, uint8_t *buffer, size_t cap, uint32_t seq) {
  if (cap < HeaderSize)
    return 0;
  Writer w{buffer, cap};
  w.pos = HeaderSize;
  w.u(MapSize, 2);
  w.d(s.clock);
  w.f(s.dayLength);
  w.u(s.unlocks, 4);
  w.u(s.utc, 8);
  w.u(s.player.x, 2);
  w.u(s.player.y, 2);
  w.u(s.direction, 1);
  w.u(s.seed, 1);
  w.u(s.muted, 1);
  for (auto n : s.inventory.counts)
    w.u(n, 2);
  for (const auto &t : s.world.tiles) {
    w.u(int(t.terrain), 1);
    w.u(t.variant, 1);
    w.u(uint16_t(t.object), 2);
  }
  w.u(s.world.objectCount, 2);
  for (int i = 0; i < s.world.objectCount; i++) {
    const auto &o = s.world.objects[i];
    w.u(o.tile.x, 2);
    w.u(o.tile.y, 2);
    w.u(int(o.kind), 1);
    w.u(o.variant, 1);
    w.u(o.active, 1);
    w.d(o.respawnAt);
  }
  w.u(s.plants.count(), 2);
  for (const auto &p : s.plants.entries)
    if (p.active) {
      w.u(p.tile.x, 2);
      w.u(p.tile.y, 2);
      w.u(p.type, 1);
      w.f(p.age);
      w.f(p.water);
    }
  if (!w.ok)
    return 0;
  // Header CRC protects sequence/length as well as the payload CRC.
  Writer h{buffer, cap};
  h.u(0x444c574c, 4);
  h.u(Version, 2);
  h.u(HeaderSize, 2);
  h.u(w.pos - HeaderSize, 4);
  h.u(seq, 4);
  h.u(crc(buffer + HeaderSize, w.pos - HeaderSize), 4);
  h.u(crc(buffer, 20), 4);
  return w.pos;
}
bool SaveCodec::decode(const uint8_t *buffer, size_t size, SaveState &s, SaveInfo &info) {
  if (size < HeaderSize || size > Capacity)
    return false;
  Reader r{buffer, size};
  if (r.u(4) != 0x444c574c || r.u(2) != Version || r.u(2) != HeaderSize)
    return false;
  auto length = r.u(4);
  uint32_t seq = r.u(4), sum = r.u(4), headerCrc = r.u(4);
  if (length != size - HeaderSize || crc(buffer, 20) != headerCrc ||
      crc(buffer + HeaderSize, length) != sum)
    return false;
  if (r.u(2) != MapSize)
    return false;
  s.clock = r.d();
  s.dayLength = r.f();
  s.unlocks = r.u(4);
  s.utc = int64_t(r.u(8));
  s.player = {int16_t(r.u(2)), int16_t(r.u(2))};
  s.direction = r.u(1);
  s.seed = r.u(1);
  auto muted = r.u(1);
  s.muted = muted;
  if (!validTime(s.clock) || !std::isfinite(s.dayLength) || s.dayLength < 60 ||
      s.dayLength > 86400 || !inBounds(s.player) || s.direction >= 4 || s.seed >= SeedCount ||
      muted > 1 || s.utc < 0 || s.utc > 4102444800LL || (s.unlocks & ~0xffu))
    return false;
  for (auto &n : s.inventory.counts) {
    n = r.u(2);
    if (n > MaxStack)
      return false;
  }
  for (auto &t : s.world.tiles) {
    auto terrain = r.u(1);
    t.terrain = Terrain(terrain);
    t.variant = r.u(1);
    t.object = int16_t(r.u(2));
    if (terrain > int(Terrain::Soil) || t.variant > 3 || t.object < -1)
      return false;
  }
  s.world.objectCount = r.u(2);
  if (s.world.objectCount > World::MaxObjects)
    return false;
  for (int i = 0; i < s.world.objectCount; i++) {
    auto &o = s.world.objects[i];
    o.tile = {int16_t(r.u(2)), int16_t(r.u(2))};
    auto kind = r.u(1);
    o.kind = ObjectKind(kind);
    o.variant = r.u(1);
    auto active = r.u(1);
    o.active = active;
    o.respawnAt = r.d();
    if (!inBounds(o.tile) || kind > int(ObjectKind::WildFlower) || o.variant > 3 || active > 1 ||
        !validTime(o.respawnAt) || (!o.active && !collectible(o.kind)))
      return false;
  }
  std::array<uint8_t, World::MaxObjects> references{};
  for (int i = 0; i < TileCount; i++) {
    const auto &t = s.world.tiles[i];
    if (t.object < 0)
      continue;
    if (t.object >= s.world.objectCount)
      return false;
    const auto &o = s.world.objects[t.object];
    if (!o.active || t.terrain == Terrain::Water)
      return false;
    TilePos p{int16_t(i % MapSize), int16_t(i / MapSize)};
    if (o.kind == ObjectKind::Cottage) {
      if (p.x < o.tile.x - 1 || p.x > o.tile.x || p.y < o.tile.y - 1 || p.y > o.tile.y)
        return false;
    } else if (p != o.tile)
      return false;
    ++references[t.object];
  }
  for (int i = 0; i < s.world.objectCount; i++) {
    auto &o = s.world.objects[i];
    if (o.active && (s.world.at(o.tile).object != i ||
                     references[i] != (o.kind == ObjectKind::Cottage ? 4 : 1)))
      return false;
    if (!o.active && references[i])
      return false;
  }
  if (!s.world.walkable(s.player))
    return false;
  s.plants.reset();
  int count = r.u(2);
  if (count > Plants::Capacity)
    return false;
  for (int i = 0; i < count; i++) {
    TilePos p{int16_t(r.u(2)), int16_t(r.u(2))};
    int type = r.u(1);
    float age = r.f(), water = r.f();
    if (!inBounds(p) || type >= SeedCount || s.world.at(p).terrain != Terrain::Soil ||
        s.world.at(p).object >= 0 || !std::isfinite(age) || age < 0 ||
        age > PlantDefs[type].growSeconds || !std::isfinite(water) || water < 0 || water > 60)
      return false;
    int id = s.plants.plant(p, type);
    if (id < 0)
      return false;
    s.plants.entries[id].age = age;
    s.plants.entries[id].water = water;
  }
  if (!r.ok || r.pos != size)
    return false;
  info = {seq, sum};
  return true;
}
} // namespace wilds

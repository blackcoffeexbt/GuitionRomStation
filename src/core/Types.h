#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
namespace wilds {
constexpr int MapSize = 64, TileCount = MapSize * MapSize, Width = 480, Height = 320,
              WorldBottom = 266;
struct Vec2 {
  float x = 0, y = 0;
};
struct TilePos {
  int16_t x = 0, y = 0;
  bool operator==(TilePos b) const { return x == b.x && y == b.y; }
  bool operator!=(TilePos b) const { return !(*this == b); }
};
inline int index(TilePos p) {
  return p.y * MapSize + p.x;
}
inline bool inBounds(TilePos p) {
  return p.x >= 0 && p.y >= 0 && p.x < MapSize && p.y < MapSize;
}
inline Vec2 project(Vec2 p) {
  return {(p.x - p.y) * 32.f, (p.x + p.y) * 16.f};
}
inline Vec2 unproject(Vec2 p) {
  return {p.x / 64.f + p.y / 32.f, p.y / 32.f - p.x / 64.f};
}
inline TilePos pick(Vec2 p) {
  auto w = unproject(p);
  return {int16_t(std::floor(w.x + .5f)), int16_t(std::floor(w.y + .5f))};
}
inline float distance(Vec2 a, Vec2 b) {
  return std::hypot(a.x - b.x, a.y - b.y);
}
inline uint32_t hash(uint32_t n) {
  n ^= n >> 16;
  n *= 0x7feb352d;
  n ^= n >> 15;
  n *= 0x846ca68b;
  return n ^ (n >> 16);
}
} // namespace wilds

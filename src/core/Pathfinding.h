#pragma once
#include "World.h"
namespace wilds {
struct Path {
  std::array<TilePos, TileCount> points{};
  int count = 0;
};
class Pathfinding {
  std::array<uint16_t, TileCount> costs{}, heap{}, positions{};
  std::array<int16_t, TileCount> parent{};
  std::array<uint8_t, TileCount> state{};
  int heapSize = 0;
  TilePos target{};
  int score(int id) const;
  void push(int id);
  int pop();
  void rise(int slot);

public:
  bool find(const World &w, TilePos from, TilePos to, Path &result);
  bool adjacent(const World &w, TilePos from, TilePos object, Path &result);
};
} // namespace wilds

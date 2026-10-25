#pragma once
#include "Game.h"
namespace wilds {
struct SaveState {
  World world;
  Plants plants;
  Inventory inventory;
  TilePos player{};
  double clock = 0;
  float dayLength = 1200;
  uint32_t unlocks = 0xff;
  uint8_t direction = 0, seed = 2;
  bool muted = false;
  int64_t utc = 0;
  void capture(const Game &game, int64_t trustedUtc = 0);
  void apply(Game &game, int64_t trustedUtc = 0) const;
};
struct SaveInfo {
  uint32_t sequence = 0, checksum = 0;
};
class SaveCodec {
public:
  static constexpr size_t Capacity = 65536, HeaderSize = 24;
  static constexpr uint16_t Version = 1;
  static uint32_t crc(const uint8_t *data, size_t size);
  static size_t encode(const SaveState &state, uint8_t *buffer, size_t capacity, uint32_t sequence);
  static bool decode(const uint8_t *buffer, size_t size, SaveState &state, SaveInfo &info);
};
} // namespace wilds

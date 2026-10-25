#pragma once
#include <cstdint>
#include <cstddef>
namespace emu {
bool begin();
// Adopts a validated PSRAM allocation on success; caller retains it on failure.
bool load(uint8_t *data, size_t bytes, bool bios = false);
bool hasGame();
void run(bool render, uint8_t pad);
void reset();
const uint8_t *pixels();
void setPixels(uint8_t *target);
void palette(uint16_t *colors);
const int16_t *audio(int channel);
int samples();
} // namespace emu

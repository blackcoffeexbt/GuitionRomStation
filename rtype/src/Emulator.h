#pragma once
#include <cstdint>
namespace emu {
bool begin();
void run(bool render, uint8_t pad);
void reset();
const uint8_t *pixels();
void setPixels(uint8_t *target);
void palette(uint16_t *colors);
const int16_t *audio(int channel);
int samples();
} // namespace emu

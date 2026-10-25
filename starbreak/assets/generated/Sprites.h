#pragma once
#include <cstdint>
namespace sb {
struct Sprite {const uint8_t* data; int w,h;};
extern const Sprite asteroid;
extern const Sprite boss;
extern const Sprite cell;
extern const Sprite fighter;
extern const Sprite gunship;
extern const Sprite player;
extern const Sprite scout;
}

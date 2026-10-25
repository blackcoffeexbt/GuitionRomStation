#pragma once
#include <cstdint>
namespace sb {
struct Sprite {const uint8_t* data; int w,h;};
extern const Sprite asteroid;
extern const Sprite boss;
extern const Sprite carrier;
extern const Sprite cell;
extern const Sprite fighter;
extern const Sprite gunship;
extern const Sprite interceptor;
extern const Sprite mine;
extern const Sprite player;
extern const Sprite scout;
extern const Sprite turret;
extern const Sprite warden;
}

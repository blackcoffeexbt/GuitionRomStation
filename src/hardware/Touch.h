#pragma once
#include "../core/Types.h"
class Touch {
 bool ready=false;bool lastDown=false;uint32_t lastValid=0;wilds::Vec2 lastPoint{};
public:
 bool begin();bool read(wilds::Vec2& point);
};

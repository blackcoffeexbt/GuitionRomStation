#pragma once
#include "../Input.h"
class Touch {
  bool ready = false;
  bool lastDown = false;
  uint32_t lastValid = 0;
  sb::Vec2 lastPoint{};

public:
  bool begin();
  bool read(sb::Vec2 &point);
};

#pragma once
#include <cstdint>
class Audio {
  int16_t interleaved[368 * 2]{};

public:
  bool begin();
  bool submit(const int16_t *left, const int16_t *right, int samples, bool muted);
};

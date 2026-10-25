#pragma once
#include <cstdlib>
constexpr int MALLOC_CAP_SPIRAM = 1;
inline void *heap_caps_calloc(size_t count, size_t bytes, int) {
  return std::calloc(count, bytes);
}

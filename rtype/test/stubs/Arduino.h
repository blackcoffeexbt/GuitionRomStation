#pragma once
#include <cstdio>
struct HostSerial {
  template <class... Args> int printf(const char *format, Args... args) {
    return std::printf(format, args...);
  }
};
struct HostESP {
  unsigned getFreeHeap() const { return 0; }
  unsigned getFreePsram() const { return 0; }
};
inline HostSerial Serial;
inline HostESP ESP;

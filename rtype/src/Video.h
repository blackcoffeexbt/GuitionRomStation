#pragma once
#include "hardware/Display.h"
#include "FrameMailbox.h"
#include <atomic>
namespace port {
class Video {
  struct Slot {
    uint8_t *pixels = nullptr;
    uint16_t colors[32]{};
  };
  Slot slots[3];
  sb::Display display;
  FrameMailbox mailbox;
  static void task(void *self);
  void loop();

public:
  std::atomic<uint32_t> menu{1}; // paused, connected, muted
  std::atomic<uint32_t> presented{0}, transferUs{0}, scaleUs{0}, frameAgeUs{0};
  bool begin();
  uint8_t *pixels() { return slots[mailbox.writing()].pixels; }
  void publish(uint32_t frame);
};
} // namespace port

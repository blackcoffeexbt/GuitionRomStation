#pragma once
#include <array>
#include <atomic>
#include <cstdint>
namespace port {
// Pixels/palettes are accessed only by their WRITING or READING owner.
// Once copied to the LCD buffer, a READING slot is released before SPI transfer.
class FrameMailbox {
  enum : uint32_t { Free, Writing, Queued, Reading };
  struct Slot {
    std::atomic<uint32_t> state{Free}, frame{0}, queuedAt{0};
  };
  std::array<Slot, 3> slots{};
  int writer = 0;

public:
  FrameMailbox() { slots[0].state = Writing; }
  int writing() const { return writer; }
  int publish(uint32_t frame, uint32_t now) {
    int next = -1;
    for (int i = 0; i < 3; i++) {
      uint32_t expected = Free;
      if (slots[i].state.compare_exchange_strong(expected, Writing)) {
        next = i;
        break;
      }
    }
    if (next < 0) {
      int oldest = -1;
      uint32_t serial = UINT32_MAX;
      for (int i = 0; i < 3; i++)
        if (slots[i].state.load() == Queued && slots[i].frame.load() < serial) {
          serial = slots[i].frame.load();
          oldest = i;
        }
      if (oldest >= 0) {
        uint32_t expected = Queued;
        if (slots[oldest].state.compare_exchange_strong(expected, Writing))
          next = oldest;
      }
    }
    if (next < 0)
      return -1;
    int published = writer;
    slots[published].frame.store(frame);
    slots[published].queuedAt.store(now);
    slots[published].state.store(Queued, std::memory_order_release);
    writer = next;
    return published;
  }
  int acquire() {
    int newest = -1;
    uint32_t serial = 0;
    for (int i = 0; i < 3; i++)
      if (slots[i].state.load(std::memory_order_acquire) == Queued &&
          slots[i].frame.load() >= serial) {
        serial = slots[i].frame.load();
        newest = i;
      }
    if (newest < 0)
      return -1;
    uint32_t expected = Queued;
    if (!slots[newest].state.compare_exchange_strong(expected, Reading))
      return -1;
    serial = slots[newest].frame.load();
    for (int i = 0; i < 3; i++)
      if (i != newest && slots[i].frame.load() <= serial) {
        uint32_t stale = Queued;
        slots[i].state.compare_exchange_strong(stale, Free);
      }
    return newest;
  }
  void release(int index) { slots[index].state.store(Free, std::memory_order_release); }
  uint32_t queuedAt(int index) const { return slots[index].queuedAt.load(); }
  uint32_t frame(int index) const { return slots[index].frame.load(); }
};
} // namespace port

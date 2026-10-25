#pragma once
#include "../core/SaveCodec.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class SaveManager {
  uint8_t *buffer = nullptr;
  wilds::SaveState *snapshot = nullptr;
  QueueHandle_t queue = nullptr;
  std::atomic<int> result{0};
  size_t length = 0;
  uint32_t sequence = 0, savedRevision = 0, pendingRevision = 0, lastChange = 0, lastSave = 0;
  uint32_t observedRevision = 0;
  bool busy = false, available = false, retryNeeded = false;
  bool exitAfterSave = false;
  int currentSlot = -1;
  static void worker(void *self);
  void request(wilds::Game &game, uint32_t now, bool exit);

public:
  bool begin(wilds::Game &game);
  void update(wilds::Game &game, uint32_t now);
  bool isBusy() const { return busy; }
  uint32_t savedSequence() const { return sequence; }
};

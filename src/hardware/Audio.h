#pragma once
#include <cstdint>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class Audio {
  struct Frame {
    int16_t pcm[368 * 2]{};
    int count = 0;
  };
  Frame pending;
  QueueHandle_t queue = nullptr;
  std::atomic<uint32_t> errors{0};
  static void task(void *self);

public:
  bool begin();
  uint32_t writeErrors() const { return errors.load(); }
  bool submit(const int16_t *left, const int16_t *right, int samples, bool muted);
};

#pragma once
#include "../Sound.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class Audio {
  QueueHandle_t events = nullptr;
  static void task(void *self);

public:
  bool begin();
  void play(sb::SoundEvent event, bool muted);
};

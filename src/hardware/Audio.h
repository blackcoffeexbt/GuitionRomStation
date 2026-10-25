#pragma once
#include "../core/Game.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class Audio {
 QueueHandle_t events=nullptr;
 static void task(void* self);
public:
 bool begin();void play(wilds::SoundEvent event,bool muted);
};

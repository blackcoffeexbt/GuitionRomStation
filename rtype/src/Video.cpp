#include "Video.h"
#include "Emulator.h"
#include <Arduino.h>
#include <esp_heap_caps.h>
namespace port {
bool Video::begin() {
  for (auto &s : slots) {
    s.pixels = (uint8_t *)heap_caps_calloc(256 * 192, 1, MALLOC_CAP_SPIRAM);
    if (!s.pixels)
      return false;
  }
  slots[0].state.store(1);
  if (!display.begin())
    return false;
  return xTaskCreatePinnedToCore(task, "sms-video", 4096, this, 1, nullptr, 0) == pdPASS;
}
void Video::publish(uint32_t frame) {
  int next = -1;
  for (int i = 0; i < 3; i++) {
    uint32_t free = 0;
    if (slots[i].state.compare_exchange_strong(free, 1)) {
      next = i;
      break;
    }
  }
  if (next < 0)
    return;
  auto &s = slots[writing];
  emu::palette(s.colors);
  s.frame = frame;
  s.state.store(2, std::memory_order_release);
  writing = next;
}
void Video::task(void *self) {
  static_cast<Video *>(self)->loop();
}
void Video::loop() {
  uint32_t oldMenu = UINT32_MAX, lastFrame = 0;
  // Keep the most recently presented frame owned while menus reuse it.
  int retained = -1;
  for (;;) {
    uint32_t flags = menu.load();
    if (flags & 1) {
      if (flags != oldMenu) {
        if (retained >= 0)
          display.frame(slots[retained].pixels, slots[retained].colors);
        else
          display.clear(252);
        display.rect(48, 48, 384, 230, 250);
        display.text(77, 64, "R-TYPE / MASTER SYSTEM", 255, 2);
        display.text(77, 108,
                     (flags & 2) ? "A RESUME / RELEASE A FIRST" : "WAKE XBOX / HOLD PAIR IF NEEDED",
                     254);
        display.text(77, 141, "A HOLD: CHARGE   X: RAPID FIRE", 255);
        display.text(77, 165, "B: FORCE POD    STICK / DPAD: MOVE", 255);
        display.text(77, 200, "Y RESET GAME    MENU PAUSE", 251);
        display.text(77, 237, (flags & 4) ? "SOUND OFF - TAP HERE" : "SOUND ON - TAP HERE", 254);
        display.present();
        oldMenu = flags;
      }
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    oldMenu = flags;
    int newest = -1;
    uint32_t serial = lastFrame;
    for (int i = 0; i < 3; i++)
      if (slots[i].state.load(std::memory_order_acquire) == 2 && slots[i].frame >= serial) {
        newest = i;
        serial = slots[i].frame;
      }
    if (newest < 0) {
      vTaskDelay(1);
      continue;
    }
    uint32_t queued = 2;
    if (!slots[newest].state.compare_exchange_strong(queued, 3))
      continue;
    if (retained >= 0)
      slots[retained].state.store(0);
    retained = newest;
    lastFrame = serial;
    for (int i = 0; i < 3; i++) {
      if (i == newest)
        continue;
      uint32_t stale = 2;
      slots[i].state.compare_exchange_strong(stale, 0);
    }
    uint32_t start = micros();
    display.frame(slots[newest].pixels, slots[newest].colors);
    scaleUs = micros() - start;
    display.present();
    transferUs = display.transferUs;
    presented.fetch_add(1);
    // Rendering can always have another queued frame. Let core 0 idle/feed WDT.
    vTaskDelay(1);
  }
}
} // namespace port

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
  if (!display.begin())
    return false;
  return xTaskCreatePinnedToCore(task, "sms-video", 4096, this, 1, nullptr, 0) == pdPASS;
}
void Video::publish(uint32_t frame) {
  // Fill metadata before publishing the pixel owner; acquire uses release/acquire order.
  auto &s = slots[mailbox.writing()];
  emu::palette(s.colors);
  mailbox.publish(frame, micros());
}
void Video::task(void *self) {
  static_cast<Video *>(self)->loop();
}
void Video::loop() {
  uint32_t oldMenu = UINT32_MAX;
  for (;;) {
    uint32_t flags = menu.load();
    if (flags & 1) {
      if (flags != oldMenu) {
        // The converted LCD buffer already holds the last frame (or prior menu).
        // Repaint the complete overlay without retaining an emulator-owned slot.
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
    int newest = mailbox.acquire();
    if (newest < 0) {
      vTaskDelay(1);
      continue;
    }
    uint32_t queuedAt = mailbox.queuedAt(newest);
    uint32_t start = micros();
    display.frame(slots[newest].pixels, slots[newest].colors);
    scaleUs = micros() - start;
    mailbox.release(newest);
    display.present();
    frameAgeUs = micros() - queuedAt;
    transferUs = display.transferUs;
    presented.fetch_add(1);
    // Rendering can always have another queued frame. Let core 0 idle/feed WDT.
    vTaskDelay(1);
  }
}
} // namespace port

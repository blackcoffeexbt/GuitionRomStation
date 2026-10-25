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
  return xTaskCreatePinnedToCore(task, "sms-video", 6144, this, 1, nullptr, 0) == pdPASS;
}
void Video::library(const LibraryView &view) {
  portENTER_CRITICAL(&libraryMux);
  libraryView = view;
  libraryView.revision = ++libraryRevision;
  portEXIT_CRITICAL(&libraryMux);
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
  uint32_t oldMenu = UINT32_MAX, oldLibrary = UINT32_MAX;
  for (;;) {
    uint32_t flags = menu.load();
    if (flags & 8) {
      LibraryView view;
      portENTER_CRITICAL(&libraryMux);
      view = libraryView;
      portEXIT_CRITICAL(&libraryMux);
      if (view.revision != oldLibrary || oldMenu != flags) {
        display.clear(252);
        display.text(16, 10, "SMS LIBRARY", 255, 2);
        char counter[24];
        snprintf(counter, sizeof(counter), "%d / %d", view.count ? view.selected + 1 : 0,
                 view.count);
        display.text(370, 20, counter, 254);
        char folder[40];
        snprintf(folder, sizeof(folder), "SD CARD  %s", view.folder);
        display.text(16, 44, folder, 254);
        for (int row = 0; row < LibraryRows; row++) {
          if (view.first + row >= view.count)
            break;
          int y = 64 + row * 38;
          display.rect(12, y, 456, 34, view.first + row == view.selected ? 253 : 250);
          display.text(22, y + 8, view.rows[row], 255);
          display.text(414, y + 8, view.sizes[row], 251);
        }
        char message[74];
        snprintf(message, sizeof(message), "%.70s", view.message);
        display.text(16, 254, message, 251);
        display.rect(12, 278, 106, 34, 250);
        display.text(25, 287, "PREV", 255);
        display.rect(132, 278, 106, 34, 250);
        display.text(145, 287, "NEXT", 255);
        display.rect(252, 278, 106, 34, 250);
        display.text(265, 287, "RESCAN", 254);
        display.rect(372, 278, 96, 34, 250);
        display.text(384, 287, view.hasGame ? "BACK" : "", 255);
        display.present();
        oldLibrary = view.revision;
        oldMenu = flags;
      }
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    if (flags & 1) {
      if (flags != oldMenu) {
        // The converted LCD buffer already holds the last frame (or prior menu).
        // Repaint the complete overlay without retaining an emulator-owned slot.
        display.rect(48, 48, 384, 230, 250);
        display.text(77, 64, "MASTER SYSTEM", 255, 2);
        display.text(77, 108,
                     (flags & 2) ? "A RESUME / RELEASE A FIRST" : "WAKE XBOX / HOLD PAIR IF NEEDED",
                     254);
        display.text(77, 141, "A BUTTON 1     B BUTTON 2", 255);
        display.text(77, 165, "X RAPID FIRE   STICK / DPAD MOVE", 255);
        display.text(77, 200, "Y RESET       X ROM LIBRARY", 251);
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

#include "Display.h"
#include "BoardConfig.h"
#include <esp_heap_caps.h>
#include <cstring>

bool Display::begin() {
  if (!psramFound()) {
    Serial.println("FATAL: OPI PSRAM unavailable");
    return false;
  }
  Serial.printf("MEM internal_free=%u largest=%u\n",
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  logical = (uint16_t *)heap_caps_malloc(480 * 320 * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  Serial.printf("MEM internal_framebuffer=%d\n", logical != nullptr);
  if (!logical)
    logical = (uint16_t *)heap_caps_malloc(480 * 320 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  physical = (uint16_t *)heap_caps_malloc(480 * 320 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!logical || !physical) {
    Serial.println("FATAL: framebuffers unavailable");
    return false;
  }
  pinMode(board::Backlight, OUTPUT);
  digitalWrite(board::Backlight, LOW);
  bus = new Arduino_ESP32QSPI(board::LcdCs, board::LcdClock, board::LcdD0, board::LcdD1,
                              board::LcdD2, board::LcdD3);
  panel = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
  if (!panel->begin(board::DisplayClockHz))
    return false;
  std::fill(logical, logical + 480 * 320, uint16_t(0));
  present();
  digitalWrite(board::Backlight, HIGH);
  return true;
}
void Display::present() {
  auto start = micros();
  // The library can gather rotated scanlines while the previous scanline's
  // DMA transfer runs. Feed it big-endian pixels; avoid a full PSRAM transpose.
  auto *words = reinterpret_cast<uint32_t *>(logical);
  for (int i = 0; i < 480 * 320 / 2; i++) {
    uint32_t p = words[i];
    words[i] = ((p & 0x00ff00ff) << 8) | ((p & 0xff00ff00) >> 8);
  }
  rotationUs = micros() - start;
  start = micros();
  panel->draw16bitBeRGBBitmapR1(0, 0, logical, 480, 320);
  transferUs = micros() - start;
  // present consumes the frame. Renderer::draw clears it completely next frame,
  // so a second whole-frame byte conversion here would be redundant.
}
void Display::benchmark() {
  std::fill(physical, physical + 480 * 320, uint16_t(0));
  // Native full-width strips avoid the controller's unreliable narrow windows.
  uint32_t start = micros();
  for (int n = 0; n < 5; n++)
    panel->draw16bitRGBBitmap(0, 0, physical, 320, 480);
  auto full = (micros() - start) / 5;
  start = micros();
  for (int n = 0; n < 5; n++)
    for (int y = 0; y < 480; y += 32)
      panel->draw16bitRGBBitmap(0, y, physical + y * 320, 320, 32);
  auto strips = (micros() - start) / 5;
  Serial.printf("BENCH full_us=%u strips32_us=%u qspi_hz=%d strategy=full-frame\n", full, strips,
                board::DisplayClockHz);
}

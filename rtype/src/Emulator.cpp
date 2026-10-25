#include "Emulator.h"
#include <esp_heap_caps.h>
#include <Arduino.h>
extern "C" {
#include "shared.h"
extern const uint8_t romStart[] asm("_binary_roms_game_sms_start");
extern const uint8_t romEnd[] asm("_binary_roms_game_sms_end");
uint8_t unalChar(const void *address) {
  return *(const uint8_t *)address;
}
void system_load_sram(void) {
  memset(sms.sram, 0, 32768);
}
}
namespace emu {
bool begin() {
  size_t length = romEnd - romStart;
  if (length < 16384 || length % 16384 || length > 1024 * 1024)
    return false;
  cart.rom = const_cast<uint8_t *>(romStart);
  cart.pages = length / 16384;
  cart.type = TYPE_SMS;
  ::sms.country = TYPE_OVERSEAS;
  ::sms.use_fm = 0;
  ::sms.sram = (uint8_t *)heap_caps_calloc(32768, 1, MALLOC_CAP_SPIRAM);
  ::sms.dummy = (uint8_t *)heap_caps_calloc(16384, 1, MALLOC_CAP_SPIRAM);
  if (!::sms.sram || !::sms.dummy)
    return false;
  bitmap.width = 256;
  bitmap.height = 192;
  bitmap.pitch = 256;
  bitmap.depth = 8;
  emu_system_init(22080);
  system_reset();
  Serial.printf("SMS ROM bytes=%u pages=%u sound=%d samples=%d heap=%u psram=%u\n",
                unsigned(length), cart.pages, snd.enabled, snd.bufsize, ESP.getFreeHeap(),
                ESP.getFreePsram());
  return snd.enabled && snd.bufsize == 368;
}
void reset() {
  system_reset();
}
void run(bool render, uint8_t pad) {
  input.pad[0] = pad;
  input.system = 0;
  sms_frame(!render);
}
void setPixels(uint8_t *target) {
  bitmap.data = target;
}
const uint8_t *pixels() {
  return bitmap.data;
}
void palette(uint16_t *colors) {
  for (int i = 0; i < 32; i++) {
    auto *c = bitmap.pal.color[i];
    colors[i] = uint16_t((c[0] >> 3) << 11 | (c[1] >> 2) << 5 | (c[2] >> 3));
  }
}
const int16_t *audio(int channel) {
  return snd.buffer[channel];
}
int samples() {
  return snd.bufsize;
}
} // namespace emu

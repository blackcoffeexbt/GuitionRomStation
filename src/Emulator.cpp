#include "Emulator.h"
#include "RomImage.h"
#include <esp_heap_caps.h>
#include <Arduino.h>
extern "C" {
#include "shared.h"
uint8_t unalChar(const void *address) {
  return *(const uint8_t *)address;
}
void system_load_sram(void) {
  memset(sms.sram, 0, 32768);
}
}
namespace emu {
namespace {
// A harmless HALT cartridge keeps reset/memory maps valid before selection.
const uint8_t blank[16384] = {0x76};
uint8_t *ownedRom = nullptr;
} // namespace
bool begin() {
  cart.rom = const_cast<uint8_t *>(blank);
  cart.pages = 1;
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
  Serial.printf("SMS ROM bytes=%u pages=%u sound=%d samples=%d heap=%u psram=%u\n", unsigned(16384),
                cart.pages, snd.enabled, snd.bufsize, ESP.getFreeHeap(), ESP.getFreePsram());
  return snd.enabled && snd.bufsize == 368;
}
bool hasGame() {
  return ownedRom != nullptr;
}
bool load(uint8_t *data, size_t bytes, bool bios) {
  if (!data || data == ownedRom || bytes < 16384 || bytes > 1024 * 1024 || bytes % 16384 ||
      port::romHeader(data, bytes))
    return false;
  auto *previous = ownedRom;
  cart.rom = data;
  cart.pages = bytes / 16384;
  cart.bios = bios;
  cart.type = TYPE_SMS;
  ::sms.country = port::romRegion(data, bytes) == 3 ? TYPE_DOMESTIC : TYPE_OVERSEAS;
  system_reset();
  SN76496_init(0, MASTER_CLOCK, 255, 22080);
  for (int i = 0; i < 2; i++)
    memset(snd.buffer[i], 0, snd.bufsize * sizeof(int16_t));
  ownedRom = data;
  free(previous);
  return true;
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

#include "shared.h"
#include <assert.h>
#include <stdint.h>
uint8_t unalChar(const void *address) {
  return *(const uint8_t *)address;
}
void system_load_sram(void) {
  memset(sms.sram, 0, 0x8000);
}
static uint32_t checksum(const uint8_t *data, size_t n) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < n; i++)
    hash = (hash ^ data[i]) * 16777619u;
  return hash;
}
static void snapshot(const char *path) {
  FILE *f = fopen(path, "wb");
  assert(f);
  fprintf(f, "P6\n256 192\n255\n");
  for (int i = 0; i < 256 * 192; i++) {
    int p = bitmap.data[i] & 31;
    fwrite(bitmap.pal.color[p], 1, 3, f);
  }
  fclose(f);
}
int main(int argc, char **argv) {
  assert(argc == 2 || argc == 3);
  FILE *file = fopen(argv[1], "rb");
  assert(file);
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);
  assert(size >= 16384 && size % 16384 == 0 && size <= 1024 * 1024);
  cart.rom = calloc(1, size);
  assert(cart.rom);
  assert(fread(cart.rom, 1, size, file) == size);
  fclose(file);
  cart.pages = size / 16384;
  cart.type = TYPE_SMS;
  sms.sram = calloc(1, 32768);
  sms.dummy = calloc(1, 16384);
  bitmap.data = calloc(1, 256 * 192);
  assert(sms.sram && sms.dummy && bitmap.data);
  bitmap.width = 256;
  bitmap.height = 192;
  bitmap.pitch = 256;
  bitmap.depth = 8;
  sms.country = TYPE_OVERSEAS;
  sms.use_fm = 0;
  emu_system_init(22080);
  system_reset();
  assert(snd.enabled && snd.bufsize == 368);
  int64_t audioEnergy = 0;
  int frameCount = argc == 3 ? 7200 : 600;
  for (int frame = 0; frame < frameCount; frame++) {
    input.pad[0] = frame > 180 ? ((frame / 5) % 2 ? INPUT_BUTTON1 : 0) : 0;
    if (argc == 3 && frame > 180) {
      input.pad[0] = (frame % 180 < 110 ? INPUT_BUTTON1 : 0) |
                     (frame % 300 < 10 ? INPUT_BUTTON2 : 0) |
                     (frame % 480 < 120   ? INPUT_UP
                      : frame % 480 < 240 ? INPUT_DOWN
                      : frame % 480 < 360 ? INPUT_LEFT
                                          : INPUT_RIGHT);
    }
    sms_frame(0);
    for (int i = 0; i < snd.bufsize; i++)
      audioEnergy += abs(snd.buffer[0][i]);
    if (frame == 179)
      snapshot(".build/title.ppm");
  }
  snapshot(".build/gameplay.ppm");
  uint32_t pixels = checksum(bitmap.data, 256 * 192), ram = checksum(sms.ram, sizeof(sms.ram));
  printf("CORE frames=%d pixels=%08x ram=%08x audio_samples=%d\n", frameCount, pixels, ram,
         snd.bufsize);
  unsigned colors = 0;
  uint8_t used[32] = {0};
  for (int i = 0; i < 256 * 192; i++)
    used[bitmap.data[i] & 31] = 1;
  for (int i = 0; i < 32; i++)
    colors += used[i];
  assert(colors >= 4 && audioEnergy > 0);
  printf("VIDEO colors=%u PSG absolute_sample_sum=%lld\n", colors, (long long)audioEnergy);
  // A reset must reinitialise bank registers and release pause before another run.
  system_reset();
  assert(sms.fcr[0] == 0 && sms.fcr[1] == 0 && sms.fcr[2] == 1 && sms.fcr[3] == 0);
  for (int frame = 0; frame < 120; frame++)
    sms_frame(frame % 2);
  puts("PASS ROM boot, video/PSG output, Sega bank reset and render skipping");
  return 0;
}

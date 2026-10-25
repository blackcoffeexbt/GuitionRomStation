#include "Emulator.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
extern "C" {
#include "shared.h"
void cpu_writeport(int port, int data);
void cpu_writemem16(int address, int data);
}
uint8_t *cartridge(uint8_t value) {
  auto *data = (uint8_t *)std::calloc(16384, 1);
  assert(data);
  uint8_t program[] = {0x3e, value, 0x32, 0x00, 0xc0, 0x76};
  std::memcpy(data, program, sizeof(program));
  std::memcpy(data + 0x3ff0, "TMR SEGA", 8);
  data[0x3fff] = 0x4b;
  return data;
}
int main() {
  std::array<uint8_t, 256 * 192> screen{};
  emu::setPixels(screen.data());
  assert(emu::begin() && !emu::hasGame());
  auto *first = cartridge(0x31);
  assert(emu::load(first, 16384) && emu::hasGame());
  emu::run(true, 0);
  assert(sms.ram[0] == 0x31);
  auto *invalid = cartridge(0x44);
  invalid[0x3fff] = 0x6b;
  assert(!emu::load(invalid, 16384));
  std::free(invalid);
  assert(cart.rom == first && sms.ram[0] == 0x31);
  assert(!emu::load(nullptr, 16384) && !emu::load(first, 16384));
  for (int i = 1; i <= 40; i++) {
    auto *next = cartridge(uint8_t(i));
    if (i % 2)
      next[0x3fff] = 0x3b;
    SN76496Write(0, 0x90);
    assert(emu::load(next, 16384) && sms.ram[0] == 0xab && sn[0].Volume[0] == 0);
    assert(sms.country == (i % 2 ? TYPE_DOMESTIC : TYPE_OVERSEAS));
    emu::run(true, 0);
    assert(sms.ram[0] == i && cart.pages == 1 && cart.rom == next);
  }
  // Built-in-game BIOS must see empty external slots while probing port 3E.
  auto *bios = (uint8_t *)std::calloc(49152, 1);
  assert(bios);
  for (int page = 0; page < 3; page++)
    bios[page * 16384] = uint8_t(0x10 + page);
  bios[0] = 0xc3;
  bios[1] = 0;
  bios[2] = 0xc1;
  std::memcpy(bios + 0x7ff0, "TMR SEGA", 8);
  bios[0x7fff] = 0x4d;
  assert(emu::load(bios, 49152, true));
  assert(sms.memctrl == 0xe0 && sms.fcr[3] == 2 && cpu_readmap[4][0] == 0x12);
  cpu_writeport(0x3e, 0xe8);
  assert(cpu_readmap[0][0] == 0xff && cpu_readmap[3][0x1ff0] == 0xff);
  cpu_writemem16(0xfffe, 2);
  assert(sms.fcr[2] == 1);
  cpu_writeport(0x3e, 0xe0);
  uint8_t probe[] = {0x3e, 0xe8, 0xd3, 0x3e, 0x3a, 0xf0, 0x7f, 0x32, 0x10, 0xc0, 0x3e,
                     0xe0, 0xd3, 0x3e, 0x3a, 0xf0, 0x7f, 0x32, 0x11, 0xc0, 0x76};
  std::memcpy(sms.ram + 0x100, probe, sizeof(probe));
  emu::run(true, 0);
  assert(sms.ram[0x10] == 0xff && sms.ram[0x11] == 'T');
  // Banking slot zero cannot replace the CPU's fixed first 1 KB vectors/code.
  auto *banked = (uint8_t *)std::calloc(49152, 1);
  assert(banked);
  uint8_t startup[] = {0x3e, 1, 0x32, 0xfd, 0xff, 0xc3, 0x00, 0x02};
  uint8_t fixed[] = {0x3e, 0x5a, 0x32, 0x10, 0xc0, 0x76};
  uint8_t wrong[] = {0x3e, 0x99, 0x32, 0x10, 0xc0, 0x76};
  std::memcpy(banked, startup, sizeof(startup));
  std::memcpy(banked + 0x200, fixed, sizeof(fixed));
  std::memcpy(banked + 0x4200, wrong, sizeof(wrong));
  std::memcpy(banked + 0x7ff0, "TMR SEGA", 8);
  banked[0x7fff] = 0x4d;
  assert(emu::load(banked, 49152));
  emu::run(true, 0);
  assert(sms.ram[0x10] == 0x5a);
  puts("PASS BIOS slot probing/open bus, bank defaults, mapper isolation and fixed first-1-KB CPU "
       "mapping");
  puts("PASS cartridge adoption, failed-load recovery, 40 live ROM switches, CPU/RAM/PSG reset and "
       "old-image release");
}

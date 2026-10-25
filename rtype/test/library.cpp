#include "RomImage.h"
#include "Library.h"
#include <array>
#include <cassert>
#include <cstdio>
int main() {
  for (size_t n : {size_t(8192), size_t(16384), size_t(49152), size_t(524288), port::MaxRomBytes}) {
    auto layout = port::romLayout(n);
    assert(layout && layout.offset == 0 && layout.payload == n && layout.mapped >= 16384);
    auto copier = port::romLayout(n + 512);
    assert(copier && copier.offset == 512 && copier.payload == n);
  }
  for (size_t n : {size_t(0), size_t(512), size_t(4096), size_t(24576), size_t(32769),
                   port::MaxRomBytes + 16384})
    assert(!port::romLayout(n));
  std::array<uint8_t, 32768> rom{};
  assert(port::romHeader(rom.data(), rom.size()));
  std::memcpy(rom.data() + 0x7ff0, "TMR SEGA", 8);
  rom[0x7fff] = 0x4c;
  assert(!port::romHeader(rom.data(), rom.size()));
  rom[0x7fff] = 0x6c;
  assert(port::romHeader(rom.data(), rom.size()));
  std::array<uint8_t, 16384> small{};
  std::memcpy(small.data() + 0x1ff0, "TMR SEGA", 8);
  small[0x1fff] = 0x4a;
  assert(!port::romHeader(small.data(), 8192));
  auto layout = port::romLayout(8192);
  port::mirrorSmallRom(small.data(), layout);
  assert(!std::memcmp(small.data(), small.data() + 8192, 8192));
  assert(port::smsFilename("Sonic.SMS") && port::smsFilename("R-Type (En).sms"));
  for (auto name : {"._R-Type.sms", ".hidden.sms", "a.zip", "game.gg", "../game.sms", "a/b.sms",
                    "a\\b.sms", ".sms", ""})
    assert(!port::smsFilename(name));
  port::LibraryModel library;
  library.count = 12;
  library.open(port::LibraryModel::Accept);
  assert(library.update(port::LibraryModel::Accept, 0) == port::LibraryModel::Action::None);
  library.update(0, 1);
  assert(library.update(port::LibraryModel::Down, 2) == port::LibraryModel::Action::None &&
         library.selected == 1);
  library.update(port::LibraryModel::Down, 351);
  assert(library.selected == 1);
  library.update(port::LibraryModel::Down, 352);
  assert(library.selected == 2);
  library.update(port::LibraryModel::Down, 462);
  assert(library.selected == 3);
  library.update(0, 463);
  library.update(port::LibraryModel::Right, 464);
  assert(library.selected == 8 && library.first() == 5);
  assert(library.touchRow(64) == 5 && library.touchRow(253) == 9 && library.touchRow(254) == -1);
  library.update(0, 465);
  assert(library.update(port::LibraryModel::Accept, 466) == port::LibraryModel::Action::Load);
  library.count = 0;
  library.selected = 0;
  library.update(0, 467);
  assert(library.update(port::LibraryModel::Accept, 468) == port::LibraryModel::Action::None);
  puts("PASS ROM bounds/copier headers, console validation, 8 KB mirror, filename filtering, "
       "browser paging and held-button repeat");
}

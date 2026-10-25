#pragma once
#include "../Library.h"
#include <cstddef>
class RomStorage {
  port::RomEntry *entries = nullptr;
  bool mounted = false;
  int total = 0, skipped = 0;
  char error[88]{};
  char directory[24] = "/roms/sms";

public:
  using Progress = void (*)(size_t done, size_t total);
  bool begin();
  bool scan(Progress progress = nullptr);
  int count() const { return total; }
  const port::RomEntry &entry(int i) const { return entries[i]; }
  const char *message() const { return error; }
  uint8_t *load(int index, size_t &mapped, Progress progress);
  void view(port::LibraryView &out, const port::LibraryModel &model, bool hasGame) const;
};

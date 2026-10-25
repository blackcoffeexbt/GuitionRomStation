#include "RomReader.h"
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cstdio>
struct Reader {
  std::vector<uint8_t> data;
  size_t position = 0, failAt = SIZE_MAX;
  bool allowSeek = true;
  bool seek(size_t at) {
    position = at;
    return allowSeek && at <= data.size();
  }
  size_t read(uint8_t *to, size_t n) {
    if (position >= failAt || position + n > data.size())
      return 0;
    std::memcpy(to, data.data() + position, n);
    position += n;
    return n;
  }
};
int main() {
  unsigned active = 0;
  auto allocate = [&](size_t n) {
    ++active;
    return std::malloc(n);
  };
  auto release = [&](void *p) {
    assert(active);
    --active;
    std::free(p);
  };
  Reader reader;
  reader.data.resize(32768);
  std::memcpy(reader.data.data() + 0x7ff0, "TMR SEGA", 8);
  reader.data[0x7fff] = 0x4c;
  auto first = port::readRom(reader, reader.data.size(), allocate, release);
  assert(first.data && !first.error && first.bytes == 32768 && active == 1);
  // Short/card-removed reads must discard only the new allocation.
  reader.failAt = 4096;
  auto failed = port::readRom(reader, reader.data.size(), allocate, release);
  assert(!failed.data && failed.error && active == 1);
  reader.failAt = SIZE_MAX;
  reader.allowSeek = false;
  failed = port::readRom(reader, reader.data.size(), allocate, release);
  assert(failed.error && active == 1);
  reader.allowSeek = true;
  reader.data[0x7fff] = 0x6c;
  failed = port::readRom(reader, reader.data.size(), allocate, release);
  assert(failed.error && active == 1);
  auto noMemory = [](size_t) -> void * { return nullptr; };
  failed = port::readRom(reader, reader.data.size(), noMemory, release);
  assert(failed.error && active == 1);
  reader.data[0x7fff] = 0x4c;
  reader.data.insert(reader.data.begin(), 512, 0);
  auto copier = port::readRom(reader, reader.data.size(), allocate, release);
  assert(copier.data && copier.bytes == 32768 && !std::memcmp(copier.data, first.data, 32768));
  release(copier.data);
  release(first.data);
  assert(active == 0);
  puts("PASS actual loader reads, copier skip, card/short-read and seek failures, OOM cleanup, "
       "header rejection and previous-image preservation");
}

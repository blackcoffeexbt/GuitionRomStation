#include "FrameMailbox.h"
#include <array>
#include <thread>
#include <cassert>
#include <cstdio>
int main() {
  port::FrameMailbox q;
  for (uint32_t n = 1; n <= 20; n++)
    assert(q.publish(n, n * 10) >= 0);
  int reading = q.acquire();
  assert(reading >= 0 && q.frame(reading) == 20);
  // Producer must keep publishing fresh frames while the consumer owns its copy.
  for (uint32_t n = 21; n <= 40; n++)
    assert(q.publish(n, n * 10) >= 0);
  assert(q.frame(reading) == 20);
  q.release(reading);
  reading = q.acquire();
  assert(reading >= 0 && q.frame(reading) == 40);
  q.release(reading);
  port::FrameMailbox live;
  std::array<std::array<uint32_t, 128>, 3> pixels{};
  std::atomic<bool> done{false};
  std::thread producer([&] {
    for (uint32_t n = 1; n <= 50000; n++) {
      pixels[live.writing()].fill(n);
      live.publish(n, n);
    }
    done = true;
  });
  uint32_t last = 0, read = 0;
  while (!done.load() || read == 0) {
    int index = live.acquire();
    if (index < 0) {
      std::this_thread::yield();
      continue;
    }
    uint32_t frame = live.frame(index);
    assert(frame >= last);
    for (auto p : pixels[index])
      assert(p == frame);
    std::this_thread::yield();
    for (auto p : pixels[index])
      assert(p == frame);
    last = frame;
    ++read;
    live.release(index);
  }
  producer.join();
  printf("PASS newest-frame replacement, independent copy/transfer lifetime and %u concurrent "
         "reads without torn frames\n",
         read);
}

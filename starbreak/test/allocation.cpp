#include "core/Game.h"
#include "core/Pilot.h"
#include <cstdlib>
#include <new>
#include <cassert>
#include <cstdio>
static unsigned allocations = 0;
void *operator new(std::size_t n) {
  ++allocations;
  if (auto *p = std::malloc(n))
    return p;
  throw std::bad_alloc();
}
void operator delete(void *p) noexcept {
  std::free(p);
}
void operator delete(void *p, std::size_t) noexcept {
  std::free(p);
}
int main() {
  auto *g = new sb::Game;
  g->connected = true;
  g->start();
  unsigned before = allocations;
  while (g->screen == sb::Screen::Playing && g->stageTime < 400) {
    g->receive(sb::pilot(*g));
    g->update(1.f / 60);
  }
  assert(g->screen == sb::Screen::Victory);
  assert(allocations == before);
  delete g;
  std::puts("PASS complete stage simulation performs no C++ heap allocations");
}

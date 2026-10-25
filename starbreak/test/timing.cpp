#include "core/Game.h"
#include "core/Pilot.h"
#include <cassert>
#include <cstdio>
#include <memory>
int main() {
  for (int renderHz : {25, 30, 40}) {
    auto g = std::make_unique<sb::Game>();
    g->connected = true;
    g->start();
    double accumulator = 0;
    while (g->screen == sb::Screen::Playing && g->stageTime < 420) {
      g->receive(sb::pilot(*g));
      accumulator += 1.0 / renderHz;
      while (accumulator >= 1.0 / 60) {
        g->update(1.f / 60);
        accumulator -= 1.0 / 60;
      }
    }
    printf("TIMING %d Hz result=%d time=%.2f shields=%d power=%d\n", renderHz, int(g->screen),
           g->stageTime, g->shields, g->power);
    assert(g->screen == sb::Screen::Victory);
  }
}

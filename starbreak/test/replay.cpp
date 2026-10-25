#include "core/Game.h"
#include "core/Pilot.h"
#include <cassert>
#include <cstdio>
int main() {
  sb::Game g;
  g.connected = true;
  uint32_t previousBest = 0, victories = 0, defeats = 0, victoryScore = 0;
  for (int run = 0; run < 32; ++run) {
    g.start();
    bool lose = run % 2;
    while (g.screen == sb::Screen::Playing && g.stageTime < 650) {
      g.receive(sb::pilot(g, lose));
      g.update(1.f / 60);
    }
    if (lose) {
      assert(g.screen == sb::Screen::GameOver);
      ++defeats;
    } else {
      assert(g.screen == sb::Screen::Victory && g.bossesDefeated == 3);
      if (victories)
        assert(g.score == victoryScore);
      victoryScore = g.score;
      ++victories;
    }
    assert(g.best >= previousBest && g.runs == uint32_t(run + 1));
    previousBest = g.best;
  }
  assert(g.wins == victories && g.losses == defeats);
  printf("PASS 32 replay loops: %u complete campaigns, %u defeats, deterministic victory score %u, "
         "%.1f simulated minutes\n",
         victories, defeats, victoryScore, g.totalTicks / 3600.f);
}

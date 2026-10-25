#include "core/Game.h"
#include <cassert>
#include <cstdio>
using namespace sb;
int main() {
  Game g;
  g.connected = true;
  g.start();
  // Real kills grant experience and rank perks without granting weapon power.
  for (int i = 0; i < 20; ++i) {
    g.spawn(EnemyKind::Interceptor, 300, 160);
    g.shots[0] = {true, {300, 160}, {300, 160}, {}, 2, 99};
    g.update(1.f / 60);
  }
  assert(g.rank >= 3 && g.power == 1 && g.score == 4400);
  for (int level = 1; level <= Game::LevelCount; ++level) {
    g.levelTime = 80.1f;
    g.shields = 2;
    g.update(1.f / 60);
    assert(g.boss.active && g.boss.maxHp == (level == 1 ? 480 : level == 2 ? 850 : 1500));
    g.boss.hp = g.boss.maxHp / 2;
    g.update(1.f / 60);
    assert(g.boss.phase == 2);
    g.boss.hp = 0;
    g.update(1.f / 60);
    assert(g.bossesDefeated == level && !g.boss.active);
    if (level < Game::LevelCount) {
      assert(g.transition > 0 && g.screen == Screen::Playing && g.shields == 3);
      auto position = g.player;
      Input disconnected;
      g.receive(disconnected);
      float remaining = g.transition;
      g.update(1);
      assert(g.transition == remaining && g.player.x == position.x);
      Input reconnected;
      reconnected.connected = reconnected.fresh = true;
      g.receive(reconnected);
      reconnected.fire = true;
      g.receive(reconnected);
      for (int i = 0; i < 250; ++i)
        g.update(1.f / 60);
      assert(g.level == level + 1 && g.levelTime < 1 && g.power == 1);
    } else
      assert(g.screen == Screen::Victory && g.wins == 1);
  }
  // Boss contact shares the shield cooldown; it cannot be flown through safely.
  {
    Game contact;
    contact.connected = true;
    contact.start();
    contact.boss.active = true;
    contact.boss.age = 5;
    contact.player = {410, 161};
    contact.invulnerable = 0;
    contact.update(1.f / 60);
    assert(contact.shields == 2);
    contact.update(1.f / 60);
    assert(contact.shields == 2);
  }
  g.confirm();
  assert(g.level == 1 && g.rank == 1 && g.bossesDefeated == 0);
  // Canyon and citadel collisions use the same bounds exposed to the renderer.
  for (int level : {2, 3}) {
    g.level = level;
    g.levelTime = 5;
    for (int x = 22; x < 450; x += 10)
      assert(g.terrainBottom(x) - g.terrainTop(x) >= 150);
    g.player = {68, 57};
    g.invulnerable = 0;
    g.shields = 3;
    g.input = {};
    g.update(1.f / 60);
    assert(g.shields == 2 && g.player.y > g.terrainTop(g.player.x));
    g.update(1.f / 60);
    assert(g.shields == 2);
    g.levelTime = 71;
    assert(g.terrainTop(68) == 34 && g.terrainBottom(68) == 278);
  }
  std::puts("PASS three bosses/phases, sector transitions, reconnect during transition, XP/rank, "
            "terrain collision and retry reset");
}

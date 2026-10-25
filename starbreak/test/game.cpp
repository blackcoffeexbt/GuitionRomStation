#include "core/Game.h"
#include "core/Pilot.h"
#include "core/SettingsCodec.h"
#include <cassert>
#include <cstdio>
#include <memory>
using namespace sb;
int main() {
  auto ptr = std::make_unique<Game>();
  auto &g = *ptr;
  Input in;
  in.connected = true;
  in.fresh = true;
  g.receive(in);
  g.start();
  assert(g.screen == Screen::Playing && g.shields == 3 && g.bombs == 1 && g.power == 1);
  auto origin = g.player;
  in.x = 1;
  in.y = 1;
  g.receive(in);
  g.update(1.f / 60);
  float travel = std::hypot(g.player.x - origin.x, g.player.y - origin.y);
  assert(std::abs(travel - 165.f / 60) < .001);
  in.connected = false;
  g.receive(in);
  auto frozen = g.player;
  g.update(2);
  assert(g.player.x == frozen.x && g.screen == Screen::Pause);
  in.connected = true;
  in.fire = true;
  g.receive(in);
  assert(g.screen == Screen::Pause);
  in.fire = false;
  g.receive(in);
  in.fire = true;
  g.receive(in);
  assert(g.screen == Screen::Playing);
  g.grantUpgrade();
  g.grantUpgrade();
  assert(g.power == 3);
  g.invulnerable = 0;
  g.spawn(EnemyKind::Scout, g.player.x, g.player.y);
  g.update(1.f / 60);
  assert(g.shields == 2 && g.power == 3);
  g.spawn(EnemyKind::Scout, g.player.x, g.player.y);
  g.update(1.f / 60);
  assert(g.shields == 2);
  g.bomb();
  assert(g.bombs == 0);
  auto score = g.score;
  g.bomb();
  assert(g.bombs == 0 && g.score == score);
  g.boss.active = true;
  g.boss.age = 3;
  g.boss.hp = 750;
  g.boss.phase = 1;
  g.update(1.f / 60);
  assert(g.boss.phase == 2);
  g.level = 3;
  g.boss.hp = 0;
  g.update(1.f / 60);
  assert(g.screen == Screen::Victory && g.best == g.score && g.wins == 1);
  Settings s{12345, 7, true}, decoded;
  auto bytes = encodeSettings(s);
  assert(decodeSettings(bytes, decoded) && decoded.best == 12345 && decoded.muted);
  bytes[8] ^= 0x80;
  assert(!decodeSettings(bytes, decoded));
  bytes = encodeSettings(s);
  bytes[4] = 2;
  assert(!decodeSettings(bytes, decoded));
  std::puts("PASS cooldown, upgrades retained, bomb consumption, boss phase/victory, reconnect "
            "confirmation, settings CRC");
  // A lethal contact ends the tick: later bullets cannot inflate an already-final result.
  auto terminal = std::make_unique<Game>();
  terminal->connected = true;
  terminal->start();
  terminal->shields = 1;
  terminal->invulnerable = 0;
  terminal->spawn(EnemyKind::Scout, terminal->player.x, terminal->player.y);
  terminal->spawn(EnemyKind::Scout, 300, 150);
  terminal->shots[0] = {true, {290, 150}, {290, 150}, {1000, 0}, 2, 9};
  terminal->update(1.f / 60);
  assert(terminal->screen == Screen::GameOver && terminal->score == terminal->best);
  // Run actual authored stage, no acceleration in simulation time (host execution is naturally
  // faster).
  auto run = std::make_unique<Game>();
  run->connected = true;
  run->start();
  while (run->screen == Screen::Playing && run->stageTime < 650) {
    auto input = pilot(*run);
    run->receive(input);
    run->update(1.f / 60);
  }
  printf("PILOT result=%d time=%.2f shields=%d power=%d score=%u peaks=%d/%d\n", int(run->screen),
         run->stageTime, run->shields, run->power, run->score, run->peakEnemies,
         run->peakEnemyShots);
  assert(run->screen == Screen::Victory);
  // Intentional loss follows real collisions, then normal restart resets run state.
  auto defeat = std::make_unique<Game>();
  defeat->connected = true;
  defeat->start();
  while (defeat->screen == Screen::Playing && defeat->stageTime < 180) {
    defeat->receive(pilot(*defeat, true));
    defeat->update(1.f / 60);
  }
  assert(defeat->screen == Screen::GameOver);
  defeat->confirm();
  assert(defeat->screen == Screen::Playing && defeat->shields == 3 && defeat->power == 1 &&
         defeat->bombs == 1);
  std::puts("PASS authored title-to-victory simulation and real-collision defeat-to-retry");
}

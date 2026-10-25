#include "Game.h"
namespace sb {
namespace {
Vec2 aim(Vec2 from, Vec2 to, float speed) {
  float x = to.x - from.x, y = to.y - from.y, m = std::hypot(x, y);
  if (m < 1)
    return {-speed, 0};
  return {x / m * speed, y / m * speed};
}
uint32_t randomBits(uint32_t v) {
  v ^= v >> 16;
  v *= 0x7feb352d;
  v ^= v >> 15;
  v *= 0x846ca68b;
  return v ^ (v >> 16);
}
} // namespace
void Game::cue(Cue c) {
  if (int(c) > int(sound))
    sound = c;
}
void Game::receive(Input state) {
  connected = state.connected;
  input = state;
  if (connected != wasConnected) {
    wasConnected = connected;
    resumeArmed = false;
    if (!connected && screen == Screen::Playing) {
      screen = Screen::Pause;
      message("SIGNAL LOST - FLIGHT PAUSED", 5);
    } else if (connected && screen == Screen::Connection)
      message("CONTROLLER READY - PRESS A", 5);
  }
  if (!connected) {
    input.x = input.y = 0;
    input.fire = input.bomb = input.menu = false;
    input.nextWeapon = input.previousWeapon = false;
    fireCharge = 0;
  }
  if (connected && state.fresh && !state.fire && !state.menu)
    resumeArmed = true;
  bool a = input.fire && !previousA, b = input.bomb && !previousB, m = input.menu && !previousMenu;
  bool next = input.nextWeapon && !previousNext, prev = input.previousWeapon && !previousPrev;
  previousNext = input.nextWeapon;
  previousPrev = input.previousWeapon;
  if (connected && (screen == Screen::Playing || screen == Screen::Pause || screen == Screen::Title)) {
    if (next) selectWeapon(1);
    else if (prev) selectWeapon(-1);
  }
  previousA = input.fire;
  previousB = input.bomb;
  previousMenu = input.menu;
  if (m) {
    if (screen == Screen::Playing)
      pause();
    else if (screen == Screen::Pause && connected && resumeArmed) {
      screen = Screen::Playing;
      resumeArmed = false;
    }
    return;
  }
  if (a && screen != Screen::Playing && resumeArmed) {
    confirm();
    return;
  }
  if (b) {
    if (screen == Screen::Playing)
      bomb();
    else
      back();
  }
}
void Game::start() {
  if (!connected) {
    screen = Screen::Connection;
    pairRequested = true;
    return;
  }
  for (auto &e : enemies)
    e = {};
  for (auto &b : shots)
    b = {};
  for (auto &b : enemyShots)
    b = {};
  for (auto &p : pickups)
    p = {};
  for (auto &e : effects)
    e = {};
  player = previousPlayer = {68, 160};
  boss = {};
  shields = 3;
  bombs = 1;
  power = 1;
  // Keep a title-screen weapon choice; reset progression, powers and runtime pools.
  buffs = {};
  modules = 0;
  for (auto &m : shotInfo) m = {};
  for (auto &b : beams) b = {};
  for (auto &b : blasts) b = {};
  for (auto &f : fields) f = {};
  mirrorHistory.fill(player);
  mirrorPosition = player;
  mirrorIndex = 0;
  lootIndex = kills = 0;
  lootTimer = 12;
  orbTimer = fireCharge = heat = 0;
  heatTarget = -2;
  shotSerial = 0;
  score = 0;
  stageTime = levelTime = transition = 0;
  level = rank = 1;
  experience = bossesDefeated = 0;
  invulnerable = 2;
  fireTimer = 0;
  waveTimer = 2;
  waveIndex = 0;
  upgradeSpawned = 0;
  bombFlash = 0;
  peakEnemies = peakEnemyShots = 0;
  screen = Screen::Playing;
  ++runs;
  message("LEFT STICK TO MOVE  /  HOLD A TO FIRE", 7);
  cue(Cue::Pickup);
}
void Game::confirm() {
  if (screen == Screen::Title || screen == Screen::GameOver || screen == Screen::Victory)
    start();
  else if (screen == Screen::Connection) {
    if (connected) {
      screen = Screen::Title;
      message("READY FOR LAUNCH", 2);
    } else
      pairRequested = true;
  } else if (screen == Screen::Pause && connected) {
    screen = Screen::Playing;
    resumeArmed = false;
  }
}
void Game::back() {
  if (screen == Screen::Playing)
    return;
  if (screen == Screen::Pause) {
    screen = Screen::Title;
    needsSave = true;
  } else
    screen = Screen::Title;
}
void Game::pause() {
  if (screen == Screen::Playing) {
    screen = Screen::Pause;
    resumeArmed = false;
    message("FLIGHT PAUSED", 2);
  }
}
void Game::tap(Vec2 p) {
  // Touch operates menus only. There are deliberately no flight actions here.
  if (screen == Screen::Playing)
    return;
  if (p.y > 280) {
    muted = !muted;
    needsSave = true;
    cue(Cue::Pickup);
    return;
  }
  if (screen == Screen::Title) {
    if (p.y > 196 && p.y < 238) {
      screen = Screen::Connection;
      pairRequested = true;
    } else if (p.y > 143 && p.y < 188)
      confirm();
    return;
  }
  if (screen == Screen::Connection) {
    if (p.y > 164 && p.y < 210) {
      pairRequested = true;
      message("HOLD XBOX PAIR FOR 3 SECONDS", 5);
    } else if (p.y > 218) {
      screen = Screen::Title;
    }
    return;
  }
  if (screen == Screen::Pause) {
    if (p.y > 142 && p.y < 187) {
      if (connected && resumeArmed)
        confirm();
    } else if (p.y > 201 && p.y < 247)
      back();
    return;
  }
  if (p.y > 182 && p.y < 229)
    confirm();
  else if (p.y > 234 && p.y < 274)
    back();
}
void Game::grantUpgrade() {
  if (power < 5) {
    ++power;
    message(power == 5 ? "WEAPON V - MAXIMUM POWER" : power == 4 ? "WEAPON IV - OVERCHARGED" : power == 3 ? "WEAPON III - AMPLIFIED" : "WEAPON II - TWIN POWER", 4);
    cue(Cue::Pickup);
  }
}
void Game::addExperience(int amount) {
  experience += amount;
  if (rank < 4 && experience >= rank * 1400) {
    experience -= rank * 1400;
    ++rank;
    message(rank == 2   ? "PILOT II - RAPID FIRE"
            : rank == 3 ? "PILOT III - ENGINE BOOST"
                        : "PILOT IV - ACE CANNON",
            4);
    cue(Cue::Pickup);
  }
}
const char *Game::levelName() const {
  return level == 1 ? "01  OUTER BELT" : level == 2 ? "02  CRYSTAL CANYON" : "03  IRON CITADEL";
}
const char *Game::bossName() const {
  return level == 1 ? "WARDEN" : level == 2 ? "HIVE CARRIER" : "DREADNAUGHT";
}
float Game::terrainTop(float x) const {
  if (level == 1 || levelTime > 70 || transition > 0)
    return 34;
  float phase = (x + levelTime * 48) / 105;
  return level == 2 ? 49 + 15 * (1 + std::sin(phase))
                    : 52 + 18 * (int((x + levelTime * 48) / 80) % 3 == 0);
}
float Game::terrainBottom(float x) const {
  if (level == 1 || levelTime > 70 || transition > 0)
    return 278;
  float phase = (x + levelTime * 48) / 113;
  return level == 2 ? 263 - 15 * (1 + std::sin(phase + 1.8f))
                    : 261 - 18 * (int((x + levelTime * 48) / 96) % 3 == 1);
}
void Game::clearLevel() {
  ++bossesDefeated;
  score += 2000 * level;
  addExperience(900);
  boss.active = false;
  particles(boss.pos, 50, 1);
  for (auto &e : enemies)
    e.active = false;
  for (auto &b : enemyShots)
    b.active = false;
  for (auto &b : shots)
    b.active = false;
  if (level == LevelCount) {
    finish(true);
    return;
  }
  transition = 4;
  invulnerable = 6;
  shields = std::min(3, shields + 1);
  message("SECTOR CLEAR - SHIELD REPAIRED", 4);
  cue(Cue::Explosion);
}
void Game::spawn(EnemyKind kind, float x, float y, float phase) {
  for (auto &e : enemies)
    if (!e.active) {
      e = {};
      e.active = true;
      e.kind = kind;
      e.pos = e.previous = {x, y};
      e.baseY = y;
      e.phase = phase;
      e.shotTimer = kind == EnemyKind::Gunship ? 1.6f : 2.8f + phase * .1f;
      e.hp = kind == EnemyKind::Scout         ? 2
             : kind == EnemyKind::Fighter     ? 4
             : kind == EnemyKind::Gunship     ? 14
             : kind == EnemyKind::Interceptor ? 3
             : kind == EnemyKind::Mine        ? 5
             : kind == EnemyKind::Turret      ? 12
                                              : 9;
      return;
    }
}
Box Game::enemyBox(const Enemy &e) const {
  float w = e.kind == EnemyKind::Gunship       ? 17
            : e.kind == EnemyKind::Asteroid    ? 11
            : e.kind == EnemyKind::Turret      ? 13
            : e.kind == EnemyKind::Interceptor ? 10
            : e.kind == EnemyKind::Fighter     ? 11
                                               : 8;
  float h = e.kind == EnemyKind::Gunship    ? 12
            : e.kind == EnemyKind::Asteroid ? 10
            : e.kind == EnemyKind::Turret   ? 11
            : e.kind == EnemyKind::Mine     ? 8
            : e.kind == EnemyKind::Fighter  ? 7
                                            : 6;
  return {e.pos.x, e.pos.y, w, h};
}
int Game::enemyCount() const {
  int n = 0;
  for (auto &e : enemies)
    n += e.active;
  return n;
}
int Game::enemyShotCount() const {
  int n = 0;
  for (auto &e : enemyShots)
    n += e.active;
  return n;
}
void Game::particles(Vec2 p, int count, uint8_t color) {
  int emitted = 0;
  for (int i = 0; i < EffectCapacity && emitted < count; i++)
    if (!effects[i].active) {
      auto h = randomBits(totalTicks * 37 + i * 599);
      float angle = (h % 6283) * .001f, speed = 15 + (h >> 16) % 60;
      auto &e = effects[i];
      e.active = true;
      e.pos = p;
      e.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
      e.life = e.maxLife = .18f + (h % 30) * .01f;
      e.color = color;
      ++emitted;
    }
}
void Game::kill(Enemy &e, bool addScore) {
  if (!e.active) return;
  e.active = false;
  if (addScore && ++kills % 3 == 0) drop(e.pos, PickupKind::Crystal);
  if (addScore) {
    uint32_t before = score;
    score += e.kind == EnemyKind::Scout         ? 100
             : e.kind == EnemyKind::Fighter     ? 180
             : e.kind == EnemyKind::Gunship     ? 450
             : e.kind == EnemyKind::Interceptor ? 220
             : e.kind == EnemyKind::Turret      ? 350
                                                : 75;
    addExperience(int(score - before));
  }
  particles(e.pos, e.kind == EnemyKind::Gunship ? 12 : 6, 1);
  cue(Cue::Explosion);
}
void Game::enemyShot(Vec2 p, Vec2 velocity, float radius) {
  for (auto &b : enemyShots)
    if (!b.active) {
      b = {true, p, p, velocity, radius, 1};
      return;
    }
}
void Game::fire() {
  fireWeapon({player.x + 16, player.y});
  if (buffs.mirror > 0) fireWeapon({mirrorPosition.x + 16, mirrorPosition.y}, true);
  cue(Cue::Fire);
}
void Game::hitPlayer() {
  if (invulnerable > 0 || screen != Screen::Playing)
    return;
  if (buffs.bubble > 0) {
    --buffs.bubble;
    invulnerable = .4f;
    particles(player, 8, 0);
    cue(Cue::Impact);
    return;
  }
  --shields;
  invulnerable = 1.2f;
  particles(player, 12, 2);
  cue(Cue::Impact);
  if (shields <= 0)
    finish(false);
}
void Game::bomb() {
  if (screen != Screen::Playing || !connected || bombs == 0)
    return;
  --bombs;
  bombFlash = .38f;
  for (auto &b : enemyShots)
    b.active = false;
  for (auto &e : enemies)
    if (e.active)
      kill(e);
  if (boss.active) {
    boss.hp = std::max(0, boss.hp - 220);
    if (boss.hp <= boss.maxHp / 2)
      boss.phase = 2;
  }
  invulnerable = std::max(invulnerable, .8f);
  cue(Cue::Bomb);
}
void Game::finish(bool won) {
  if (screen != Screen::Playing)
    return;
  if (won) {
    score += 5000 + shields * 750;
    ++wins;
    screen = Screen::Victory;
  } else {
    ++losses;
    screen = Screen::GameOver;
  }
  best = std::max(best, score);
  needsSave = true;
  cue(Cue::Explosion);
}
void Game::spawnWave() {
  float t = levelTime;
  float y = 104 + (waveIndex * 41) % 112;
  if (level == 1) {
    if (t < 22) {
      for (int i = 0; i < 3; i++)
        spawn(EnemyKind::Scout, 500 + i * 32, y + (i % 2) * 18, float(i));
      waveTimer = 6;
    } else if (t < 48) {
      for (int i = 0; i < 3; i++)
        spawn(EnemyKind::Fighter, 502 + i * 42, 100 + (waveIndex * 31 + i * 43) % 120,
              float(i + waveIndex));
      waveTimer = 7;
    } else {
      spawn(EnemyKind::Gunship, 518, y);
      for (int i = 0; i < 3; i++)
        spawn(EnemyKind::Asteroid, 520 + i * 48, 100 + (waveIndex * 23 + i * 51) % 120, float(i));
      waveTimer = 7;
    }
  } else if (level == 2) {
    for (int i = 0; i < 3; i++)
      spawn(i == 1 ? EnemyKind::Mine : EnemyKind::Interceptor, 510 + i * 53,
            103 + (waveIndex * 29 + i * 47) % 110, float(i + waveIndex));
    if (t > 28)
      spawn(EnemyKind::Gunship, 570, y);
    waveTimer = 7;
  } else {
    spawn(EnemyKind::Turret, 510, waveIndex % 2 ? 94 : 221);
    for (int i = 0; i < 3; i++)
      spawn(i == 1 ? EnemyKind::Fighter : EnemyKind::Interceptor, 550 + i * 37,
            108 + (waveIndex * 31 + i * 39) % 105, float(i));
    if (t > 35)
      spawn(EnemyKind::Mine, 620, y);
    waveTimer = 7;
  }
  ++waveIndex;
}
void Game::updateEnemies(float dt) {
  for (auto &e : enemies)
    if (e.active) {
      e.previous = e.pos;
      e.age += dt;
      float speed = e.kind == EnemyKind::Scout         ? 62
                    : e.kind == EnemyKind::Fighter     ? 76
                    : e.kind == EnemyKind::Gunship     ? 39
                    : e.kind == EnemyKind::Interceptor ? 112
                    : e.kind == EnemyKind::Turret      ? 48
                    : e.kind == EnemyKind::Mine        ? 52
                                                       : 69;
      e.pos.x -= speed * dt;
      if (e.kind == EnemyKind::Fighter)
        e.pos.y = e.baseY + std::sin(e.age * 2.1f + e.phase) * 25;
      if (e.kind == EnemyKind::Asteroid)
        e.pos.y = e.baseY + std::sin(e.age * 1.2f + e.phase) * 8;
      if (e.kind == EnemyKind::Interceptor)
        e.pos.y = e.baseY + std::sin(e.age * 3.2f + e.phase) * 18;
      if (e.kind == EnemyKind::Mine)
        e.pos.y = e.baseY + std::sin(e.age * 1.8f) * 10;
      e.shotTimer -= dt;
      if (e.shotTimer <= 0 && e.pos.x < 452 && e.pos.x > player.x + 45 &&
          e.kind != EnemyKind::Asteroid) {
        if (e.kind == EnemyKind::Mine) {
          for (int i = -1; i <= 1; ++i)
            enemyShot(e.pos, {-74, float(i) * 48});
          e.shotTimer = 4;
        } else if (e.kind == EnemyKind::Turret) {
          auto v = aim(e.pos, player, 94);
          enemyShot(e.pos, v, 4);
          enemyShot(e.pos, {v.x, v.y + 18}, 3);
          e.shotTimer = 2.6f;
        } else if (e.kind == EnemyKind::Gunship) {
          auto v = aim(e.pos, player, 86);
          enemyShot(e.pos, {v.x, v.y - 25}, 4);
          enemyShot(e.pos, v, 4);
          enemyShot(e.pos, {v.x, v.y + 25}, 4);
          e.shotTimer = 2.8f;
        } else if (e.kind == EnemyKind::Fighter) {
          enemyShot(e.pos, aim(e.pos, player, 92));
          e.shotTimer = 2.1f;
        } else {
          enemyShot(e.pos, {-86, 0});
          e.shotTimer = 3.5f;
        }
      }
      if (e.pos.x < -40)
        e.active = false;
      if (e.active && overlaps(enemyBox(e), playerBox())) {
        hitPlayer();
        kill(e, false);
      }
    }
}
void Game::updateBoss(float dt) {
  if (!boss.active)
    return;
  boss.previous = boss.pos;
  boss.age += dt;
  boss.pos.x = std::max(410.f, 520 - boss.age * 55);
  boss.pos.y = 160 + std::sin(boss.age * .62f) * 51;
  if (boss.hp <= boss.maxHp / 2 && boss.phase == 1) {
    boss.phase = 2;
    boss.shotTimer = .8f;
    for (auto &b : enemyShots)
      b.active = false;
    particles(boss.pos, 24, 1);
    message("BOSS ARMOUR BROKEN - PRESS THE ATTACK", 4);
  }
  if (boss.hp > 0 && overlaps(bossBox(), playerBox())) {
    hitPlayer();
    if (screen != Screen::Playing)
      return;
  }
  boss.shotTimer -= dt;
  if (boss.age > 2.5f && boss.shotTimer <= 0) {
    Vec2 muzzle{boss.pos.x - (level == 1 ? 35 : level == 2 ? 41 : 44), boss.pos.y};
    auto v = aim(muzzle, player, boss.phase == 1 ? 90 : 108);
    if (level == 1) {
      for (int i = -1; i <= 1; i++)
        enemyShot(muzzle, {v.x, v.y + i * (boss.phase == 1 ? 22.f : 34.f)}, 3);
      boss.shotTimer = boss.phase == 1 ? 1.8f : 1.4f;
    } else if (level == 2) {
      for (int i = -2; i <= 2; i++)
        enemyShot(muzzle, {-92, float(i) * 27}, 3);
      if (boss.phase == 2)
        spawn(EnemyKind::Interceptor, 470, boss.pos.y, boss.age);
      boss.shotTimer = boss.phase == 1 ? 2.f : 1.65f;
    } else if (boss.phase == 1) {
      for (int i = -1; i <= 1; i++)
        enemyShot({muzzle.x, muzzle.y + i * 19}, {v.x, v.y + i * 13}, 4);
      boss.shotTimer = 1.35f;
    } else {
      for (int i = -2; i <= 2; i++)
        enemyShot(muzzle, {v.x, v.y + i * 23}, 3);
      boss.shotTimer = .95f;
    }
  }
  if (boss.hp <= 0) {
    clearLevel();
  }
}
void Game::updateShots(float dt) {
  updatePlayerShots(dt);
  for (auto &b : enemyShots)
    if (b.active) {
      b.previous = b.pos;
      b.pos.x += b.velocity.x * dt;
      b.pos.y += b.velocity.y * dt;
      if (swept({b.previous.x - previousPlayer.x, b.previous.y - previousPlayer.y},
                {b.pos.x - player.x, b.pos.y - player.y}, b.radius, {0, 0, 10, 7})) {
        b.active = false;
        hitPlayer();
      }
      if (b.pos.x < -15 || b.pos.y < 35 || b.pos.y > 286 || b.pos.x > 500)
        b.active = false;
    }
}
void Game::update(float dt) {
  visualTime += dt;
  bannerTime = std::max(0.f, bannerTime - dt);
  if (screen != Screen::Playing)
    return;
  if (!connected) {
    pause();
    return;
  }
  ++totalTicks;
  stageTime += dt;
  if (transition > 0) {
    transition -= dt;
    if (transition <= 0) {
      transition = 0;
      ++level;
      levelTime = 0;
      waveIndex = 0;
      waveTimer = 2;
      boss = {};
      player = previousPlayer = {68, 160};
      message(levelName(), 5);
    }
    return;
  }
  levelTime += dt;
  updateArsenal(dt);
  invulnerable = std::max(0.f, invulnerable - dt);
  bombFlash = std::max(0.f, bombFlash - dt);
  previousPlayer = player;
  auto move = normalizeMove(input.x, input.y);
  player.x = std::clamp(player.x + move.x * (rank >= 3 ? 180 : 165) * dt, 22.f, 450.f);
  player.y = std::clamp(player.y + move.y * (rank >= 3 ? 180 : 165) * dt, 57.f, 260.f);
  if (player.y - 7 < terrainTop(player.x) || player.y + 7 > terrainBottom(player.x)) {
    hitPlayer();
    player.y = std::clamp(player.y, terrainTop(player.x) + 9, terrainBottom(player.x) - 9);
    if (screen != Screen::Playing)
      return;
  }
  fireTimer -= dt;
  if (input.fire && fireTimer <= 0) {
    fire();
    float cadence = rank == 4 ? .84f : rank >= 2 ? .92f : 1.f;
    fireTimer = definition(weapon).interval * cadence * (buffs.overdrive > 0 ? .5f : 1.f);
    if (weapon == Weapon::Pulse) fireTimer *= 1.f - (power-1)*.07f;
  }
  waveTimer -= dt;
  if (waveTimer <= 0 && levelTime < 70)
    spawnWave();
  if (upgradeSpawned < 2 && stageTime > float(upgradeSpawned ? 55 : 22)) {
    for (auto &p : pickups)
      if (!p.active) {
        p = {true, {465, player.y}, 0};
        break;
      }
    ++upgradeSpawned;
    message("POWER CELL INBOUND - COLLECT IT", 4);
  }
  for (auto &p : pickups)
    if (p.active) {
      p.age += dt;
      p.pos.x -= 66 * dt;
      float dx = player.x - p.pos.x, dy = player.y - p.pos.y;
      float dist = std::hypot(dx, dy);
      if (dist < (buffs.magnet > 0 ? 480 : 60) && dist > 1) {
        p.pos.x += dx / dist * (buffs.magnet > 0 ? 230 : 110) * dt;
        p.pos.y += dy / dist * (buffs.magnet > 0 ? 230 : 110) * dt;
      }
      if (overlaps({p.pos.x, p.pos.y, 10, 10}, playerBox())) {
        p.active = false;
        collect(p.kind, p.value);
      }
      if (p.pos.x < -20)
        p.active = false;
    }
  if (levelTime > 80 && !boss.active && boss.age == 0) {
    boss.active = true;
    boss.hp = boss.maxHp = level == 1 ? 480 : level == 2 ? 850 : 1500;
    boss.pos = boss.previous = {520, 160};
    for (auto &e : enemies)
      e.active = false;
    for (auto &b : enemyShots)
      b.active = false;
    message(level == 1   ? "WARNING - BELT WARDEN"
            : level == 2 ? "WARNING - HIVE CARRIER"
                         : "WARNING - DREADNAUGHT",
            5);
  }

  updateEnemies(dt * (buffs.timeWarp > 0 ? .42f : 1.f));
  if (screen != Screen::Playing)
    return;
  updateBoss(dt * (buffs.timeWarp > 0 ? .55f : 1.f));
  if (screen != Screen::Playing)
    return;
  updateShots(dt);
  for (auto &e : effects)
    if (e.active) {
      e.life -= dt;
      e.pos.x += e.velocity.x * dt;
      e.pos.y += e.velocity.y * dt;
      if (e.life <= 0)
        e.active = false;
    }
  peakEnemies = std::max(peakEnemies, enemyCount());
  peakEnemyShots = std::max(peakEnemyShots, enemyShotCount());
}
} // namespace sb

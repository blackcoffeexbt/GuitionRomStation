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
  }
  if (connected && state.fresh && !state.fire && !state.menu)
    resumeArmed = true;
  bool a = input.fire && !previousA, b = input.bomb && !previousB, m = input.menu && !previousMenu;
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
  score = 0;
  stageTime = 0;
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
  if (power < 3) {
    ++power;
    message(power == 2 ? "CANNON II - TWIN STREAM" : "CANNON III - STARLANCE", 4);
    cue(Cue::Pickup);
  }
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
      e.hp = kind == EnemyKind::Scout     ? 2
             : kind == EnemyKind::Fighter ? 4
             : kind == EnemyKind::Gunship ? 14
                                          : 9;
      return;
    }
}
Box Game::enemyBox(const Enemy &e) const {
  float w = e.kind == EnemyKind::Gunship    ? 17
            : e.kind == EnemyKind::Asteroid ? 11
            : e.kind == EnemyKind::Fighter  ? 11
                                            : 8;
  float h = e.kind == EnemyKind::Gunship    ? 12
            : e.kind == EnemyKind::Asteroid ? 10
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
  e.active = false;
  if (addScore)
    score += e.kind == EnemyKind::Scout     ? 100
             : e.kind == EnemyKind::Fighter ? 180
             : e.kind == EnemyKind::Gunship ? 450
                                            : 75;
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
  int count = power == 1 ? 1 : power == 2 ? 2 : 3;
  for (int lane = 0; lane < count; lane++)
    for (auto &b : shots)
      if (!b.active) {
        float offset = count == 1 ? 0 : count == 2 ? (lane ? 5 : -5) : float(lane - 1) * 8;
        float vy = power == 3 ? float(lane - 1) * 26 : 0;
        Vec2 p{player.x + 19, player.y + offset};
        b = {true, p, p, {440, vy}, 2, power};
        break;
      }
  cue(Cue::Fire);
}
void Game::hitPlayer() {
  if (invulnerable > 0 || screen != Screen::Playing)
    return;
  --shields;
  invulnerable = 1.5f;
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
    if (boss.hp <= 750)
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
  float t = stageTime;
  if (t < 42) {
    int n = t < 16 ? 2 : 4;
    float y = 85 + (waveIndex * 47) % 155;
    for (int i = 0; i < n; i++)
      spawn(EnemyKind::Scout, 500 + i * 31, y + (i % 2) * 20, float(i));
    waveTimer = t < 16 ? 8 : 6.5f;
  } else if (t < 105) {
    int n = t < 65 ? 2 : 3;
    for (int i = 0; i < n; i++)
      spawn(EnemyKind::Fighter, 502 + i * 42, 90 + (waveIndex * 31 + i * 66) % 145,
            float(i + waveIndex));
    waveTimer = 7;
  } else if (t < 165) {
    spawn(EnemyKind::Gunship, 518, 90 + (waveIndex * 63) % 140, float(waveIndex));
    for (int i = 0; i < 3; i++)
      spawn(EnemyKind::Scout, 500 + i * 35, 70 + (waveIndex * 43 + i * 41) % 180, float(i));
    waveTimer = 8;
  } else if (t < 215) {
    for (int i = 0; i < 4; i++)
      spawn(EnemyKind::Asteroid, 500 + i * 46, 64 + (waveIndex * 53 + i * 61) % 195,
            float(i + waveIndex));
    if (waveIndex % 2)
      spawn(EnemyKind::Fighter, 560, 150, float(waveIndex));
    waveTimer = 5.5f;
  } else if (t < 250) {
    spawn(EnemyKind::Gunship, 520, 80 + (waveIndex * 71) % 160);
    for (int i = 0; i < 3; i++)
      spawn(EnemyKind::Fighter, 510 + i * 45, 78 + (waveIndex * 23 + i * 65) % 170, float(i));
    waveTimer = 8;
  } else
    waveTimer = 999;
  ++waveIndex;
}
void Game::updateEnemies(float dt) {
  for (auto &e : enemies)
    if (e.active) {
      e.previous = e.pos;
      e.age += dt;
      float speed = e.kind == EnemyKind::Scout     ? 62
                    : e.kind == EnemyKind::Fighter ? 76
                    : e.kind == EnemyKind::Gunship ? 39
                                                   : 69;
      e.pos.x -= speed * dt;
      if (e.kind == EnemyKind::Fighter)
        e.pos.y = e.baseY + std::sin(e.age * 2.1f + e.phase) * 25;
      if (e.kind == EnemyKind::Asteroid)
        e.pos.y = e.baseY + std::sin(e.age * 1.2f + e.phase) * 8;
      e.shotTimer -= dt;
      if (e.shotTimer <= 0 && e.pos.x < 452 && e.pos.x > player.x + 45 &&
          e.kind != EnemyKind::Asteroid) {
        if (e.kind == EnemyKind::Gunship) {
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
  if (boss.hp <= 750 && boss.phase == 1) {
    boss.phase = 2;
    boss.shotTimer = .8f;
    for (auto &b : enemyShots)
      b.active = false;
    particles(boss.pos, 24, 1);
    message("DREADNAUGHT - CORE EXPOSED", 4);
  }
  boss.shotTimer -= dt;
  if (boss.age > 2.5f && boss.shotTimer <= 0) {
    Vec2 muzzle{boss.pos.x - 42, boss.pos.y};
    auto v = aim(muzzle, player, boss.phase == 1 ? 90 : 108);
    if (boss.phase == 1) {
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
    boss.active = false;
    particles(boss.pos, 50, 1);
    finish(true);
  }
}
void Game::updateShots(float dt) {
  for (auto &b : shots)
    if (b.active) {
      b.previous = b.pos;
      b.pos.x += b.velocity.x * dt;
      b.pos.y += b.velocity.y * dt;
      for (auto &e : enemies)
        if (e.active) {
          auto box = enemyBox(e);
          box.x = box.y = 0;
          if (swept({b.previous.x - e.previous.x, b.previous.y - e.previous.y},
                    {b.pos.x - e.pos.x, b.pos.y - e.pos.y}, b.radius, box)) {
            b.active = false;
            e.hp -= b.damage;
            particles(b.pos, 2, 0);
            if (e.hp <= 0)
              kill(e);
            break;
          }
        }
      if (b.active && boss.active && boss.age > 2 &&
          swept({b.previous.x - boss.previous.x, b.previous.y - boss.previous.y},
                {b.pos.x - boss.pos.x, b.pos.y - boss.pos.y}, b.radius, {0, 0, 43, 42})) {
        b.active = false;
        boss.hp -= b.damage;
        particles(b.pos, 2, 0);
      }
      if (b.pos.x > 500 || b.pos.y < 42 || b.pos.y > 280)
        b.active = false;
    }
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
  invulnerable = std::max(0.f, invulnerable - dt);
  bombFlash = std::max(0.f, bombFlash - dt);
  previousPlayer = player;
  auto move = normalizeMove(input.x, input.y);
  player.x = std::clamp(player.x + move.x * 165 * dt, 22.f, 450.f);
  player.y = std::clamp(player.y + move.y * 165 * dt, 57.f, 260.f);
  fireTimer -= dt;
  if (input.fire && fireTimer <= 0) {
    fire();
    fireTimer = .135f;
  }
  waveTimer -= dt;
  if (waveTimer <= 0 && stageTime < 250)
    spawnWave();
  if (upgradeSpawned < 2 && stageTime > float(upgradeSpawned ? 153 : 68)) {
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
      if (dist < 60 && dist > 1) {
        p.pos.x += dx / dist * 110 * dt;
        p.pos.y += dy / dist * 110 * dt;
      }
      if (overlaps({p.pos.x, p.pos.y, 10, 10}, playerBox())) {
        p.active = false;
        grantUpgrade();
      }
      if (p.pos.x < -20)
        p.active = false;
    }
  if (stageTime > 265 && !boss.active && boss.age == 0) {
    boss.active = true;
    boss.pos = boss.previous = {520, 160};
    for (auto &e : enemies)
      e.active = false;
    for (auto &b : enemyShots)
      b.active = false;
    message("WARNING - DREADNAUGHT APPROACHING", 5);
  }
  if (stageTime > 165 && stageTime - dt <= 165)
    message("ASTEROID BELT - WATCH YOUR VECTOR", 5);
  updateEnemies(dt);
  if (screen != Screen::Playing)
    return;
  updateBoss(dt);
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

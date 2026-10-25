#pragma once
#include "Collision.h"
#include <array>
namespace sb {
enum class Screen : uint8_t { Title, Connection, Playing, Pause, GameOver, Victory };
enum class EnemyKind : uint8_t { Scout, Fighter, Gunship, Asteroid, Interceptor, Mine, Turret };
enum class Cue : uint8_t { None, Fire, Impact, Pickup, Explosion, Bomb };
struct Bullet {
  bool active = false;
  Vec2 pos{}, previous{}, velocity{};
  float radius = 2;
  int damage = 1;
};
struct Enemy {
  bool active = false;
  EnemyKind kind = EnemyKind::Scout;
  Vec2 pos{}, previous{};
  float baseY = 0, age = 0, shotTimer = 0, phase = 0;
  int hp = 1;
};
struct Pickup {
  bool active = false;
  Vec2 pos{};
  float age = 0;
};
struct Effect {
  bool active = false;
  Vec2 pos{}, velocity{};
  float life = 0, maxLife = 0;
  uint8_t color = 0;
};
struct Boss {
  bool active = false;
  Vec2 pos{414, 160}, previous{414, 160};
  int hp = 1500, maxHp = 1500, phase = 1;
  float age = 0, shotTimer = 1.5f;
};
class Game {
  float fireTimer = 0, waveTimer = 0;
  int waveIndex = 0;
  bool previousA = false, previousB = false, previousMenu = false, resumeArmed = false,
       wasConnected = false;
  int upgradeSpawned = 0;
  void spawnWave();
  void updateEnemies(float dt);
  void updateBoss(float dt);
  void updateShots(float dt);
  void fire();
  void hitPlayer();
  void kill(Enemy &enemy, bool score = true);
  void particles(Vec2 p, int count, uint8_t color);
  void finish(bool won);
  void clearLevel();
  void addExperience(int amount);
  void enemyShot(Vec2 p, Vec2 velocity, float radius = 3);
  void cue(Cue c);

public:
  static constexpr int EnemyCapacity = 32, ShotCapacity = 96, EnemyShotCapacity = 160,
                       EffectCapacity = 64;
  std::array<Enemy, EnemyCapacity> enemies{};
  std::array<Bullet, ShotCapacity> shots{};
  std::array<Bullet, EnemyShotCapacity> enemyShots{};
  std::array<Pickup, 4> pickups{};
  std::array<Effect, EffectCapacity> effects{};
  Screen screen = Screen::Title;
  Vec2 player{68, 160}, previousPlayer{68, 160};
  Boss boss{};
  Input input{};
  static constexpr int LevelCount = 3;
  int shields = 3, bombs = 1, power = 1;
  int level = 1, rank = 1, experience = 0, bossesDefeated = 0;
  float levelTime = 0, transition = 0;
  uint32_t score = 0, best = 0, runs = 0, wins = 0, losses = 0;
  float stageTime = 0, visualTime = 0, invulnerable = 0, bombFlash = 0, bannerTime = 0;
  const char *banner = "";
  bool muted = false, needsSave = false, pairRequested = false;
  Cue sound = Cue::None;
  uint32_t totalTicks = 0;
  int peakEnemies = 0, peakEnemyShots = 0;
  bool connected = false;
  void receive(Input state);
  void update(float dt);
  void start();
  void confirm();
  void back();
  void pause();
  void bomb();
  void tap(Vec2 p);
  void spawn(EnemyKind kind, float x, float y, float phase = 0);
  void message(const char *text, float seconds = 3) {
    banner = text;
    bannerTime = seconds;
  }
  void grantUpgrade();
  const char *levelName() const;
  const char *bossName() const;
  // Shared collision/rendering boundaries. Terrain retracts before each boss.
  float terrainTop(float x) const;
  float terrainBottom(float x) const;
  int enemyCount() const;
  int enemyShotCount() const;
  Box enemyBox(const Enemy &e) const;
  Box bossBox() const {
    return {boss.pos.x, boss.pos.y,
            level == 1   ? 34.f
            : level == 2 ? 40.f
                         : 43.f,
            level == 1   ? 31.f
            : level == 2 ? 34.f
                         : 41.f};
  }
  Box playerBox() const { return {player.x, player.y, 10, 7}; }
};
} // namespace sb

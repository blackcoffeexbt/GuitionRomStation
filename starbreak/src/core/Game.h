#pragma once
#include "Collision.h"
#include "Arsenal.h"
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
  PickupKind kind = PickupKind::Upgrade;
  uint8_t value = 0;
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
  int upgradeSpawned = 0, lootIndex = 0, kills = 0;
  float lootTimer = 12, orbTimer = 0, fireCharge = 0, heat = 0;
  int heatTarget = -2;
  bool previousNext = false, previousPrev = false;
  uint32_t shotSerial = 0;
  uint8_t mirrorIndex = 0;
  std::array<Vec2, 30> mirrorHistory{};
  void updateArsenal(float dt);
  void updatePlayerShots(float dt);
  void fireWeapon(Vec2 origin, bool echo = false, bool charged = false);
  void arc(Vec2 origin, int damage, int links, float range);
  void ray(Vec2 from, Vec2 to, int damage, float width, uint32_t color, bool prism = false);
  int target(Vec2 origin, float range, uint32_t excluded = 0) const;
  Vec2 targetPosition(int index) const;
  bool damageTarget(int index, int damage, Weapon source);
  void blast(Vec2 pos, int damage, float radius, bool chain = false, bool gravity = false);
  void field(Vec2 pos, Weapon kind);
  Bullet *projectile(Vec2 origin, Vec2 velocity, int damage, float radius, Weapon kind, float life = 2);
  void drop(Vec2 pos, PickupKind kind, uint8_t value = 0);
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
  std::array<Pickup, 12> pickups{};
  std::array<ShotInfo, ShotCapacity> shotInfo{};
  std::array<Beam, 24> beams{};
  std::array<Blast, 12> blasts{};
  std::array<Field, 6> fields{};
  Weapon weapon = Weapon::Pulse;
  Buffs buffs{};
  uint32_t modules = 0;
  Vec2 mirrorPosition{68,160};
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
  void selectWeapon(int direction);
  void applyPowerUp(PowerUp kind);
  void collect(PickupKind kind, uint8_t value = 0);
  bool hasModule(Weapon kind) const { return modules & (1u << int(kind)); }
  float chargeLevel() const { return fireCharge / 1.4f; }
  float heatLevel() const { return heat / 2.f; }
  Vec2 satellitePosition(int index, bool mirror = false) const;
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

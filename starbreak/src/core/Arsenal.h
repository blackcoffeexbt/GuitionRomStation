#pragma once
#include "../Input.h"
#include <array>
namespace sb {
enum class Weapon : uint8_t { Pulse, Plasma, Swarm, Arc, Nova, Scatter, Rail, Singularity, Boomerang, Flak, Prism, Thermal, Drones, GravityMine, Torpedo, Vortex, Count };
constexpr int WeaponCount = int(Weapon::Count);
struct WeaponDef { const char *name, *tag; uint32_t color; float interval; };
inline constexpr std::array<WeaponDef, WeaponCount> Weapons{{
 {"PULSE CANNON","PULSE",0x66e7ff,.15f}, {"PLASMA LANCE","LANCE",0x63ff96,.18f},
 {"SWARM MISSILES","SWARM",0xffb868,.72f}, {"ARC LIGHTNING","ARC",0x9faaff,.38f},
 {"NOVA CANNON","NOVA",0xff79bd,.7f}, {"SCATTER BLASTER","SCAT",0xffe782,.3f},
 {"RAILGUN","RAIL",0xe5fcff,.75f}, {"SINGULARITY GUN","SING",0xb68aff,1.65f},
 {"BOOMERANG BLADES","BLADE",0x80ffd4,.65f}, {"FLAK CANNON","FLAK",0xff9955,.65f},
 {"PRISM LASER","PRISM",0x8affed,.34f}, {"THERMAL BEAM","HEAT",0xff7d55,.15f},
 {"SEEKER DRONES","DRONE",0xa3ffa0,2.4f}, {"GRAVITY MINE","MINE",0xb895ff,1.5f},
 {"PHASE TORPEDO","PHASE",0x8ce9ff,.8f}, {"VORTEX CANNON","VORTX",0x8cafff,.26f}
}};
inline const WeaponDef &definition(Weapon w) { return Weapons[int(w)]; }
enum class PowerUp : uint8_t { Bubble, Overdrive, Satellites, TimeWarp, Mirror, Magnet, Repair, Nuke, Count };
inline constexpr const char *PowerNames[] = {"SHIELD BUBBLE", "OVERDRIVE", "SATELLITE ORBS", "TIME WARP", "MIRROR SHIP", "LOOT MAGNET", "HULL REPAIR", "SCREEN NUKE"};
inline constexpr const char *PowerTags[] = {"SH", "OD", "OR", "TW", "MI", "MG", "HP", "NU"};
enum class PickupKind : uint8_t { Upgrade, Module, Buff, Crystal };
struct ShotInfo {
 Weapon kind = Weapon::Pulse;
 uint8_t flags = 0;
 float age = 0, life = 2;
 uint32_t hits = 0;
 Vec2 origin{};
};
struct Beam {
 bool active = false;
 Vec2 from{}, to{};
 float width = 1, life = 0;
 uint32_t color = 0;
};
struct Blast {
 bool active = false, chain = false, gravity = false;
 Vec2 pos{};
 float age = 0, duration = .45f, radius = 40;
 int damage = 6;
 uint32_t hits = 0;
 bool bossHit = false;
};
struct Field {
 bool active = false;
 Weapon kind = Weapon::Singularity;
 Vec2 pos{};
 float age = 0, life = 2, radius = 55;
};
struct Buffs {
 int bubble = 0;
 float overdrive = 0, satellites = 0, timeWarp = 0, mirror = 0, magnet = 0;
};
} // namespace sb

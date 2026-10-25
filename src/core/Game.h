#pragma once
#include "Player.h"
#include "Camera.h"
#include "Input.h"
#include "Plants.h"
namespace wilds {
enum class SoundEvent { None, Tap, Arrive, Blocked, Harvest };
enum class Tool : uint8_t { Hand, Seeds, Hoe, Water };
enum class Panel : uint8_t { None, Guide, Bag, Seeds, Home };
class Game {
  Pathfinding pathfinder;
  bool route(TilePos p);
  void perform();
  void changed();
  void resources();
  bool actionPending = false;
  TilePos actionTile{};
  Tool actionTool = Tool::Hand;
  float resourceTick = 0;

public:
  World world;
  Player player;
  Camera camera;
  Inventory inventory;
  Plants plants;
  float time = 0, noticeTimer = 0, destinationTimer = 0;
  double clock = 0;
  float dayLength = 1200;
  uint32_t unlocks = 0xff;
  uint32_t revision = 0;
  const char *notice = "Welcome home. Tap the seed pouch to gather.";
  char noticeBuffer[100]{};
  TilePos destination{34, 34};
  Tool tool = Tool::Hand;
  uint8_t selectedSeed = 2;
  Panel panel = Panel::None;
  bool guide = false, muted = false, resting = false;
  bool saveRequested = false, exitRequested = false;
  const char *saveStatus = "Starting...";
  uint32_t taps = 0;
  SoundEvent sound = SoundEvent::None;
  void begin();
  void update(float dt);
  void gesture(const Gesture &g, TilePos picked = {-1, -1});
  void walkTo(TilePos p);
  void act(TilePos p, Tool requested);
  void notify(const char *format, ...);
  void message(const char *text, float duration = 3) {
    notice = text;
    noticeTimer = duration;
  }
  void markChanged() { changed(); }
  bool hasAction() const { return actionPending; }
};
} // namespace wilds

#include "Game.h"
#include <cstdarg>
#include <cstdio>
namespace wilds {
void Game::notify(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(noticeBuffer, sizeof(noticeBuffer), fmt, args);
  va_end(args);
  message(noticeBuffer, 4);
}
void Game::changed() {
  ++revision;
}
void Game::begin() {
  world.generate();
  plants.reset();
  inventory.starter();
  camera.center(player.position);
  noticeTimer = 8;
}
bool Game::route(TilePos p) {
  bool ok = world.walkable(p)
                ? pathfinder.find(world, player.settled, p, player.path)
                : inBounds(p) && world.at(p).object >= 0 &&
                      pathfinder.adjacent(world, player.settled,
                                          world.objects[world.at(p).object].tile, player.path);
  player.next = 0;
  if (ok) {
    camera.following = true;
    destination = player.path.count ? player.path.points[player.path.count - 1] : player.settled;
    destinationTimer = 4;
    sound = SoundEvent::Tap;
  } else {
    actionPending = false;
    message("That spot is out of reach");
    sound = SoundEvent::Blocked;
  }
  return ok;
}
void Game::walkTo(TilePos p) {
  actionPending = false;
  resting = false;
  if (!inBounds(p)) {
    message("The wilds end here");
    sound = SoundEvent::Blocked;
    return;
  }
  if (player.moving() &&
      distance(player.position, {float(player.settled.x), float(player.settled.y)}) > .001f) {
    player.pending = p;
    player.hasPending = true;
    return;
  }
  player.hasPending = false;
  route(p);
}
void Game::act(TilePos p, Tool requested) {
  if (!inBounds(p)) {
    message("The wilds end here");
    return;
  }
  // Finish the current movement edge before rerouting to an interaction.
  if (player.moving() &&
      distance(player.position, {float(player.settled.x), float(player.settled.y)}) > .001f) {
    player.pending = p;
    player.hasPending = true;
    actionPending = true;
    actionTile = p;
    actionTool = requested;
    return;
  }
  actionPending = true;
  actionTile = p;
  actionTool = requested;
  resting = false;
  player.hasPending = false;
  bool ok = pathfinder.adjacent(world, player.settled, p, player.path);
  player.next = 0;
  if (!ok) {
    actionPending = false;
    message("No clear place to stand beside it");
    sound = SoundEvent::Blocked;
    return;
  }
  camera.following = true;
  destination = p;
  destinationTimer = 4;
  sound = SoundEvent::Tap;
  if (!player.moving())
    perform();
}
void Game::perform() {
  if (!actionPending)
    return;
  actionPending = false;
  if (std::abs(player.settled.x - actionTile.x) + std::abs(player.settled.y - actionTile.y) > 1)
    return;
  auto &tile = world.tiles[index(actionTile)];
  int plantId = plants.at(actionTile);
  int objectId = tile.object;
  if (actionTool == Tool::Hand) {
    if (plantId >= 0) {
      auto &p = plants.entries[plantId];
      if (plants.mature(plantId)) {
        if (plants.harvest(plantId, inventory)) {
          changed();
          notify("Harvested %s + seeds", PlantDefs[p.type].name);
          sound = SoundEvent::Harvest;
        } else
          message("Your bag is full");
      } else
        notify("%s: %d%% grown. Water to help it along.", PlantDefs[p.type].name,
               int(p.age * 100 / PlantDefs[p.type].growSeconds));
      return;
    }
    if (objectId >= 0) {
      auto &o = world.objects[objectId];
      if (o.kind == ObjectKind::Cottage) {
        panel = Panel::Home;
        return;
      }
      if (collectible(o.kind)) {
        Item item = Item::Flower;
        int qty = 2;
        switch (o.kind) {
        case ObjectKind::WildSeeds:
          item = Item(selectedSeed);
          qty = 3;
          break;
        case ObjectKind::LooseWood:
        case ObjectKind::Log:
          item = Item::Wood;
          break;
        case ObjectKind::LooseStone:
          item = Item::Stone;
          break;
        case ObjectKind::Berries:
        case ObjectKind::Bush:
          item = Item::Berry;
          break;
        case ObjectKind::Mushroom:
          item = Item::Mushroom;
          break;
        default:
          break;
        }
        if (!inventory.add(item, qty)) {
          message("Your bag is full");
          return;
        }
        o.active = false;
        o.respawnAt = clock + 300;
        tile.object = -1;
        if (item == Item::Flower)
          inventory.add(Item::FlowerSeed);
        if (item == Item::Berry)
          inventory.add(Item::BerrySeed);
        changed();
        notify("Gathered %d %s", qty, ItemDefs[int(item)].name);
        sound = SoundEvent::Harvest;
        return;
      }
      message("A peaceful part of your little wilds");
      return;
    }
    message("Choose Hoe to make a garden patch");
    return;
  }
  if (actionTool == Tool::Water) {
    if (plantId < 0) {
      message("Tap a planted crop to water it");
      return;
    }
    auto &p = plants.entries[plantId];
    if (p.water < 60) {
      p.water = 60;
      changed();
      message("Watered. Growing twice as fast for a minute.");
      sound = SoundEvent::Harvest;
    } else
      message("Already watered");
    return;
  }
  if (objectId >= 0 || plantId >= 0) {
    message("This spot is already occupied");
    return;
  }
  if (actionTool == Tool::Hoe) {
    if (tile.terrain != Terrain::Grass && tile.terrain != Terrain::Dirt) {
      message("Use the hoe on clear grass or dirt");
      return;
    }
    tile.terrain = Terrain::Soil;
    changed();
    message("A fresh garden patch. Choose Seeds to plant.");
    sound = SoundEvent::Harvest;
    return;
  }
  if (tile.terrain != Terrain::Soil) {
    message("Cultivate the ground with your hoe first");
    return;
  }
  if (!inventory.counts[selectedSeed]) {
    message("No seeds of that kind. Gather a seed pouch.");
    return;
  }
  if (plants.plant(actionTile, selectedSeed) < 0) {
    message("Your garden is full for now");
    return;
  }
  inventory.take(Item(selectedSeed));
  changed();
  notify("Planted %s. Water helps it grow.", PlantDefs[selectedSeed].name);
  sound = SoundEvent::Harvest;
}
void Game::resources() {
  for (int i = 0; i < world.objectCount; i++) {
    auto &o = world.objects[i];
    if (!o.active && o.respawnAt > 0 && clock >= o.respawnAt && world.at(o.tile).object < 0 &&
        plants.at(o.tile) < 0 && world.at(o.tile).terrain == Terrain::Grass) {
      o.active = true;
      o.respawnAt = 0;
      world.tiles[index(o.tile)].object = i;
      changed();
    }
  }
}
void Game::update(float dt) {
  time += dt;
  noticeTimer = std::max(0.f, noticeTimer - dt);
  destinationTimer = std::max(0.f, destinationTimer - dt);
  if (resting)
    return;
  clock += dt;
  plants.update(dt);
  resourceTick += dt;
  if (resourceTick >= 5) {
    resourceTick = 0;
    resources();
  }
  bool moving = player.moving();
  auto before = player.settled;
  player.update(dt);
  if (player.settled != before)
    changed();
  if (player.hasPending && player.settled != before) {
    auto p = player.pending;
    player.hasPending = false;
    if (actionPending)
      act(p, actionTool);
    else
      route(p);
  } else if (moving && !player.moving()) {
    sound = SoundEvent::Arrive;
    if (actionPending)
      perform();
  }
  camera.update(player.position, dt);
}
void Game::gesture(const Gesture &g, TilePos picked) {
  if (g.kind == GestureKind::Drag) {
    if (panel == Panel::None && !resting && g.point.y < WorldBottom)
      camera.pan(g.delta.x, g.delta.y);
    return;
  }
  if (g.kind != GestureKind::Tap)
    return;
  ++taps;
  exitRequested = false;
  if (resting) {
    resting = false;
    panel = Panel::None;
    message("Welcome back to your garden");
    return;
  }
  if (panel != Panel::None) {
    if (panel == Panel::Seeds && g.point.x >= 32 && g.point.x < 448 && g.point.y >= 78 &&
        g.point.y < 222) {
      int col = int(g.point.x - 32) / 104, row = int(g.point.y - 78) / 72;
      selectedSeed = row * 4 + col;
      tool = Tool::Seeds;
      panel = Panel::None;
      notify("%s selected. Tap cultivated soil.", PlantDefs[selectedSeed].name);
      return;
    }
    if (panel == Panel::Home && g.point.x >= 46 && g.point.x < 434 && g.point.y >= 162 &&
        g.point.y < 204) {
      saveRequested = true;
      exitRequested = true;
      message("Saving your little wilds...");
      panel = Panel::None;
      return;
    }
    panel = Panel::None;
    guide = false;
    return;
  }
  if (g.point.y < 40) {
    if (g.point.x >= 395) {
      muted = !muted;
      changed();
      message(muted ? "Sounds off" : "Sounds on");
    } else if (g.point.x >= 290) {
      camera.center(player.position);
      camera.following = true;
    } else if (g.point.x >= 190) {
      saveRequested = true;
      message("Saving your garden...");
    } else {
      panel = Panel::Guide;
      guide = true;
    }
    return;
  }
  if (g.point.y >= WorldBottom) {
    int slot = std::clamp(int(g.point.x) / 96, 0, 4);
    if (slot == 4) {
      panel = Panel::Bag;
      return;
    }
    tool = Tool(slot);
    if (tool == Tool::Seeds)
      panel = Panel::Seeds;
    else
      message(tool == Tool::Hoe     ? "Hoe: tap clear grass to cultivate"
              : tool == Tool::Water ? "Water: tap a growing plant"
                                    : "Hand: gather, inspect and harvest");
    return;
  }
  auto p = inBounds(picked) ? picked : camera.tile(g.point);
  if (!inBounds(p))
    return;
  if (tool == Tool::Hand && world.at(p).object < 0 && plants.at(p) < 0)
    walkTo(p);
  else
    act(p, tool);
}
} // namespace wilds

#include "Game.h"
namespace wilds {
void Game::begin() {
  world.generate();
  camera.center(player.position);
  noticeTimer = 6;
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
    message("That spot is out of reach");
    sound = SoundEvent::Blocked;
  }
  return ok;
}
void Game::walkTo(TilePos p) {
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
void Game::update(float dt) {
  time += dt;
  noticeTimer = std::max(0.f, noticeTimer - dt);
  destinationTimer = std::max(0.f, destinationTimer - dt);
  bool moving = player.moving();
  auto before = player.settled;
  player.update(dt);
  if (player.hasPending && player.settled != before) {
    auto p = player.pending;
    player.hasPending = false;
    route(p);
  } else if (moving && !player.moving())
    sound = SoundEvent::Arrive;
  camera.update(player.position, dt);
}
void Game::gesture(const Gesture &g) {
  if (g.kind == GestureKind::Drag) {
    if (!guide && g.point.y < WorldBottom)
      camera.pan(g.delta.x, g.delta.y);
    return;
  }
  if (g.kind != GestureKind::Tap)
    return;
  ++taps;
  if (guide) {
    guide = false;
    return;
  }
  if (g.point.y < 34) {
    if (g.point.x >= 395) {
      muted = !muted;
      message(muted ? "Sounds off" : "Sounds on");
    } else
      guide = true;
    return;
  }
  if (g.point.y >= WorldBottom) {
    if (g.point.x < 170) {
      message("Tap ground to walk. Drag to explore.");
      camera.following = true;
    } else if (g.point.x < 320) {
      camera.following = true;
      camera.center(player.position);
      message("Back to your little wanderer");
    } else
      guide = true;
    sound = SoundEvent::Tap;
    return;
  }
  TilePos p = camera.tile(g.point);
  if (inBounds(p) && world.at(p).object >= 0) {
    auto k = world.objects[world.at(p).object].kind;
    const char *names[] = {
        "An old oak. A lovely patch of shade.",  "A pine reaching for the sky.",
        "A stone, warmed by the sun.",           "A berry bush. Farming comes next.",
        "Wildflowers brighten the meadow.",      "Your cottage. Welcome to Little Wilds.",
        "A fallen log. A place for tiny lives.", "Reeds beside the water."};
    message(names[int(k)]);
  }
  walkTo(p);
}
} // namespace wilds

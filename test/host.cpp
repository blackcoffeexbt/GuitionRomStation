#include "core/Game.h"
#include "render/Renderer.h"
#include <cassert>
#include <cstdio>
#include <queue>
#include <memory>
using namespace wilds;
static int shortest(const World &w, TilePos from, TilePos to) {
  std::array<int, TileCount> d;
  d.fill(-1);
  std::queue<TilePos> q;
  q.push(from);
  d[index(from)] = 0;
  while (!q.empty()) {
    auto p = q.front();
    q.pop();
    if (p == to)
      return d[index(p)];
    for (auto v : std::array<TilePos, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
      TilePos n{int16_t(p.x + v.x), int16_t(p.y + v.y)};
      if (w.walkable(n) && d[index(n)] < 0) {
        d[index(n)] = d[index(p)] + 1;
        q.push(n);
      }
    }
  }
  return -1;
}
int main(int argc, char **argv) {
  for (int y = 0; y < MapSize; y++)
    for (int x = 0; x < MapSize; x++) {
      Vec2 p{float(x), float(y)};
      auto projected = project(p);
      auto inverse = unproject(projected);
      assert(distance(p, inverse) < .0001f);
      assert((pick(projected) == TilePos{int16_t(x), int16_t(y)}));
      for (Vec2 offset : std::array<Vec2, 4>{{{20, 3}, {-20, -3}, {2, 10}, {-2, -10}}})
        assert((pick({projected.x + offset.x, projected.y + offset.y}) ==
                TilePos{int16_t(x), int16_t(y)}));
    }
  auto g = std::make_unique<Game>();
  g->begin();
  assert(g->world.walkable({34, 34}));
  assert(!g->world.walkable({31, 31}));
  assert(!g->world.walkable({32, 32}));
  Pathfinding pf;
  Path path;
  for (int n = 0; n < 75; n++) {
    auto h = hash(n + 784);
    TilePos goal{int16_t(h % 64), int16_t((h >> 8) % 64)};
    if (!g->world.walkable(goal))
      continue;
    int expected = shortest(g->world, {34, 34}, goal);
    bool ok = pf.find(g->world, {34, 34}, goal, path);
    assert(ok == (expected >= 0));
    if (!ok)
      continue;
    assert(path.count == expected);
    TilePos prev{34, 34};
    for (int i = 0; i < path.count; i++) {
      assert(g->world.walkable(path.points[i]));
      assert(std::abs(path.points[i].x - prev.x) + std::abs(path.points[i].y - prev.y) == 1);
      prev = path.points[i];
    }
    assert(prev == goal);
  }
  assert(!pf.find(g->world, {34, 34}, {-1, 4}, path));
  assert(!pf.find(g->world, {34, 34}, {31, 31}, path));
  assert(pf.adjacent(g->world, {34, 34}, {31, 31}, path));
  g->walkTo({38, 34});
  g->update(.1f);
  g->walkTo({32, 34});
  for (int i = 0; i < 1800; i++) {
    g->update(1.f / 60);
    auto p = g->player.position;
    assert(g->world.walkable(pick(project(p))));
  }
  assert((g->player.settled == TilePos{32, 34}));
  assert(!g->player.moving());
  Input input;
  assert(input.sample(true, {10, 10}, 0).kind == GestureKind::None);
  assert(input.sample(false, {}, 40).kind == GestureKind::Tap);
  input.sample(true, {10, 10}, 100);
  assert(input.sample(true, {25, 10}, 150).kind == GestureKind::Drag);
  assert(input.sample(false, {}, 200).kind == GestureKind::None);
  g->camera.center({34, 34});
  auto s = g->camera.screen({36, 33});
  assert((g->camera.tile(s) == TilePos{36, 33}));
  g->camera.pan(10000, -10000);
  auto center = unproject({240 - g->camera.offset.x, 180 - g->camera.offset.y});
  assert(center.x >= 4 && center.x <= 59 && center.y >= 4 && center.y <= 59);
  std::puts("PASS projection, tile picking, A* vs BFS, obstacle routing, movement retargeting, "
            "gestures, camera bounds");
  if (argc > 1) {
    g->begin();
    g->player = Player{};
    g->camera.center(g->player.position);
    std::array<uint16_t, Width * Height> pixels;
    Surface surface(pixels.data());
    Renderer renderer;
    for (Vec2 offscreen : std::array<Vec2, 6>{
             {{-1000, 100}, {1000, 100}, {100, -1000}, {100, 1000}, {0, 0}, {479, 319}}})
      surface.sprite(int(offscreen.x), int(offscreen.y), objectSprite(ObjectKind::Oak, 0));
    renderer.draw(surface, *g);
    FILE *f = fopen(argv[1], "wb");
    assert(f);
    fprintf(f, "P6\n%d %d\n255\n", Width, Height);
    for (auto c : pixels) {
      uint8_t rgb8[] = {uint8_t((c >> 11) * 255 / 31), uint8_t(((c >> 5) & 63) * 255 / 63),
                        uint8_t((c & 31) * 255 / 31)};
      fwrite(rgb8, 1, 3, f);
    }
    fclose(f);
    printf("Preview tiles=%d sprites=%d\n", renderer.renderedTiles, renderer.renderedSprites);
  }
}

#include "Renderer.h"
#include <cstdio>
#include <cstring>
namespace wilds {
namespace {
constexpr auto Ink = rgb(0x364f47), Cream = rgb(0xf6f1dc), Muted = rgb(0x87927b),
               Accent = rgb(0x527a62);
}
void Renderer::leaf(Surface &s, int x, int y, uint16_t c) {
  s.ellipse(x, y, 5, 8, c);
  s.line(x - 3, y + 6, x + 3, y - 5, Cream);
}
void Renderer::toolbar(Surface &s, const Game &g) {
  char text[100];
  s.rounded(8, 272, 464, 44, 10, rgb(0x607d63));
  s.rounded(8, 269, 464, 44, 10, Cream);
  const char *tools[] = {"Hand", "Seeds", "Hoe", "Water", "Bag"};
  for (int i = 0; i < 5; i++) {
    int x = i * 96 + 48;
    if (i < 4 && int(g.tool) == i)
      s.rounded(i * 96 + 13, 274, 72, 34, 7, rgb(0xdce3c7));
    if (i == 0) {
      s.ellipse(x - 13, 287, 5, 5, Accent);
      s.line(x - 13, 278, x - 13, 287, Accent);
      s.line(x - 17, 280, x - 17, 286, Accent);
    }
    if (i == 1)
      leaf(s, x - 16, 286, Accent);
    if (i == 2) {
      s.line(x - 20, 280, x - 10, 294, Accent);
      s.rect(x - 22, 278, 12, 4, Accent);
    }
    if (i == 3) {
      s.ellipse(x - 16, 287, 4, 6, rgb(0x7caaa9));
      s.line(x - 16, 278, x - 19, 285, rgb(0x7caaa9));
    }
    if (i == 4) {
      s.rounded(x - 22, 282, 12, 12, 3, rgb(0xb29b74));
      s.line(x - 19, 281, x - 19, 278, Accent);
      s.line(x - 19, 278, x - 13, 278, Accent);
      s.line(x - 13, 278, x - 13, 281, Accent);
    }
    s.text(x - 6, 280, tools[i], Ink);
  }
  s.rounded(10, 9, 173, 28, 8, Cream);
  leaf(s, 24, 23, Accent);
  s.text(36, 15, "LITTLE WILDS", Ink);
  s.rounded(189, 9, 94, 28, 8, Cream);
  const char *saveLabel = g.saveStatus;
  if (!strcmp(saveLabel, "Unsaved changes"))
    saveLabel = "Save *";
  else if (s.textWidth(saveLabel) > 78)
    saveLabel = "Save issue";
  s.text(198, 15, saveLabel, Ink);
  s.rounded(290, 9, 95, 28, 8, Cream);
  s.text(302, 15, "Find me", Ink);
  s.rounded(393, 9, 77, 28, 8, Cream);
  s.text(402, 15, g.muted ? "Quiet" : "Sound", Ink);
  if (g.noticeTimer > 0) {
    int w = std::min(462, s.textWidth(g.notice) + 16);
    s.rounded((Width - w) / 2, 242, w, 23, 6, Cream);
    s.text(std::max(12, (Width - s.textWidth(g.notice)) / 2), 246, g.notice, Ink);
  }
  if (g.panel != Panel::None) {
    s.rounded(23, 45, 434, 211, 12, rgb(0x647e64));
    s.rounded(23, 42, 434, 211, 12, Cream);
    if (g.panel == Panel::Seeds) {
      s.text(37, 49, "Choose seeds  /  tap outside to close", Ink);
      for (int i = 0; i < 8; i++) {
        int x = 32 + (i % 4) * 104, y = 78 + (i / 4) * 72;
        s.rounded(x + 2, y, 98, 64, 7, rgb(i == g.selectedSeed ? 0xdce3c7 : 0xebe7d1));
        s.sprite(x + 27, y + 36, resource0);
        s.ellipse(x + 27, y + 24, 4, 4, rgb(ItemDefs[i].color));
        snprintf(text, sizeof(text), "%u", g.inventory.counts[i]);
        s.text(x + 57, y + 13, text, Ink);
        s.text(x + 7, y + 43, PlantDefs[i].name, Ink);
      }
    } else if (g.panel == Panel::Bag) {
      s.text(37, 49, "Your gathering bag  /  tap outside to close", Ink);
      for (int i = 0; i < ItemCount; i++) {
        int x = 34 + (i % 4) * 105, y = 76 + (i / 4) * 34;
        s.ellipse(x + 4, y + 7, 4, 4, rgb(ItemDefs[i].color));
        snprintf(text, sizeof(text), "%u", g.inventory.counts[i]);
        s.text(x + 14, y, text, Ink);
        s.text(x, y + 14, ItemDefs[i].name, Muted);
      }
    } else if (g.panel == Panel::Home) {
      s.text(46, 61, "Home, sweet little home", Ink);
      s.text(46, 92, "Your garden saves automatically as you play.", Ink);
      s.text(46, 115, "Save & rest before switching off the board.", Ink);
      s.rounded(46, 162, 388, 42, 8, rgb(0xdce3c7));
      s.text(161, 175, "Save & rest", Ink);
      s.text(46, 222, "Tap outside to keep exploring", Muted);
    } else {
      s.text(46, 56, "A garden at your own pace", Ink);
      s.text(46, 85, "Hand: gather resources and harvest ripe plants.", Ink);
      s.text(46, 108, "Hoe: make soil. Seeds: choose and plant.", Ink);
      s.text(46, 131, "Water: help plants grow. They never wither.", Ink);
      s.text(46, 154, "Bag: see supplies. Drag to look around.", Ink);
      s.text(46, 177, "Tap Saved to save. Visit the cottage to rest.", Ink);
      s.text(46, 209, "Growth resumes safely when you return.", Muted);
      s.text(46, 230, "Tap anywhere to return", Accent);
    }
  }
  if (g.resting) {
    s.rounded(38, 94, 404, 109, 12, Cream);
    s.text(93, 114, "Your little wilds are safely saved", Ink);
    s.text(81, 144, "You can switch off, or tap to keep playing.", Ink);
    s.text(182, 173, "Rest peacefully", Accent);
  }
}
TilePos Renderer::pickInteraction(const Game &g, Vec2 point) const {
  TilePos chosen{-1, -1};
  float depth = -1;
  auto hit = [&](TilePos tile, const Sprite &sp, float order) {
    auto pos = g.camera.screen({float(tile.x), float(tile.y)});
    int x = int(point.x - std::lround(pos.x)) + sp.anchorX,
        y = int(point.y - std::lround(pos.y)) + sp.anchorY;
    if (order >= depth && x >= 0 && y >= 0 && x < sp.w && y < sp.h &&
        sp.pixels[y * sp.w + x] != 0xf81f) {
      chosen = tile;
      depth = order;
    }
  };
  for (int i = 0; i < g.world.objectCount; i++) {
    auto &o = g.world.objects[i];
    if (o.active)
      hit(o.tile, objectSprite(o.kind, o.variant), o.tile.x + o.tile.y);
  }
  for (int i = 0; i < Plants::Capacity; i++) {
    auto &p = g.plants.entries[i];
    if (p.active)
      hit(p.tile, cropSprite(p.type, g.plants.stage(i)), p.tile.x + p.tile.y + .01f);
  }
  return chosen;
}
void Renderer::draw(Surface &s, const Game &g) {
  renderedSprites = renderedTiles = 0;
  s.clear(rgb(0xa9bc8a));
  auto centre = unproject({Width * .5f - g.camera.offset.x, 150 - g.camera.offset.y});
  int minX = std::max(0, int(centre.x) - 14), maxX = std::min(MapSize - 1, int(centre.x) + 14);
  int minY = std::max(0, int(centre.y) - 14), maxY = std::min(MapSize - 1, int(centre.y) + 14);
  static constexpr uint16_t greens[] = {rgb(0xa8bc87), rgb(0xaabe89), rgb(0xa5ba84), rgb(0xadc18d)};
  for (int y = minY; y <= maxY; y++)
    for (int x = minX; x <= maxX; x++) {
      TilePos p{int16_t(x), int16_t(y)};
      auto a = g.camera.screen({float(x), float(y)});
      int sx = std::lround(a.x), sy = std::lround(a.y);
      if (sx < -33 || sx > Width + 33 || sy < -17 || sy > Height + 17)
        continue;
      const auto &t = g.world.at(p);
      uint16_t c = greens[t.variant % 4];
      switch (t.terrain) {
      case Terrain::Water:
        c = rgb(0x78aaa4);
        break;
      case Terrain::Path:
        c = rgb(0xd3c39b);
        break;
      case Terrain::Soil:
        c = rgb(0x9e8965);
        break;
      case Terrain::Dirt:
        c = rgb(0xc3b68c);
        break;
      default:
        break;
      }
      s.diamond(sx, sy, c);
      ++renderedTiles;
      auto h = hash(index(p) + 77);
      if (t.terrain == Terrain::Grass) {
        if (h % 3 == 0) {
          int px = sx + int(h % 23) - 11, py = sy + int((h >> 8) % 13) - 6;
          s.line(px, py, px - 2, py - 3, rgb(0x8ca675));
          s.line(px, py, px + 2, py - 4, rgb(0x91ab79));
        }
        if (h % 19 == 0) {
          s.pixel(sx + 9, sy - 3, rgb(0xe3dcb1));
          s.pixel(sx + 10, sy - 3, rgb(0xe3dcb1));
        }
      } else if (t.terrain == Terrain::Water) {
        int phase = int(g.time * 2);
        int px = sx + int(hash(h + phase) % 17) - 8;
        s.line(px - 6, sy, px + 5, sy, rgb(0xa5c9bb));
        s.line(sx - 4, sy + 7, sx + 3, sy + 7, rgb(0x8bb9ab));
      } else if (t.terrain == Terrain::Soil) {
        for (int j = -2; j <= 2; j++) {
          s.line(sx - 15 + j * 4, sy - 5 + j * 2, sx + 4 + j * 4, sy + 4 + j * 2, rgb(0x8c775b));
        }
      } else if (t.terrain == Terrain::Path) {
        s.pixel(sx - 7, sy + 2, rgb(0xbcac87));
        s.pixel(sx + 15, sy, rgb(0xe6d8b2));
      }
    }
  // Draw route markers on the ground, so they are occluded by sprites.
  if (g.player.moving())
    for (int i = g.player.next; i < g.player.path.count; i++) {
      auto p = g.player.path.points[i];
      auto a = g.camera.screen({float(p.x), float(p.y)});
      if (a.x > 0 && a.x < Width && a.y > 35 && a.y < WorldBottom)
        s.ellipse(int(a.x), int(a.y), 2, 1, rgb(0xf0e6bd));
    }
  if (g.destinationTimer > 0) {
    auto a = g.camera.screen({float(g.destination.x), float(g.destination.y)});
    s.diamondOutline(int(a.x), int(a.y), rgb(0xf5edc8));
  }
  int count = 0;
  for (int i = 0; i < g.world.objectCount; i++) {
    const auto &o = g.world.objects[i];
    if (!o.active)
      continue;
    const auto &sp = objectSprite(o.kind, o.variant);
    auto a = g.camera.screen({float(o.tile.x), float(o.tile.y)});
    int x = std::lround(a.x), y = std::lround(a.y);
    if (x + sp.w - sp.anchorX < 0 || x - sp.anchorX >= Width || y + 8 < 0 ||
        y - sp.anchorY >= Height)
      continue;
    drawList[count++] = {float(o.tile.x + o.tile.y), i, x, y};
  }
  for (int i = 0; i < Plants::Capacity; i++) {
    const auto &p = g.plants.entries[i];
    if (!p.active)
      continue;
    auto a = g.camera.screen({float(p.tile.x), float(p.tile.y)});
    if (a.x < -30 || a.x > Width + 30 || a.y < -10 || a.y > Height + 62)
      continue;
    drawList[count++] = {float(p.tile.x + p.tile.y) + .01f, World::MaxObjects + i,
                         int(std::lround(a.x)), int(std::lround(a.y))};
  }
  auto pp = g.camera.screen(g.player.position);
  drawList[count++] = {g.player.position.x + g.player.position.y, -1, int(std::lround(pp.x)),
                       int(std::lround(pp.y))};
  std::sort(drawList.begin(), drawList.begin() + count, [](const DrawItem &a, const DrawItem &b) {
    if (a.depth == b.depth)
      return a.id < b.id;
    return a.depth < b.depth;
  });
  for (int i = 0; i < count; i++) {
    auto a = drawList[i];
    if (a.id < 0)
      s.sprite(
          a.x, a.y,
          playerSprite(g.player.direction, g.player.moving() ? int(g.player.animation) % 4 : 0));
    else if (a.id >= World::MaxObjects) {
      int id = a.id - World::MaxObjects;
      const auto &p = g.plants.entries[id];
      s.sprite(a.x, a.y, cropSprite(p.type, g.plants.stage(id)));
      if (p.water > 0)
        s.ellipse(a.x + 12, a.y - 9, 2, 3, rgb(0x7caeb2));
      if (g.plants.mature(id)) {
        int rise = int(std::sin(g.time * 3) * 2);
        s.ellipse(a.x, a.y - 57 + rise, 3, 2, rgb(0xf5df9b));
      }
    } else {
      auto &o = g.world.objects[a.id];
      int sway = o.kind == ObjectKind::Flowers ? int(std::sin(g.time * 2 + o.tile.x) * 1.1f) : 0;
      s.sprite(a.x + sway, a.y, objectSprite(o.kind, o.variant));
    }
    ++renderedSprites;
  }
  // A few tiny floating motes: deterministic, no particles allocated per frame.
  for (int i = 0; i < 5; i++) {
    float t = g.time * .12f + i * 1.9f;
    int x = int(230 + std::sin(t) * 200), y = int(120 + std::cos(t * .7f + i) * 90);
    s.blend(x, y, Cream, 120);
  }
  toolbar(s, g);
}
} // namespace wilds

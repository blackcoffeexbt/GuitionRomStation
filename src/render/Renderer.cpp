#include "Renderer.h"
#include <cstdio>
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
  s.rounded(12, 273, 456, 42, 12, rgb(0x55745b));
  s.rounded(12, 270, 456, 42, 12, Cream);
  s.rounded(17, 275, 143, 32, 8, rgb(0xdce3c7));
  leaf(s, 34, 290, Accent);
  s.text(49, 283, "Wander", Ink);
  // Centre icon and a simple open field-guide icon.
  s.line(185, 290, 199, 282, Accent);
  s.line(199, 282, 211, 290, Accent);
  s.line(211, 290, 199, 298, Accent);
  s.line(199, 298, 185, 290, Accent);
  s.ellipse(199, 290, 2, 2, Accent);
  s.text(223, 283, "Find me", Ink);
  s.rect(336, 283, 12, 15, rgb(0xcbd6b9));
  s.rect(349, 283, 12, 15, rgb(0xe1d5ad));
  s.line(348, 282, 348, 300, Accent);
  s.text(374, 283, "Guide", Ink);
  s.line(173, 281, 173, 301, rgb(0xe1dfc9));
  s.line(321, 281, 321, 301, rgb(0xe1dfc9));
  s.rounded(12, 10, 170, 28, 9, Cream);
  leaf(s, 27, 24, Accent);
  s.text(40, 16, "LITTLE WILDS", Ink);
  s.rounded(395, 10, 73, 28, 9, Cream);
  s.ellipse(410, 23, 4, 4, rgb(0xc8ad6c));
  s.text(421, 16, g.muted ? "Quiet" : "Sound", Ink);
  if (g.noticeTimer > 0) {
    int w = std::min(452, s.textWidth(g.notice) + 24);
    s.rounded((Width - w) / 2, 242, w, 23, 7, Cream);
    s.text((Width - s.textWidth(g.notice)) / 2, 246, g.notice, Ink);
  }
  if (g.guide) {
    for (int y = 45; y < 264; y++)
      for (int x = 26; x < 454; x++)
        s.blend(x, y, Ink, 95);
    s.rounded(38, 51, 404, 203, 13, Cream);
    leaf(s, 65, 78, Accent);
    s.text(83, 66, "A little place to slow down", Ink, 1);
    s.text(60, 100, "Tap the ground to take a walk.", Ink);
    s.text(60, 122, "Tap a tree, stone or cottage to visit it.", Ink);
    s.text(60, 144, "Drag the meadow to look around.", Ink);
    s.text(60, 166, "Find me brings you back to your wanderer.", Ink);
    s.text(60, 196, "First playable meadow  /  Phase 1", Muted);
    s.text(60, 222, "Tap anywhere to return to the wilds", Accent);
  }
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
    const auto &sp = objectSprite(o.kind, o.variant);
    auto a = g.camera.screen({float(o.tile.x), float(o.tile.y)});
    int x = std::lround(a.x), y = std::lround(a.y);
    if (x + sp.w - sp.anchorX < 0 || x - sp.anchorX >= Width || y + 8 < 0 ||
        y - sp.anchorY >= Height)
      continue;
    drawList[count++] = {float(o.tile.x + o.tile.y), i, x, y};
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
    else {
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

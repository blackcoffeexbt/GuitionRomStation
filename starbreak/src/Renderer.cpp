#include "Renderer.h"
#include <cstdio>
#include "../assets/generated/Sprites.cpp"
namespace sb {
namespace {
constexpr auto White = color(0xeaffff), Cyan = color(0x66e7ff), Gold = color(0xffce66),
               Pink = color(0xff7bbf), Blue = color(0x285b9e);
}
void Renderer::button(Display &d, int y, const char *label, bool enabled) {
  d.rect(94, y, 292, 34, color(enabled ? 0x20416c : 0x102044));
  d.rect(94, y, 3, 34, enabled ? Cyan : Blue);
  d.text(112, y + 10, label, enabled ? White : Blue);
}
void Renderer::draw(Display &d, const Game &g) {
  d.clear(color(0x050919));
  for (int i = 0; i < 110; i++) {
    float speed = 5 + (i % 3) * 18;
    int x = (int(i * 137 - g.visualTime * speed) % 480 + 480) % 480, y = 34 + (i * 71) % 244;
    d.rect(x, y, i % 7 == 0 ? 2 : 1, 1, i % 3 == 0 ? color(0x749ac0) : color(0x284467));
  }
  if (g.screen == Screen::Playing || g.screen == Screen::Pause || g.screen == Screen::GameOver ||
      g.screen == Screen::Victory) {
    for (auto &p : g.pickups)
      if (p.active) {
        int x = int(p.pos.x), y = int(p.pos.y + std::sin(p.age * 4) * 3);
        d.rect(x - 12, y - 12, 24, 24, color(0x25416c));
        d.sprite(x, y, cell);
      }
    for (auto &e : g.enemies)
      if (e.active) {
        const Sprite *s = e.kind == EnemyKind::Scout     ? &scout
                          : e.kind == EnemyKind::Fighter ? &fighter
                          : e.kind == EnemyKind::Gunship ? &gunship
                                                         : &asteroid;
        d.sprite(int(e.pos.x), int(e.pos.y), *s);
      }
    if (g.boss.active) {
      d.sprite(int(g.boss.pos.x), int(g.boss.pos.y), boss);
      if (g.boss.phase == 2) {
        d.rect(int(g.boss.pos.x) - 7, int(g.boss.pos.y) - 7, 14, 14, Pink);
        d.rect(int(g.boss.pos.x) - 3, int(g.boss.pos.y) - 3, 6, 6, White);
      }
    }
    for (auto &b : g.shots)
      if (b.active) {
        d.rect(int(b.pos.x) - 5, int(b.pos.y) - 1, 10, 2, Cyan);
        d.rect(int(b.pos.x) + 3, int(b.pos.y), 3, 1, White);
      }
    for (auto &b : g.enemyShots)
      if (b.active) {
        int x = int(b.pos.x), y = int(b.pos.y), r = int(b.radius);
        d.rect(x - r - 1, y - r - 1, r * 2 + 3, r * 2 + 3, Blue);
        d.rect(x - r, y - r, r * 2 + 1, r * 2 + 1, Gold);
        d.rect(x - 1, y - 1, 3, 3, White);
      }
    for (auto &e : g.effects)
      if (e.active)
        d.rect(int(e.pos.x), int(e.pos.y), e.life > .2f ? 2 : 1, e.life > .2f ? 2 : 1,
               e.color == 0   ? Cyan
               : e.color == 1 ? Gold
                              : Pink);
    if (g.screen != Screen::GameOver && (g.invulnerable <= 0 || int(g.visualTime * 14) % 2 == 0)) {
      d.sprite(int(g.player.x), int(g.player.y), player);
      d.rect(int(g.player.x) - 18, int(g.player.y) - 1, 3 + int(g.visualTime * 30) % 3, 3, Gold);
    }
    if (g.bombFlash > 0) {
      int edge = int((.38f - g.bombFlash) * 650);
      d.rect(int(g.player.x) - edge, 44, 2, 232, Cyan);
      d.rect(int(g.player.x) + edge, 44, 2, 232, Cyan);
    }
    d.rect(0, 0, 480, 32, color(0x102044));
    char text[64];
    snprintf(text, sizeof(text), "%07u", g.score);
    d.text(10, 9, text, White);
    d.text(110, 9, "SHIELD", Cyan);
    for (int i = 0; i < 3; i++)
      d.rect(166 + i * 14, 12, 9, 9, i < g.shields ? Cyan : Blue);
    snprintf(text, sizeof(text), "BOMB %d   CANNON %d", g.bombs, g.power);
    d.text(231, 9, text, White);
    int section = g.stageTime < 42    ? 1
                  : g.stageTime < 105 ? 2
                  : g.stageTime < 165 ? 3
                  : g.stageTime < 215 ? 4
                  : g.stageTime < 265 ? 5
                                      : 6;
    snprintf(text, sizeof(text), "%02d", section);
    d.text(445, 9, text, Gold);
    d.rect(0, 278, 480, 42, color(0x050919));
    if (g.boss.active) {
      d.text(10, 286, g.boss.phase == 1 ? "DREADNAUGHT" : "EXPOSED CORE", Pink);
      d.rect(150, 291, 315, 7, Blue);
      d.rect(150, 291, int(315 * std::max(0, g.boss.hp) / 1500.f), 7,
             g.boss.phase == 1 ? Gold : Pink);
    } else if (g.bannerTime > 0)
      d.text(10, 287, g.banner, Gold);
    else
      d.text(10, 287, "A FIRE     B BOMB     MENU PAUSE", color(0x749ac0));
  }
  if (g.screen == Screen::Playing)
    return;
  if (g.screen == Screen::Title || g.screen == Screen::Connection) {
    d.text(55, 60, "STARBREAK", White, 3);
    d.text(58, 108, "ONE SHIP. ONE CHANCE TO BREAK THROUGH.", Cyan);
  } else {
    d.rect(52, 47, 376, 232, color(0x050919));
    d.text(89, 70,
           g.screen == Screen::Pause     ? "FLIGHT PAUSED"
           : g.screen == Screen::Victory ? "SECTOR CLEAR"
                                         : "SIGNAL LOST",
           g.screen == Screen::Victory ? Cyan : Pink, 2);
  }
  char line[80];
  if (g.screen == Screen::Title) {
    button(d, 149, g.connected ? "A  LAUNCH / TOUCH TO START" : "CONNECT CONTROLLER TO LAUNCH",
           g.connected);
    button(d, 201, g.connected ? "CONTROLLER CONNECTED / PAIR" : "PAIR XBOX CONTROLLER");
    snprintf(line, sizeof(line), "BEST %07u", g.best);
    d.text(174, 252, line, Gold);
  }
  if (g.screen == Screen::Connection) {
    d.text(52, 131,
           g.connected ? "CONNECTED - A TO RETURN" : "TURN ON XBOX. HOLD PAIR FOR 3 SECONDS.",
           White);
    button(d, 173, "PAIR / SEARCH AGAIN");
    button(d, 224, "B  BACK");
  }
  if (g.screen == Screen::Pause) {
    d.text(92, 119, g.connected ? "RELEASE A, THEN PRESS TO RESUME" : "CONTROLLER DISCONNECTED",
           g.connected ? White : Gold);
    button(d, 150, g.connected ? "A  RESUME FLIGHT" : "WAITING FOR YOUR CONTROLLER", g.connected);
    button(d, 207, "B  ABANDON RUN / TITLE");
  }
  if (g.screen == Screen::GameOver || g.screen == Screen::Victory) {
    snprintf(line, sizeof(line), "SCORE %07u    BEST %07u", g.score, g.best);
    d.text(82, 123, line, White);
    if (g.screen == Screen::Victory) {
      snprintf(line, sizeof(line), "SHIELD BONUS %d   /   %d:%02d", g.shields * 750,
               int(g.stageTime) / 60, int(g.stageTime) % 60);
      d.text(102, 152, line, Gold);
    } else
      d.text(115, 150, "THE NEXT RUN IS YOURS", Gold);
    button(d, 191, "A  RETRY / FLY AGAIN");
    button(d, 238, "B  RETURN TO TITLE");
  }
  d.text(16, 296, g.muted ? "SOUND OFF - TAP TO CHANGE" : "SOUND ON - TAP TO CHANGE", Cyan);
}
} // namespace sb

#pragma once
#include "Game.h"
namespace sb {
// Deterministic validation pilot. It uses normal controls, collisions and weapon rules.
// It has no immunity, score grants, clock acceleration or access to touch flight controls.
inline Input pilot(const Game &g, bool lose = false) {
  Input in;
  in.connected = true;
  in.fresh = true;
  if (g.screen != Screen::Playing) {
    in.fire = (int(g.visualTime * 4) % 2) == 0;
    return in;
  }
  in.fire = true;
  float targetX = 68, targetY = 160;
  if (lose) {
    in.fire = false;
    float best = 10000;
    for (auto &e : g.enemies)
      if (e.active) {
        float d = std::hypot(e.pos.x - g.player.x, e.pos.y - g.player.y);
        if (d < best) {
          best = d;
          targetX = e.pos.x;
          targetY = e.pos.y;
        }
      }
  } else {
    bool cellFound = false;
    for (auto &p : g.pickups)
      if (p.active) {
        targetX = 68;
        targetY = p.pos.y;
        cellFound = true;
        break;
      }
    if (!cellFound) {
      if (g.boss.active)
        targetY = g.boss.pos.y;
      else {
        float nearest = 999;
        for (auto &e : g.enemies)
          if (e.active && e.kind != EnemyKind::Asteroid && e.pos.x > g.player.x) {
            float d = e.pos.x - g.player.x;
            if (d < nearest) {
              nearest = d;
              targetY = e.pos.y;
            }
          }
      }
    }
    float bestCost = 1e20f, bestY = targetY;
    for (int candidate = 0; candidate < 22; candidate++) {
      float y = 58 + candidate * 9.5f, cost = std::abs(y - targetY) * .33f;
      // Score the real movement trajectory, not just a desired future lane.
      float vx = std::clamp((targetX - g.player.x) * 3.f, -165.f, 165.f),
            vy = std::clamp((y - g.player.y) * 3.f, -165.f, 165.f);
      for (auto &b : g.enemyShots)
        if (b.active) {
          for (float t = .1f; t < 1.25f; t += .15f) {
            float px = g.player.x + std::clamp(vx * t, std::min(0.f, targetX - g.player.x),
                                               std::max(0.f, targetX - g.player.x));
            float py = g.player.y + std::clamp(vy * t, std::min(0.f, y - g.player.y),
                                               std::max(0.f, y - g.player.y));
            float dx = std::abs(b.pos.x + b.velocity.x * t - px),
                  dy = std::abs(b.pos.y + b.velocity.y * t - py);
            if (dx < 36 && dy < 30)
              cost += (36 - dx) * (30 - dy) * (.8f + 1.2f - t);
          }
        }
      for (auto &e : g.enemies)
        if (e.active) {
          for (float t = .1f; t < 1.25f; t += .2f) {
            float speed = e.kind == EnemyKind::Scout     ? 62
                          : e.kind == EnemyKind::Fighter ? 76
                          : e.kind == EnemyKind::Gunship ? 39
                                                         : 69;
            float dx = std::abs(e.pos.x - speed * t - g.player.x),
                  py = g.player.y + std::clamp(vy * t, std::min(0.f, y - g.player.y),
                                               std::max(0.f, y - g.player.y));
            float dy = std::abs(e.pos.y - py);
            if (dx < 55 && dy < 35)
              cost += (55 - dx) * (35 - dy) * 2;
          }
        }
      cost += std::abs(y - g.player.y) * .12f;
      if (cost < bestCost) {
        bestCost = cost;
        bestY = y;
      }
    }
    targetY = bestY;
    if (g.bombs && g.shields == 1 && g.enemyShotCount() > 3)
      in.bomb = true;
  }
  float dx = targetX - g.player.x, dy = targetY - g.player.y;
  auto move = normalizeMove(dx * .08f, dy * .08f);
  in.x = move.x;
  in.y = move.y;
  return in;
}
} // namespace sb

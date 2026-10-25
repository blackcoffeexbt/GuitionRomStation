#pragma once
#include "Input.h"
namespace port {
// Native SMS button bits are deliberately independent of the emulator C headers.
inline uint8_t buttons(const sb::Input &in, uint32_t frame, bool blocked) {
  if (!in.connected || blocked)
    return 0;
  return (in.y < -.25f ? 1 : 0) | (in.y > .25f ? 2 : 0) | (in.x < -.25f ? 4 : 0) |
         (in.x > .25f ? 8 : 0) | (in.bomb ? 0x10 : 0) |
         ((in.fire || (in.nextWeapon && frame % 6 < 3)) ? 0x20 : 0);
}
struct Session {
  bool paused = true, connected = false, armed = false, blockA = true, resetRequested = false;
  bool lastA = false, lastMenu = false, lastY = false;
  void receive(const sb::Input &in) {
    bool a = in.fire && !lastA, menu = in.menu && !lastMenu, y = in.previousWeapon && !lastY;
    lastA = in.fire;
    lastMenu = in.menu;
    lastY = in.previousWeapon;
    if (in.connected != connected) {
      connected = in.connected;
      armed = false;
      if (!connected) {
        paused = true;
        blockA = true;
      }
    }
    if (!connected) {
      lastA = lastMenu = lastY = false;
      return;
    }
    if (in.fresh && !in.fire && !in.menu) {
      armed = true;
      blockA = false;
    }
    if (menu) {
      if (!paused) {
        paused = true;
        armed = false;
      } else if (armed)
        resume();
    } else if (a && paused && armed)
      resume();
    if (y && paused) {
      resetRequested = true;
    }
  }
  void resume() {
    if (connected && armed) {
      paused = false;
      blockA = true;
      armed = false;
    }
  }
  void pause() {
    paused = true;
    armed = false;
  }
};
} // namespace port

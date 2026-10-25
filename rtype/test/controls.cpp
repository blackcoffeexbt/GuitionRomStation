#include "Controls.h"
#include <cassert>
#include <cstdio>
int main() {
  port::Session s;
  sb::Input i;
  i.connected = i.fresh = true;
  s.receive(i);
  i.fire = true;
  s.receive(i);
  assert(!s.paused && s.blockA);
  assert(port::buttons(i, 0, s.blockA) == 0);
  i.fire = false;
  s.receive(i);
  assert(!s.blockA);
  i.fire = i.bomb = true;
  i.x = 1;
  i.y = -1;
  s.receive(i);
  assert(port::buttons(i, 0, s.blockA) == (1 | 8 | 16 | 32));
  i.connected = false;
  s.receive(i);
  assert(s.paused && port::buttons(i, 0, false) == 0);
  i.connected = true;
  s.receive(i);
  assert(s.paused);
  i.fire = false;
  s.receive(i);
  i.fire = true;
  s.receive(i);
  assert(!s.paused && s.blockA);
  i = {};
  i.connected = i.fresh = true;
  i.nextWeapon = true;
  assert(port::buttons(i, 0, false) == 32 && port::buttons(i, 3, false) == 0);
  s.receive(i);
  i.menu = true;
  s.receive(i);
  assert(s.paused);
  i.menu = false;
  s.receive(i);
  i.previousWeapon = true;
  s.receive(i);
  assert(s.resetRequested);
  puts("PASS SMS button mapping, hold/rapid fire, disconnect freeze, fresh confirmation and reset "
       "edge");
}

#pragma once
#include "../core/Game.h"
#include "../core/SettingsCodec.h"
#include <Preferences.h>
namespace sb {
class SettingsStore {
  Preferences prefs;
  int slot = -1;
  Settings current{};
  uint32_t nextRetry = 0;
  bool ready = false;

public:
  bool begin(Game &g);
  void update(Game &g);
#ifdef STARBREAK_VALIDATION
  bool corruptLatest();
#endif
};
} // namespace sb

#pragma once
#include "../Input.h"
#include <Bluepad32.h>
namespace sb {
class ControllerInput {
  static ControllerPtr pad;
  static void connect(ControllerPtr p);
  static void disconnect(ControllerPtr p);

public:
  Input input{};
  uint32_t packets = 0, maxPollGap = 0, lastPoll = 0, lastPacket = 0;
  void begin();
  void update();
  void pair(bool enabled);
};
} // namespace sb

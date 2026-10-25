#include "Controller.h"
#include <Arduino.h>
namespace sb {
ControllerPtr ControllerInput::pad = nullptr;
void ControllerInput::connect(ControllerPtr p) {
  if (pad || !p->isGamepad()) {
    p->disconnect();
    return;
  }
  pad = p;
  auto prop = p->getProperties();
  Serial.printf("PAD CONNECTED model=%s vid=%04x pid=%04x heap=%u\n", p->getModelName().c_str(),
                prop.vendor_id, prop.product_id, ESP.getFreeHeap());
}
void ControllerInput::disconnect(ControllerPtr p) {
  if (pad == p) {
    pad = nullptr;
    Serial.println("PAD DISCONNECTED; input cleared; resume confirmation required");
  }
}
void ControllerInput::begin() {
  BP32.setup(connect, disconnect);
  BP32.enableVirtualDevice(false);
  BP32.enableNewBluetoothConnections(true);
  Serial.printf("BLUEPAD32 %s: remembered keys retained\n", BP32.firmwareVersion());
}
void ControllerInput::pair(bool enabled) {
  BP32.enableNewBluetoothConnections(enabled);
}
void ControllerInput::update() {
  uint32_t now = micros();
  if (lastPoll)
    maxPollGap = std::max(maxPollGap, now - lastPoll);
  lastPoll = now;
  bool fresh = BP32.update();
  input = {};
  if (!pad || !pad->isConnected()) {
    lastPacket = 0;
    return;
  }
  input.connected = true;
  input.fresh = fresh && pad->hasData();
  if (fresh && pad->hasData()) {
    ++packets;
    lastPacket = millis();
  }
  auto direction = stick(pad->axisX(), pad->axisY());
  uint8_t d = pad->dpad();
  if (d) {
    direction = normalizeMove(float(bool(d & DPAD_RIGHT)) - float(bool(d & DPAD_LEFT)),
                              float(bool(d & DPAD_DOWN)) - float(bool(d & DPAD_UP)));
  }
  input.x = direction.x;
  input.y = direction.y;
  input.fire = pad->a();
  input.bomb = pad->b();
  input.nextWeapon = pad->x();
  input.previousWeapon = pad->y();
  input.menu = pad->miscStart();
  input.sampledAt = lastPacket;
}
} // namespace sb

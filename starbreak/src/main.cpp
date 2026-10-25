#include <Arduino.h>
#include <esp_heap_caps.h>
#include "hardware/Controller.h"
#include "hardware/Display.h"
#include "hardware/Touch.h"
#include "hardware/Audio.h"
using namespace sb;
Display display;
ControllerInput controller;
Touch touch;
Audio audio;
float shipX = 90, shipY = 160, clockTime = 0, accumulator = 0;
bool resumeArmed = false;
bool previousA = false, previousB = false, previousMenu = false, knownConnected = false,
     paused = true, stress = false, muted = false;
uint32_t lastUpdate = 0, lastFrame = 0, lastReport = 0, frames = 0, renderUs = 0, aPresses = 0,
         bPresses = 0, menuPresses = 0;
uint32_t minHeap = UINT32_MAX;
char command[48];
int commandLength = 0;
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("STARBREAK / HARDWARE GATE / Bluepad32 %s\n", BP32.firmwareVersion());
  controller.begin();
  Serial.printf("BT BUDGET heap=%u largest=%u psram=%u\n", ESP.getFreeHeap(),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                ESP.getFreePsram());
  Serial.printf("AUDIO ready=%d\n", audio.begin());
  if (!display.begin()) {
    Serial.println("FATAL display allocation/init");
    for (;;)
      delay(1000);
  }
  touch.begin();
  lastUpdate = lastReport = millis();
}
void loop() {
  controller.update();
  auto in = controller.input;
  uint32_t now = millis();
  if (in.connected != knownConnected) {
    knownConnected = in.connected;
    paused = true;
    resumeArmed = false;
    Serial.printf("CONNECTION connected=%d paused=1 pos=(%.2f,%.2f)\n", knownConnected, shipX,
                  shipY);
  }
  if (in.connected && in.fresh && !in.fire && !in.menu)
    resumeArmed = true;
  if (in.fire && !previousA) {
    ++aPresses;
    if (in.connected && paused && resumeArmed)
      paused = false;
    audio.play(SoundEvent::Tap, muted);
    Serial.printf("BUTTON A count=%u\n", aPresses);
  }
  if (in.bomb && !previousB) {
    ++bPresses;
    audio.play(SoundEvent::Blocked, muted);
    Serial.printf("BUTTON B count=%u\n", bPresses);
  }
  if (in.menu && !previousMenu) {
    ++menuPresses;
    paused = !paused;
    resumeArmed = false;
    Serial.printf("BUTTON MENU count=%u paused=%d\n", menuPresses, paused);
  }
  previousA = in.fire;
  previousB = in.bomb;
  previousMenu = in.menu;
  Vec2 point;
  static bool wasTouch = false;
  bool down = touch.read(point);
  if (down && !wasTouch) {
    if (point.y > 268 && point.x < 240) {
      controller.pair(true);
      paused = true;
    } else if (point.y > 268) {
      muted = !muted;
    }
    Serial.printf("TOUCH %.0f %.0f\n", point.x, point.y);
  }
  wasTouch = down;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n') {
      command[commandLength] = 0;
      if (!strcmp(command, "stress"))
        stress = !stress;
      else if (!strcmp(command, "pair")) {
        controller.pair(true);
        paused = true;
      } else if (!strcmp(command, "reboot"))
        ESP.restart();
      else if (!strcmp(command, "mute"))
        muted = !muted;
      commandLength = 0;
    } else if (commandLength < 47)
      command[commandLength++] = c;
  }
  float dt = std::min(.1f, float(now - lastUpdate) / 1000);
  lastUpdate = now;
  accumulator += dt;
  while (accumulator >= 1.f / 60) {
    accumulator -= 1.f / 60;
    clockTime += 1.f / 60;
    if (in.connected && !paused) {
      shipX = std::clamp(shipX + in.x * 170.f / 60, 25.f, 455.f);
      shipY = std::clamp(shipY + in.y * 170.f / 60, 72.f, 246.f);
    }
  }
  if (now - lastFrame < 33) {
    delay(1);
    return;
  }
  lastFrame = now;
  uint32_t start = micros();
  display.clear(color(0x050919));
  for (int i = 0; i < 100; i++) {
    int speed = 5 + (i % 3) * 12;
    int x = (int(i * 137 - clockTime * speed) % 480 + 480) % 480, y = 52 + (i * 71) % 204;
    display.rect(x, y, i % 3 == 0 ? 2 : 1, 1, color(i % 3 == 0 ? 0x73acbf : 0x243b64));
  }
  display.ship(int(shipX), int(shipY), in.fire && in.connected && !paused);
  if (stress) {
    for (int i = 0; i < 140; i++) {
      int x = (i * 37 + int(clockTime * 40)) % 480, y = 60 + (i * 29) % 190;
      display.rect(x, y, 6, 2, color(i % 2 ? 0xffcf5f : 0xff75bc));
    }
    for (int i = 0; i < 24; i++)
      display.ship(260 + (i % 6) * 35, 72 + (i / 6) * 43, false);
    if ((now / 120) % 2 == 0)
      audio.play(SoundEvent::Tap, muted);
  }
  auto white = color(0xecffff), cyan = color(0x66e7ff), yellow = color(0xffcf6f);
  display.text(12, 8, "STARBREAK", white, 2);
  display.text(300, 12, "CONTROLLER CHECK", cyan);
  display.rect(0, 43, 480, 1, color(0x275577));
  if (!in.connected) {
    display.rect(30, 95, 420, 122, color(0x0c1f38));
    display.text(48, 107, "PAIR YOUR XBOX CONTROLLER", cyan);
    display.text(48, 137, "Turn on Xbox, hold Pair for 3 seconds.", white);
    display.text(48, 160, "Waiting for Bluetooth LE controller...", white);
    display.text(48, 187, "Flight stays paused until connected + A.", yellow);
  } else if (paused) {
    display.rect(80, 104, 320, 88, color(0x0c1f38));
    display.text(102, 119, "CONTROLLER CONNECTED", cyan);
    display.text(102, 148, "Press A to confirm / resume", white);
    display.text(102, 169, "Move: stick or D-pad   Menu: pause", yellow);
  }
  char text[100];
  snprintf(text, sizeof(text), "A %u   B %u   MENU %u   X %.2f Y %.2f", aPresses, bPresses,
           menuPresses, in.x, in.y);
  display.text(12, 249, text, white);
  display.rect(12, 278, 222, 30, color(0x143858));
  display.text(35, 285, "Pair controller", cyan);
  display.rect(246, 278, 222, 30, color(0x143858));
  display.text(306, 285, muted ? "Sound off" : "Sound on", white);
  renderUs = micros() - start;
  display.present();
  ++frames;
  minHeap = std::min(minHeap, ESP.getFreeHeap());
  if (now - lastReport >= 3000) {
    float fps = frames * 1000.f / (now - lastReport);
    Serial.printf("PROOF fps=%.1f render_us=%u transfer_us=%u heap=%u minheap=%u psram=%u "
                  "connected=%d paused=%d packets=%u pollgap_us=%u pos=(%.2f,%.2f) "
                  "axis=(%.3f,%.3f) A=%u B=%u Menu=%u stress=%d\n",
                  fps, renderUs, display.transferUs, ESP.getFreeHeap(), minHeap, ESP.getFreePsram(),
                  in.connected, paused, controller.packets, controller.maxPollGap, shipX, shipY,
                  in.x, in.y, aPresses, bPresses, menuPresses, stress);
    lastReport = now;
    frames = 0;
    controller.maxPollGap = 0;
  }
}

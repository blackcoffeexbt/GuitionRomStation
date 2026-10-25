#include <Arduino.h>
#include <esp_heap_caps.h>
#include <new>
#include "core/Game.h"
#include "Renderer.h"
#include "hardware/Controller.h"
#include "hardware/Touch.h"
#include "hardware/Audio.h"
#include "hardware/SettingsStore.h"
#ifdef STARBREAK_VALIDATION
#include "core/Pilot.h"
#endif
using namespace sb;
Display display;
ControllerInput controller;
Touch touch;
Audio audio;
SettingsStore settings;
Renderer renderer;
Game game;
uint32_t lastUpdate = 0, lastFrame = 0, lastReport = 0, frames = 0, renderUs = 0, simulationUs = 0,
         minHeap = UINT32_MAX;
float accumulator = 0;
char command[64];
int commandLength = 0;
#ifdef STARBREAK_VALIDATION
bool autoplay = false, loseNext = false;
uint32_t resultAt = 0, soakStart = 0;
uint32_t soakTicks = 0;
int victories = 0, defeats = 0;
#endif
void status() {
  Serial.printf("STATE screen=%d t=%.2f pos=(%.2f,%.2f) shield=%d bomb=%d power=%d score=%u "
                "best=%u boss=%d/%d connected=%d runs=%u wins=%u losses=%u mute=%d ticks=%u\n",
                int(game.screen), game.stageTime, game.player.x, game.player.y, game.shields,
                game.bombs, game.power, game.score, game.best, game.boss.hp, game.boss.phase,
                game.connected, game.runs, game.wins, game.losses, game.muted, game.totalTicks);
}
void serialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n') {
      command[commandLength] = 0;
      if (!strcmp(command, "status"))
        status();
      else if (!strcmp(command, "reboot"))
        ESP.restart();
      else if (!strcmp(command, "pair")) {
        controller.pair(true);
        game.screen = Screen::Connection;
      } else if (!strcmp(command, "mute")) {
        game.muted = !game.muted;
        game.needsSave = true;
      }
#ifdef STARBREAK_VALIDATION
      else if (!strcmp(command, "autoplay")) {
        autoplay = !autoplay;
        game.screen = Screen::Title;
        soakStart = millis();
        soakTicks = 0;
        resultAt = 0;
        Serial.printf("AUTOPLAY %d - normal simulation, collisions and controls\n", autoplay);
      } else if (!strcmp(command, "defeat")) {
        autoplay = true;
        loseNext = true;
        game.screen = Screen::Title;
        resultAt = 0;
      } else if (!strcmp(command, "corrupt_settings")) {
        Serial.printf("CORRUPT shooter_namespace_only=%d\n", settings.corruptLatest());
      }
#endif
      commandLength = 0;
    } else if (commandLength < 63)
      command[commandLength++] = c;
  }
}
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.printf("STARBREAK / stage build / Bluepad32 %s\n", BP32.firmwareVersion());
  controller.begin();
  Serial.printf("BT BUDGET heap=%u largest=%u\n", ESP.getFreeHeap(),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  settings.begin(game);
  Serial.printf("AUDIO ready=%d\n", audio.begin());
  if (!display.begin()) {
    Serial.println("FATAL framebuffer/display");
    for (;;)
      delay(1000);
  }
  touch.begin();
  lastUpdate = lastReport = millis();
  status();
}
void loop() {
  controller.update();
  uint32_t now = millis();
  sb::Input in = controller.input;
#ifdef STARBREAK_VALIDATION
  if (autoplay && in.connected) {
    if (game.screen == Screen::GameOver || game.screen == Screen::Victory) {
      if (!resultAt) {
        resultAt = now;
        status();
        if (game.screen == Screen::Victory)
          ++victories;
        else
          ++defeats;
        Serial.printf("RUN_RESULT victory=%d wins=%d defeats=%d soak_ticks=%u elapsed_ms=%u\n",
                      game.screen == Screen::Victory, victories, defeats, soakTicks,
                      now - soakStart);
        loseNext = game.screen == Screen::Victory && (victories % 2 == 1);
      }
      if (now - resultAt > 2500) {
        game.screen = Screen::Title;
        resultAt = 0;
      }
    }
    if (game.screen == Screen::GameOver || game.screen == Screen::Victory) {
      in = {};
      in.connected = controller.input.connected;
      in.fresh = true;
    } else
      in = pilot(game, loseNext);
    in.connected = controller.input.connected;
  }
#endif
  game.receive(in);
  static bool wasTouch = false;
  static uint32_t lastTouchPoll = 0;
  if (now - lastTouchPoll >= 12) {
    lastTouchPoll = now;
    Vec2 p;
    bool down = touch.read(p);
    if (down && !wasTouch)
      game.tap(p);
    wasTouch = down;
  }
  serialCommands();
  if (game.pairRequested) {
    controller.pair(true);
    game.pairRequested = false;
  }
  float dt = std::min(.1f, float(now - lastUpdate) / 1000.f);
  lastUpdate = now;
  accumulator += dt;
  auto simulationStart = micros();
  while (accumulator >= 1.f / 60) {
    game.update(1.f / 60);
    accumulator -= 1.f / 60;
#ifdef STARBREAK_VALIDATION
    if (autoplay && game.screen == Screen::Playing)
      ++soakTicks;
#endif
  }
  simulationUs = micros() - simulationStart;
  if (game.sound != Cue::None) {
    audio.play(SoundEvent(int(game.sound)), game.muted);
    game.sound = Cue::None;
  }
  // Writes are limited to settings/results screens, never during active flight.
  if (game.screen != Screen::Playing)
    settings.update(game);
  if (now - lastFrame < 33) {
    delay(1);
    return;
  }
  lastFrame = now;
  auto start = micros();
  renderer.draw(display, game);
  renderUs = micros() - start;
  display.present();
  ++frames;
  minHeap = std::min(minHeap, ESP.getFreeHeap());
  if (now - lastReport >= 3000) {
    float fps = frames * 1000.f / (now - lastReport);
    Serial.printf("PERF fps=%.1f render_us=%u simulation_us=%u transfer_us=%u heap=%u minheap=%u "
                  "psram=%u pad=%d packets=%u pollgap_us=%u screen=%d stage=%.2f enemies=%d "
                  "shots=%d peaks=%d/%d shield=%d power=%d boss=%d/%d\n",
                  fps, renderUs, simulationUs, display.transferUs, ESP.getFreeHeap(), minHeap,
                  ESP.getFreePsram(), game.connected, controller.packets, controller.maxPollGap,
                  int(game.screen), game.stageTime, game.enemyCount(), game.enemyShotCount(),
                  game.peakEnemies, game.peakEnemyShots, game.shields, game.power, game.boss.hp,
                  game.boss.phase);
    frames = 0;
    lastReport = now;
    controller.maxPollGap = 0;
  }
}

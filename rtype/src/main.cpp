#include <Arduino.h>
#include <Preferences.h>
#include <esp_timer.h>
#include <esp_system.h>
#include "Emulator.h"
#include "Controls.h"
#include "Video.h"
#include "hardware/Controller.h"
#include "hardware/Touch.h"
#include "hardware/Audio.h"
sb::ControllerInput controller;
Touch touch;
Audio audio;
port::Video video;
port::Session session;
Preferences settings;
bool muted = false;
uint32_t emulated = 0, emuUs = 0, audioErrors = 0, benchFrames = 0;
void status() {
  Serial.printf(
      "SMS STATE paused=%d connected=%d frames=%u mute=%d heap=%u psram=%u audio_errors=%u\n",
      session.paused, session.connected, emulated, muted, ESP.getFreeHeap(), ESP.getFreePsram(),
      audioErrors);
}
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.printf("R-TYPE / Master System / hi631 SMSPlus reset_reason=%d\n",
                int(esp_reset_reason()));
  controller.begin();
  settings.begin("rtype-sms", false);
  muted = settings.getBool("mute", false);
  if (!audio.begin() || !video.begin()) {
    Serial.println("FATAL hardware init");
    while (true)
      delay(1000);
  }
  emu::setPixels(video.pixels());
  if (!emu::begin()) {
    Serial.println("FATAL SMS init");
    while (true)
      delay(1000);
  }
  touch.begin();
  status();
}
void loop() {
  static int64_t deadline = esp_timer_get_time();
  static uint32_t report = millis(), previousFrames = 0, previousPresented = 0, touchAt = 0;
  static bool wasTouch = false;
  static char command[32];
  static int commandLength = 0;
  controller.update();
  session.receive(controller.input);
  if (millis() - touchAt >= 20) {
    touchAt = millis();
    sb::Vec2 p;
    bool down = touch.read(p);
    if (down && !wasTouch) {
      if (!session.paused)
        session.pause();
      else if (p.y > 225) {
        muted = !muted;
        settings.putBool("mute", muted);
      } else if (p.y > 190)
        session.resetRequested = true;
      else
        session.resume();
    }
    wasTouch = down;
  }
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n') {
      command[commandLength] = 0;
      if (!strcmp(command, "bench") && session.paused) {
        emu::reset();
        benchFrames = 480;
        Serial.printf("BENCH begin frames=480 input=none actual_controller=%d\n",
                      session.connected);
      } else if (!strcmp(command, "status"))
        status();
      else if (!strcmp(command, "pair"))
        controller.pair(true);
      else if (!strcmp(command, "pause"))
        session.pause();
      else if (!strcmp(command, "resume"))
        session.resume();
      else if (!strcmp(command, "reset"))
        session.resetRequested = true;
      else if (!strcmp(command, "mute")) {
        muted = !muted;
        settings.putBool("mute", muted);
      }
      commandLength = 0;
    } else if (commandLength < 31)
      command[commandLength++] = c;
  }
  if (session.resetRequested) {
    emu::reset();
    session.resetRequested = false;
    session.paused = true;
  }
  video.menu.store((session.paused && !benchFrames ? 1u : 0u) | (session.connected ? 2u : 0u) |
                   (muted ? 4u : 0u));
  int64_t now = esp_timer_get_time();
  if (now >= deadline) {
    bool benchmark = benchFrames > 0;
    if (benchmark || (!session.paused && session.connected)) {
      bool draw = (emulated % 2) == 0;
      uint32_t start = micros();
      emu::run(draw, benchmark ? 0 : port::buttons(controller.input, emulated, session.blockA));
      emuUs = micros() - start;
      if (draw) {
        video.publish(emulated + 1);
        emu::setPixels(video.pixels());
      }
      ++emulated;
      if (benchmark && --benchFrames == 0) {
        session.pause();
        Serial.println("BENCH complete; emulator paused");
      }
    }
    if (!audio.submit(emu::audio(0), emu::audio(1), emu::samples(),
                      muted || (session.paused && !benchmark)))
      ++audioErrors;
    deadline += 16667;
    if (now - deadline > 66668)
      deadline = now + 16667;
  }
  if (millis() - report >= 3000) {
    uint32_t elapsed = millis() - report, shown = video.presented.load();
    Serial.printf("SMS PERF emu_fps=%.1f display_fps=%.1f emu_us=%u scale_us=%u transfer_us=%u "
                  "heap=%u psram=%u pad=%d packets=%u paused=%d audio_errors=%u frames=%u\n",
                  (emulated - previousFrames) * 1000.f / elapsed,
                  (shown - previousPresented) * 1000.f / elapsed, emuUs, video.scaleUs.load(),
                  video.transferUs.load(), ESP.getFreeHeap(), ESP.getFreePsram(), session.connected,
                  controller.packets, session.paused, audioErrors, emulated);
    previousFrames = emulated;
    previousPresented = shown;
    report = millis();
  }
  delay(1);
}

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
#include "hardware/RomStorage.h"
sb::ControllerInput controller;
sb::Input controls{};
#ifdef SMSEMU_VALIDATION
bool simulated = false;
uint16_t simulatedButtons = 0;
#endif
void sampleInput() {
  controller.update();
  controls = controller.input;
#ifdef SMSEMU_VALIDATION
  if (simulated) {
    controls = {};
    controls.connected = !(simulatedButtons & 512);
    controls.fresh = true;
    controls.x = float(bool(simulatedButtons & 8)) - float(bool(simulatedButtons & 4));
    controls.y = float(bool(simulatedButtons & 2)) - float(bool(simulatedButtons & 1));
    controls.fire = simulatedButtons & 16;
    controls.bomb = simulatedButtons & 32;
    controls.nextWeapon = simulatedButtons & 64;
    controls.menu = simulatedButtons & 128;
    controls.previousWeapon = simulatedButtons & 256;
    controls.sampledAt = millis();
  }
#endif
}

Touch touch;
Audio audio;
port::Video video;
port::Session session;
Preferences settings;
bool muted = false;
uint32_t emulated = 0, emuUs = 0, audioErrors = 0, benchFrames = 0;
uint32_t romFrames = 0, inputChanges = 0;
uint8_t appliedInputs = 0, lastAppliedInput = 0;

RomStorage storage;
port::LibraryModel libraryModel;
bool libraryOpen = true, timingChanged = false;
char currentGame[port::LibraryNameBytes] = "No game loaded";
uint8_t libraryButtons(const sb::Input &in) {
  if (!in.connected)
    return 0;
  return (in.y < -.25f ? port::LibraryModel::Up : 0) |
         (in.y > .25f ? port::LibraryModel::Down : 0) |
         (in.x < -.25f ? port::LibraryModel::Left : 0) |
         (in.x > .25f ? port::LibraryModel::Right : 0) |
         (in.fire ? port::LibraryModel::Accept : 0) | (in.bomb ? port::LibraryModel::Back : 0) |
         (in.nextWeapon ? port::LibraryModel::Refresh : 0);
}
void refreshLibrary(const char *message = nullptr, bool loading = false) {
  libraryModel.count = storage.count();
  if (libraryModel.selected >= libraryModel.count)
    libraryModel.selected = 0;
  port::LibraryView view;
  storage.view(view, libraryModel, emu::hasGame());
  if (message)
    snprintf(view.message, sizeof(view.message), "%s", message);
  view.loading = loading;
  video.library(view);
}
void openLibrary() {
  benchFrames = 0;
  session.pause();
  libraryOpen = true;
  libraryModel.open(libraryButtons(controls));
  refreshLibrary();
  video.menu.fetch_or(9u);
}
void loadProgress(size_t done, size_t total) {
  sampleInput();
  static uint32_t audioAt = 0;
  static int lastPercent = -1;
  if (millis() - audioAt >= 16) {
    audio.submit(emu::audio(0), emu::audio(1), emu::samples(), true);
    audioAt = millis();
  }
  if (!total)
    return;
  int percent = int(done * 100 / total);
  if (percent != lastPercent) {
    char message[72];
    snprintf(message, sizeof(message), "Loading %d%% - please wait", percent);
    refreshLibrary(message, true);
    lastPercent = percent;
  }
}
void scanLibrary() {
  refreshLibrary("Scanning SD card...", true);
  storage.scan(loadProgress);
  libraryModel.selected = 0;
  refreshLibrary();
  timingChanged = true;
}
void loadSelected() {
  if (!libraryModel.count)
    return;
  session.pause();
  refreshLibrary("Loading game...", true);
  size_t bytes = 0;
  uint32_t start = millis();
  auto *rom = storage.load(libraryModel.selected, bytes, loadProgress);
  bool bios = !strncasecmp(storage.entry(libraryModel.selected).name, "[BIOS]", 6);
  if (rom && emu::load(rom, bytes, bios)) {
    snprintf(currentGame, sizeof(currentGame), "%s", storage.entry(libraryModel.selected).name);
    libraryOpen = false;
    romFrames = inputChanges = 0;
    appliedInputs = lastAppliedInput = 0;
    session.paused = !controls.connected;
    session.blockA = controls.fire;
    session.lastA = controls.fire;
    session.armed = false;
    Serial.printf("ROM loaded name=%s bytes=%u load_ms=%u\n", currentGame, unsigned(bytes),
                  millis() - start);
  } else {
    if (rom)
      free(rom);
    refreshLibrary();
  }
  timingChanged = true;
}
void libraryAction(port::LibraryModel::Action action) {
  if (action == port::LibraryModel::Action::Load)
    loadSelected();
  else if (action == port::LibraryModel::Action::Scan)
    scanLibrary();
  else if (action == port::LibraryModel::Action::Close && emu::hasGame()) {
    libraryOpen = false;
    session.pause();
    session.blockA = true;
  }
}
void status() {
  Serial.printf(
      "SMS STATE paused=%d connected=%d frames=%u mute=%d heap=%u psram=%u audio_errors=%u\n",
      session.paused, session.connected, emulated, muted, ESP.getFreeHeap(), ESP.getFreePsram(),
      audioErrors + audio.writeErrors());
#ifdef SMSEMU_VALIDATION
  Serial.printf("CONTROL simulated=%d actual_pad=%d mask=%u rom_frames=%u applied=%u changes=%u\n",
                simulated, controller.input.connected, simulatedButtons, romFrames, appliedInputs,
                inputChanges);
#else
  Serial.printf("CONTROL simulated=0 actual_pad=%d rom_frames=%u applied=%u changes=%u\n",
                controller.input.connected, romFrames, appliedInputs, inputChanges);
#endif
  Serial.printf("LIBRARY open=%d count=%d game=%s message=%s\n", libraryOpen, storage.count(),
                currentGame, storage.message());
}
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.printf("Master System SD / hi631 SMSPlus reset_reason=%d\n", int(esp_reset_reason()));
  controller.begin();
  settings.begin("rtype-sms", false);
  muted = settings.getBool("mute", false);
  video.menu.store(9);
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
  refreshLibrary("Scanning SD card...", true);
  storage.begin();
  refreshLibrary();
  status();
}
void loop() {
  static int64_t deadline = esp_timer_get_time();
  static uint32_t report = millis(), previousFrames = 0, previousPresented = 0, touchAt = 0;
  static bool wasTouch = false, lastLibraryX = false;
  static char command[32];
  static int commandLength = 0;
  sampleInput();
  if (libraryOpen) {
    auto quiet = controls;
    quiet.fire = quiet.bomb = quiet.menu = quiet.nextWeapon = quiet.previousWeapon = false;
    session.receive(quiet);
    int before = libraryModel.selected;
    auto action = libraryModel.update(libraryButtons(controls), millis());
    if (before != libraryModel.selected)
      refreshLibrary();
    libraryAction(action);
  } else {
    session.receive(controls);
    if (session.paused && controls.nextWeapon && !lastLibraryX)
      openLibrary();
  }
  lastLibraryX = controls.nextWeapon;
  if (millis() - touchAt >= (session.paused ? 20u : 80u)) {
    touchAt = millis();
    sb::Vec2 p;
    bool down = touch.read(p);
    if (down && !wasTouch) {
      if (libraryOpen) {
        if (p.y >= 278) {
          if (p.x < 120) {
            libraryModel.move(-port::LibraryRows);
            refreshLibrary();
          } else if (p.x < 240) {
            libraryModel.move(port::LibraryRows);
            refreshLibrary();
          } else if (p.x < 360)
            scanLibrary();
          else
            libraryAction(port::LibraryModel::Action::Close);
        } else {
          int entry = libraryModel.touchRow(int(p.y));
          if (entry >= 0) {
            libraryModel.selected = entry;
            loadSelected();
          }
        }
      } else if (!session.paused)
        session.pause();
      else if (p.y > 225) {
        muted = !muted;
        settings.putBool("mute", muted);
      } else if (p.y > 190) {
        if (p.x < 240)
          session.resetRequested = true;
        else
          openLibrary();
      } else
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
      if (!strcmp(command, "bench") && session.paused && !libraryOpen && emu::hasGame()) {
        emu::reset();
        benchFrames = 480;
        Serial.printf("BENCH begin frames=480 input=none actual_controller=%d\n",
                      session.connected);
      }
#ifdef SMSEMU_VALIDATION
      else if (!strcmp(command, "sim off")) {
        simulated = false;
      } else if (!strncmp(command, "sim ", 4)) {
        char *end = nullptr;
        long mask = strtol(command + 4, &end, 10);
        if (end != command + 4 && !*end && mask >= 0 && mask < 1024) {
          simulated = true;
          simulatedButtons = uint16_t(mask);
        }
      }
#endif
      else if (!strcmp(command, "status"))
        status();
      else if (!strcmp(command, "pair"))
        controller.pair(true);
      else if (!strcmp(command, "pause"))
        session.pause();
      else if (!strcmp(command, "library"))
        openLibrary();
      else if (!strcmp(command, "rescan")) {
        openLibrary();
        scanLibrary();
      } else if (!strncmp(command, "load ", 5) && libraryOpen) {
        char *end = nullptr;
        long index = strtol(command + 5, &end, 10);
        if (end != command + 5 && !*end && index >= 0 && index < storage.count()) {
          libraryModel.selected = index;
          loadSelected();
        }
      } else if (!strcmp(command, "resume") && !libraryOpen)
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
    if (emu::hasGame())
      emu::reset();
    session.resetRequested = false;
    session.paused = true;
  }
  video.menu.store((session.paused && !benchFrames ? 1u : 0u) | (session.connected ? 2u : 0u) |
                   (muted ? 4u : 0u) | (libraryOpen ? 8u : 0u));
  int64_t now = esp_timer_get_time();
  if (timingChanged) {
    deadline = now;
    timingChanged = false;
  }
  if (now >= deadline) {
    bool benchmark = benchFrames > 0;
    if (benchmark || (!libraryOpen && emu::hasGame() && !session.paused && session.connected)) {
      // Re-poll after any touch/settings work, immediately before applying input.
      sampleInput();
      session.receive(controls);
      if (session.paused && !benchmark) {
        delay(1);
        return;
      }
      bool draw = true;
      uint32_t start = micros();
      uint8_t nativeInput = benchmark ? 0 : port::buttons(controls, emulated, session.blockA);
      if (nativeInput != lastAppliedInput)
        ++inputChanges;
      lastAppliedInput = nativeInput;
      appliedInputs |= nativeInput;
      ++romFrames;
      emu::run(draw, nativeInput);
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
                  "heap=%u psram=%u pad=%d packets=%u paused=%d audio_errors=%u frames=%u "
                  "pollgap_us=%u frame_age_us=%u\n",
                  (emulated - previousFrames) * 1000.f / elapsed,
                  (shown - previousPresented) * 1000.f / elapsed, emuUs, video.scaleUs.load(),
                  video.transferUs.load(), ESP.getFreeHeap(), ESP.getFreePsram(),
                  controller.input.connected, controller.packets, session.paused,
                  audioErrors + audio.writeErrors(), emulated, controller.maxPollGap,
                  video.frameAgeUs.load());
    controller.maxPollGap = 0;
    previousFrames = emulated;
    previousPresented = shown;
    report = millis();
  }
  delay(1);
}

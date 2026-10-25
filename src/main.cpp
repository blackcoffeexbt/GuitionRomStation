#include <Arduino.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <new>
#include "core/Game.h"
#include "render/Renderer.h"
#include "hardware/Display.h"
#include "hardware/Touch.h"
#include "hardware/Audio.h"
#include "hardware/SaveManager.h"
using namespace wilds;
Display display;
Touch touch;
Audio audio;
Input input;
SaveManager saves;
Game *game = nullptr;
Renderer *renderer = nullptr;
uint32_t lastFrame = 0, lastInput = 0, lastReport = 0, frames = 0, renderUs = 0;
float accumulator = 0;
bool demo = false;
int demoStep = 0;
uint32_t nextDemo = 0;
char command[80];
size_t commandLength = 0;
void dispatch(const Gesture &gesture) {
  TilePos picked{-1, -1};
  if (gesture.kind == GestureKind::Tap && game->panel == Panel::None && gesture.point.y >= 40 &&
      gesture.point.y < WorldBottom && (game->tool == Tool::Hand || game->tool == Tool::Water))
    picked = renderer->pickInteraction(*game, gesture.point);
  game->gesture(gesture, picked);
}
void status() {
  Serial.printf("STATE pos=(%.2f,%.2f) settled=(%d,%d) path=%d/%d taps=%u clock=%.2f plants=%d "
                "revision=%u save_seq=%u save=%s resting=%d seed=%d tool=%d\n",
                game->player.position.x, game->player.position.y, game->player.settled.x,
                game->player.settled.y, game->player.next, game->player.path.count, game->taps,
                game->clock, game->plants.count(), game->revision, saves.savedSequence(),
                game->saveStatus, game->resting, game->selectedSeed, int(game->tool));
  Serial.print("INVENTORY");
  for (int i = 0; i < ItemCount; i++)
    Serial.printf(" %d:%u", i, game->inventory.counts[i]);
  Serial.println();
  for (int i = 0; i < Plants::Capacity; i++) {
    const auto &p = game->plants.entries[i];
    if (p.active)
      Serial.printf("PLANT tile=(%d,%d) type=%d age=%.3f water=%.3f stage=%d\n", p.tile.x, p.tile.y,
                    p.type, p.age, p.water, game->plants.stage(i));
  }
}
void serialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n') {
      command[commandLength] = 0;
      int x, y;
#ifdef WILDS_SAVE_TESTING
      if (sscanf(command, "fault %d", &x) == 1) {
        saves.testFault(x);
        commandLength = 0;
        continue;
      }
#endif
      if (sscanf(command, "walk %d %d", &x, &y) == 2) {
        demo = false;
        game->walkTo({int16_t(x), int16_t(y)});
        status();
      } else if (sscanf(command, "tap %d %d", &x, &y) == 2) {
        dispatch({GestureKind::Tap, {float(x), float(y)}, {}});
        status();
      } else if (sscanf(command, "act %d %d", &x, &y) == 2) {
        demo = false;
        game->act({int16_t(x), int16_t(y)}, game->tool);
        status();
      } else if (sscanf(command, "tool %d", &x) == 1 && x >= 0 && x < 4) {
        game->tool = Tool(x);
        game->panel = Panel::None;
      } else if (sscanf(command, "seed %d", &x) == 1 && x >= 0 && x < SeedCount) {
        game->selectedSeed = x;
      } else if (!strcmp(command, "save")) {
        game->saveRequested = true;
      } else if (!strcmp(command, "rest")) {
        game->saveRequested = true;
        game->exitRequested = true;
      } else if (!strcmp(command, "farmstatus")) {
        status();
      } else if (!strcmp(command, "demo")) {
        demo = !demo;
        nextDemo = millis();
        Serial.printf("DEMO %d\n", demo);
      } else if (!strcmp(command, "status"))
        status();
      else if (!strcmp(command, "bench"))
        display.benchmark();
      else if (!strcmp(command, "reboot"))
        ESP.restart();
      else if (!strcmp(command, "mute")) {
        game->muted = !game->muted;
        game->markChanged();
        Serial.printf("MUTE %d\n", game->muted);
      } else
        Serial.println("COMMANDS: status | walk x y | tap x y | act x y | tool 0..3 | seed 0..7 | "
                       "save | rest | demo | bench | mute | reboot");
      commandLength = 0;
    } else if (commandLength < sizeof(command) - 1)
      command[commandLength++] = c;
  }
}
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.printf("\nLITTLE WILDS / Phase 2 / %s %s\n", __DATE__, __TIME__);
  Serial.printf("HARDWARE chip=%s rev=%d cpu=%u flash=%u psram=%u\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getFlashChipSize(),
                ESP.getPsramSize());
  auto *gm = heap_caps_malloc(sizeof(Game), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  auto *rm = heap_caps_malloc(sizeof(Renderer), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!gm || !rm || !display.begin()) {
    Serial.println("FATAL startup failed");
    for (;;)
      delay(1000);
  }
  game = new (gm) Game();
  renderer = new (rm) Renderer();
  game->begin();
  touch.begin();
  Serial.printf("AUDIO I2S ready=%d BCLK=42 LRCLK=2 DATA=41\n", audio.begin());
  saves.begin(*game);
  display.benchmark();
  Surface surface(display.framebuffer());
  renderer->draw(surface, *game);
  display.present();
  audio.play(SoundEvent::Arrive, false);
  lastFrame = lastReport = millis();
  status();
}
void loop() {
  uint32_t now = millis();
  serialCommands();
  if (now - lastInput >= 8) {
    lastInput = now;
    Vec2 point;
    bool down = touch.read(point);
    auto gesture = input.sample(down, point, now);
    if (gesture.kind != GestureKind::None) {
      if (gesture.kind == GestureKind::Tap) {
        demo = false;
        Serial.printf("GESTURE tap=(%.0f,%.0f)\n", gesture.point.x, gesture.point.y);
      }
      dispatch(gesture);
    }
  }
  if (demo && int32_t(now - nextDemo) >= 0) {
    constexpr TilePos tour[] = {{35, 34}, {36, 35}, {34, 37}, {32, 34},
                                {30, 34}, {33, 33}, {34, 34}};
    game->walkTo(tour[demoStep++ % 7]);
    nextDemo = now + 3500;
  }
  if (now - lastFrame < 33) {
    delay(1);
    return;
  }
  float dt = std::min(.15f, float(now - lastFrame) / 1000.f);
  lastFrame = now;
  accumulator += dt;
  while (accumulator >= 1.f / 60.f) {
    game->update(1.f / 60.f);
    accumulator -= 1.f / 60.f;
  }
  saves.update(*game, millis());
  audio.play(game->sound, game->muted);
  game->sound = SoundEvent::None;
  auto start = micros();
  Surface surface(display.framebuffer());
  renderer->draw(surface, *game);
  renderUs = micros() - start;
  display.present();
  ++frames;
  if (now - lastReport >= 3000) {
    float fps = frames * 1000.f / (now - lastReport);
    renderer->fps = fps;
    Serial.printf("PERF fps=%.1f render_us=%u rotate_us=%u transfer_us=%u heap=%u psram=%u "
                  "sprites=%d tiles=%d taps=%u\n",
                  fps, renderUs, display.rotationUs, display.transferUs, ESP.getFreeHeap(),
                  ESP.getFreePsram(), renderer->renderedSprites, renderer->renderedTiles,
                  game->taps);
    frames = 0;
    lastReport = now;
  }
}

# Starbreak

A compact, colourful side-scrolling shooter for the Guition JC3248W535 / C ESP32-S3 touchscreen. **Branch: `starbreak`**. Little Wilds stays recoverable on `main` and in the root project; farm saves and the SD card are untouched.

A replayable three-sector campaign of approximately five minutes: Outer Belt, Crystal Canyon and Iron Citadel. Seven enemy/hazard types include scouts, weaving fighters, gunships, asteroids, fast interceptors, drifting mines and station turrets. Every sector ends with its own two-phase boss: Warden, Hive Carrier and Dreadnaught.

Earn four pilot ranks through kills: rapid fire, an engine boost and ace cannon cadence. Two power cells upgrade the cannon independently; damage preserves upgrades. Three shields, one bomb per run and one shield repaired between sectors. Scrolling canyon walls and station structures are real collision hazards, with a broad central corridor. Original pixel sprites remain compact.

## Controls

- Stick or D-pad: move. Diagonal speed is normalised.
- Hold A: fire. A confirms menus and retries after a result.
- B: bomb during flight; back in menus.
- Menu: pause/resume.
- Touch: title/pairing/pause/results menus and mute only. No touch flight controls.

Turn the Xbox controller on, then hold Pair for about three seconds on the pairing screen. Pairing is remembered and reconnection is attempted after boot. A lost link immediately pauses flight and clears motion. Release A, then press it to confirm resuming after reconnection.

The HUD shows score, shields, bomb, cannon level sector, pilot rank and experience. The boss health bar appears at the bottom. Enemy bullets are gold/white; player fire is cyan. Upgrades persist through damage. Mute and best score survive reboot.

## Build and flash

```sh
python3 starbreak/scripts/bootstrap.py
pio run -d starbreak -e guition
pio run -d starbreak -e guition -t upload --upload-port /dev/cu.usbmodem101
pio device monitor -d starbreak --port /dev/cu.usbmodem101 --baud 115200
```

Pinned: Espressif32 6.10.0, official Arduino + Bluepad32 4.1.0 (Arduino 2.0.17), Arduino_GFX 1.5.9, C++17. Bootstrap downloads the official package, validates its SHA256 and prepares an isolated dependency directory. Little Wilds' existing framework is not replaced.

**Do not erase flash or upload a filesystem image.** Normal game uploads preserve the existing partition layout and the farm's LittleFS partition at `0xc10000`. Shooter score/settings use their own versioned `starbreak` NVS namespace; Bluetooth retains its pairing keys. The shooter does not use LittleFS or SD.

To return to Little Wilds, build/upload the root project, without `-d starbreak`. Its source remains on this branch and on `main`.

## Tests and validation

```sh
./starbreak/scripts/test_host.sh
```

Host checks cover radial stick/dead zone and diagonal speed, swept collision, damage cooldown, retained upgrades, bomb consumption, three bosses and sector transitions, terrain collisions, rank progression, reconnect confirmation, settings CRC/corruption, 32 repeated campaign/defeat loops, complete victory and defeat/retry simulations, and input timing at 25/30/40 Hz. Address/undefined-behaviour sanitizers can run against the portable core. A host simulation is not a substitute for the hardware tests.

The optional `validation` build adds a test pilot using ordinary controls, collisions and the real 60 Hz clock. It requires the actual controller link and has no invulnerability, artificial score or time acceleration. It alternates successful and deliberate-loss runs for endurance checks:

```sh
pio run -d starbreak -e validation -t upload --upload-port /dev/cu.usbmodem101
python3 starbreak/scripts/soak.py --minutes 32
```

The final delivery uses `guition`, with the pilot and corruption hooks excluded. See docs/VALIDATION.md for measured results and limits. **The expanded campaign has completed four physical-board victories and two defeats/retries, with almost 23 measured gameplay minutes at 29.7–30.3 FPS and stable free memory. The 30-minute endurance gate remains pending after Xbox sleep.** The validation boot is preserved for continuation; disconnected waiting does not count. USB normal-build commands: `status`, `pair`, `mute`, `reboot`. Every three seconds `PERF` reports frame times, memory, controller status and stage/pool counts. The validation build additionally accepts `autoplay`, `defeat` and `corrupt_settings`.

## Project layout

- `src/core/`: portable game, swept collision, input-independent settings codec and validation pilot.
- `src/hardware/`: display, Bluepad32 input, I²C touch, queued I²S audio and NVS settings.
- `src/Renderer.*`: native-layout sprite blits, parallax stars, HUD and menu screens.
- `assets/source/`: original transparent pixel-art PNGs; `assets/generated/`: compact indexed sprites.
- `scripts/`: pinned dependency bootstrap, asset generation and hardware/host checks.
- `test/`: meaningful simulation/input/collision/settings tests.

See [art and scaling](docs/ART.md), [architecture](docs/ARCHITECTURE.md), [delivery contract](docs/BRIEF.md) and [hardware evidence](docs/VALIDATION.md).

To render host-only campaign previews using the actual renderer and source font:

```sh
cd starbreak
c++ -std=c++17 -O2 -Itest/stubs -Isrc -Iassets -Iassets/generated src/core/Game.cpp src/Renderer.cpp test/preview.cpp -o .build/preview
.build/preview
```

The generated `.build/sector-*.ppm` images are host previews, separate from webcam evidence.

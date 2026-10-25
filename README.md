# Little Wilds

A peaceful miniature isometric meadow for the **Guition JC3248W535 / JC3248W535C**, ESP32-S3 N16R8, AXS15231B touchscreen. C++17, PlatformIO, Arduino, RGB565. Runs without Wi-Fi or internet.

## Current milestone: Phase 1

- 64 × 64 seeded world with a cottage, paths, garden beds, trees, rocks, flowers and stream.
- 480 × 320 landscape software renderer with viewport culling, transparent shaded sprites, ground-contact depth sorting and four-direction walking animation.
- Tap accessible ground to walk using A*. Tap an object to approach its accessible side and inspect it.
- Drag the meadow to pan. **Find me** recentres and follows the player. **Guide** opens instructions; tap outside to close.
- Built-in I²S speaker: quiet tap/arrival/unreachable tones; **Sound** toggles mute.
- Fixed 60 Hz simulation, capped 30 FPS rendering, no per-frame dynamic allocations in game code.
- PSRAM world state, fast internal drawing buffer, LittleFS partition and USB serial diagnostics.

**Farming, inventory, building and persistent game saves are Phase 2/3 work, not present in this prototype.** The world resets at boot. LittleFS mounts but no game-save file is written. The inserted SD card is not modified.

## Build and flash

Install PlatformIO Core, then run from this folder:

```sh
pio run
pio device list
pio run -t upload --upload-port /dev/cu.usbmodem101
pio device monitor --port /dev/cu.usbmodem101 --baud 115200
```

The port can change after reboot. On this Mac, PlatformIO is also available as `/Users/mark/.platformio/penv/bin/pio`. First build downloads pinned packages; later builds work offline. If flashing cannot connect, hold **BOOT**, press/release **RST**, then release **BOOT** and retry. No SD filesystem upload is needed. Do not use `uploadfs` once game saves are introduced without preserving those saves.

## Controls and diagnostics

USB serial commands end with a newline:

| Command | Action |
| --- | --- |
| `status` | Position, path, touch count, camera and world state |
| `walk 35 34` | Route to a tile for hardware movement verification |
| `tap 240 150` | Simulate a screen tap (does not prove physical touch) |
| `demo` | Toggle a gentle seven-stop walk around the clearing |
| `bench` | Compare full-frame and full-width strip display transfers |
| `mute` | Toggle audio |
| `reboot` | Restart the firmware |

For a scripted USB check (requires `pyserial`), use `python3 scripts/device_check.py --tour --seconds 24`. On macOS, `scripts/capture_device.sh` takes one authorised webcam snapshot.

Every three seconds `PERF` reports measured FPS, rendering/rotation/transfer times, free heap/PSRAM, visible tiles/sprites and tap count. Boot reports chip, flash, PSRAM, touch probe, filesystem mount and transfer benchmark. See [hardware](docs/HARDWARE.md) and [performance](docs/PERFORMANCE.md).

## Host checks and preview

```sh
./scripts/test_host.sh
```

Uses a host C++17 compiler; tests projection round trips, tile picking, A* shortest paths against independent BFS, obstacles, retargeting, gesture thresholds and camera bounds. Writes `.build/preview.ppm` using the **same renderer and assets** as the ESP32 firmware. No hardware emulation is claimed.

## Artwork

Original placeholder PNGs live in `assets/source/`, generated RGB565 arrays in `assets/generated/`. The firmware reads sprites directly from flash. Game state and the benchmark buffer live in PSRAM; the drawing buffer prefers internal RAM for speed.

```sh
python3 -m pip install -r scripts/requirements.txt
python3 scripts/make_placeholders.py  # optional: regenerate original placeholder PNGs
python3 scripts/convert_assets.py     # regenerate firmware asset arrays from PNGs
```

The placeholder generator uses the included DejaVu Sans font and licence. Existing PNGs and generated headers are checked in, so building needs neither Python nor a font installation. The converter accepts any RGBA PNG and reserves magenta RGB565 `0xf81f` for transparency. Alpha below 128 becomes transparent; solid magenta is remapped. Sprite anchors use `(width/2,height-8)` as ground contact. Original source art is separate from embedded arrays.

## Layout

- `include/BoardConfig.h`: hardware pins and orientation.
- `src/hardware/`: Display, Touch and Audio adapters.
- `src/core/`: World, Game, Player, Pathfinding, Camera and Input; no Arduino dependencies.
- `src/render/`: reusable Surface, Renderer and Assets.
- `assets/`: source PNGs and generated RGB565 data.
- `test/`: host simulation and rendering checks.
- `docs/`: hardware, measurements, save-format plan and milestone scope.

Next: implement data-driven inventory/plants and recoverable versioned world saves, without coupling simulation to the hardware renderer. See [save design](docs/SAVES.md).

# Little Wilds

A peaceful isometric farming game for the **Guition JC3248W535 / JC3248W535C**, ESP32-S3 N16R8, AXS15231B 480 × 320 touchscreen. C++17, PlatformIO, Arduino, custom RGB565 renderer. Runs entirely offline; no LVGL, Wi-Fi or internet dependency.

## Phase 2: a persistent little garden

Explore a 64 × 64 world, gather natural supplies, cultivate soil, plant eight species, water, grow and harvest. Your farm automatically saves to internal flash and survives reboot. The inserted SD card is not modified.

- **Hand:** tap resources to approach and gather; tap ripe plants to harvest. Tap the cottage to open Save & rest.
- **Seeds:** choose grass, wildflowers, carrots, pumpkins, berry bushes, small trees, apple trees or oaks, then tap cultivated soil.
- **Hoe:** tap clear grass or dirt to cultivate it.
- **Water:** tap a plant for a minute of faster growth. Plants never wither.
- **Bag:** see seeds, resources and produce. Tap outside a panel to close it.
- **Drag** to pan; **Find me** resumes following the wanderer.
- Tap the **save status** at the top for an immediate save, the title for instructions, or **Sound** to mute.

Seeds return on harvest. Perennials regrow; annuals leave the soil ready for another seed. Wild resources return after five minutes of simulation time, if their ground is still free grass. There is no combat or penalty.

Use the cottage's **Save & rest** before switching off: wait for the safe-to-switch-off message. Rest pauses simulation; tap to continue. Without an RTC, plants resume from their last saved game time after power-off; this build does not invent elapsed wall-clock time. Saves are debounced after changes, with a 30-second clock/growth checkpoint. Sudden power loss can lose progress since the last completed save.

Buildings, fences and editable settlement structures remain Phase 3 work.

## Build and flash

Install PlatformIO Core, then:

```sh
pio run -e guition
pio device list
pio run -e guition -t upload --upload-port /dev/cu.usbmodem101
pio device monitor --port /dev/cu.usbmodem101 --baud 115200
```

The port can change after reboot. On this Mac, PlatformIO is also at `/Users/mark/.platformio/penv/bin/pio`. First build downloads pinned packages; subsequent builds can run offline. If flashing cannot connect, hold BOOT, press/release RST, release BOOT and retry.

**Normal firmware uploads retain saves.** Do not erase flash, upload a filesystem image (`uploadfs`), or change the partition layout if you want to retain the farm. A preformatted LittleFS partition from Phase 1 is present on the development board. A fresh unformatted device must have LittleFS provisioned deliberately: **only for an empty/new board**, run `pio run -e guition -t uploadfs` once using the supplied `data/` image, then restart. This overwrites the filesystem and must not be used to update a farm you want to keep. The game refuses to auto-format a filesystem that fails to mount.

## Validation and diagnostics

```sh
./scripts/test_host.sh
python3 scripts/device_check.py --tour --seconds 24
./scripts/capture_device.sh
```

Host checks use a C++17 compiler. The test suite covers projection/picking, A* vs independent BFS, movement retargeting, gestures, clipping, the farming loop, all plant species, growth/watering, regrowth, inventory overflow, save round trips, checksums, truncation, invalid save data and trusted/untrusted offline time. The shared renderer writes `.build/preview.ppm` for host previews.

USB serial commands (newline terminated):

| Command | Action |
| --- | --- |
| `status` / `farmstatus` | Position, clock, inventory, plants and save sequence/status |
| `walk x y` | Walk to a world tile |
| `tap x y` | Simulate a screen tap through UI and sprite hit testing |
| `tool 0..3` | Hand / Seeds / Hoe / Water |
| `seed 0..7` | Select a plant species |
| `act x y` | Approach and use current tool on a world tile |
| `save` | Request a complete save |
| `rest` | Save and pause at a safe tile centre |
| `demo` | Toggle the diagnostic walking tour |
| `bench` | Benchmark full-frame vs full-width strip transfers |
| `mute` / `reboot` | Toggle audio / restart firmware |

Serial farming commands exercise the same simulation as touchscreen actions, but do not by themselves prove physical touchscreen response. `scripts/phase2_device_check.py` is a **one-time new-farm** check using starter resources; do not rerun it on a modified farm expecting identical results. It does not accelerate time.

An optional `guition_validation` build includes guarded save fault injection, documented in [the save format](SAVES.md). The final normal build excludes those commands. Actual hardware evidence and performance are in [PERFORMANCE.md](PERFORMANCE.md).

## Architecture and assets

- `include/BoardConfig.h`: verified board pins and landscape mapping.
- `src/hardware/`: Display, Touch, Audio and asynchronous LittleFS SaveManager.
- `src/core/`: Game, World, Player, Pathfinding, Camera, Input, Inventory, Plants and portable SaveCodec.
- `src/render/`: framebuffer Surface, Renderer, sprite picking and Assets.
- `assets/source/`: original transparent PNG placeholders; `assets/generated/`: RGB565 arrays and glyph atlas.
- `test/`: host movement, farming and save tests.
- `docs/`: hardware, save format, phases and measurements.

Game state, save snapshots and buffers live in PSRAM. A 307,200-byte drawing buffer prefers internal SRAM for speed and has a slower PSRAM fallback. Sprites stay in flash. Save encoding captures a consistent snapshot on the main thread; a worker writes and verifies it without touching live game state.

To regenerate artwork:

```sh
python3 -m pip install -r scripts/requirements.txt
python3 scripts/make_placeholders.py
python3 scripts/convert_assets.py
```

Generated assets and source PNGs are checked in, so firmware builds require neither Python nor fonts. DejaVu Sans and its licence are bundled. PNG alpha below 128 becomes transparent RGB565 colour key `0xf81f`; opaque collisions are remapped. Anchors share a ground-contact point at `(width/2,height-8)`.

See [Phase 2 controls](PHASE2.md), [hardware configuration](HARDWARE.md), [save format](SAVES.md) and [next milestones](MILESTONES.md).

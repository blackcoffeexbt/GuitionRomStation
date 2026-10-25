# Phase 1 validation — 25 October 2026

## Measured on the user's board

The final firmware was built and flashed by USB, run on the physical Guition board, inspected using the MacBook FaceTime HD camera, and exercised using both physical touch and serial-directed routes. The user confirmed touch and sound both work. These are actual hardware checks, not emulator-only results.

| Check | Result |
| --- | --- |
| Chip / flash / PSRAM | ESP32-S3, 16,777,216-byte flash, 8,386,295-byte reported PSRAM |
| LCD | AXS15231B QSPI at 40 MHz; correct 480 × 320 landscape orientation and colours in webcam captures |
| Touch | I²C 0x3b detected; physical screen taps logged, user confirmed correct response and drag operation |
| Speaker | I²S driver initialized, quiet game tones confirmed by user |
| LittleFS | Mounted, 4,063,232 usable bytes reported, no world-save writes in Phase 1 |
| Walking tour | Reached (29,32), (34,34), (37,35), then returned home (34,34) |
| Depth ordering | Webcam inspection showed tree foliage occluding the character while walking behind it |
| Final tour FPS | **22.0–23.5 FPS** in eight three-second samples, meeting the initial 20 FPS floor in the tested area |
| Rendering time | 12.5–15.4 ms per frame in the final tour samples |
| Byte-order conversion | ~2.9 ms per frame |
| Scanline gather + LCD transfer | ~26.0 ms per frame |
| Free internal heap | 38,824 bytes, stable across the final tour |
| Free PSRAM | 7,993,551 bytes, stable across the final tour |
| Visible objects / tiles | 16–23 sprites, 187–196 tiles in final tour samples |

The short tour is an initial benchmark, not a long soak or an exhaustive worst-case test of every map tile. Sustained 30 FPS is not achieved yet. The inserted SD card was left untouched. Board-family pins and capacities are verified by successful bring-up, but the exact C/EN rear-label suffix has not been independently inspected.

## Buffer strategy and experiments

An initial pair of PSRAM framebuffers with full-frame software transpose ran at ~10.5 FPS: rendering ~36 ms, transpose ~31 ms, display ~27 ms. Aligned fills and tile-copy transpose alone did not solve the bottleneck.

The final logical drawing buffer allocates 307,200 bytes in internal RAM, with an explicit slower PSRAM fallback if allocation fails. World/pathfinding state and a benchmark buffer allocate in PSRAM; sprites stay in flash. The full drawing frame converts to big-endian once, then Arduino_GFX gathers rotated scanlines into internal DMA buffers while sending the previous scanline. The display uses one full native portrait address window, avoiding unstable narrow-window updates. The renderer clears every pixel on the next frame, eliminating a redundant endian conversion back.

Five-frame native transfer comparisons measured ~25.4–27.0 ms for full-frame writes and ~25.8–27.3 ms for full-width 32-row strips. Strip refreshes did not justify their extra complexity for a camera that scrolls. `bench` reruns this check and briefly blanks the display; gameplay redraw follows.

The firmware caps rendering at 30 FPS and updates simulation using a fixed 60 Hz step. Touch polling and serial handling run between frames. No gameplay heap allocations occur inside the main update/render loop. The audio task uses its own DMA queue on core 0. No Wi-Fi or Bluetooth starts.

## Build and host checks

Final pinned toolchain: PlatformIO Espressif32 6.10.0 / Arduino ESP32 2.0.17 / Arduino_GFX 1.5.9, C++17 and `-O2`.

- Final firmware build and upload succeeded, flash data hashes verified.
- Application: 686,789 bytes of the 6 MiB slot (10.9%).
- Static internal RAM: 19,708 bytes (the runtime drawing buffer is additional).
- Host tests pass for projection and tile picking, A* shortest-path lengths compared against an independent BFS, blocked/adjacent destinations, movement retargeting, gestures and camera limits.
- AddressSanitizer and UndefinedBehaviorSanitizer host run passes, including edge/offscreen sprite clipping.
- The same renderer generates the host preview; it is not a screenshot of the physical device.

Local evidence: `device-review/boot.log`, `device-review/final-validation.log`, `device-review/behind-tree.jpg`, and `device-review/little-wilds-phase1.jpg`. Camera images and runtime logs are ignored by Git. Reproduce with `scripts/test_host.sh`, `scripts/device_check.py --tour --seconds 24`, and `scripts/capture_device.sh`.

## Limits for the next phase

Only ~38 KB internal heap remains with the fast full-screen buffer, so future subsystem allocations need care. Retain a PSRAM fallback, avoid network stacks unless explicitly needed, and measure filesystem write latency before introducing saving. Asset and terrain caches, transfer pipelining or more selective refreshes are candidates for 30 FPS. World saves and growth are not yet implemented; no save/load duration can be reported.

# Phase 2 hardware validation — 25 October 2026

The Phase 2 firmware was built, flashed and tested on the same physical board. Webcam images checked the farming toolbar, planted/watered crops, mature plants, inventory and all eight seed choices. Game actions and UI taps in these tests were driven through USB diagnostics using the same simulation/gesture dispatch; no claim is made that automated tests physically touched the glass. The original touchscreen driver/pin mapping remains unchanged.

## Gameplay and persistence checks

- Gathered starter wild seeds, wood, stone, berries, mushrooms and flowers by walking to their adjacent tiles. Inventory gains matched the gathered objects.
- Cultivated grass at (35,33), planted carrots, watered them and watched them progress through growth stages at normal simulation speed. No test clock acceleration was added.
- Saved and restarted the firmware with that growing crop: soil, inventory, crop age, remaining water, depleted resource state and player tile loaded successfully.
- Harvested ripe carrots through the screen-tap/sprite-picking dispatch: three carrots and two returned seeds; the annual crop was removed and soil retained.
- Planted/watered carrots, pumpkins and wildflowers in three existing garden beds, then verified all three and their inventory costs across another normal-firmware reboot.
- Harvested the perennial wildflowers on-device: the plant remained active at its regrowth stage, flowers increased from two to four, and flower seeds from three to five. Reboot preserved those rewards and all three plants.
- The cottage's visible Save & rest button completed a save and paused simulation. Two successive status reads showed an unchanged clock. Reboot loaded the same inventory and garden.
- Host checks cover all eight plant species, regrowth versus replanting, inventory overflow, queued seed selection, rest cancellation, serialization, standard CRC32 check vector and corrupted/invalid save input. AddressSanitizer/UndefinedBehaviorSanitizer runs pass.

## Save recovery checks on the device

A temporary `guition_validation` build preserved a verified backup before corrupting the newest completed save. Boot rejected corrupted slot B sequence 16 and loaded valid slot A sequence 15 with the recovery notice; inventory was unchanged. Restoring the untouched backup returned both slots to valid state and selected sequence 16 again. A half-length `/wilds.tmp` file followed by restart was safely ignored, preserving both completed slots and inventory. This simulates interrupted storage states; power was not physically pulled during a write.

The final flashed `guition` build excludes fault-injection commands; its ELF has no `SaveManager::testFault` symbol. Normal firmware uploads preserved the actual farm. The SD card was neither mounted nor modified, and Wi-Fi remained off.

## Measurements and limitations

- Normal garden samples were generally **20–22 FPS**, depending on camera position and visible sprites. Three-species movement/planting/autosave samples included **19.0–21.9 FPS**; fuller cottage/rest overlays reached **18.9–20.6 FPS**. The game remains responsive, but 30 FPS is not achieved and the 20 FPS floor is not universal during overlays/save activity.
- Background write plus readback: approximately **0.19–0.35 seconds** in observed samples. The simulation/main loop does not wait for those transfers.
- Consistent snapshot encoding after the CRC table optimisation: **4.4 ms** in the explicit-rest sample.
- Loading/validating two save slots: approximately **70–111 ms** observed across normal and recovery boots.
- Save sizes: **24,519 bytes** without crops, **24,558 bytes** with three crops.
- Free internal heap: **33,900–34,100 bytes** in observed Phase 2 runs; free PSRAM **7,870,599 bytes**, stable through the tests.
- The added SRAM pressure remains a constraint for future features. Existing saves were retained through the checksum optimisation, confirming wire-format compatibility.

Local evidence (ignored by Git): `device-review/phase2-farming.log`, `phase2-harvest.log`, `phase2-recovery.log`, `phase2-garden.log`, `phase2-rest.log`, `phase2-final.log`, plus `phase2-first.jpg`, `phase2-ripe.jpg`, `phase2-garden.jpg`, `phase2-bag.jpg`, `phase2-seeds.jpg` and `little-wilds-phase2.jpg`. These are actual camera captures; host previews remain distinct.

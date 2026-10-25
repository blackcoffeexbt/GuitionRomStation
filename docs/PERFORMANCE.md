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

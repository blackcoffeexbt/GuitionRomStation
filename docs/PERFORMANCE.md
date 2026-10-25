# Phase 1 validation

## Host

`./scripts/test_host.sh` passes projection/tile picking, A* checked against independent breadth-first shortest paths, blocked destinations, adjacent object routes, movement retargeting, tap-vs-drag thresholds and clamped camera centres. The host preview renders 187 visible tiles and 23 sprites near spawn.

## Firmware build

Pinned PlatformIO Espressif32 6.10.0 / Arduino ESP32 2.0.17 / Arduino_GFX 1.5.9 builds for ESP32-S3. Initial firmware: ~689 KB of the 6 MiB application partition; ~19.7 KB statically allocated internal RAM. Game/pathfinding state and both 307,200-byte framebuffers allocate explicitly in PSRAM. I²S DMA and audio task stack use internal RAM.

Hardware results will be recorded after serial and webcam checks. Build success alone does not establish physical display, touch, sound or frame-rate performance.

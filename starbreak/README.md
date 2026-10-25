# Starbreak

A separate ESP32-S3 side-scrolling shooter project on branch `starbreak`. Little Wilds source and its flash partition layout are retained; `main` remains at the working Phase 2 farm. The SD card is untouched.

## Current milestone: hardware gate

This build is a **controller/display/audio diagnostic**, not the authored shooter stage. It has a scrolling starfield, a movable pixel ship, Xbox controls, touch pairing/mute, reconnect confirmation and a synthetic stress scene. The stage must wait until the actual Xbox controller and combined hardware pass the gate.

- Xbox left stick or D-pad: move; diagonals have equal maximum speed.
- A: confirm/resume; held A is shown by the ship's firing indicator.
- B: button counter and test tone. No bomb gameplay exists yet.
- Menu: pause; a lost connection clears input and pauses flight.
- Reconnect requires releasing A then pressing it to confirm; no touchscreen flight control.
- Touch Pair controller enables Bluetooth discovery. Pair keys are never erased at boot.

## Pinned initial toolchain

- PlatformIO Espressif32 6.10.0.
- Official Arduino + Bluepad32 **4.1.0** release, downloaded by `scripts/bootstrap.py` and SHA256 verified.
- Arduino_GFX 1.5.9, the same AXS15231B QSPI driver and pins as Little Wilds.
- Per-project dependency directory; the Little Wilds framework installation is not replaced.

```sh
python3 starbreak/scripts/bootstrap.py
pio run -d starbreak
pio run -d starbreak -t upload --upload-port /dev/cu.usbmodem101
pio device monitor -d starbreak --port /dev/cu.usbmodem101 --baud 115200
```

The bootstrap reuses installed compiler tools when available; PlatformIO fetches missing tools. Never upload a filesystem image or erase flash: the copied 16 MB partition table deliberately preserves the farm's LittleFS partition at `0xc10000`. This proof does not mount or write that partition. Bluepad32 stores its own pairing keys in NVS. The eventual shooter score/settings will use a separate versioned `starbreak` namespace.

USB proof commands: `pair`, `stress` (toggle 24 ships and 140 shot markers with active I2S test sounds), `mute`, `reboot`. Every three seconds `PROOF` reports FPS, rendering/transfer time, free/minimum heap, PSRAM, controller connection/pause, input packet count, maximum polling gap, position/axes and A/B/Menu counters.

## Graphics and memory

The 307,200-byte RGB565 framebuffer from Little Wilds leaves too little internal memory for a Bluetooth host. Starbreak uses a 153,600-byte RGB332 indexed framebuffer in native portrait memory layout, with the same software landscape mapping `(px,py)=(319-y,x)`. Palette expansion and a complete native screen window still use Arduino_GFX's proven QSPI transport. Internal allocation is attempted only with at least 70 KB spare; otherwise PSRAM is used. No second full-screen framebuffer or full-frame rotation copy is required.

The glyph atlas is reused from Little Wilds; the DejaVu licence accompanies it. The game artwork and RGB565/indexed asset conversion pipeline will be added after the hardware gate.

## Verification status

Built and flashed on the actual Guition board; landscape pairing screen reviewed with the MacBook webcam. Initial unpaired scanning: ~29.5 FPS, ~87.5 KB internal heap free. The user then paired the Xbox controller and exercised stick/D-pad, A and B: actual input packets and ship movement were logged. With that controller connected, the 24-ship / 140-marker stress scene and audio measured ~29.4–29.5 FPS, ~84.9 KB internal heap free and ~29 ms maximum main-loop polling interval. These are diagnostic results, not the authored stage's performance acceptance or an end-to-end radio latency measurement.

Pending: Menu, disconnect/reconnect and reboot reconnection; then a playable slice, authored five-minute stage, boss, persistence and 30-minute repeated-play acceptance. See docs/BRIEF.md.

To restore Little Wilds on the board, build/upload the root project (not `-d starbreak`). Its farm saves remain in the same LittleFS partition.

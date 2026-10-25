# Starbreak architecture

## Simulation

core/Game is platform independent C++17. Input contains normalised axes, held buttons, connection status and a fresh-report flag. The Bluepad32 adapter handles the Xbox-specific reports, while host tests and the guarded validation pilot use the same gameplay interface. There are no touchscreen flight inputs.

The simulation uses fixed 1/60-second updates, independent from presentation. Normal frame catch-up is bounded after long stalls. A controller disconnect clears input and moves Playing to Pause before simulation advances. A fresh released-button report arms confirmation, then an A edge may resume. Menu toggles pause; B returns in menus and consumes the single bomb during flight. Held fire cannot accidentally retry after defeat because menu confirmation uses an edge.

Fixed capacities: 32 enemies, 96 player projectiles, 160 enemy projectiles, four upgrades and 64 effects. Objects reuse inactive slots; no game objects allocate during updates. Effects and cosmetic variation are deterministic hashes, with no dynamic RNG. Wave composition, times and trajectories are deterministic.

Player and enemy shots use swept relative-motion segment/AABB tests. Their previous positions are compared with target previous positions, preventing tunnelling for both moving projectiles and moving targets. Damage grants 1.5 seconds of invulnerability and never removes cannon upgrades. Lethal contact terminates the simulation tick so later collisions cannot change an already-final result.

## Authored stage

- 0–42 s: small scout formations; a gentle introduction.
- 42–105 s: weaving fighters with aimed shots.
- 105–165 s: armoured gunships and supporting scouts.
- 165–215 s: drifting asteroid belt with occasional fighters.
- 215–250 s: mixed final formations.
- 250–265 s: transition and approach.
- 265 s onward: dreadnaught, 1,500 HP; at 750 HP the exposed core changes attack pattern.

Power cells appear at approximately 68 and 153 seconds, moving through the playfield with short-range attraction. Cannon levels one/two/three produce single/twin/spread fire; taking damage preserves the level. The boss awards 5,000 points plus 750 per remaining shield. Timing depends on actual play; the validation pilot's completed stage is approximately five minutes, without advancing the clock artificially.

## Hardware

The verified AXS15231B QSPI configuration, 480 × 320 software landscape mapping, I²C touch mapping and I²S speaker pins are reused. Bluepad32 4.1.0 bundles Arduino ESP32 2.0.17 and BTstack; the package is SHA256 pinned and lives in an isolated project package directory.

The framebuffer is 153,600 bytes of indexed colour in the LCD's native memory order. Palette expansion still sends a complete native screen window through the proven Arduino_GFX transport. Internal SRAM is used only with at least 70 KB spare before allocation; otherwise PSRAM is available. This avoids the 307,200-byte RGB565 drawing buffer that would crowd out Bluetooth.

Bluetooth and the queued audio task run on core 0; simulation/rendering run on core 1. Controller polling occurs between frame transfers. The measured polling interval is a software bound, not a measurement of end-to-end physical controller latency.

## Settings and farm isolation

Only best score and mute setting persist, in NVS namespace `starbreak`, using alternating `cfg0`/`cfg1` records. Each record has magic, version, flags, best score, sequence and CRC. Invalid records are ignored; the latest valid record wins. Writes occur only on settings/results screens and are read back for verification. Failed writes retry with a delay.

The partition table is identical to Little Wilds. Shooter firmware never mounts or writes the farm's LittleFS partition. Do not erase flash or upload a filesystem image when switching games. The SD card is unused. Bluepad32's pairing keys are retained on boot and use its own storage.

## Build modes

`guition` is the normal controller-operated game. `validation` adds guarded serial commands for the normal-input pilot and corruption testing. No immunity, score injection, time acceleration or fake Bluetooth connection is available. The pilot only supplies controls while the actual Xbox link is connected. Normal builds exclude the pilot and corruption hooks.

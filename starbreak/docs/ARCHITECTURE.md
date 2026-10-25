# Starbreak architecture

## Simulation

core/Game is platform independent C++17. Input contains normalised axes, held buttons, connection status and a fresh-report flag. The Bluepad32 adapter handles the Xbox-specific reports, while host tests and the guarded validation pilot use the same gameplay interface. There are no touchscreen flight inputs.

The simulation uses fixed 1/60-second updates, independent from presentation. Normal frame catch-up is bounded after long stalls. A controller disconnect clears input and moves Playing to Pause before simulation advances. A fresh released-button report arms confirmation, then an A edge may resume. Menu toggles pause; B returns in menus and consumes the single bomb during flight. Held fire cannot accidentally retry after defeat because menu confirmation uses an edge.

Fixed capacities: 32 enemies, 96 player projectiles, 160 enemy projectiles, four upgrades and 64 effects. Objects reuse inactive slots; no game objects allocate during updates. Effects and cosmetic variation are deterministic hashes, with no dynamic RNG. Wave composition, times and trajectories are deterministic.

Player and enemy shots use swept relative-motion segment/AABB tests. Their previous positions are compared with target previous positions, preventing tunnelling for both moving projectiles and moving targets. Damage grants 1.5 seconds of invulnerability and never removes cannon upgrades. Lethal contact terminates the simulation tick so later collisions cannot change an already-final result.

## Three-sector campaign

Each sector has 70 seconds of deterministic formations, a ten-second clear approach and a two-phase boss. Sector clear freezes flight for four seconds, repairs one shield (maximum three), awards points and advances to the next sector. The final boss ends the campaign. Power, ranks, remaining bomb and score carry between sectors; retry resets the entire campaign.

- Outer Belt: teaching scout formations, weaving fighters, gunships and asteroids; Warden (480 HP) fires aimed fans.
- Crystal Canyon: fast weaving interceptors, drifting mines and gunships; Hive Carrier (850 HP) fires a lane fan and launches interceptors in phase two.
- Iron Citadel: turret batteries and mixed formations; Dreadnaught (1,500 HP) changes from aimed salvos to a five-shot spread.

All bosses change phase at half health. Boss collision bounds match each sprite's dimensions. Power cells appear at 22 and 55 seconds of total flight. Three cannon levels provide single/twin/spread fire. Each boss awards 2,000 × sector points; final victory adds 5,000 plus 750 per remaining shield. Host pilot runs take approximately 315–325 seconds.

Kills grant experience equal to their score value. Ranks two, three and four require 1,400, 2,800 and 4,200 additional XP, respectively. Rank two reduces firing interval from 135 to 120 ms; rank three raises movement speed from 165 to 180 pixels/second; rank four reduces firing interval to 105 ms. Cannon pickup levels remain independent.

Canyon and citadel terrain uses deterministic scrolling upper/lower boundary functions shared by renderer and collision. The corridor remains at least 150 pixels high. Contact costs a shield through the normal cooldown and pushes the ship clear. Terrain retracts before bosses. The terrain scroll and sector transition freeze during disconnect/pause.

## Hardware

The verified AXS15231B QSPI configuration, 480 × 320 software landscape mapping, I²C touch mapping and I²S speaker pins are reused. Bluepad32 4.1.0 bundles Arduino ESP32 2.0.17 and BTstack; the package is SHA256 pinned and lives in an isolated project package directory.

The framebuffer is 153,600 bytes of indexed colour in the LCD's native memory order. Palette expansion still sends a complete native screen window through the proven Arduino_GFX transport. Internal SRAM is used only with at least 70 KB spare before allocation; otherwise PSRAM is available. This avoids the 307,200-byte RGB565 drawing buffer that would crowd out Bluetooth.

Bluetooth and the queued audio task run on core 0; simulation/rendering run on core 1. Controller polling occurs between frame transfers. The measured polling interval is a software bound, not a measurement of end-to-end physical controller latency.

## Settings and farm isolation

Only best score and mute setting persist, in NVS namespace `starbreak`, using alternating `cfg0`/`cfg1` records. Each record has magic, version, flags, best score, sequence and CRC. Invalid records are ignored; the latest valid record wins. Writes occur only on settings/results screens and are read back for verification. Failed writes retry with a delay.

The partition table is identical to Little Wilds. Shooter firmware never mounts or writes the farm's LittleFS partition. Do not erase flash or upload a filesystem image when switching games. The SD card is unused. Bluepad32's pairing keys are retained on boot and use its own storage.

## Build modes

`guition` is the normal controller-operated game. `validation` adds guarded serial commands for the normal-input pilot and corruption testing. No immunity, score injection, time acceleration or fake Bluetooth connection is available. The pilot only supplies controls while the actual Xbox link is connected. Normal builds exclude the pilot and corruption hooks.

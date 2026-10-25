# Starbreak validation

## Hardware proof

Guition ESP32-S3 with the existing 480 × 320 landscape AXS15231B panel, capacitive touch and built-in I²S speaker. Bluepad32 4.1.0 (official packaged Arduino ESP32 2.0.17), Espressif32 6.10.0, Arduino_GFX 1.5.9.

The user paired their Xbox Series X/S controller and exercised stick/D-pad, A and B. Serial logs recorded real axes, ship movement, packets and button counts. A subsequent manual check confirmed Menu, power-off/reconnect and the pause safeguard; logs also recorded Menu presses. After firmware restarts, the actual controller reconnected using remembered keys without another pairing procedure.

Synthetic connected-controller stress (24 diagnostic ship silhouettes and 140 projectile markers, with audio active) measured roughly 29.4–29.5 FPS. Free internal heap settled near 84.9 KB; polling gaps were about 29 ms. This establishes initial combined hardware headroom, not final stage acceptance or measured end-to-end radio latency.

## Playable stage and host tests

The playable sprite scale was reduced at the user's request: 26 × 16 player, 18 × 14 scouts, 24 × 18 fighters, 36 × 26 gunships. Webcam checks confirm the display orientation, HUD and open playfield. Player shots are cyan; hostile shots are gold with white cores and blue outlines.

Host checks cover:

- Radial dead zone and equal straight/diagonal maximum speed.
- Swept collision: fast crossings, near misses, moving-relative targets, zero movement, bounds and projectile radii.
- Shield damage cooldown; upgrades retained through damage; single bomb consumption.
- Two-phase boss transition, score/best-score result, defeat and retry reset.
- Disconnect freeze and release/press confirmation on reconnect.
- Settings magic/version/CRC rejection and valid decode.
- Complete authored victory and deliberate collision-driven defeat/retry, with 25/30/40 Hz input sampling and fixed 60 Hz simulation.
- AddressSanitizer and UndefinedBehaviorSanitizer pass on the portable simulation.

A preliminary physical-board run reached victory in 304.07 simulation seconds, with both upgrades and the second boss phase. Normal gameplay rules, damage and collision were active. A deliberate-loss run exercised the real defeat-to-retry path. The initial result-screen hold in the test pilot was too short for reliable webcam capture, so the final endurance run was restarted after fixing only that harness timing and an end-of-tick score guard.

## Endurance acceptance — still pending

The full 30-minute flight test is not complete. After restarting the test firmware, the Xbox was disconnected and the board remained safely at the title screen; a request to switch it back on was left pending. Waiting time was excluded from actual flight ticks. The waiting readers were stopped and the normal controller-operated game was installed for delivery.

The preliminary connected run exercised victory and defeat/retry, with approximately 7.5 minutes of observed activity. Gameplay FPS ranged from 29.4 to about 30.3 in those samples; free internal heap stayed at 71,904 bytes, including across result/settings saves. These observations support the design but do not substitute for the required 30-minute repeated-play acceptance.

 The validation build's deterministic pilot supplies ordinary controls while the actual Xbox Bluetooth link stays connected. It grants no immunity, resources, score or accelerated time. Enemies, collisions, shields, upgrades, bomb and boss logic are identical to the normal game. The pilot and settings-corruption command are excluded from the final `guition` firmware.

Raw local evidence is written under `starbreak/device-review/`: controller-check.log, reconnect-check.log, preliminary-runs.log, soak.log and the generated summary. Actual webcam captures are kept separately from host previews. The remaining requirement is a fresh, measured 30 minutes of actual flight with the Xbox connected; `scripts/soak.py` now counts 60 Hz flight ticks rather than controller-off wall time. It also accepts `--resume` to retain a live run/log.

Farm isolation: the partition table remains byte-for-byte identical to Little Wilds. The shooter never mounts/writes LittleFS and does not access SD. Score/mute use NVS namespace `starbreak`; Bluetooth pairing uses its own keys. No flash erase/filesystem image upload is used.

## Shooter settings checked on the physical board

High score 25,720 survived firmware restart. Mute was changed to on, saved, and restored on reboot. Corrupting the newest shooter-only record caused the previous valid record to load with the same best score and its coherent mute setting. Both records were then replaced with valid settings, sound restored on, and another reboot loaded cleanly without a recovery warning. These tests touched only NVS namespace `starbreak`; the farm filesystem and SD were not accessed.

Final host checks also verify the entire stage performs no C++ heap allocations. The normal build compiles successfully; AddressSanitizer/UndefinedBehaviorSanitizer checks pass for the portable simulation and collision code. Normal firmware size is approximately 708 KB, with approximately 81 KB statically allocated internal RAM plus the 153.6 KB runtime drawing buffer.

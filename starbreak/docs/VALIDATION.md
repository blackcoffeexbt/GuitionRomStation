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

## Expanded campaign validation — physical endurance pending

The three-sector campaign passes host title-to-victory simulation at 25/30/40 Hz input sampling with fixed 60 Hz updates, and deliberate defeat/retry. Additional tests cover each boss's HP and phase transition, all three sector clears, shield repair, carry-over/reset, disconnect during sector transition, XP/rank progression and canyon/citadel damage cooldown. A full campaign still performs no C++ heap allocations. The new sector tests pass AddressSanitizer/UndefinedBehaviorSanitizer. Host renderer previews show all three terrains, both phases of every boss, compact sprite scale and readable HUD/footer.

Both expanded firmware configurations build successfully. With the Xbox awake, the board completed the three-sector campaign in 316.23 seconds with three shields, cannon level III, pilot rank IV and score 43,435. Both phases of each boss were observed in the live logs. A deliberate collision-driven defeat followed, then a fresh campaign started through the normal retry path. All three flight sectors, six boss-phase screens, victory and defeat were captured through the MacBook webcam.

At this milestone, gameplay ranged from 29.7 to 30.3 FPS and free internal heap remained 71,888 bytes. Audio was unmuted and the actual Xbox link remained connected. The high score saved successfully. The real-time endurance loop is still running; the 30-minute acceptance gate is not yet passed.

A repeated host loop completed 32 runs: 16 full campaigns and 16 intentional defeats, totalling 90.4 simulated minutes. Every victory produced the same score (43,435); best score and run counters remained coherent across retries. The repeated-loop test also passes AddressSanitizer/UndefinedBehaviorSanitizer. These are host simulation results, not 90 minutes of hardware testing.

Previous-version evidence: the uninterrupted single-sector boot accumulated 914.7 seconds of flight, including two victories and deliberate defeat/retry, before Xbox sleep. Gameplay samples ranged from 29.4 to 30.3 FPS; connected free internal heap remained 71,904 bytes. This evidence is retained as `device-review/single-sector-soak.log`; it does not substitute for expanded-campaign testing.

The validation pilot supplies ordinary controls while the actual Xbox link stays connected. It grants no immunity, resources, score or accelerated time. The normal firmware excludes pilot and settings-corruption hooks. `scripts/soak.py` counts actual 60 Hz flight ticks and supports resuming its reader; `scripts/analyse_soak.py` audits the cumulative raw log. Paused/disconnected time cannot pass the 30-minute gate.

Farm isolation: the partition table remains byte-for-byte identical to Little Wilds. The shooter never mounts/writes LittleFS and does not access SD. Score/mute use NVS namespace `starbreak`; Bluetooth pairing uses its own keys. No flash erase/filesystem image upload is used.

## Shooter settings checked on the physical board

High score 25,720 survived firmware restart. Mute was changed to on, saved, and restored on reboot. Corrupting the newest shooter-only record caused the previous valid record to load with the same best score and its coherent mute setting. Both records were then replaced with valid settings, sound restored on, and another reboot loaded cleanly without a recovery warning. These tests touched only NVS namespace `starbreak`; the farm filesystem and SD were not accessed.

Final host checks also verify the entire stage performs no C++ heap allocations. The expanded normal build compiles successfully; AddressSanitizer/UndefinedBehaviorSanitizer checks pass for the portable simulation and collision code. Normal firmware size is approximately 723 KB, with approximately 81 KB statically allocated internal RAM plus the 153.6 KB runtime drawing buffer.

A renderer regression check verifies pause/results screens show the sound footer without overlapping flight hints or boss health. The expanded build includes that correction.

Final normal-firmware webcam check confirms the three-sector title, menu readability and retained best score of 25,720. Serial status reports clean title-screen operation at 30.3 FPS, 74,528 bytes free internal heap and 8,385,947 bytes free PSRAM with the Xbox disconnected. This is an idle-screen observation, not the gameplay performance gate. Final firmware size is 722,629 bytes, with 81,112 bytes static RAM.

## Endurance checkpoint after controller sleep

The expanded campaign has accumulated 82,734 actual 60 Hz ticks (1,378.9 seconds since boot; 1,374.68 seconds after the initial monitored status). Four complete victories and two deliberate defeats/retries are recorded. All six boss phases are covered. Across 460 gameplay samples, FPS ranged from 29.7 to 30.3 (median 30.3), and free internal heap stayed exactly 71,888 bytes. No crashes, tick resets or disconnected gameplay samples occurred.

The Xbox subsequently went to sleep. The game paused at sector 1 time 81.24, position (68.00, 190.93), with ticks frozen at 82,734. The test firmware and live reader are retained to continue this same boot after reconnect; no reboot or replacement firmware was installed at this checkpoint. The 30-minute gate remains incomplete. Disconnected waiting is excluded.

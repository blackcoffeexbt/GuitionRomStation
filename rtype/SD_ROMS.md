# SD ROM library

## Prepare the card

Use a FAT32 microSD card. Copy uncompressed game images into this folder using your computer:

```text
/roms/sms/
    R-Type.sms
    Sonic.sms
    Another Game.SMS
```

The preferred location is directly in `sms/`. If that folder is missing or contains no matching files, the browser checks the card’s root instead; this version does not descend into other subfolders. `.sms` is case-insensitive. Hidden files and macOS `._` resource files are ignored. The browser shows up to 128 entries, sorted by filename; names must fit within 191 bytes. Short, readable Latin-character filenames work best with the built-in font.

The firmware never formats the card or creates, deletes or modifies files. It opens the folder and ROMs for reading only. An absent, unreadable or unsupported-format card leaves an explanatory screen with a rescan button. The user’s card has now mounted successfully on the physical board in 1-bit SDMMC mode. No card files were moved or changed.

## Choose and switch games

The firmware opens the library at boot and shows the actual directory being used. Tap a game, or use the Xbox stick/D-pad and A. Up/down selects, left/right moves five entries, and holding a direction repeats. X rescans. B returns to the currently loaded game’s pause menu; Back is unavailable until a game has been loaded.

The touch footer provides Prev, Next, Rescan and Back. During play, Menu pauses; X from the pause menu opens the library. The pause menu also has a touch target for the library. A selected game starts when a controller is connected; the selection press is consumed so it cannot also start/fire in the native game. Without a controller, the loaded cartridge waits paused until connection and confirmation.

A file is read completely into PSRAM, checked, then adopted by the emulator. All file handles close before play resumes. The SD card is not consulted during emulation. The previous ROM stays alive until a replacement has loaded successfully, so a read error, card removal, invalid header or insufficient memory leaves the previous game available.

## Supported files and compatibility

| Image | Support |
| --- | --- |
| Raw Sega Master System `.sms` | Supported when it has a recognised SMS header and uses standard Sega banking |
| 8 KB image | Mirrored into a 16 KB bank |
| 16 KB-aligned image up to 1 MB | Supported by the loader |
| 512-byte copier header | Detected and stripped while reading |
| Game Gear `.gg` or a renamed GG image | Unsupported; GG headers are rejected |
| ZIP, 7z and other archives | Not scanned; extract the `.sms` first |
| Mega Drive/Genesis, NES, Game Boy, PC Engine or arcade ROMs | Unsupported |
| Codemasters/Korean or other special mappers | Not implemented by this core; a valid SMS header alone does not guarantee compatibility |
| Light-gun, paddle, 3D-glasses or other special-accessory games | Not supported by the two-button controller adapter |

A `TMR SEGA` header at the usual 8/16/32 KB location is required, with SMS region 3 or 4. Headerless homebrew is currently rejected. The loader checks sizes and console headers, not cartridge authenticity or every game’s compatibility. The core uses 60 Hz/NTSC timing; PAL-only behaviour and optional FM sound are not implemented or validated.

The card’s supplied R-Type image and its built-in-game Alex Kidd and Sonic images all reached gameplay on the physical board, confirmed by webcam. Files whose names start with `[BIOS]` (case-insensitive) use the BIOS memory-slot profile; retain that prefix on built-in-game BIOS dumps. Normal cartridge files use the standard cartridge profile. Their data is never modified. These short checks do not establish complete-game compatibility. Use your own cartridge dumps or homebrew images matching the requirements above. No game data is downloaded by the project.

Save states and persistent cartridge SRAM are not implemented. Games which depend on battery-backed saves will lose that progress after reset, cartridge switching or power-off. Mute remains a separate persistent firmware setting.

## Hardware configuration and verification

The SD interface is configured as native 1-bit SDMMC: CLK GPIO12, CMD GPIO11 and D0 GPIO13. GPIO10/DAT3 is pulled high. The mapping follows the repository’s existing [hardware notes](../docs/HARDWARE.md); the driver calls `setPins` explicitly instead of using ESP32-S3 defaults. These pins do not overlap the verified LCD, touch or speaker pins. The mount uses `format_if_mount_failed=false`.

Host tests exercise the same reader used by the SD adapter: valid images, copier headers, 8 KB mirroring, invalid sizes/console headers, allocation failure, interrupted reads, recovery and repeated cartridge adoption. Browser tests cover paging, held navigation and selection edge handling. The firmware builds without an embedded game ROM. Physical mounting, the three-entry browser and gameplay in all three games are webcam-verified. Validation-only simulated controller reports passed movement, buttons, pause and reconnect guards without resets or audio-write errors. BIOS slot/vector fixtures also pass. The normal Xbox firmware is restored. Physical-radio responsiveness and complete-game compatibility still require user feedback.

USB diagnostics add `library`, `rescan` and `load N` (zero-based index while the library is open). `status` includes the library count and current game name. Use the reset-safe checker described in the main README.

## Measured card check

| Card image | Bytes | Load time | Short check |
| --- | ---: | ---: | --- |
| Alex Kidd in Miracle World (built-in-game image) | 131,072 | 107 ms | Gameplay visible on webcam |
| Sonic the Hedgehog (built-in-game image) | 262,144 | 183 ms | Gameplay visible on webcam |
| R-Type (Taiwan image) | 524,288 | 348 ms | Gameplay visible on webcam |

The images were found in the card root. Full-running samples were about 60 emulated FPS and 25.5–29.3 presented FPS. The tests used simulated controls with the real Xbox disconnected. Every game passed disconnect freeze, held-A reconnect protection, fresh-press resume, Menu pause and library navigation/rescan. Free internal heap stayed at 101,024 bytes across switches; PSRAM changed with ROM size as expected. No resets or audio-write errors occurred. This was a short check, not an endurance run or a physical Bluetooth-latency measurement.

The initial no-input intro checks missed BIOS boot failures; webcam gameplay review exposed them. BIOS memory-control support resolved Alex Kidd’s SEGA-screen stall and Sonic’s software error. Final gameplay photos and logs remain local under `device-review/`. `scripts/test_sd_live.py` is an explicit intro check; `scripts/test_controller_live.py` adds validation-only controller simulation and captures gameplay photos for manual review. Neither is a background/endurance loop. The installed normal build has no simulated-control commands.

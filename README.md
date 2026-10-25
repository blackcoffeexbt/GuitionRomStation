# Master System / R-Type for the Guition ESP32-S3

The current project runs **Sega Master System ROMs, including R-Type, from an SD card** on the Guition JC3248W535 / JC3248W535C touchscreen board, using an Xbox Series X/S controller and the board’s speaker. **Branch: `rtype-sms`.**

The emulator uses the SMSPlus core from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu), adapted to the verified Guition display, Bluetooth controller and I²S audio drivers. It runs a locally supplied cartridge image; game ROMs and firmware binaries containing them are excluded from Git.

## Current status

| Area | Status |
| --- | --- |
| R-Type gameplay | Installed; the user confirmed stable gameplay and sound |
| Xbox controls | Movement, shot/charge, Force pod and rapid fire confirmed; input lag subsequently reported |
| Latency improvements | Installed with the SD build; host/race tests pass; user comparison pending |
| ROM loading | Installed; all three card ROMs reached gameplay, verified by webcam with simulated controls |
| SD ROM browser | Touch/Xbox selection from `/roms/sms/`, with card-root fallback; SD mount verified |
| Save states / cartridge SRAM persistence | Not implemented |

The board now runs the SD-library build, including the latency revision. Alex Kidd, Sonic and R-Type loaded from the card in about 107, 183 and 348 ms and reached gameplay on the webcam. Short samples measured about 60 emulated FPS and 25.5–29.3 displayed FPS. Simulated controller reports exercised movement, both native buttons, rapid fire, pause, disconnect freeze and fresh-press reconnect confirmation for every game. The normal Xbox firmware is installed; these checks do not certify complete-game compatibility or physical radio latency. No long automated test is running or scheduled.

## Hardware

- ESP32-S3, 16 MB flash and 8 MB PSRAM.
- AXS15231B QSPI display, 480 × 320 landscape, capacitive touch.
- Built-in I²S amplifier/speaker connection.
- Xbox controller over Bluetooth using Bluepad32.

The original 256 × 192 picture scales to 426 × 320 with narrow side borders, preserving 4:3 aspect. The emulator targets 60 Hz; the display runs independently. Short checks of the installed build measured about 60 emulated FPS and 23–28 displayed FPS depending on controller activity and scene. Those early measurements predate the latency revision. The latest short SD checks are recorded above; no endurance certification is claimed.

See the verified pins and orientation in [BoardConfig.h](rtype/include/BoardConfig.h) and the detailed [R-Type project notes](rtype/README.md).

## Controls

| Control | Action |
| --- | --- |
| Left stick / D-pad | Move |
| A | SMS button 1; R-Type uses it for shot/charge |
| B | SMS button 2; R-Type uses it for the Force pod |
| X | Rapid fire during play; ROM library while paused |
| Menu | Pause / resume and show controls |
| Y while paused | Reset the cartridge |
| Touch | Pause/menu/resume and mute; no touch flight controls |

Choose a ROM from the library. With a connected controller it starts automatically; use A/B according to that game’s native title screen and rules. R-Type uses A to start. Pairing is remembered. If necessary, hold the controller’s Pair button while the board is waiting for it. Losing the connection freezes emulation and clears input; release A and press it again to confirm resuming after reconnection.

## Build and flash

Install PlatformIO Core, then run these commands from the repository root:

```sh
python3 rtype/scripts/bootstrap.py
pio run -d rtype
pio run -d rtype -t upload --upload-port /dev/cu.usbmodem101
```

No cartridge image is needed to build the SD firmware. Put uncompressed `.sms` files in `/roms/sms/` on a FAT32 card (the root is also scanned if that folder has no matching files); choose a game using touch or Xbox controls after boot. Use Menu, then X, to choose another game. ROMs load fully into PSRAM before play, with no SD reads during emulation. See [SD preparation and ROM compatibility](rtype/SD_ROMS.md).

The optional bootstrap argument `--rom '/full/path/to/game.sms'` imports a local image for cartridge-specific host tests only. It otherwise uses the single `.sms` in local `type-r/` when available. No ROM is embedded in this firmware. Local test inputs, screenshots, dependencies and build outputs remain ignored by Git.

Pinned dependencies: Espressif32 6.10.0, official Arduino/Bluepad32 4.1.0 (Arduino ESP32 2.0.17), Arduino_GFX 1.5.9 and C++17. Emulator source is vendored with its revision and per-file licensing recorded in [THIRD_PARTY.md](rtype/THIRD_PARTY.md).

The application upload preserves the existing flash partition layout and the farm’s LittleFS partition. The emulator does not mount LittleFS; the new SD adapter reads ROM files without formatting or writing the card. Its mute setting uses the separate `rtype-sms` NVS namespace.

## Tests and diagnostics

```sh
./rtype/scripts/test_host.sh
python3 rtype/scripts/check_board.py --seconds 20
```

Host checks cover ROM loading and interrupted-read recovery, repeated cartridge switching, browser navigation, ROM boot, video/PSG output, bank reset, button mapping, reconnect confirmation, scaling/rotation and concurrent frame ownership. The core also passed 7,200 simulated frames under AddressSanitizer; the latency revision’s mailbox passed ThreadSanitizer. Host tests require a C/C++ compiler; the cartridge-specific integration check is skipped if no local ROM is supplied.

The provided board checker leaves USB DTR/RTS reset lines untouched and disables hang-up-on-close. Ordinary serial-monitor settings previously caused `USB_UART_CHIP_RESET`, interrupting gameplay; use the supplied checker for brief observations.

USB commands: `status`, `pair`, `pause`, `resume`, `reset`, `mute`, `library`, `rescan`, `load N`. An explicit `bench` command, accepted after a game has loaded and is paused outside the library, resets the cartridge and runs one eight-second intro diagnostic with no input, then pauses automatically:

```sh
python3 rtype/scripts/check_board.py --seconds 12 --bench
```

Diagnostics report emulation/display FPS, rendering and transfer times, memory, controller activity and audio errors. The current build also reports polling gaps and frame age. These are software timings, not measured radio-to-screen latency.

## Repository layout and previous games

| Path | Purpose |
| --- | --- |
| `rtype/` | Current Master System emulator project, hardware adapters, tests and build scripts |
| `rtype/lib/smsplus/` | Vendored emulator core and original notices |
| `rtype/roms/` | Ignored optional cartridge image for host tests |
| `type-r/` | Ignored user-supplied ROM directory |
| `starbreak/` | Previous original space-shooter project |
| Root `src/`, `assets/`, `platformio.ini` | Preserved Little Wilds farming project |
| `docs/` | Original hardware/farming documentation and archived Little Wilds README |

Root-level `pio run` still builds **Little Wilds**; use `-d rtype` for the current project. Its original instructions are preserved in [Little Wilds](docs/LITTLE_WILDS.md). The last complete Starbreak build is `4ae5d85`; `e6a0c33` preserves unfinished weapon-expansion work. Both previous projects and the existing farm saves remain recoverable.

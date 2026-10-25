# R-Type for the Guition ESP32-S3

The current project runs the **Sega Master System version of R-Type** on the Guition JC3248W535 / JC3248W535C touchscreen board, using an Xbox Series X/S controller and the board’s speaker. **Branch: `rtype-sms`.**

The emulator uses the SMSPlus core from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu), adapted to the verified Guition display, Bluetooth controller and I²S audio drivers. It runs a locally supplied cartridge image; game ROMs and firmware binaries containing them are excluded from Git.

## Current status

| Area | Status |
| --- | --- |
| R-Type gameplay | Installed; the user confirmed stable gameplay and sound |
| Xbox controls | Movement, shot/charge, Force pod and rapid fire confirmed; input lag subsequently reported |
| Latency improvements | Built and host-tested in `05b7993`; **not flashed or hardware-validated**, at the user’s request |
| ROM loading | A local `.sms` image is embedded into application flash during the build |
| SD ROM browser | Discussed, not implemented; the SD card has not been modified |
| Save states / cartridge SRAM persistence | Not implemented |

The board retains the stable build from `6b980e1`. Building and uploading the current source will install the newer latency revision. No long automated test is running or scheduled.

## Hardware

- ESP32-S3, 16 MB flash and 8 MB PSRAM.
- AXS15231B QSPI display, 480 × 320 landscape, capacitive touch.
- Built-in I²S amplifier/speaker connection.
- Xbox controller over Bluetooth using Bluepad32.

The original 256 × 192 picture scales to 426 × 320 with narrow side borders, preserving 4:3 aspect. The emulator targets 60 Hz; the display runs independently. Short checks of the installed build measured about 60 emulated FPS and 23–28 displayed FPS depending on controller activity and scene. These measurements do not validate the newer latency revision or constitute an endurance test.

See the verified pins and orientation in [BoardConfig.h](rtype/include/BoardConfig.h) and the detailed [R-Type project notes](rtype/README.md).

## Controls

| Control | Action |
| --- | --- |
| Left stick / D-pad | Move |
| A | Fire; hold to charge using R-Type’s native weapon rules |
| B | Launch / recall the Force pod |
| X | Rapid fire |
| Menu | Pause / resume and show controls |
| Y while paused | Reset the cartridge |
| Touch | Pause/menu/resume and mute; no touch flight controls |

Wake the controller, press A to leave the emulator menu, then press A at R-Type’s own title screen. Pairing is remembered. If necessary, hold the controller’s Pair button while the board is waiting for it. Losing the connection freezes emulation and clears input; release A and press it again to confirm resuming after reconnection.

## Build and flash

Install PlatformIO Core, then run these commands from the repository root:

```sh
python3 rtype/scripts/bootstrap.py --rom '/full/path/to/game.sms'
pio run -d rtype
pio run -d rtype -t upload --upload-port /dev/cu.usbmodem101
```

If `--rom` is omitted, bootstrap imports the single `.sms` file in the local `type-r/` directory. It prepares the pinned toolchain in Starbreak’s isolated dependency directory. ROM files in `type-r/` and `rtype/roms/`, screenshots, dependencies and build outputs are ignored by Git.

Pinned dependencies: Espressif32 6.10.0, official Arduino/Bluepad32 4.1.0 (Arduino ESP32 2.0.17), Arduino_GFX 1.5.9 and C++17. Emulator source is vendored with its revision and per-file licensing recorded in [THIRD_PARTY.md](rtype/THIRD_PARTY.md).

The application upload preserves the existing flash partition layout and the farm’s LittleFS partition. R-Type does not mount LittleFS or access SD. Its mute setting uses the separate `rtype-sms` NVS namespace.

## Tests and diagnostics

```sh
./rtype/scripts/test_host.sh
python3 rtype/scripts/check_board.py --seconds 20
```

Host checks cover ROM boot, video and PSG sound output, bank reset, frame skipping, native button mapping, reconnect confirmation, scaling/rotation and concurrent frame ownership. The core also passed 7,200 simulated frames under AddressSanitizer; the latency revision’s mailbox passed ThreadSanitizer. Host tests require the locally supplied ROM and a C/C++ compiler.

The provided board checker leaves USB DTR/RTS reset lines untouched and disables hang-up-on-close. Ordinary serial-monitor settings previously caused `USB_UART_CHIP_RESET`, interrupting gameplay; use the supplied checker for brief observations.

USB commands: `status`, `pair`, `pause`, `resume`, `reset`, `mute`. An explicit `bench` command, accepted while paused, resets the cartridge and runs one eight-second intro diagnostic with no input, then pauses automatically:

```sh
python3 rtype/scripts/check_board.py --seconds 12 --bench
```

Diagnostics report emulation/display FPS, rendering and transfer times, memory, controller activity and audio errors. The pending latency build also reports polling gaps and frame age. These are software timings, not measured radio-to-screen latency.

## Repository layout and previous games

| Path | Purpose |
| --- | --- |
| `rtype/` | Current Master System emulator project, hardware adapters, tests and build scripts |
| `rtype/lib/smsplus/` | Vendored emulator core and original notices |
| `rtype/roms/` | Ignored local cartridge image used by the build |
| `type-r/` | Ignored user-supplied ROM directory |
| `starbreak/` | Previous original space-shooter project |
| Root `src/`, `assets/`, `platformio.ini` | Preserved Little Wilds farming project |
| `docs/` | Original hardware/farming documentation and archived Little Wilds README |

Root-level `pio run` still builds **Little Wilds**; use `-d rtype` for the current project. Its original instructions are preserved in [Little Wilds](docs/LITTLE_WILDS.md). The last complete Starbreak build is `4ae5d85`; `e6a0c33` preserves unfinished weapon-expansion work. Both previous projects and the existing farm saves remain recoverable.

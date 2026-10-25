# GuitionRomStation

Run Sega Master System `.sms` ROMs from a microSD card on the **Guition JC3248W535 / JC3248W535C ESP32-S3** board, with an Xbox Series X/S controller and the board’s speaker. The firmware works offline; no ROM is embedded or downloaded.

The SMSPlus core is adapted from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu). The repository now contains only this emulator project. Earlier Little Wilds and Starbreak projects remain available in Git history; `48ef526` is the working SD-loader checkpoint before repository cleanup.

## Play

Use a FAT32 microSD card with uncompressed `.sms` files in `/roms/sms/`. If that folder is absent or has no matching files, the browser scans the card root. Choose a game with touch or Xbox directions and A. X rescans; B returns to the loaded game. The firmware reads the card without formatting or changing its files.

| Control | Action |
| --- | --- |
| Left stick / D-pad | SMS directions |
| A | SMS button 1; shot/charge in R-Type |
| B | SMS button 2; Force pod in R-Type |
| X during play | Rapid fire |
| Menu | Pause / resume |
| X while paused | Open ROM library |
| Y while paused | Reset game |
| Touch | Library, pause/resume and mute |

Pairing is remembered. Hold the Xbox Pair button if it does not connect automatically. A lost connection freezes emulation and clears input. Release A, then press it again to resume after reconnecting. There are no touchscreen gameplay controls.

The picture is scaled to 426 × 320, preserving the original 4:3 aspect on the 480 × 320 landscape display. Sound is emulated SN76496 PSG audio through the built-in I²S amplifier.

## Build and flash

Install Python 3 and PlatformIO Core. Run from this repository’s root:

```sh
python3 scripts/bootstrap.py
pio run
pio run -t upload --upload-port /dev/cu.usbmodem101
```

Choose the board’s actual USB port if it differs. Bootstrap prepares a checksum-pinned official Bluepad32 Arduino package in `.deps/`. PlatformIO pins Espressif32 6.10.0, Bluepad32 4.1.0 / Arduino ESP32 2.0.17, Arduino_GFX 1.5.9 and C++17. No game image is needed to compile.

Target hardware: ESP32-S3, 16 MB flash, 8 MB OPI PSRAM, AXS15231B QSPI display and capacitive touch. Verified display, touch, audio and SD pins are in [hardware notes](docs/HARDWARE.md) and [BoardConfig.h](include/BoardConfig.h).

Upload the application only. The partition layout is retained so existing LittleFS data survives; the emulator does not mount that partition. Mute is stored separately in the `rtype-sms` NVS namespace.

## ROM compatibility

The loader accepts SMS-header images up to 1 MB using standard Sega banking, including supported 8 KB images and 512-byte copier headers. Keep the `[BIOS]` filename prefix on built-in-game BIOS dumps so the correct memory profile is selected.

Game Gear, other consoles, archives, special mappers and special controllers are unsupported. Save states and persistent cartridge SRAM are not implemented. Optional FM audio and PAL-only behaviour are not validated. See [SD setup and compatibility](SD_ROMS.md) for exact requirements.

Supply your own cartridge dumps or compatible homebrew. ROMs, local test inputs, screenshots and build outputs are ignored by Git.

## Tests and diagnostics

```sh
./scripts/test_host.sh
python3 scripts/check_board.py --seconds 20
```

Host tests cover ROM validation and interrupted reads, cartridge switching and recovery, BIOS memory mapping, input/reconnect guards, library navigation, scaling and concurrent frame ownership. To include a private cartridge integration test:

```sh
python3 scripts/bootstrap.py --rom '/full/path/to/game.sms'
./scripts/test_host.sh
```

Use the supplied board checker for serial observations: it avoids USB reset-line toggles that can interrupt gameplay. USB commands include `status`, `pair`, `pause`, `resume`, `reset`, `mute`, `library`, `rescan` and `load N` (zero-based library index). After loading a game and pausing outside the library, `--bench` runs one eight-second intro diagnostic and pauses automatically.

Alex Kidd, Sonic and R-Type were loaded from the attached SD card and reached gameplay, verified by webcam with simulated controller reports. Movement, buttons, pause and reconnect safeguards passed for all three. Short samples measured about 60 emulated FPS and 25.5–29.3 displayed FPS, with no resets or audio-write errors. The normal Xbox build is installed. These checks do not certify complete-game compatibility, endurance or physical radio latency. Earlier manual R-Type play confirmed Xbox controls and sound. Detailed evidence and limits are in [validation notes](docs/VALIDATION.md).

The optional `validation` build enables USB input simulation for the explicit `scripts/test_controller_live.py` development check. Normal builds exclude simulated controls. No background test runs at boot.

## Layout

| Path | Purpose |
| --- | --- |
| `src/` | Emulator adapter, ROM library, video, audio and controls |
| `src/hardware/` | Guition display, touch, Bluetooth and SD drivers |
| `include/` | Board configuration |
| `lib/smsplus/` | Vendored emulator core and original notices |
| `assets/` | Menu font |
| `scripts/` | Toolchain setup and host/device checks |
| `test/` | Host tests and hardware stubs |
| `docs/` | Hardware and validation notes |
| `roms/`, `type-r/` | Ignored private local ROM inputs |

See [third-party source and licensing](THIRD_PARTY.md), including the CPU core’s individual license terms.

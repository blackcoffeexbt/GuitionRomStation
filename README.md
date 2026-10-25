# GuitionRomStation

Run Sega Master System `.sms` ROMs from a microSD card on the **Guition JC3248W535 / JC3248W535C ESP32-S3** board, with an Xbox Series X/S controller and the board’s speaker. The firmware works offline; no ROM is embedded or downloaded.

Built on the SMSPlus core from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu), with a touchscreen ROM browser, Bluetooth controller support and I²S audio.

## Hardware requirements

- Guition JC3248W535 / JC3248W535C with ESP32-S3, 16 MB flash and 8 MB OPI PSRAM.
- AXS15231B QSPI display and capacitive touch, used in 480 × 320 landscape orientation.
- Xbox Series X/S controller with Bluetooth support.
- FAT32 microSD card and USB data cable.
- Speaker connected to the board’s speaker port for audio.

Display, touch, audio and SD wiring are documented in [hardware notes](docs/HARDWARE.md) and [BoardConfig.h](include/BoardConfig.h). Other boards and controllers are not validated.

## Play

Use a FAT32 microSD card with uncompressed `.sms` files in `/roms/sms/`. If that folder is absent or has no matching files, the browser scans the card root. Choose a game with touch or Xbox directions and A. X rescans; B returns to the loaded game. The firmware reads the card without formatting or changing its files.

| Control | Action |
| --- | --- |
| Left stick / D-pad | SMS directions |
| A | SMS button 1 / confirm menu selection |
| B | SMS button 2 / return from library |
| X during play | Rapid fire |
| Menu | Pause / resume |
| X while paused | Open ROM library |
| Y while paused | Reset game |
| Touch | Library, pause/resume and mute |

Turn on the controller and hold its Pair button until the Xbox light flashes rapidly. Pairing is remembered for subsequent connections. A game selected without a connected controller waits paused. A lost connection freezes emulation and clears input. Release A, then press it again to resume after reconnecting. There are no touchscreen gameplay controls.

The picture is scaled to 426 × 320, preserving the original 4:3 aspect on the 480 × 320 landscape display. Sound is emulated SN76496 PSG audio through the built-in I²S amplifier.

## Build and flash

Install Python 3 and PlatformIO Core. Run from this repository’s root:

```sh
python3 scripts/bootstrap.py
pio run
pio run -t upload --upload-port <PORT>
```

Replace `<PORT>` with the board’s serial port, such as `/dev/cu.usbmodem101` on macOS, `/dev/ttyACM0` on Linux or `COM3` on Windows. Bootstrap prepares a checksum-pinned official Bluepad32 Arduino package in `.deps/`. PlatformIO pins Espressif32 6.10.0, Bluepad32 4.1.0 / Arduino ESP32 2.0.17, Arduino_GFX 1.5.9 and C++17. No game image is needed to compile.

The standard build uses real controller input. Upload the application with the command above; a filesystem upload is not required. The firmware reads ROMs from SD and stores the mute setting in NVS. It does not use LittleFS for game storage.

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

The emulator targets 60 Hz independently of the display. Short hardware checks measured approximately 60 emulated FPS and 25–29 displayed FPS, depending on the scene. Complete-game compatibility, long-term stability and controller latency have not been comprehensively validated. See [validation notes](docs/VALIDATION.md) for test coverage and limitations.

For development, the optional `validation` build enables USB input simulation used by `scripts/test_controller_live.py`. This script expects the three-game test library described in the validation notes; it is not a general compatibility test. Normal builds exclude simulated controls.

## Project layout

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

## Credits and licensing

See [third-party source and licensing](THIRD_PARTY.md) for source revisions and individual license terms. These include GPL notices for SMSPlus, separate non-commercial/freeware terms for the Z80 CPU core, and the DejaVu font notice. Do not assume a single license applies to every component.

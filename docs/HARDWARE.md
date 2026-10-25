# Guition hardware bring-up

Target reported by user: JC3248W535 / JC3248W535C. Treat EN/C naming as a board-family assumption until the physical rear label is checked. Firmware reports the actual chip/flash/PSRAM at boot.

| Function | Configuration |
| --- | --- |
| CPU | ESP32-S3, 240 MHz |
| Flash | 16 MB, DIO, 80 MHz |
| PSRAM | 8 MB OPI; Arduino memory type `qio_opi` |
| LCD | AXS15231B QSPI, native 320 × 480, 40 MHz |
| LCD CS/CLK | GPIO45 / GPIO47 |
| LCD D0/D1/D2/D3 | GPIO21 / GPIO48 / GPIO40 / GPIO39 |
| LCD reset/DC | No dedicated pin; driver software reset |
| Backlight | GPIO1, active high |
| Touch | I²C SDA4 / SCL8, 400 kHz, address 0x3b, interrupt3 unused |
| Speaker amplifier | On-board NS4168 I²S, BCLK42 / LRCLK2 / DIN41 |
| USB | ESP32-S3 native USB Serial/JTAG, GPIO19/20 reserved |
| SD | SDMMC 1-bit CLK12 / CMD11 / D0 13, verified read-only ROM loading; DAT3 GPIO10 pulled high |

Do not use GPIO35–37: occupied by octal PSRAM. Do not drive GPIO38 as a touch reset: references disagree and board documentation identifies it as LCD TE. No touch reset is used here.

## Driver choice and rotation

Pinned Arduino_GFX 1.5.9 contains the AXS15231B initialization table and QSPI transport. No LVGL. There was no existing driver or firmware in this repository when work started.

The LCD stays in native portrait rotation 0. The SMS picture is scaled from 256 × 192 to 426 × 320 within a logical 480 × 320 landscape view, preserving 4:3 aspect. A tiled conversion uses an internal 49 KB source cache and an indexed PSRAM framebuffer, then transfers a complete native 320 × 480 frame in 4,096-pixel QSPI blocks. Triple source buffers and explicit ownership keep emulation independent of display transfer. The display task yields after each frame.

Touch uses the inverse mapping: logical `(x,y)=(raw_y,319-raw_x)`. This orientation has been verified on the attached board and is fixed for this installation.

Touch packets use the AXS read command and 12-bit coordinates. Invalid coordinates and I²C failures are ignored; brief bus failures retain the last valid press for up to 70 ms. Touch is polled between frames. The current orientation is fixed for this board installation, not auto-rotated.

## References

- [Arduino_GFX AXS15231B implementation, pinned version](https://github.com/moononournation/Arduino_GFX/tree/v1.5.9/src)
- [JC3248W535 driver and pin references](https://github.com/me-processware/JC3248W535-Driver)
- [Board hardware notes transcribed from schematic/vendor demo](https://github.com/victorbvalle/esphome-ha-panel/blob/main/docs/en/hardware.md)

Pin references are supporting evidence; physical checks and their limits are recorded in [VALIDATION.md](VALIDATION.md). The physical screen was reviewed through the MacBook FaceTime camera using `imagesnap` with user authorization. The camera turns off after each snapshot.

## Storage and USB

The SD driver explicitly selects 1-bit SDMMC pins rather than the ESP32-S3 defaults. It opens directories and ROMs read-only, with automatic formatting disabled. ROMs load entirely into PSRAM before emulation.

`partitions.csv` retains the previously installed layout. Ordinary application uploads leave the reserved LittleFS partition intact; the emulator does not mount it. Settings use the `rtype-sms` NVS namespace. Do not erase flash or upload a filesystem image to install this application.

Use `scripts/check_board.py` for serial observations. It leaves USB DTR/RTS lines untouched and disables hang-up-on-close; ordinary monitor line toggles previously caused board resets.

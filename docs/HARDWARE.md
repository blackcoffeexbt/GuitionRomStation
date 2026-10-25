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
| SD | SDMMC 1-bit CLK12 / CMD11 / D0 13, unused in Phase 1 |

Do not use GPIO35–37: occupied by octal PSRAM. Do not drive GPIO38 as a touch reset: references disagree and board documentation identifies it as LCD TE. No touch reset is used here.

## Driver choice and rotation

Pinned Arduino_GFX 1.5.9 contains the AXS15231B initialization table and QSPI transport. No LVGL. There was no existing driver or firmware in this repository when work started.

The LCD stays in native portrait rotation0. The custom renderer uses a 480 × 320 logical framebuffer. The logical framebuffer prefers internal SRAM, with a slower PSRAM fallback. The drawing code clears all pixels each frame. Presentation byte-swaps the frame in place and calls the library's rotated scanline transfer, producing native `(px,py)=(319-y,x)` while overlapping scanline gathering with DMA. This consumes the buffer; no redundant conversion back is needed before the next complete redraw. A PSRAM buffer is reserved for benchmarking full and partial transfers. A complete 320 × 480 image is transferred in one controller window. This avoids unreliable rotated/narrow controller windows. Touch is mapped inversely: logical `(x,y)=(raw_y,319-raw_x)`; validate on the physical board, especially if the panel is mounted the other way up.

Touch packets use the AXS read command and 12-bit coordinates. Invalid coordinates and I²C failures are ignored; brief bus failures retain the last valid press for up to 70 ms. Touch is polled between frames. The current orientation is fixed for this board installation, not auto-rotated.

## References

- [Arduino_GFX AXS15231B implementation, pinned version](https://github.com/moononournation/Arduino_GFX/tree/v1.5.9/src)
- [JC3248W535 driver and pin references](https://github.com/me-processware/JC3248W535-Driver)
- [Board hardware notes transcribed from schematic/vendor demo](https://github.com/victorbvalle/esphome-ha-panel/blob/main/docs/en/hardware.md)

Pin references are supporting evidence, not a substitute for the actual runtime results in PERFORMANCE.md. The physical screen is reviewed through the MacBook FaceTime camera using `imagesnap`, with explicit user authorization. The camera turns off after each snapshot.

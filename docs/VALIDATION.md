# Hardware validation

The user played the original ROM and confirmed movement, charge/shot, Force pod and rapid fire controls. Early short checks exposed two integration issues: a slow PSRAM transpose and USB monitor line toggles that reset the chip. The monitor produced the explicit boot-ROM cause `USB_UART_CHIP_RESET`. `scripts/board_serial.py` avoids DTR/RTS ioctls and clears hang-up-on-close; a completed 480-frame counter survived closing and reopening it, confirming the monitor fix.

The display task also explicitly yields after each transfer so it cannot starve the idle task when frames remain queued. Scaling uses an internal 49 KB source cache, tiled packed 32-bit stores to the rotated PSRAM framebuffer, and 4,096-pixel QSPI transfer blocks. Host tests verify every scaled pixel and guard bytes.

The final eight-second, 480-frame intro diagnostic completed without reset or audio write errors. Full-window samples measured 59.9–60.1 emulated FPS and 27.9–28.3 displayed FPS. Scaling took about 8–10 ms and display transfer about 25–27 ms. Final samples were taken with the controller disconnected; an earlier eight-second diagnostic completed with the actual Xbox linked, before the final copy optimisation. Free internal heap settled near 107 KB and free PSRAM was about 8.04 MB. These are short observations, not a long endurance certification. The user subsequently confirmed stable manual gameplay with sound after the monitor fix.

For a brief observation using the no-reset connection:

```sh
python3 scripts/check_board.py --seconds 20
# Optional one-shot eight-second intro diagnostic; automatically returns to pause:
python3 scripts/check_board.py --seconds 12 --bench
```

`bench` is an explicit USB diagnostic, only accepted after a ROM has loaded and is paused outside the library. It resets the cartridge, supplies no input, runs 480 real-time frames, then pauses. It does not simulate a connected Xbox or run an endless play loop. Ordinary gameplay still freezes immediately on disconnection.

The host core also passed 7,200 emulated frames with varied movement, hold fire and Force presses under AddressSanitizer. This covers two minutes of *simulated* play and does not substitute for a physical-board run. No long automated test is scheduled.

## Controller-latency revision — installed

After the user reported lag, the display pipeline was changed to release its source slot immediately after conversion, before LCD transfer. The producer can replace stale queued frames, and every emulated frame is now eligible for presentation. The former design held one source through transfer and could strand a queued old frame while the producer had no free slot. A portable mailbox test checks freshness and concurrent ownership; ThreadSanitizer passes without torn frames/data races.

Audio writes now run in their own task behind a single latest-frame queue, removing blocking I²S writes from input polling. DMA capacity is reduced from about 70 ms to about 17 ms. The stick's digital activation threshold is lower, touch polling is less frequent during flight, and controller state is sampled again immediately before emulation. New diagnostics report controller polling gap and frame publication-to-presentation age; neither is a measurement of radio-to-photon latency.

Host tests and the firmware build pass. Flashing was initially deferred, then the user reattached the board and requested installation and testing. The latency changes are installed with the SD revision below. The source of reduced buffering is tested; subjective input responsiveness remains awaiting user comparison.

## SD revision — installed and short-tested

The boot library scans `/roms/sms/` with a read-only card-root fallback, supports touch/Xbox paging, and validates and loads ROMs into PSRAM. Files close before emulation resumes. A failed new load preserves the old cartridge; switching resets CPU, RAM and PSG and releases the old image. Tests cover 40 cartridge switches, interrupted reads, allocation failure, copier headers, 8 KB mirroring, header/console validation and browser navigation. See [SD_ROMS.md](../SD_ROMS.md) for formats, limitations and the now-verified SDMMC mapping.

The user’s card mounted and all three files reached gameplay: Alex Kidd (107 ms load), Sonic (183 ms) and R-Type (348 ms). Webcam images show the actual game worlds. The two `[BIOS]` built-in-game dumps initially stalled or reported a software error; implementing BIOS memory-slot control, empty external slots and fixed CPU vectors resolved both. ROM files were not patched. Host fixtures cover this memory behaviour.

A validation-only build supplied simulated controller reports through the normal gameplay input interface. For each game it exercised library selection, movement, both native buttons, rapid fire, disconnect freeze, held-A reconnect protection, fresh-press resume and Menu pause. All passed, with no resets or audio-write errors. Actual Bluetooth was disconnected during these tests, so physical-radio latency is not measured. The final normal Xbox build is restored, without simulated control commands. Full-running samples measured about 60 emulated FPS and 25.5–29.3 presented FPS; internal heap remained 101,024 bytes across the switches. No SD files were written and no automated test remains running.

The explicit short development script `scripts/test_controller_live.py` requires the validation build and does not run at boot. See [SD_ROMS.md](../SD_ROMS.md) for compatibility limits.

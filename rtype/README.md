# R-Type — Master System on Guition ESP32-S3

Separate project on branch `rtype-sms`, using the SMSPlus core from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu). This runs the user's Master System cartridge image, rather than recreating its levels in Starbreak.

## Play

- Left stick or D-pad: move.
- A: shot / hold to charge, as implemented by the original game.
- B: launch/recall the Force pod, as implemented by the original game.
- X: 10 Hz rapid-fire button.
- Menu: pause and show controls. Release A, then press A to resume after reconnect.
- Y while paused: reset the cartridge.
- Touch: pause/menu/resume and mute. No touch flight controls.

Wake the Xbox, press A to leave the emulator's menu, then press A at the game's own title screen. Pairing keys are reused; hold the controller's Pair button if necessary. A lost Bluetooth link immediately clears input and freezes emulation. Pause confirmation requires a fresh released report before A can resume.

The SMS's 256×192 image is scaled to 426×320, preserving 4:3 aspect with narrow side borders. The emulator targets 60 Hz; fresh frames are offered to the independent display task, which replaces stale queued frames. Triple buffers have explicit ownership so the emulation thread never modifies a frame being displayed. Audio is real SN76496 PSG output sent to the board's I²S amplifier at 22,080 Hz.

## Build

Supply your own local `.sms` image; no game files are downloaded or committed:

```sh
python3 rtype/scripts/bootstrap.py --rom '/full/path/to/game.sms'
pio run -d rtype
pio run -d rtype -t upload --upload-port /dev/cu.usbmodem101
```

The bootstrap imports a single `.sms` under the local `type-r/` directory when `--rom` is omitted. ROM inputs, build products and screenshots are ignored by Git. The toolchain is shared with Starbreak's isolated dependency directory: Espressif32 6.10.0, official Arduino/Bluepad32 4.1.0, Arduino_GFX 1.5.9. The core is vendored and its origin is pinned in [THIRD_PARTY.md](THIRD_PARTY.md).

The flash partition table is identical to the previous games. Only the app firmware is uploaded; the farm's LittleFS partition and SD card are not accessed. Mute uses its own `rtype-sms` NVS namespace. This port does not implement save states or persistent cartridge SRAM. R-Type's native game rules, difficulty, checkpoints and progression are provided by the ROM.

To return to Starbreak's last complete build, use commit `4ae5d85` and its project directory. Later user commit `e6a0c33` preserves unfinished arsenal work; it is not the last buildable Starbreak release. Little Wilds remains in the root project and on `main`; its instructions are preserved in [the archived README](../docs/LITTLE_WILDS.md).

## Tests and measurements

```sh
./rtype/scripts/test_host.sh
```

Host checks boot the supplied ROM, run 600 actual emulated frames, verify multicolour video and nonzero PSG audio, check bank-register reset and render skipping, and test native button mapping, charge/rapid-fire handling, disconnect freeze and resume confirmation. Host screenshots are local `.build/*.ppm` files. A host run is not a physical-board gameplay check.

USB diagnostics: `status`, `pair`, `pause`, `resume`, `reset`, `mute`. Every three seconds the board reports emulation FPS, presented FPS, simulation/scale/transfer times, memory, controller packets and audio write errors. No automated long endurance run is enabled.

## Hardware observations

The user played the original ROM and confirmed movement, charge/shot, Force pod and rapid fire controls. Early short checks exposed two integration issues: a slow PSRAM transpose and USB monitor line toggles that reset the chip. The monitor produced the explicit boot-ROM cause `USB_UART_CHIP_RESET`. `scripts/board_serial.py` avoids DTR/RTS ioctls and clears hang-up-on-close; a completed 480-frame counter survived closing and reopening it, confirming the monitor fix.

The display task also explicitly yields after each transfer so it cannot starve the idle task when frames remain queued. Scaling uses an internal 49 KB source cache, tiled packed 32-bit stores to the rotated PSRAM framebuffer, and 4,096-pixel QSPI transfer blocks. Host tests verify every scaled pixel and guard bytes.

The final eight-second, 480-frame intro diagnostic completed without reset or audio write errors. Full-window samples measured 59.9–60.1 emulated FPS and 27.9–28.3 displayed FPS. Scaling took about 8–10 ms and display transfer about 25–27 ms. Final samples were taken with the controller disconnected; an earlier eight-second diagnostic completed with the actual Xbox linked, before the final copy optimisation. Free internal heap settled near 107 KB and free PSRAM was about 8.04 MB. These are short observations, not a long endurance certification. The user subsequently confirmed stable manual gameplay with sound after the monitor fix.

For a brief observation using the no-reset connection:

```sh
python3 rtype/scripts/check_board.py --seconds 20
# Optional one-shot eight-second intro diagnostic; automatically returns to pause:
python3 rtype/scripts/check_board.py --seconds 12 --bench
```

`bench` is an explicit USB diagnostic, only accepted while paused. It resets the cartridge, supplies no input, runs 480 real-time frames, then pauses. It does not simulate a connected Xbox or run an endless play loop. Ordinary gameplay still freezes immediately on disconnection.

The host core also passed 7,200 emulated frames with varied movement, hold fire and Force presses under AddressSanitizer. This covers two minutes of *simulated* play and does not substitute for a physical-board run. No long automated test is scheduled.

## Controller-latency revision — built, not yet flashed

After the user reported lag, the display pipeline was changed to release its source slot immediately after conversion, before LCD transfer. The producer can replace stale queued frames, and every emulated frame is now eligible for presentation. The former design held one source through transfer and could strand a queued old frame while the producer had no free slot. A portable mailbox test checks freshness and concurrent ownership; ThreadSanitizer passes without torn frames/data races.

Audio writes now run in their own task behind a single latest-frame queue, removing blocking I²S writes from input polling. DMA capacity is reduced from about 70 ms to about 17 ms. The stick's digital activation threshold is lower, touch polling is less frequent during flight, and controller state is sampled again immediately before emulation. New diagnostics report controller polling gap and frame publication-to-presentation age; neither is a measurement of radio-to-photon latency.

Host tests and the firmware build pass. The USB device disappeared before upload, and the user explicitly deferred flashing. This revision is therefore **not installed or hardware-validated**. The board retains the previously confirmed stable build. No SD loader has been implemented and the SD card remains untouched.

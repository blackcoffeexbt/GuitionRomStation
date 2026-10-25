# Third-party source

The vendored `lib/smsplus` core comes from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu), revision `0d883ba029c479d057a8c35ea88b0f08c4bf36f8`. Its original upstream is SMSPlus by Charles MacDonald, adapted for ESP31/ESP32 by the upstream authors. Original notices remain in the files.

SMSPlus files carry GPL version 2 or later notices. The older Z80 CPU files have their own non-commercial/freeware terms in `z80.c`; those terms are retained. The upstream repository's top-level Apache license does not replace the individual emulator-file notices. `lib/smsplus/COPYING` contains GPL v2 text, sourced from the SMSPlus copy in Retro-Go revision `4ced120669750ca7228fd0414211430c1d923166`.

Local core changes: `shared.h` uses standard allocation declarations and an explicitly unsigned byte-read callback; `render.c` uses `uintptr_t` for alignment checks and removes console logging on normal cache replacement; `lut.h` removes a duplicate `const`. The CPU byte-read callback is implemented in the platform adapter and works with flash-mapped ROM on ESP32-S3. Emulation algorithms and lookup values are otherwise retained.

The Guition board configuration, AXS15231B transfer/orientation, touchscreen, font and Bluepad32 adapter derive from this repository's existing Little Wilds/Starbreak hardware work. New platform glue supplies synchronized triple buffering, tiled scaling, I²S PCM output and pause/reconnect controls.

The game ROM is supplied locally by the user. No R-Type ROM, source assets, screenshots, embedded firmware binary containing the ROM, or cartridge data is committed to Git. Runtime build/output directories and both `type-r/` and `rtype/roms/` are ignored.

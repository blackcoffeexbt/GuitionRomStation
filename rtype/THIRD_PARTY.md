# Third-party source

The vendored `lib/smsplus` core comes from [hi631/esp32-smsemu](https://github.com/hi631/esp32-smsemu), revision `0d883ba029c479d057a8c35ea88b0f08c4bf36f8`. Its original upstream is SMSPlus by Charles MacDonald, adapted for ESP31/ESP32 by the upstream authors. Original notices remain in the files.

SMSPlus files carry GPL version 2 or later notices. The older Z80 CPU files have their own non-commercial/freeware terms in `z80.c`; those terms are retained. The upstream repository's top-level Apache license does not replace the individual emulator-file notices. `lib/smsplus/COPYING` contains GPL v2 text, sourced from the SMSPlus copy in Retro-Go revision `4ced120669750ca7228fd0414211430c1d923166`.

Local core changes: `shared.h` uses standard allocation declarations and an explicitly unsigned byte-read callback; `render.c` uses `uintptr_t` for alignment checks and removes console logging on normal cache replacement; `lut.h` removes a duplicate `const`. ROM storage is owned by the platform adapter in PSRAM. `sms.c`, `sms.h` and `system.h` add BIOS-slot identification, port 0x3E memory control, empty-slot reads and BIOS-specific reset banking. `z80.c` preserves the fixed first 1 KB CPU-vector mapping when bank zero changes. These additions allow the supplied built-in-game BIOS dumps to boot without altering their data; original notices are retained. Other emulation algorithms and lookup values are retained.

The Guition board configuration, AXS15231B transfer/orientation, touchscreen, font and Bluepad32 adapter derive from this repository's existing Little Wilds/Starbreak hardware work. New platform glue supplies synchronized triple buffering, tiled scaling, I²S PCM output and pause/reconnect controls.

The game ROM is supplied locally by the user. No R-Type ROM, source assets, screenshots, embedded firmware binary containing the ROM, or cartridge data is committed to Git. Runtime build/output directories and both `type-r/` and `rtype/roms/` are ignored.

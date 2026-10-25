# Little Wilds placeholder art

All PNG sprites in `source/` were drawn procedurally for this project by `scripts/make_placeholders.py`, using shaded polygons, ellipses and a consistent ground-contact anchor. These are original placeholders, not third-party game art. Source PNGs preserve transparency and can be replaced individually.

`generated/Sprites.cpp` contains host-endian uint16_t RGB565 values with magenta `0xf81f` transparency. The converter also supports a standalone little-endian binary export. Firmware uses arrays in flash and two reusable RGB565 runtime buffers in PSRAM. It does not decode PNGs during gameplay.

`Font.h` is a small raster glyph atlas generated from the system Arial font for the prototype UI. It is not a redistributable font file. Replace the atlas with an openly licensed UI font before public distribution; the font generator is macOS-specific, while existing generated assets build on any PlatformIO host.

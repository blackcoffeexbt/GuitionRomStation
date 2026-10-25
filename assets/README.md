# Little Wilds placeholder art

All PNG sprites in `source/` were drawn procedurally for this project by `scripts/make_placeholders.py`, using shaded polygons, ellipses and a consistent ground-contact anchor. These are original placeholders, not third-party game art. Source PNGs preserve transparency and can be replaced individually.

`generated/Sprites.cpp` contains host-endian uint16_t RGB565 values with magenta `0xf81f` transparency. The converter also supports a standalone little-endian binary export. Firmware uses arrays in flash and a fast internal RGB565 drawing buffer plus a PSRAM benchmark buffer. It does not decode PNGs during gameplay.

`Font.h` is a small raster glyph atlas generated from the included DejaVu Sans font. The original font and its licence are in `fonts/`. The asset pipeline is portable; existing generated assets build without Python.

#pragma once
// Guition JC3248W535 / C / EN, ESP32-S3 N16R8. See docs/HARDWARE.md.
namespace board {
constexpr int LcdCs = 45, LcdClock = 47, LcdD0 = 21, LcdD1 = 48, LcdD2 = 40, LcdD3 = 39;
constexpr int Backlight = 1, TouchSda = 4, TouchScl = 8, TouchInterrupt = 3;
constexpr int TouchAddress = 0x3b;
constexpr int AudioBclk = 42, AudioLrclk = 2, AudioData = 41;
// Vendor SDMMC 1-bit mapping; physical mount remains to be checked on this unit.
constexpr int SdClock = 12, SdCommand = 11, SdData0 = 13, SdData3 = 10;
constexpr int ScreenWidth = 480, ScreenHeight = 320;
// Software rotation: native portrait (px,py) = (319-y,x).
constexpr int TouchRotation = 1;
constexpr int DisplayClockHz = 40000000;
} // namespace board

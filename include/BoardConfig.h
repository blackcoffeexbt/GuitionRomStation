#pragma once
// Guition JC3248W535 / C / EN, ESP32-S3 N16R8. See docs/HARDWARE.md.
namespace board {
constexpr int LcdCs=45, LcdClock=47, LcdD0=21, LcdD1=48, LcdD2=40, LcdD3=39;
constexpr int Backlight=1, TouchSda=4, TouchScl=8, TouchInterrupt=3;
constexpr int TouchAddress=0x3b;
constexpr int AudioBclk=42, AudioLrclk=2, AudioData=41;
constexpr int ScreenWidth=480, ScreenHeight=320;
// Software rotation: native portrait (px,py) = (319-y,x).
constexpr int TouchRotation=1;
constexpr int DisplayClockHz=40000000;
}

#include "Display.h"
#include "BoardConfig.h"
#include <esp_heap_caps.h>
bool Display::begin(){
 if(!psramFound()){Serial.println("FATAL: OPI PSRAM unavailable");return false;}
 logical=(uint16_t*)heap_caps_malloc(480*320*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
 physical=(uint16_t*)heap_caps_malloc(480*320*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
 if(!logical||!physical){Serial.println("FATAL: framebuffers unavailable");return false;}
 pinMode(board::Backlight,OUTPUT);digitalWrite(board::Backlight,LOW);
 bus=new Arduino_ESP32QSPI(board::LcdCs,board::LcdClock,board::LcdD0,board::LcdD1,board::LcdD2,board::LcdD3);
 panel=new Arduino_AXS15231B(bus,GFX_NOT_DEFINED,0,false,320,480);
 if(!panel->begin(board::DisplayClockHz))return false;
 std::fill(logical,logical+480*320,uint16_t(0));present();
 digitalWrite(board::Backlight,HIGH);return true;
}
void Display::present(){
 auto start=micros();
 // Portrait stays native on the controller. Rotate the complete software image.
 // Blocked traversal improves locality in both PSRAM buffers.
 for(int by=0;by<320;by+=16)for(int bx=0;bx<480;bx+=16)
  for(int y=by;y<by+16;y++)for(int x=bx;x<bx+16;x++)physical[x*320+319-y]=logical[y*480+x];
 rotationUs=micros()-start;start=micros();
 panel->draw16bitRGBBitmap(0,0,physical,320,480);transferUs=micros()-start;
}
void Display::benchmark(){
 // Native full-width strips avoid the controller's unreliable narrow windows.
 uint32_t start=micros();for(int n=0;n<5;n++)panel->draw16bitRGBBitmap(0,0,physical,320,480);
 auto full=(micros()-start)/5;
 start=micros();for(int n=0;n<5;n++)for(int y=0;y<480;y+=32)panel->draw16bitRGBBitmap(0,y,physical+y*320,320,32);
 auto strips=(micros()-start)/5;
 Serial.printf("BENCH full_us=%u strips32_us=%u qspi_hz=%d strategy=full-frame\n",full,strips,board::DisplayClockHz);
}

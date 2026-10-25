#include "Touch.h"
#include "BoardConfig.h"
#include <Arduino.h>
#include <Wire.h>
bool Touch::begin(){Wire.begin(board::TouchSda,board::TouchScl,400000);Wire.setTimeOut(5);Wire.beginTransmission(board::TouchAddress);ready=Wire.endTransmission()==0;Serial.printf("TOUCH addr=0x3b found=%d rotation=%d\n",ready,board::TouchRotation);return ready;}
bool Touch::read(wilds::Vec2& p){
 if(!ready)return false;
 constexpr uint8_t request[11]={0xb5,0xab,0xa5,0x5a,0,0,0,8,0,0,0};uint8_t data[8];
 Wire.beginTransmission(board::TouchAddress);Wire.write(request,sizeof(request));
 bool ok=Wire.endTransmission()==0;
 if(ok){delayMicroseconds(50);ok=Wire.requestFrom(uint8_t(board::TouchAddress),uint8_t(8))==8;}
 if(ok)for(auto&b:data)b=Wire.read();
 if(!ok){while(Wire.available())Wire.read();p=lastPoint;return lastDown&&millis()-lastValid<70;}
 bool down=data[0]==0&&data[1]==1;
 if(down){int x=((data[2]&15)<<8)|data[3],y=((data[4]&15)<<8)|data[5];
  if(x>=320||y>=480){p=lastPoint;return lastDown&&millis()-lastValid<70;}
  p={float(y),float(319-x)};lastPoint=p;lastValid=millis();
 }else p=lastPoint;
 if(down&&!lastDown)Serial.printf("TOUCH down screen=(%.0f,%.0f) raw=(%d,%d)\n",p.x,p.y,((data[2]&15)<<8)|data[3],((data[4]&15)<<8)|data[5]);
 lastDown=down;return down;
}

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <new>
#include "core/Game.h"
#include "render/Renderer.h"
#include "hardware/Display.h"
#include "hardware/Touch.h"
#include "hardware/Audio.h"
using namespace wilds;
Display display;Touch touch;Audio audio;Input input;
Game* game=nullptr;Renderer* renderer=nullptr;
uint32_t lastFrame=0,lastInput=0,lastReport=0,frames=0,renderUs=0;float accumulator=0;
bool demo=false;int demoStep=0;uint32_t nextDemo=0;
char command[80];size_t commandLength=0;
void status(){Serial.printf("STATE pos=(%.2f,%.2f) settled=(%d,%d) path=%d/%d taps=%u follow=%d objects=%d time=%.2f\n",game->player.position.x,game->player.position.y,game->player.settled.x,game->player.settled.y,game->player.next,game->player.path.count,game->taps,game->camera.following,game->world.objectCount,game->time);}
void serialCommands(){
 while(Serial.available()){char c=Serial.read();if(c=='\r')continue;if(c=='\n'){
  command[commandLength]=0;int x,y;
  if(sscanf(command,"walk %d %d",&x,&y)==2){demo=false;game->walkTo({int16_t(x),int16_t(y)});status();}
  else if(sscanf(command,"tap %d %d",&x,&y)==2){game->gesture({GestureKind::Tap,{float(x),float(y)},{}});status();}
  else if(!strcmp(command,"demo")){demo=!demo;nextDemo=millis();Serial.printf("DEMO %d\n",demo);}
  else if(!strcmp(command,"status"))status();
  else if(!strcmp(command,"bench"))display.benchmark();
  else if(!strcmp(command,"reboot"))ESP.restart();
  else if(!strcmp(command,"mute")){game->muted=!game->muted;Serial.printf("MUTE %d\n",game->muted);}
  else Serial.println("COMMANDS: status | walk x y | tap x y | demo | bench | mute | reboot");
  commandLength=0;
 }else if(commandLength<sizeof(command)-1)command[commandLength++]=c;}
}
void setup(){
 Serial.begin(115200);delay(800);
 Serial.printf("\nLITTLE WILDS / Phase 1 / %s %s\n",__DATE__,__TIME__);
 Serial.printf("HARDWARE chip=%s rev=%d cpu=%u flash=%u psram=%u\n",ESP.getChipModel(),ESP.getChipRevision(),ESP.getCpuFreqMHz(),ESP.getFlashChipSize(),ESP.getPsramSize());
 auto* gm=heap_caps_malloc(sizeof(Game),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);auto* rm=heap_caps_malloc(sizeof(Renderer),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
 if(!gm||!rm||!display.begin()){Serial.println("FATAL startup failed");for(;;)delay(1000);}
 game=new(gm)Game();renderer=new(rm)Renderer();game->begin();touch.begin();
 Serial.printf("AUDIO I2S ready=%d BCLK=42 LRCLK=2 DATA=41\n",audio.begin());
 auto fsStart=micros();bool fs=LittleFS.begin(true,"/littlefs",10,"littlefs");Serial.printf("STORAGE LittleFS=%d mount_us=%u bytes=%u (Phase 1: world saves not enabled)\n",fs,micros()-fsStart,fs?LittleFS.totalBytes():0);
 display.benchmark();Surface surface(display.framebuffer());renderer->draw(surface,*game);display.present();
 audio.play(SoundEvent::Arrive,false);lastFrame=lastReport=millis();status();
}
void loop(){
 uint32_t now=millis();serialCommands();
 if(now-lastInput>=8){lastInput=now;Vec2 point;bool down=touch.read(point);auto gesture=input.sample(down,point,now);
  if(gesture.kind!=GestureKind::None){if(gesture.kind==GestureKind::Tap){demo=false;Serial.printf("GESTURE tap=(%.0f,%.0f)\n",gesture.point.x,gesture.point.y);}game->gesture(gesture);}}
 if(demo&&int32_t(now-nextDemo)>=0){constexpr TilePos tour[]={{35,34},{36,35},{34,37},{32,34},{30,34},{33,33},{34,34}};game->walkTo(tour[demoStep++%7]);nextDemo=now+3500;}
 if(now-lastFrame<33){delay(1);return;}
 float dt=std::min(.15f,float(now-lastFrame)/1000.f);lastFrame=now;accumulator+=dt;
 while(accumulator>=1.f/60.f){game->update(1.f/60.f);accumulator-=1.f/60.f;}
 audio.play(game->sound,game->muted);game->sound=SoundEvent::None;
 auto start=micros();Surface surface(display.framebuffer());renderer->draw(surface,*game);renderUs=micros()-start;display.present();++frames;
 if(now-lastReport>=3000){float fps=frames*1000.f/(now-lastReport);renderer->fps=fps;Serial.printf("PERF fps=%.1f render_us=%u rotate_us=%u transfer_us=%u heap=%u psram=%u sprites=%d tiles=%d taps=%u\n",fps,renderUs,display.rotationUs,display.transferUs,ESP.getFreeHeap(),ESP.getFreePsram(),renderer->renderedSprites,renderer->renderedTiles,game->taps);frames=0;lastReport=now;}
}

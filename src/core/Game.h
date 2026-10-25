#pragma once
#include "Player.h"
#include "Camera.h"
#include "Input.h"
namespace wilds {
enum class SoundEvent {None,Tap,Arrive,Blocked};
class Game {
 Pathfinding pathfinder;
 bool route(TilePos p);
public:
 World world;Player player;Camera camera;
 float time=0,noticeTimer=0,destinationTimer=0;
 const char* notice="Tap a clearing to wander";
 TilePos destination{34,34};bool guide=false,muted=false;uint32_t taps=0;SoundEvent sound=SoundEvent::None;
 void begin();void update(float dt);void gesture(const Gesture& g);void walkTo(TilePos p);
 void message(const char* text,float duration=3){notice=text;noticeTimer=duration;}
};
}

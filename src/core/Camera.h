#pragma once
#include "Types.h"
namespace wilds {
class Camera {
public:
 Vec2 offset{0,0};bool following=true;
 Vec2 screen(Vec2 world)const{auto p=project(world);return {p.x+offset.x,p.y+offset.y};}
 TilePos tile(Vec2 screenPoint)const{return pick({screenPoint.x-offset.x,screenPoint.y-offset.y});}
 void center(Vec2 p){auto a=project(p);offset={Width*.5f-a.x,150.f-a.y};clamp();}
 void update(Vec2 p,float dt){if(!following)return;auto a=project(p);float ease=std::min(1.f,dt*7.f);offset.x+=(Width*.5f-a.x-offset.x)*ease;offset.y+=(150.f-a.y-offset.y)*ease;clamp();}
 void pan(float dx,float dy){following=false;offset.x+=dx;offset.y+=dy;clamp();}
 void clamp(){
  // Clamp the camera centre in world space, including the slanted map edges.
  auto p=unproject({Width*.5f-offset.x,150.f-offset.y});
  p.x=std::clamp(p.x,4.f,59.f);p.y=std::clamp(p.y,4.f,59.f);
  auto a=project(p);offset={Width*.5f-a.x,150.f-a.y};
 }
};
}

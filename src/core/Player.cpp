#include "Player.h"
namespace wilds {
void Player::update(float dt){
 if(!moving()){animation=0;return;}
 float budget=dt*3.f;animation+=dt*9.f;
 while(budget>0&&moving()){
  auto p=path.points[next];Vec2 goal{float(p.x),float(p.y)};float d=distance(position,goal);
  float dx=goal.x-position.x,dy=goal.y-position.y;
  if(std::abs(dx)>std::abs(dy))direction=dx>0?0:2;else direction=dy>0?1:3;
  if(d<=budget){position=goal;settled=p;++next;budget-=d;if(hasPending)break;}
  else{position.x+=dx/d*budget;position.y+=dy/d*budget;budget=0;}
 }
}
}

#pragma once
#include "Surface.h"
#include "Assets.h"
#include "../core/Game.h"
namespace wilds {
class Renderer {
 struct DrawItem {float depth;int id;int x,y;};
 std::array<DrawItem,World::MaxObjects+1> drawList;
 void toolbar(Surface& s,const Game& game);void leaf(Surface& s,int x,int y,uint16_t c);
public:
 int renderedSprites=0,renderedTiles=0;float fps=0;
 void draw(Surface& surface,const Game& game);
};
}

#pragma once
#include "Sprites.h"
#include "../core/World.h"
namespace wilds {
const Sprite& objectSprite(ObjectKind kind,int variant);
const Sprite& playerSprite(int direction,int frame);
}

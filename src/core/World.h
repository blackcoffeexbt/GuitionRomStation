#pragma once
#include "Types.h"
#include <array>
namespace wilds {
enum class Terrain:uint8_t {Grass,Dirt,Path,Water,Soil};
enum class ObjectKind:uint8_t {Oak,Pine,Rock,Bush,Flowers,Cottage,Log,Reeds};
struct Tile { Terrain terrain=Terrain::Grass; uint8_t variant=0; int16_t object=-1; };
struct Object { TilePos tile; ObjectKind kind; uint8_t variant=0; };
class World {
public:
 static constexpr int MaxObjects=700;
 std::array<Tile,TileCount> tiles{};
 std::array<Object,MaxObjects> objects{};
 int objectCount=0;
 void generate(uint32_t seed=75621);
 bool walkable(TilePos p)const;
 bool addObject(TilePos p,ObjectKind kind,uint8_t variant=0);
 const Tile& at(TilePos p)const{return tiles[index(p)];}
};
}

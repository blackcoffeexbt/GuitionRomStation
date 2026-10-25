#include "Pathfinding.h"
#include <limits>
namespace wilds {
int Pathfinding::score(int id)const{return costs[id]+10*(std::abs(id%MapSize-target.x)+std::abs(id/MapSize-target.y));}
void Pathfinding::rise(int s){while(s>0){int p=(s-1)/2;if(score(heap[p])<=score(heap[s]))break;std::swap(heap[p],heap[s]);positions[heap[p]]=p;positions[heap[s]]=s;s=p;}}
void Pathfinding::push(int id){heap[heapSize]=id;positions[id]=heapSize;rise(heapSize++);}
int Pathfinding::pop(){int id=heap[0];--heapSize;if(heapSize){heap[0]=heap[heapSize];positions[heap[0]]=0;int s=0;while(s*2+1<heapSize){int n=s*2+1;if(n+1<heapSize&&score(heap[n+1])<score(heap[n]))++n;if(score(heap[s])<=score(heap[n]))break;std::swap(heap[s],heap[n]);positions[heap[s]]=s;positions[heap[n]]=n;s=n;}}return id;}
bool Pathfinding::find(const World& w,TilePos from,TilePos to,Path& out){
 out.count=0;if(!inBounds(from)||!w.walkable(from)||!w.walkable(to))return false;
 costs.fill(UINT16_MAX);parent.fill(-1);state.fill(0);heapSize=0;target=to;
 int first=index(from),goal=index(to);costs[first]=0;push(first);state[first]=1;
 constexpr int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
 while(heapSize){int id=pop();if(id==goal){int cur=id;while(cur!=first){out.points[out.count++]={int16_t(cur%MapSize),int16_t(cur/MapSize)};cur=parent[cur];}std::reverse(out.points.begin(),out.points.begin()+out.count);return true;}state[id]=2;
  for(int d=0;d<4;d++){TilePos p{int16_t(id%MapSize+dx[d]),int16_t(id/MapSize+dy[d])};if(!w.walkable(p))continue;int n=index(p);if(state[n]==2)continue;int c=costs[id]+10;if(c>=costs[n])continue;costs[n]=c;parent[n]=id;if(state[n]==1)rise(positions[n]);else{push(n);state[n]=1;}}
 }
 return false;
}
bool Pathfinding::adjacent(const World& w,TilePos from,TilePos obj,Path& out){
 constexpr int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
 // Try nearest accessible side first; search handles unreachable sides.
 std::array<int,4> dirs{0,1,2,3};std::sort(dirs.begin(),dirs.end(),[&](int a,int b){return std::abs(obj.x+dx[a]-from.x)+std::abs(obj.y+dy[a]-from.y)<std::abs(obj.x+dx[b]-from.x)+std::abs(obj.y+dy[b]-from.y);});
 for(int d:dirs)if(find(w,from,{int16_t(obj.x+dx[d]),int16_t(obj.y+dy[d])},out))return true;
 out.count=0;return false;
}
}

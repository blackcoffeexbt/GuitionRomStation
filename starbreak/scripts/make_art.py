#!/usr/bin/env python3
"""Original compact arcade pixel sprites; PNG sources and native-layout RGB332 data."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'assets/source';OUT.mkdir(exist_ok=True)
COL={'edge':'#193751','hull':'#4ca7bc','light':'#c5f5f0','glass':'#3265a4','orange':'#ffc16b','red':'#d66d85','pink':'#f0a6b8','purple':'#8974bd','shade':'#424f81'}
def make(name,size,parts):
 im=Image.new('RGBA',size);d=ImageDraw.Draw(im)
 for kind,shape,col in parts:getattr(d,kind)(shape,fill=COL.get(col,col))
 im.save(OUT/(name+'.png'))
make('player',(26,16),[
 ('polygon',[(1,0),(8,0),(13,4),(20,5),(25,8),(20,10),(13,11),(8,15),(1,15),(5,9)],'edge'),
 ('polygon',[(3,1),(7,1),(12,5),(21,6),(24,8),(20,9),(12,10),(7,14),(3,14),(7,9),(7,6)],'hull'),
 ('polygon',[(7,6),(17,6),(21,8),(17,10),(7,9)],'light'),('rectangle',(10,6,16,8),'glass'),('rectangle',(1,6,5,9),'orange')])
make('scout',(18,14),[
 ('polygon',[(17,0),(9,0),(1,5),(0,7),(1,9),(9,13),(17,13),(13,8)],'edge'),
 ('polygon',[(16,1),(10,1),(2,6),(1,7),(3,8),(10,12),(16,12),(11,8),(8,7),(12,5)],'red'),
 ('rectangle',(3,6,9,8),'pink'),('rectangle',(10,5,13,8),'shade')])
make('fighter',(24,18),[
 ('polygon',[(22,0),(11,1),(9,5),(2,6),(0,9),(3,11),(9,12),(12,16),(23,17),(18,9)],'edge'),
 ('polygon',[(21,1),(13,2),(11,6),(3,7),(2,9),(5,10),(11,11),(14,15),(21,16),(16,10),(10,9),(17,7)],'purple'),
 ('rectangle',(4,7,13,10),'pink'),('rectangle',(10,7,16,8),'light')])
make('gunship',(36,26),[
 ('polygon',[(8,0),(30,0),(35,6),(34,19),(29,25),(8,25),(8,20),(1,17),(0,9),(8,5)],'edge'),
 ('rectangle',(10,2,29,23),'shade'),('rectangle',(8,6,32,19),'purple'),
 ('rectangle',(2,9,25,16),'red'),('rectangle',(3,10,13,14),'orange'),
 ('rectangle',(12,3,26,6),'light'),('rectangle',(12,20,26,22),'light'),
 ('rectangle',(21,9,29,16),'edge'),('rectangle',(31,9,35,15),'orange')])
make('asteroid',(24,24),[
 ('polygon',[(8,0),(17,1),(23,8),(21,18),(15,23),(5,21),(0,14),(2,5)],'edge'),
 ('polygon',[(8,2),(16,3),(21,9),(19,17),(14,21),(6,19),(2,13),(4,6)],'#938d9a'),
 ('polygon',[(8,2),(16,3),(14,10),(6,12),(4,6)],'#c2bcae'),
 ('rectangle',(6,7,10,11),'#696b81'),('rectangle',(14,13,18,17),'#696b81')])
make('cell',(16,16),[
 ('polygon',[(8,0),(15,8),(8,15),(0,8)],'edge'),('polygon',[(8,2),(13,8),(8,13),(2,8)],'orange'),
 ('rectangle',(6,4,9,11),'light'),('rectangle',(4,6,11,9),'light')])
make('boss',(88,84),[
 ('polygon',[(29,0),(70,1),(84,15),(87,39),(82,67),(67,83),(28,82),(23,69),(9,65),(4,53),(0,42),(4,28),(10,19),(23,15)],'edge'),
 ('polygon',[(31,3),(68,4),(80,17),(83,40),(78,65),(64,79),(31,78),(28,65),(14,61),(9,51),(5,42),(9,31),(15,23),(28,19)],'shade'),
 ('rectangle',(31,6,66,20),'purple'),('rectangle',(31,62,66,76),'purple'),
 ('rectangle',(12,28,62,56),'red'),('rectangle',(7,34,24,49),'orange'),
 ('rectangle',(27,26,56,58),'edge'),('rectangle',(32,31,51,53),'pink'),('rectangle',(37,36,46,48),'light'),
 ('rectangle',(62,24,77,60),'purple'),('rectangle',(69,29,81,36),'orange'),('rectangle',(69,48,81,55),'orange'),
 ('rectangle',(37,9,60,12),'light'),('rectangle',(37,69,60,72),'light')])
headers=['#pragma once','#include <cstdint>','namespace sb {','struct Sprite {const uint8_t* data; int w,h;};']
source=['#include "Sprites.h"','namespace sb {']
for p in sorted(OUT.glob('*.png')):
 im=Image.open(p).convert('RGBA');data=[]
 for x in range(im.width):
  for y in reversed(range(im.height)):
   r,g,b,a=im.getpixel((x,y));v=(r&224)|((g>>3)&28)|(b>>6)
   data.append(0 if a<128 else max(1,v))
 source.append('static const uint8_t '+p.stem+'_data[]={'+','.join(map(str,data))+'};')
 source.append(f'const Sprite {p.stem}={{{p.stem}_data,{im.width},{im.height}}};');headers.append('extern const Sprite '+p.stem+';')
headers.append('}');source.append('}');gen=ROOT/'assets/generated';gen.mkdir(exist_ok=True)
(gen/'Sprites.h').write_text('\n'.join(headers)+'\n');(gen/'Sprites.cpp').write_text('\n'.join(source)+'\n')
print('Generated compact original pixel sprites')

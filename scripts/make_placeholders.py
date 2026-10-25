#!/usr/bin/env python3
"""Original shaded diorama placeholders. Regenerate with Pillow, then convert_assets.py.
All sprites share a ground-contact anchor at (width/2, height-8).
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
import math, random
from convert_assets import convert_all
ROOT=Path(__file__).resolve().parents[1]; OUT=ROOT/'assets/source';OUT.mkdir(exist_ok=True)
S=3
class Art:
 def __init__(self,w,h):self.w=w;self.h=h;self.im=Image.new('RGBA',(w*S,h*S));self.d=ImageDraw.Draw(self.im)
 def poly(self,pts,c):self.d.polygon([(round(x*S),round(y*S)) for x,y in pts],fill=c)
 def ellipse(self,box,c):self.d.ellipse(tuple(round(v*S) for v in box),fill=c)
 def rect(self,box,c):self.d.rectangle(tuple(round(v*S) for v in box),fill=c)
 def line(self,pts,c,w=1):self.d.line([(round(x*S),round(y*S)) for x,y in pts],fill=c,width=round(w*S))
 def save(self,name):self.im.resize((self.w,self.h),Image.Resampling.LANCZOS).save(OUT/(name+'.png'))
 def shadow(self,x,y,rx,ry):self.ellipse((x-rx,y-ry,x+rx,y+ry),(42,68,45,65))
for variant in range(3):
 a=Art(88,112);a.shadow(47,104,29,7)
 a.poly([(39,104),(48,101),(48,64),(40,64)],'#765443');a.poly([(40,104),(44,102),(44,62),(40,63)],'#a37b54')
 a.poly([(42,86),(29,72),(31,69),(44,79)],'#92704b')
 colors=[('#326858','#4c8460','#7ba46b','#a3bc7a'),('#586b48','#788b4d','#9dae61','#bac77a'),('#426c55','#67965d','#90b36c','#c1cd88')][variant]
 # Broad faceted lobes, brighter upper-left and darker lower-right.
 lobes=[(43,36,29),(28,52,23),(58,52,24),(43,66,23),(42,25,21)]
 for x,y,r in lobes:
  a.poly([(x-r,y+2),(x-r*.8,y-r*.55),(x-r*.15,y-r),(x+r*.64,y-r*.72),(x+r,y),(x+r*.7,y+r*.68),(x,y+r*.8),(x-r*.8,y+r*.5)],colors[1])
  a.poly([(x-r,y+2),(x-r*.8,y-r*.55),(x-r*.15,y-r),(x+r*.64,y-r*.72),(x,y)],colors[2])
  a.poly([(x,y),(x+r*.64,y-r*.72),(x+r,y),(x+r*.7,y+r*.68),(x,y+r*.8)],colors[0])
  a.poly([(x-r*.8,y-r*.55),(x-r*.15,y-r),(x+r*.2,y-r*.85),(x-r*.25,y-r*.3)],colors[3])
 a.save('oak'+str(variant))
a=Art(76,116);a.shadow(41,108,25,6);a.rect((35,81,41,108),'#8f7354')
for y,r in [(81,32),(64,27),(48,22),(32,16)]:
 a.poly([(38,y-r),(38+r,y+12),(38,y+23),(38-r,y+9)],'#356b60')
 a.poly([(38,y-r),(38,y+14),(38-r,y+9)],'#81ab85')
 a.poly([(38,y-r),(38+r,y+12),(38,y+14)],'#508970')
a.save('pine')
for v in range(2):
 a=Art(58,44);a.shadow(31,36,24,6)
 a.poly([(7,29),(13,12),(30,6),(47,15),(52,29),(36,37),(16,35)],'#899e99')
 a.poly([(7,29),(13,12),(30,6),(31,24)],'#c2cec0')
 a.poly([(30,6),(47,15),(31,24)],'#aabbb0')
 a.poly([(31,24),(47,15),(52,29),(36,37)],'#6b8783')
 a.line([(13,12),(26,16),(31,24)],'#d7dfca');a.save('rock'+str(v))
for v in range(3):
 a=Art(48,42);a.shadow(26,34,20,5)
 for x,y,r in [(15,24,12),(29,22,14),(24,15,11)]:
  a.ellipse((x-r,y-r,x+r,y+r),'#4c8064');a.ellipse((x-r,y-r,x+r-3,y+r-5),'#79a371');a.ellipse((x-r+3,y-r+2,x+1,y-2),'#aac389')
 for x,y in [(12,21),(28,13),(33,25),(21,27)]:a.ellipse((x,y,x+4,y+4),'#c57e7a')
 a.save('bush'+str(v))
for v in range(3):
 a=Art(44,38);a.shadow(23,30,17,4)
 for x,y in [(10,17),(23,10),(33,20),(20,23)]:
  a.line([(x,y+10),(x,y)],'#4d8669',2);a.poly([(x,y+8),(x-7,y+4),(x,y+4),(x+6,y)],'#79a46e')
  color=['#f2cc78','#f1e7c0','#c8a9c5'][v]
  for j in range(5):
   angle=j*math.tau/5;cx=x+math.cos(angle)*3;cy=y+math.sin(angle)*2.5;a.ellipse((cx-2,cy-2,cx+2,cy+2),color)
  a.ellipse((x-1,y-1,x+1,y+1),'#ba9753')
 a.save('flowers'+str(v))
a=Art(144,130);a.shadow(78,117,61,11)
# Isometric cottage faces and foundation.
a.poly([(17,80),(72,105),(129,77),(129,113),(73,138-12),(17,101)],'#9d9178')
a.poly([(20,62),(73,87),(73,117),(20,92)],'#efe0b6')
a.poly([(73,87),(125,61),(125,92),(73,117)],'#c6bd96')
# Gable and terracotta roof.
a.poly([(20,62),(40,27),(74,50),(73,87)],'#f6e9c6')
a.poly([(8,60),(39,20),(98,46),(72,90)],'#bf7960')
a.poly([(39,20),(98,46),(137,62),(107,26),(69,9)],'#d8976e')
a.poly([(72,90),(98,46),(137,62),(126,68)],'#885b51')
for t in [.25,.5,.75]:a.line([(8+(39-8)*t,60+(20-60)*t),(72+(98-72)*t,90+(46-90)*t)],'#a56b59')
a.poly([(96,14),(105,10),(113,14),(113,36),(105,40),(96,36)],'#b8aa91');a.poly([(96,14),(105,18),(113,14),(105,10)],'#e9d8b0')
a.poly([(43,84),(56,90),(56,110),(43,104)],'#8b7356');a.ellipse((52,98,54,100),'#e5c27e')
a.poly([(26,73),(35,77),(35,88),(26,84)],'#7ba3a0');a.line([(30,75),(30,86)],'#f7e8bc');a.line([(26,79),(35,83)],'#f7e8bc')
a.poly([(91,87),(109,78),(109,91),(91,100)],'#628a84');a.line([(100,83),(100,95)],'#edddb5');a.line([(91,93),(109,85)],'#edddb5')
a.poly([(39,106),(57,114),(65,111),(47,103)],'#c1b291');a.im=a.im.resize((124*S,112*S),Image.Resampling.LANCZOS);a.w=124;a.h=112;a.save('cottage')
a=Art(60,38);a.shadow(31,30,25,5);a.poly([(8,16),(41,7),(52,18),(19,29)],'#997452');a.poly([(19,29),(52,18),(52,25),(19,35)],'#735d48');a.ellipse((7,15,27,34),'#cab088');a.ellipse((11,18,23,30),'#9c805c');a.ellipse((14,21,20,27),'#cfb58b');a.save('log')
# Four facings, four walk frames each; feet share the same ground anchor.
for direction in range(4):
 for frame in range(4):
  a=Art(36,52);bob=1 if frame in [1,3] else 0;stride=[0,3,0,-3][frame];a.shadow(19,44,11,3)
  a.line([(14,34-bob),(13+stride,43)],'#5d6b73',5);a.line([(22,34-bob),(23-stride,43)],'#4a5a64',5)
  a.ellipse((9+stride,41,17+stride,45),'#765f4e');a.ellipse((19-stride,41,27-stride,45),'#765f4e')
  a.ellipse((9,23-bob,27,39-bob),'#cc9a67');a.poly([(11,27-bob),(24,25-bob),(26,36-bob),(11,37-bob)],'#c98b62')
  a.line([(10,26-bob),(8-stride*.4,34-bob)],'#efd1a0',4);a.line([(26,26-bob),(28+stride*.4,34-bob)],'#e1b98a',4)
  a.ellipse((10,11-bob,26,27-bob),'#f1c99a');a.ellipse((8,8-bob,27,22-bob),'#6b5a4c')
  if direction<2:
   a.ellipse((12,16-bob,26,28-bob),'#edc69c');ex=23 if direction==0 else 15;a.rect((ex,21-bob,ex+1,23-bob),'#4b5552')
  else:a.ellipse((10,13-bob,26,25-bob),'#72604b')
  # Soft sage hat with sunlit crown.
  a.ellipse((5,12-bob,31,20-bob),'#8f9f70');a.ellipse((10,5-bob,26,17-bob),'#b1bc88');a.ellipse((12,6-bob,23,11-bob),'#d2d5a4')
  if direction>=2:a.ellipse((13,27-bob,25,37-bob),'#7d9179')
  a.save(f'player{direction}_{frame}')
# Phase 2: seeds, loose resources and four shaded growth stages.
for kind in range(6):
 a=Art(38,40);a.shadow(20,32,14,4)
 if kind==0:
  a.poly([(9,12),(28,10),(30,32),(10,34)],'#d9c39b');a.rect((10,12,28,16),'#f1dfb6');a.ellipse((16,20,23,28),'#7f9c70')
 elif kind==1:
  a.poly([(5,23),(26,15),(33,21),(12,31)],'#b08c64');a.poly([(12,31),(33,21),(33,27),(12,36)],'#82694f');a.ellipse((5,22,16,35),'#d6b386')
 elif kind==2:
  a.poly([(6,28),(13,16),(25,17),(32,28),(19,34)],'#a8bbb2');a.poly([(6,28),(13,16),(22,22),(19,34)],'#c8d1c1');a.poly([(22,22),(25,17),(32,28),(19,34)],'#809a95')
 elif kind==3:
  for x,y in [(13,26),(23,27),(18,20)]:a.ellipse((x-5,y-5,x+5,y+5),'#b57589');a.ellipse((x-3,y-4,x,y-1),'#dca0a4')
 elif kind==4:
  for x,y in [(13,18),(26,24)]:a.rect((x-2,y,x+2,y+12),'#e8d5af');a.ellipse((x-8,y-5,x+8,y+4),'#b88970');a.ellipse((x-4,y-4,x,y-1),'#e9bc91')
 else:
  for x,y in [(12,19),(25,24)]:
   a.line([(x,y),(x,y+11)],'#70956b',2)
   for dx,dy in [(0,-4),(4,0),(0,4),(-4,0)]:a.ellipse((x+dx-3,y+dy-3,x+dx+3,y+dy+3),'#e5bfd0')
   a.ellipse((x-2,y-2,x+2,y+2),'#e5bf73')
 a.save('resource'+str(kind))
for kind in range(8):
 for stage in range(4):
  a=Art(54,62);a.shadow(29,54,20,5);a.ellipse((17,49,37,56),'#8d7759')
  if stage==0:
   a.line([(27,52),(27,44)],'#6f9368',2);a.ellipse((20,40,27,46),'#aac282');a.ellipse((27,38,35,45),'#87ac72')
  else:
   tall=12+stage*5
   if kind>=5:
    a.poly([(24,53),(29,52),(29,31-tall//2),(25,32-tall//2)],'#a38961')
    for x,y,r in [(21,32-stage*3,10+stage),(34,32-stage*3,11+stage),(27,23-stage*3,10+stage)]:
     a.ellipse((x-r,y-r,x+r,y+r),'#5f936e');a.ellipse((x-r,y-r,x+r-4,y+r-6),'#95b780')
    if kind==6 and stage==3:
     for x,y in [(16,21),(33,14),(38,28)]:a.ellipse((x-3,y-3,x+3,y+3),'#ce8c74')
   elif kind==3 and stage>=2:
    a.ellipse((10,36,43,55),'#dba55c');a.ellipse((15,34,38,54),'#edba69');a.line([(26,36),(25,31)],'#719162',3)
    for x in [21,30]:a.line([(x,38),(x-1,51)],'#cf934e')
   elif kind==4:
    for x,y,r in [(18,43,10),(34,43,11),(27,34,11)]:a.ellipse((x-r,y-r,x+r,y+r),'#7fa46f')
    if stage==3:
     for x,y in [(15,39),(28,31),(36,43)]:a.ellipse((x-3,y-3,x+3,y+3),'#b77d98')
   else:
    for dx,dy in [(-13,-7),(13,-10),(-7,-17),(4,-tall)]:
     a.line([(27,52),(27+dx,50+dy)],'#6c986a',2);a.ellipse((23+dx,44+dy,31+dx,50+dy),'#9bbc7c')
     if kind==1 and stage==3:a.ellipse((23+dx,40+dy,30+dx,47+dy),'#e4bad0');a.ellipse((25+dx,42+dy,27+dx,45+dy),'#e7c97e')
    if kind==2 and stage==3:
     for x in [18,26,34]:a.poly([(x-3,47),(x+3,47),(x,56)],'#e1a46f')
  a.save(f'crop{kind}_{stage}')
# Font atlas: small readable proportional glyphs, alpha retained for colour tinting.
font=ImageFont.truetype(str(ROOT/'assets/fonts/DejaVuSans.ttf'),12)
widths=[];data=[]
for c in range(32,128):
 im=Image.new('L',(12,16));d=ImageDraw.Draw(im);d.text((0,-2),chr(c),font=font,fill=255)
 widths.append(min(12,max(3,round(font.getlength(chr(c))))));data.extend(im.getdata())
h=ROOT/'assets/generated/Font.h';h.parent.mkdir(exist_ok=True)
h.write_text('#pragma once\n#include <cstdint>\nnamespace wilds {\ninline constexpr uint8_t glyphWidths[]={'+','.join(map(str,widths))+'};\ninline constexpr uint8_t glyphPixels[]={'+','.join(map(str,data))+'};\n}\n')
convert_all()

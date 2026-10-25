#!/usr/bin/env python3
"""Convert straight-alpha source PNGs to little-endian RGB565 + colour key.

Usage: python scripts/convert_assets.py
Optional single asset: python scripts/convert_assets.py input.png output.bin
Magenta 0xf81f is reserved. Alpha < 128 is transparent; RGB collisions are remapped.
"""
from pathlib import Path
from PIL import Image
import struct, sys
ROOT = Path(__file__).resolve().parents[1]
KEY = 0xf81f

def pixels(image):
    result=[]
    for r,g,b,a in image.convert('RGBA').getdata():
        c=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
        result.append(KEY if a<128 else (c^1 if c==KEY else c))
    return result

def convert_all():
    files=sorted((ROOT/'assets/source').glob('*.png'))
    out=ROOT/'assets/generated';out.mkdir(exist_ok=True)
    header=['#pragma once','#include <cstdint>','namespace wilds {',
            'struct Sprite {const uint16_t* pixels; int16_t w,h,anchorX,anchorY;};']
    source=['#include "Sprites.h"','namespace wilds {']
    for p in files:
        im=Image.open(p);w,h=im.size;name=p.stem;data=pixels(im)
        source.append(f'static const uint16_t data_{name}[] = {{')
        source.extend(','.join(f'0x{v:04x}' for v in data[i:i+20])+',' for i in range(0,len(data),20))
        source.extend(['};',f'const Sprite {name}={{data_{name},{w},{h},{w//2},{h-8}}};'])
        header.append(f'extern const Sprite {name};')
    header.append('}');source.append('}')
    (out/'Sprites.h').write_text('\n'.join(header)+'\n')
    (out/'Sprites.cpp').write_text('\n'.join(source)+'\n')
    print(f'Converted {len(files)} sprites')
if __name__=='__main__':
    if len(sys.argv)==3:
        image=Image.open(sys.argv[1]);Path(sys.argv[2]).write_bytes(struct.pack('<'+'H'*(image.width*image.height),*pixels(image)))
    else: convert_all()

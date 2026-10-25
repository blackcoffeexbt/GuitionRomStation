#!/usr/bin/env python3
"""Prepare the SD-based firmware toolchain; optionally import a local ROM for host tests."""
from pathlib import Path
import argparse, subprocess, sys, zlib, json
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--rom',type=Path);args=p.parse_args()
source=args.rom
if source is None:
    candidates=list((root.parent/'type-r').glob('*.sms'))
    if len(candidates)==1:source=candidates[0]
if source is not None:
    data=source.read_bytes()
    if len(data)%8192==512:data=data[512:]
    if not 8192<=len(data)<=1024*1024 or (len(data)!=8192 and len(data)%16384):
        raise SystemExit('Expected a 16 KB-aligned Master System ROM, at most 1 MB.')
    headers=[at for at in (0x7ff0,0x3ff0,0x1ff0) if at+16<=len(data) and data[at:at+8]==b'TMR SEGA']
    if not headers or data[headers[0]+15]>>4 not in (3,4):
        raise SystemExit('Expected a Master System cartridge header, not a Game Gear image.')
    if len(data)==8192:data=data*2
    target=root/'roms';target.mkdir(exist_ok=True)
    (target/'game.sms').write_bytes(data)
    (target/'source.json').write_text(json.dumps({'source':source.name,'bytes':len(data),'crc32':f'{zlib.crc32(data):08x}'},indent=2)+'\n')
    print(f'Imported local ROM: {len(data)} bytes (ignored by Git).')
if not (root/'roms/game.sms').exists():
    print('SD-only firmware needs no embedded ROM. Optional cartridge host test will be skipped.')
subprocess.run([sys.executable,str(root.parent/'starbreak/scripts/bootstrap.py')],check=True)

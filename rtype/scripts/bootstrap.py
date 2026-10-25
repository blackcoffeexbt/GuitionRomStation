#!/usr/bin/env python3
"""Prepare the pinned toolchain and import a locally supplied SMS cartridge dump."""
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
    if len(data)%16384==512:data=data[512:]
    if not 16384<=len(data)<=1024*1024 or len(data)%16384:
        raise SystemExit('Expected a 16 KB-aligned Master System ROM, at most 1 MB.')
    if len(data)>=32768 and data[0x7ff0:0x7ff8]!=b'TMR SEGA':
        raise SystemExit('Missing Master System cartridge header; check the supplied ROM.')
    target=root/'roms';target.mkdir(exist_ok=True)
    (target/'game.sms').write_bytes(data)
    (target/'source.json').write_text(json.dumps({'source':source.name,'bytes':len(data),'crc32':f'{zlib.crc32(data):08x}'},indent=2)+'\n')
    print(f'Imported local ROM: {len(data)} bytes (ignored by Git).')
if not (root/'roms/game.sms').exists():
    raise SystemExit('Supply your ROM using --rom /full/path/to/game.sms. No game data is downloaded.')
subprocess.run([sys.executable,str(root.parent/'starbreak/scripts/bootstrap.py')],check=True)

#!/usr/bin/env python3
"""Prepare the SD-based firmware toolchain; optionally import a local ROM for host tests."""
from pathlib import Path
import argparse, hashlib, json, urllib.request, zipfile, zlib
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--rom',type=Path);args=p.parse_args()
source=args.rom
if source is None:
    candidates=list((root/'type-r').glob('*.sms'))
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
URL='https://github.com/ricardoquesada/esp32-arduino-lib-builder/releases/download/4.1.0/esp32-bluepad32-4.1.0.zip'
SHA='2b4916baba6c8d4e40b0330e12dd6a21ca7b95540337e6c2d2713831ccf3c394'
deps=root/'.deps';deps.mkdir(exist_ok=True)
core=deps/'esp32-bluepad32-4.1.0'
if not core.exists():
 archive=deps/'bluepad32.zip'
 if not archive.exists():urllib.request.urlretrieve(URL,archive)
 assert hashlib.sha256(archive.read_bytes()).hexdigest()==SHA,'Official package checksum mismatch'
 with zipfile.ZipFile(archive) as z:z.extractall(deps)
manifest={'name':'framework-arduinoespressif32','version':'3.20017.410','description':'Official Arduino ESP32 + Bluepad32 4.1.0, SHA256 pinned by bootstrap.py'}
(core/'package.json').write_text(json.dumps(manifest,indent=2)+'\n')
packages=deps/'packages';packages.mkdir(exist_ok=True)
# This project's package directory is isolated; installed compiler tools can be reused.
existing=Path.home()/'.platformio/packages'
for name,source in {
 'toolchain-xtensa-esp32s3':'toolchain-xtensa-esp32s3@8.4.0+2021r2-patch5',
 'toolchain-riscv32-esp':'toolchain-riscv32-esp',
 'tool-esptoolpy':'tool-esptoolpy',
}.items():
 target=packages/name;src=existing/source
 if src.exists() and not target.exists():target.symlink_to(src,target_is_directory=True)
print('Prepared official Bluepad32 4.1.0; isolated dependencies:',deps)

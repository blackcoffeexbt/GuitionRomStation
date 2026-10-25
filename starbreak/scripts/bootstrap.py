#!/usr/bin/env python3
"""Install the pinned official Bluepad32 Arduino core without changing Little Wilds.
Run once before `pio run -d starbreak`. Only the ignored .deps directory is changed.
"""
import hashlib,json,urllib.request,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
URL='https://github.com/ricardoquesada/esp32-arduino-lib-builder/releases/download/4.1.0/esp32-bluepad32-4.1.0.zip'
SHA='2b4916baba6c8d4e40b0330e12dd6a21ca7b95540337e6c2d2713831ccf3c394'
deps=ROOT/'.deps';deps.mkdir(exist_ok=True)
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

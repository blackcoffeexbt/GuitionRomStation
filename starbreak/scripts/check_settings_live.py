#!/usr/bin/env python3
"""Use the active validation reader's command queue; never touch farm files."""
from pathlib import Path
import time,re
out=Path(__file__).resolve().parents[1]/'device-review';log=out/'soak.log';commands=out/'commands.txt'
def send(command):
 end=time.monotonic()+4
 while commands.exists() and time.monotonic()<end:time.sleep(.1)
 if commands.exists():raise RuntimeError('Reader not consuming commands')
 commands.write_text(command+'\n')
def expect(pattern,start,timeout=8):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  text=log.read_text()[start:]
  if re.search(pattern,text):print('PASS',pattern,flush=True);return text
  time.sleep(.15)
 raise AssertionError('Missing '+pattern+'\n'+text[-3000:])
start=len(log.read_text());send('status');initial=expect(r'STATE.*best=(\d+)',start)
best=int(re.findall(r'STATE.*best=(\d+)',initial)[-1]);assert best>0
# Stop the test pilot and keep settings writes on a menu screen.
send('autoplay');time.sleep(.4)
start=len(log.read_text());send('mute');expect(r'SETTINGS save_us=.*best='+str(best)+r'.*ok=1',start)
start=len(log.read_text());send('reboot');expect(r'SETTINGS namespace=starbreak best='+str(best)+r' mute=1 .*recovered=0',start)
start=len(log.read_text());send('corrupt_settings');expect(r'CORRUPT shooter_namespace_only=1',start)
start=len(log.read_text());send('reboot');expect(r'SETTINGS namespace=starbreak best='+str(best)+r' mute=0 .*recovered=1',start)
# Replace the damaged record, restore sound-on, and verify a clean subsequent boot.
start=len(log.read_text());send('mute');expect(r'SETTINGS save_us=.*ok=1',start)
start=len(log.read_text());send('mute');expect(r'SETTINGS save_us=.*ok=1',start)
start=len(log.read_text());send('reboot');expect(r'SETTINGS namespace=starbreak best='+str(best)+r' mute=0 .*recovered=0',start)
send('autoplay')
print('PASS on-device best score, mute reboot persistence, corrupt-slot recovery, clean restored settings; farm namespace untouched',flush=True)
(out/'settings-result.txt').write_text('PASS best='+str(best)+'; mute persists; corrupt latest falls back; clean sound-on settings restored\n')

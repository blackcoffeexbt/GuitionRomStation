#!/usr/bin/env python3
"""Exercise real Phase 2 actions through the same game logic as touchscreen input.
No clock acceleration; planted crops grow at normal simulation speed.
Run on a new Phase 2 farm, with the supplied starter seed pouch intact.
"""
import serial,time
from pathlib import Path
Path('device-review').mkdir(exist_ok=True)
with serial.Serial('/dev/cu.usbmodem101',115200,timeout=.1) as board, open('device-review/phase2-farming.log','w') as log:
 def read(seconds):
  end=time.monotonic()+seconds
  while time.monotonic()<end:
   data=board.read(4096).decode(errors='replace')
   if data:print(data,end='',flush=True);log.write(data);log.flush()
 def send(command,seconds=1):
  print('CHECK:',command,flush=True);log.write('CHECK: '+command+'\n');board.write((command+'\n').encode());read(seconds)
 send('status')
 send('tool 0');send('act 35 33',2)
 send('tool 2');send('act 35 33')
 send('tool 1');send('seed 2');send('act 35 33')
 send('tool 3');send('act 35 33')
 send('save',4);send('status')
 send('reboot',5);send('status')
 # Subsequent tests gather every loose resource type on the board.
 send('tool 0')
 for tile in ['35 35','36 34','33 37','34 31','36 36']:send('act '+tile,3)
 send('save',3);send('status')

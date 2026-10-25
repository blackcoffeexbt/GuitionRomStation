#!/usr/bin/env python3
"""Plant a small persistent demonstration garden on the provided empty garden beds.
Uses actual movement/growth; run once after the initial gathering/harvest check.
"""
from pathlib import Path
import serial,time,re
Path('device-review').mkdir(exist_ok=True)
with serial.Serial('/dev/cu.usbmodem101',115200,timeout=.1) as board,open('device-review/phase2-garden.log','w') as log:
 def send(command,seconds=1):
  board.write((command+'\n').encode());end=time.monotonic()+seconds;text=''
  while time.monotonic()<end:
   data=board.read(4096).decode(errors='replace')
   if data:print(data,end='',flush=True);log.write(data);log.flush();text+=data
  return text
 # Exercise the visible seed chooser, not just the direct tool command.
 send('tap 144 290',.3);send('tap 290 100',.3)
 for seed,tile in [(2,'31 35'),(3,'32 35'),(1,'33 35')]:
  send('seed '+str(seed),.2);send('act '+tile,3)
 send('tool 3',.2)
 for tile in ['31 35','32 35','33 35']:send('act '+tile,3)
 send('save',2)
 before=send('status',1);assert 'plants=3' in before
 inventory=re.findall(r'INVENTORY[^\r\n]*',before)[-1]
 after=send('reboot',5)
 assert 'loaded=1 damaged=0' in after and 'plants=3' in after
 assert re.findall(r'INVENTORY[^\r\n]*',after)[-1]==inventory
 send('walk 34 34',2)
 print('PASS three-species watered garden and inventory preserved across normal-firmware reboot')
 log.write('PASS three-species watered garden and inventory preserved across normal-firmware reboot\n')

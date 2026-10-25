#!/usr/bin/env python3
"""Temporary guition_validation build only. Corrupt newest slot, recover older,
restore untouched backup, and test an incomplete temp file. Preserves the farm.
"""
import serial,time,re
from pathlib import Path
Path('device-review').mkdir(exist_ok=True)
with serial.Serial('/dev/cu.usbmodem101',115200,timeout=.1) as board,open('device-review/phase2-recovery.log','w') as log:
 def send(command,seconds):
  board.write((command+'\n').encode());end=time.monotonic()+seconds;text=''
  while time.monotonic()<end:
   data=board.read(4096).decode(errors='replace')
   if data:print(data,end='',flush=True);log.write(data);log.flush();text+=data
  return text
 send('save',2);send('save',2)
 before=send('status',1)
 inventory=re.findall(r'INVENTORY[^\r\n]*',before)[-1]
 corrupt=send('fault 1',5)
 assert 'backup_valid=1 ok=1' in corrupt, 'Fault command did not execute'
 assert 'damaged=1' in corrupt and 'loaded=1' in corrupt, 'Did not recover the older save'
 assert re.findall(r'INVENTORY[^\r\n]*',corrupt)[-1]==inventory,'Recovery changed inventory'
 restored=send('fault 3',5)
 assert 'restored_backup' in restored and 'damaged=0' in restored,'Backup not restored'
 interrupted=send('fault 2',5)
 assert 'interrupted_temp' in interrupted and 'loaded=1 damaged=0' in interrupted
 assert re.findall(r'INVENTORY[^\r\n]*',interrupted)[-1]==inventory,'Interrupted write changed inventory'
 print('PASS on-device corrupt-slot fallback, exact backup restore, interrupted temporary-write recovery')
 log.write('PASS on-device corrupt-slot fallback, exact backup restore, interrupted temporary-write recovery\n')

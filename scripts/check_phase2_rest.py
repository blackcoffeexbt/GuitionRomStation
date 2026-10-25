#!/usr/bin/env python3
"""Exercise the cottage Save & rest screen and verify paused clock + reboot loading."""
import serial,time,re
with serial.Serial('/dev/cu.usbmodem101',115200,timeout=.1) as board,open('device-review/phase2-rest.log','w') as log:
 def send(command,seconds=1):
  board.write((command+'\n').encode());end=time.monotonic()+seconds;text=''
  while time.monotonic()<end:
   data=board.read(4096).decode(errors='replace')
   if data:print(data,end='',flush=True);log.write(data);log.flush();text+=data
  return text
 send('tool 0',.2);send('walk 34 34',2)
 send('tap 240 85',3) # visible cottage, using sprite hit testing
 send('tap 240 180',3) # Save & rest button
 first=send('status',2);second=send('status',1)
 states=lambda text:re.findall(r'STATE[^\r\n]*',text)
 assert 'resting=1' in states(first)[-1] and 'resting=1' in states(second)[-1]
 clock=lambda text:re.findall(r'clock=([\d.]+)',states(text)[-1])[-1]
 assert clock(first)==clock(second),'Clock advanced during rest'
 inv=re.findall(r'INVENTORY[^\r\n]*',second)[-1]
 loaded=send('reboot',5)
 assert 'loaded=1 damaged=0' in loaded
 assert re.findall(r'INVENTORY[^\r\n]*',loaded)[-1]==inv
 send('tap 432 290',1) # bag for webcam review
 print('PASS cottage screen Save & rest, frozen clock, inventory restored across reboot')
 log.write('PASS cottage screen Save & rest, frozen clock, inventory restored across reboot\n')

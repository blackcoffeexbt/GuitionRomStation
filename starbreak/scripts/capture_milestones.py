#!/usr/bin/env python3
"""Capture a few actual future on-device milestone screens from the live soak log."""
from pathlib import Path
import time,re,subprocess
root=Path(__file__).resolve().parents[1];out=root/'device-review'
wanted={'boss-phase1','boss-phase2','victory','defeat'};end=time.monotonic()+1000
with open(out/'soak.log') as f:
 f.seek(0,2)
 while wanted and time.monotonic()<end:
  line=f.readline()
  if not line:time.sleep(.15);continue
  tag=None
  m=re.search(r'screen=2 stage=([\d.]+).*boss=(-?\d+)/(\d+)',line)
  if m and float(m[1])>265 and int(m[2])>0:tag='boss-phase'+m[3]
  if 'STATE screen=5 ' in line:tag='victory'
  if 'STATE screen=4 ' in line:tag='defeat'
  if tag in wanted:
   result=subprocess.run(['imagesnap','-q','-d','FaceTime HD Camera','-w','0.3',str(out/(tag+'.jpg'))],capture_output=True)
   print('CAPTURE',tag,'ok='+str(result.returncode==0),flush=True)
   if result.returncode==0:wanted.remove(tag)

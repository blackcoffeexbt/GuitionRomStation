#!/usr/bin/env python3
"""Full-speed on-device validation. Requires the validation build and paired Xbox.
The pilot supplies normal controls; game rules, collision and the 60 Hz clock are unchanged.
"""
import argparse,time,serial,re,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--minutes',type=float,default=32);p.add_argument('--port',default='/dev/cu.usbmodem101');args=p.parse_args()
root=Path(__file__).resolve().parents[1];out=root/'device-review';out.mkdir(exist_ok=True)
fps=[];heaps=[];results=[];connected=False
with serial.Serial(args.port,115200,timeout=.2) as board,open(out/'soak.log','w') as log:
 board.write(b'status\nautoplay\n');start=time.monotonic();last_summary=start
 while time.monotonic()-start<args.minutes*60:
  line=board.readline().decode(errors='replace').strip()
  if line:
   log.write(line+'\n');log.flush()
   if 'STATE ' in line or 'RUN_RESULT' in line or 'CONNECTED' in line or 'DISCONNECTED' in line:print(line,flush=True)
   if 'RUN_RESULT' in line:results.append(line)
   if 'PERF ' in line:
    m=re.search(r'fps=([\d.]+).*?\bheap=(\d+)\b.*pad=(\d+).*screen=(\d+).*stage=([\d.]+)',line)
    if m:
     connected=m[3]=='1';heaps.append(int(m[2]));
     if int(m[4])==2:fps.append(float(m[1]))
   if 'FATAL' in line or 'Guru Meditation' in line or 'CORRUPT HEAP' in line:print('HARD FAILURE:',line,flush=True);break
  now=time.monotonic()
  if now-last_summary>=30:
   print(f'SOAK elapsed={now-start:.0f}s pad={int(connected)} runs={len(results)} min_fps={min(fps) if fps else 0} heap={heaps[-1] if heaps else 0}',flush=True);last_summary=now
 board.write(b'status\n')
 summary={'wall_seconds':time.monotonic()-start,'results':results,'min_play_fps':min(fps) if fps else None,'fps_samples':len(fps),'heap_min':min(heaps) if heaps else None,'heap_max':max(heaps) if heaps else None,'heap_last':heaps[-1] if heaps else None}
 (out/'soak-summary.json').write_text(json.dumps(summary,indent=2))
 print(json.dumps(summary),flush=True)

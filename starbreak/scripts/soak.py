#!/usr/bin/env python3
"""Full-speed hardware validation, counting actual 60 Hz flight ticks.
Requires the validation build and paired Xbox. --resume retains the current log and pilot.
No clock acceleration, fake connections, invulnerability or score injection.
"""
import argparse,time,serial,re,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--minutes',type=float,default=32,help='minimum actual flight minutes');p.add_argument('--port',default='/dev/cu.usbmodem101');p.add_argument('--resume',action='store_true');p.add_argument('--max-wall-minutes',type=float,default=65);args=p.parse_args()
root=Path(__file__).resolve().parents[1];out=root/'device-review';out.mkdir(exist_ok=True)
fps=[];heaps=[];results=[];ticks=0;connected=False;failure=None
with serial.Serial(args.port,115200,timeout=.15) as board,open(out/'soak.log','a' if args.resume else 'w') as log:
 board.write(b'status\n' if args.resume else b'status\nautoplay\n');start=time.monotonic();last_summary=start;last_status=0
 while ticks<args.minutes*3600 and time.monotonic()-start<args.max_wall_minutes*60:
  line=board.readline().decode(errors='replace').strip()
  if line:
   log.write(line+'\n');log.flush()
   if 'STATE ' in line or 'RUN_RESULT' in line or 'CONNECTED' in line or 'DISCONNECTED' in line:print(line,flush=True)
   if 'RUN_RESULT' in line:results.append(line)
   m=re.search(r'\bticks=(\d+)',line)
   if m:ticks=int(m[1])
   m=re.search(r'soak_ticks=(\d+)',line)
   if m:ticks=max(ticks,int(m[1]))
   if 'PERF ' in line:
    m=re.search(r'fps=([\d.]+).*?\bheap=(\d+)\b.*pad=(\d+).*screen=(\d+).*stage=([\d.]+)',line)
    if m:
     connected=m[3]=='1';heaps.append(int(m[2]));
     if int(m[4])==2:fps.append(float(m[1]))
   if 'FATAL' in line or 'Guru Meditation' in line or 'CORRUPT HEAP' in line:failure=line;print('HARD FAILURE:',line,flush=True);break
  now=time.monotonic()
  if now-last_status>=20:board.write(b'status\n');last_status=now
  if now-last_summary>=30:
   print(f'SOAK wall={now-start:.0f}s flight={ticks/60:.0f}s pad={int(connected)} results={len(results)} min_fps={min(fps) if fps else 0} heap={heaps[-1] if heaps else 0}',flush=True);last_summary=now
  commands=out/'commands.txt'
  if commands.exists():
   allowed={'status','mute','pair','reboot','autoplay','defeat','corrupt_settings'}
   for command in commands.read_text().splitlines():
    if command in allowed:board.write((command+'\n').encode())
   commands.unlink()
 if ticks>=args.minutes*3600:board.write(b'autoplay\nstatus\n')
 summary={'wall_seconds':time.monotonic()-start,'flight_ticks':ticks,'flight_seconds':ticks/60,'target_met':ticks>=args.minutes*3600,'results':results,'min_play_fps':min(fps) if fps else None,'fps_samples':len(fps),'heap_min':min(heaps) if heaps else None,'heap_max':max(heaps) if heaps else None,'heap_last':heaps[-1] if heaps else None,'failure':failure}
 (out/'soak-summary.json').write_text(json.dumps(summary,indent=2));print(json.dumps(summary),flush=True)

#!/usr/bin/env python3
"""Audit raw hardware logs, including resumed-reader segments; no firmware changes."""
from pathlib import Path
import re,json,statistics
root=Path(__file__).resolve().parents[1];path=root/'device-review/soak.log';text=path.read_text();rows=[];states=[];results=[]
for line in text.splitlines():
 if line.startswith('PERF '):
  row={k:v for k,v in re.findall(r'(\w+)=([^ ]+)',line)}
  if 'fps' in row and 'heap' in row and 'screen' in row:rows.append(row)
 if line.startswith('STATE '):states.append({k:v for k,v in re.findall(r'(\w+)=([^ ]+)',line)})
 if line.startswith('RUN_RESULT '):results.append({k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',line)})
playing=[r for r in rows if r.get('screen')=='2'];fps=[float(r['fps']) for r in playing];heap=[int(r['heap']) for r in playing]
max_ticks=max([int(s.get('ticks',0)) for s in states]+[0]);first_ticks=int(states[0].get('ticks',0)) if states else 0
report={'flight_ticks_this_boot':max_ticks,'flight_seconds_this_boot':max_ticks/60,'flight_seconds_after_initial_status':(max_ticks-first_ticks)/60,'completed_victories':sum(r.get('victory',0) for r in results),'completed_defeats':sum(1-r.get('victory',0) for r in results),'gameplay_fps_samples':len(fps),'min_gameplay_fps':min(fps) if fps else None,'median_gameplay_fps':statistics.median(fps) if fps else None,'max_gameplay_fps':max(fps) if fps else None,'heap_min':min(heap) if heap else None,'heap_max':max(heap) if heap else None,'heap_first':heap[0] if heap else None,'heap_last':heap[-1] if heap else None,'heap_decline':heap[0]-heap[-1] if heap else None,'disconnected_samples':sum(r.get('pad')!='1' for r in rows),'fatal_errors':bool(re.search(r'Guru Meditation|CORRUPT HEAP|FATAL|assert failed',text,re.I)),'peak_enemies':max([int(r.get('peaks','0/0').split('/')[0]) for r in rows]+[0]),'peak_enemy_shots':max([int(r.get('peaks','0/0').split('/')[1]) for r in rows]+[0])}
tick_values=[int(s.get('ticks',0)) for s in states]
report['unexpected_tick_resets']=sum(b<a for a,b in zip(tick_values,tick_values[1:]))
report['playing_disconnected_samples']=sum(r.get('pad')!='1' for r in playing)
report['audio_unmuted_all_states']=all(s.get('mute')=='0' for s in states)
report['sectors']={}
for level in ('1','2','3'):
 sector=[r for r in playing if r.get('level')==level]
 values=[float(r['fps']) for r in sector]
 phases=sorted({int(r['boss'].split('/')[1]) for r in sector if r.get('active_boss')=='1'})
 report['sectors'][level]={'samples':len(sector),'min_fps':min(values) if values else None,'boss_phases_observed':phases,'heap_min':min([int(r['heap']) for r in sector]) if sector else None}
report['three_sector_coverage']=all(v['samples']>0 and v['boss_phases_observed']==[1,2] for v in report['sectors'].values())
report['acceptance_passed']=bool(report['flight_seconds_after_initial_status']>=1800 and report['completed_victories']>=1 and report['completed_defeats']>=1 and fps and min(fps)>=25 and report['heap_decline']<=256 and not report['fatal_errors'] and not report['playing_disconnected_samples'] and report['audio_unmuted_all_states'] and report['unexpected_tick_resets']==0 and report['three_sector_coverage'])
(root/'device-review/endurance-audit.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

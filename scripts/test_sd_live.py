#!/usr/bin/env python3
"""Explicit short SD cartridge-switch check; three bounded eight-second intro runs."""
import time,re
from pathlib import Path
from board_serial import BoardSerial
out=Path(__file__).resolve().parents[1]/'device-review';out.mkdir(exist_ok=True)
with BoardSerial('/dev/cu.usbmodem101',115200,timeout=.1) as board,(out/'sd-live.log').open('w') as log:
    def send(command):board.write((command+'\n').encode())
    def read_until(predicate,seconds):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            line=board.readline().decode(errors='replace').strip()
            if line:
                log.write(line+'\n');log.flush();print(line,flush=True)
                if predicate(line):return line
        return None
    results=[]
    for index in range(3):
        send('library');send(f'load {index}')
        loaded=read_until(lambda s:s.startswith('ROM loaded '),5)
        if not loaded:
            send('status');read_until(lambda s:s.startswith('LIBRARY '),2)
            results.append({'index':index,'loaded':False});continue
        send('pause');send('bench')
        ended=read_until(lambda s:'BENCH complete' in s,12)
        send('status');read_until(lambda s:s.startswith('LIBRARY '),2)
        results.append({'index':index,'loaded':True,'completed':bool(ended),'detail':loaded})
    import json
    (out/'sd-live-summary.json').write_text(json.dumps(results,indent=2)+'\n')
    print('SUMMARY',json.dumps(results),flush=True)

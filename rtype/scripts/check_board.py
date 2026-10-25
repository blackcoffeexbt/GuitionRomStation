#!/usr/bin/env python3
"""Short, bounded board observations without asserting USB reset lines."""
import argparse,time
from pathlib import Path
from board_serial import BoardSerial
p=argparse.ArgumentParser();p.add_argument('--seconds',type=float,default=15);p.add_argument('--bench',action='store_true');p.add_argument('--port',default='/dev/cu.usbmodem101');args=p.parse_args()
if not 1<=args.seconds<=60:raise SystemExit('Use a short observation of 1–60 seconds.')
root=Path(__file__).resolve().parents[1];folder=root/'device-review';folder.mkdir(exist_ok=True)
with BoardSerial(args.port,115200,timeout=.2) as board,(folder/'latest-check.log').open('w') as log:
    board.write(b'status\n')
    if args.bench:board.write(b'bench\n')
    end=time.monotonic()+args.seconds
    while time.monotonic()<end:
        line=board.readline().decode(errors='replace').strip()
        if line:print(line,flush=True);log.write(line+'\n');log.flush()

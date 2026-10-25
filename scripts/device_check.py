#!/usr/bin/env python3
"""Read diagnostics and optionally exercise movement on an attached board.
Requires pyserial. Uses no filesystem writes on the board.
"""
import argparse
from pathlib import Path
import time
import serial

parser=argparse.ArgumentParser()
parser.add_argument('--port',default='/dev/cu.usbmodem101')
parser.add_argument('--seconds',type=float,default=15)
parser.add_argument('--tour',action='store_true')
args=parser.parse_args()
Path('device-review').mkdir(exist_ok=True)
with serial.Serial(args.port,115200,timeout=.2) as device, open('device-review/final-validation.log','w') as log:
    device.write(b'status\n')
    start=time.monotonic()
    destinations=[(29,32),(34,34),(37,35),(34,34)]
    next_move=0
    while time.monotonic()-start<args.seconds:
        elapsed=time.monotonic()-start
        if args.tour and next_move<len(destinations) and elapsed>=next_move*5:
            x,y=destinations[next_move]
            device.write(f'walk {x} {y}\n'.encode())
            next_move+=1
        text=device.read(4096).decode(errors='replace')
        if text:
            print(text,end='',flush=True)
            log.write(text)
    # A direct route disables any debug tour; leave the device playable at home.
    if args.tour:
        device.write(b'walk 34 34\n')

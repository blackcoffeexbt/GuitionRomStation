#!/usr/bin/env python3
"""Observe a live validation log and capture real campaign milestones via Mac webcam."""
from pathlib import Path
import time, re, subprocess
out = Path(__file__).resolve().parents[1] / 'device-review'
wanted = {f'level-{level}-{part}' for level in range(1, 4) for part in ('flight', 'boss-1', 'boss-2')}
wanted |= {'victory', 'defeat'}
end = time.monotonic() + 1500
with (out / 'soak.log').open() as log:
    log.seek(0, 2)
    while wanted and time.monotonic() < end:
        line = log.readline()
        if not line:
            time.sleep(.15)
            continue
        row = dict(re.findall(r'(\w+)=([^ ]+)', line))
        tag = None
        if line.startswith('PERF ') and row.get('screen') == '2':
            level = row.get('level')
            if row.get('active_boss') == '1':
                tag = f"level-{level}-boss-{row['boss'].split('/')[1]}"
            elif 25 < float(row.get('level_time', 0)) < 60:
                tag = f'level-{level}-flight'
        if line.startswith('STATE screen=5 '): tag = 'victory'
        if line.startswith('STATE screen=4 '): tag = 'defeat'
        if tag in wanted:
            result = subprocess.run(['/usr/local/bin/imagesnap', '-q', '-d', 'FaceTime HD Camera', '-w', '0.3', str(out / f'campaign-{tag}.jpg')], capture_output=True)
            print('CAPTURE', tag, 'ok=' + str(result.returncode == 0), flush=True)
            if result.returncode == 0: wanted.remove(tag)
print('CAPTURE remaining:', ', '.join(sorted(wanted)) or 'none', flush=True)

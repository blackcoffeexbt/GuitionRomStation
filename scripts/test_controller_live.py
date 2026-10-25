#!/usr/bin/env python3
"""On-device controller simulation; requires the explicit validation build.
Uses normal frontend controls and native SMS input. It does not emulate a radio link.
"""
from pathlib import Path
import json,re,time,subprocess
from board_serial import BoardSerial
out=Path(__file__).resolve().parents[1]/'device-review';out.mkdir(exist_ok=True)
results=[]
with BoardSerial('/dev/cu.usbmodem101',115200,timeout=.05) as board,(out/'controller-live.log').open('w') as log:
    def send(cmd):board.write((cmd+'\n').encode())
    def read(seconds,until=None):
        end=time.monotonic()+seconds;found=None
        while time.monotonic()<end:
            line=board.readline().decode(errors='replace').strip()
            if line:
                log.write(line+'\n');log.flush();print(line,flush=True)
                if until and until(line):found=line;break
        return found
    def hold(mask,seconds):send(f'sim {mask}');read(seconds)
    def snapshot():
        send('status');state=control=library=None;end=time.monotonic()+2
        while time.monotonic()<end:
            line=read(.2,lambda s:s.startswith(('SMS STATE','CONTROL ','LIBRARY ')))
            if not line:continue
            row=dict(re.findall(r'(\w+)=([^ ]+)',line))
            if line.startswith('SMS STATE'):state=row
            elif line.startswith('CONTROL'):control=row
            elif line.startswith('LIBRARY'):library=row
            if state and control and library:return state,control,library
        raise RuntimeError('Missing status response')
    try:
        hold(0,.2)
        state,control,library=snapshot()
        assert control['simulated']=='1' and library['open']=='1' and int(library['count'])==3
        for index in range(3):
            if index:hold(2,.12);hold(0,.12)
            send('sim 16')
            loaded=read(3,lambda s:s.startswith('ROM loaded '));assert loaded,'Library selection failed'
            hold(0,3.6)
            hold(16,.18);hold(0,.4)
            hold(64,1.5)
            for mask in (72,65,68,66):hold(mask,.65)
            hold(48,.2);hold(16,.8);hold(0,.15);hold(64,.3)
            shot=out/f'sd-controller-game-{index}.jpg'
            camera=subprocess.run(['/usr/local/bin/imagesnap','-q','-d','FaceTime HD Camera','-w','0.3',str(shot)],capture_output=True)
            hold(0,.15)
            state,control,library=snapshot()
            assert state['paused']=='0' and int(control['rom_frames'])>300 and int(control['applied'])==63
            assert int(state['audio_errors'])==0
            hold(512,.2);frozen,_,_=snapshot();hold(512,.2);again,_,_=snapshot()
            assert frozen['paused']=='1' and frozen['frames']==again['frames']
            hold(16,.2);held,_,_=snapshot();assert held['paused']=='1' and held['frames']==again['frames']
            hold(0,.12);hold(16,.12);hold(0,.2);resumed,_,_=snapshot();assert resumed['paused']=='0' and int(resumed['frames'])>int(held['frames'])
            hold(128,.15);hold(0,.15);paused,_,_=snapshot();hold(0,.15);still,_,_=snapshot();assert paused['paused']=='1' and still['frames']==paused['frames']
            hold(64,.15);hold(0,.15);_,_,library=snapshot();assert library['open']=='1'
            result={'index':index,'loaded':loaded,'native_frames':int(control['rom_frames']),'applied_input_bits':int(control['applied']),'input_changes':int(control['changes']),'disconnect_guard':True,'menu_pause':True,'camera':camera.returncode==0,'visual_gameplay':'requires image review'}
            results.append(result);print('INPUT PATH PASS',json.dumps(result),flush=True)
        hold(64,.2);hold(0,.2);_,_,library=snapshot();assert int(library['count'])==3
        print('PASS controller navigation, all native buttons, movement, disconnect/reconnect, pause and rescan for all three games',flush=True)
    finally:
        send('sim off');send('status');read(.5)
        (out/'controller-live-summary.json').write_text(json.dumps(results,indent=2)+'\n')

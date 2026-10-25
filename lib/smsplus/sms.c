/* Guition SD port: memory-control/open-bus and standalone built-in BIOS mapping. */

#include "shared.h"
void ym2413_write(int chip, int offset, int data);

/* SMS context */
t_sms sms;

/* Empty external slots read pulled-up data; writes must never modify this table. */
static const uint8 open_bus[0x2000] = {[0 ... 0x1fff]=0xff};
static void map_slots(void)
{
 int active=cart.bios ? !(sms.memctrl&0x08) : !(sms.memctrl&0x40);
 for(int slot=0;slot<3;slot++){
  uint8 *base=active?cart.rom+((sms.fcr[slot+1]%cart.pages)<<14):(uint8*)open_bus;
  cpu_readmap[slot*2]=base;
  cpu_readmap[slot*2+1]=active?base+0x2000:(uint8*)open_bus;
  cpu_writemap[slot*2]=cpu_writemap[slot*2+1]=sms.dummy;
 }
 if(active&&!cart.bios&&(sms.fcr[0]&8)){
  uint8 *base=sms.sram+((sms.fcr[0]&4)?0x4000:0);
  cpu_readmap[4]=cpu_writemap[4]=base;
  cpu_readmap[5]=cpu_writemap[5]=base+0x2000;
  sms.save=1;
 }
 cpu_readmap[6]=cpu_readmap[7]=(sms.memctrl&0x10)?(uint8*)open_bus:sms.ram;
 cpu_writemap[6]=cpu_writemap[7]=(sms.memctrl&0x10)?sms.dummy:sms.ram;
}
/* Run the virtual console emulation for one frame */
void sms_frame(int skip_render)
{
    /* Take care of hard resets */
    if(input.system & INPUT_HARD_RESET)
    {
        system_reset();
    }

    /* Debounce pause key */
    if(input.system & INPUT_PAUSE)
    {
        if(!sms.paused)
        {
            sms.paused = 1;

            z80_set_nmi_line(ASSERT_LINE);
            z80_set_nmi_line(CLEAR_LINE);
        }
    }
    else
    {
         sms.paused = 0;
    }

    if(snd.log) snd.callback(0x00);

    for(vdp.line = 0; vdp.line < 262; vdp.line += 1)
    {
        /* Handle VDP line events */
        vdp_run();

        /* Draw the current frame */
        if(!skip_render) render_line(vdp.line);

        /* Run the Z80 for a line */
        z80_execute(227);
    }

    /* Update the emulated sound stream */
    if(snd.enabled) 
    {
/*
        int count;

        SN76496Update(0, snd.psg_buffer, snd.bufsize, sms.psg_mask);

//        if(sms.use_fm)
//        {
//            int i;
//            for(i = 0; i < snd.bufsize; i++)
//            {
//                snd.fm_buffer[i] = OPLL_calc(opll);
//            }
//        }

        for(count = 0; count < snd.bufsize; count += 1)
        {
            signed short left   = 0;
            signed short right  = 0;
//            left = right = snd.fm_buffer[count];
			left=0;
            left  += snd.psg_buffer[0][count];
            right += snd.psg_buffer[1][count];
            snd.buffer[0][count] = left;
            snd.buffer[1][count] = right;
        }
*/
        SN76496Update(0, snd.buffer, snd.bufsize, sms.psg_mask);
    }
}


void sms_init(void)
{
    cpu_reset();
    sms_reset();
}


void sms_reset(void)
{
    /* Clear SMS context */
    memset(sms.dummy, 0, 0x2000);
    memset(sms.ram, 0, 0x2000);
    //memset(sms.sram, 0, 0x8000);
    sms.paused = sms.save = sms.port_3F = sms.port_F2 = sms.irq = 0x00;
    sms.psg_mask = 0xFF;

    sms.fcr[0]=0;sms.fcr[1]=0;sms.fcr[2]=1;sms.fcr[3]=cart.bios?2:0;
    sms.memctrl=cart.bios?0xe0:0xab;
    if(!cart.bios)sms.ram[0]=sms.memctrl;
    map_slots();
}


/* Reset Z80 emulator */
void cpu_reset(void)
{
    z80_reset(0);
    z80_set_irq_callback(sms_irq_callback);
}


/* Write to memory */
void cpu_writemem16(int address, int data)
{
    cpu_writemap[(address >> 13)][(address & 0x1FFF)] = data;
    if(address >= 0xFFFC) sms_mapper_w(address & 3, data);
}

/* Write to an I/O port */
void cpu_writeport(int port, int data)
{
    switch(port & 0xFF)
    {
        case 0x3E: /* Guition: memory control and BIOS/cartridge selection */
            sms.memctrl=data;map_slots();break;
        case 0x01: /* GG SIO */
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            break;

        case 0x06: /* GG STEREO */
            if(snd.log) {
            snd.callback(0x04);
            snd.callback(data);
            }
            sms.psg_mask = (data & 0xFF);
            break;

        case 0x7E: /* SN76489 PSG */
        case 0x7F:
            if(snd.log) {
            snd.callback(0x03);
            snd.callback(data);
            }
            if(snd.enabled) SN76496Write(0, data);
            break;

        case 0xBE: /* VDP DATA */
            vdp_data_w(data);
            break;

        case 0xBD: /* VDP CTRL */ 
        case 0xBF:
            vdp_ctrl_w(data);
            break;

        case 0xF0: /* YM2413 */
        case 0xF1:
            if(snd.log) {
            snd.callback((port & 1) ? 0x06 : 0x05);
            snd.callback(data);
            }
            if(snd.enabled && sms.use_fm) ym2413_write(0, port & 1, data);
            break;

        case 0xF2: /* YM2413 DETECT */
            if(sms.use_fm) sms.port_F2 = (data & 1);
            break;

        case 0x3F: /* TERRITORY CTRL. */
             sms.port_3F = ((data & 0x80) | (data & 0x20) << 1) & 0xC0;
            if(sms.country == TYPE_DOMESTIC) sms.port_3F ^= 0xC0;
            break;
    }
}


/* Read from an I/O port */
int cpu_readport(int port)
{
    uint8 temp = 0xFF;

    switch(port & 0xFF)
    {
        case 0x01: /* GG SIO */
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            return (0x00);
    
        case 0x7E: /* V COUNTER */
            return (vdp_vcounter_r());
            break;
    
        case 0x7F: /* H COUNTER */
            return (vdp_hcounter_r());
            break;
    
        case 0x00: /* INPUT #2 */
            temp = 0xFF;
            if(input.system & INPUT_START) temp &= ~0x80;
            if(sms.country == TYPE_DOMESTIC) temp &= ~0x40;
            return (temp);
    
        case 0xC0: /* INPUT #0 */  
        case 0xDC:
            if(sms.memctrl&4)return 0xff;
            temp = 0xFF;
            if(input.pad[0] & INPUT_UP)      temp &= ~0x01;
            if(input.pad[0] & INPUT_DOWN)    temp &= ~0x02;
            if(input.pad[0] & INPUT_LEFT)    temp &= ~0x04;
            if(input.pad[0] & INPUT_RIGHT)   temp &= ~0x08;
            if(input.pad[0] & INPUT_BUTTON2) temp &= ~0x10;
            if(input.pad[0] & INPUT_BUTTON1) temp &= ~0x20;
            if(input.pad[1] & INPUT_UP)      temp &= ~0x40;
            if(input.pad[1] & INPUT_DOWN)    temp &= ~0x80;
            return (temp);
    
        case 0xC1: /* INPUT #1 */
        case 0xDD:
            if(sms.memctrl&4)return 0xff;
            temp = 0xFF;
            if(input.pad[1] & INPUT_LEFT)    temp &= ~0x01;
            if(input.pad[1] & INPUT_RIGHT)   temp &= ~0x02;
            if(input.pad[1] & INPUT_BUTTON2) temp &= ~0x04;
            if(input.pad[1] & INPUT_BUTTON1) temp &= ~0x08;
            if(input.system & INPUT_SOFT_RESET) temp &= ~0x10;
            return ((temp & 0x3F) | (sms.port_3F & 0xC0));

        case 0xBE: /* VDP DATA */
            return (vdp_data_r());
    
        case 0xBD:
        case 0xBF: /* VDP CTRL */
            return (vdp_ctrl_r());

        case 0xF2: /* YM2413 DETECT */
            if(sms.use_fm) return (sms.port_F2);
            break;
    }
    return (0xFF);     
}


void sms_mapper_w(int address, int data)
{
    /* The internal BIOS mapper is not selected while probing empty external slots. */
    if(cart.bios&&(sms.memctrl&8))return;
    sms.fcr[address&3]=data;
    map_slots();
}


int sms_irq_callback(int param)
{
    return (0xFF);
}


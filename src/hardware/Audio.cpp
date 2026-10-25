#include "Audio.h"
#include "BoardConfig.h"
#include <driver/i2s.h>
#include <Arduino.h>
#include <cmath>
bool Audio::begin(){
 i2s_config_t config={};config.mode=i2s_mode_t(I2S_MODE_MASTER|I2S_MODE_TX);config.sample_rate=22050;config.bits_per_sample=I2S_BITS_PER_SAMPLE_16BIT;config.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT;config.communication_format=I2S_COMM_FORMAT_STAND_I2S;config.intr_alloc_flags=ESP_INTR_FLAG_LEVEL1;config.dma_buf_count=4;config.dma_buf_len=256;config.use_apll=false;config.tx_desc_auto_clear=true;
 if(i2s_driver_install(I2S_NUM_0,&config,0,nullptr)!=ESP_OK)return false;
 i2s_pin_config_t pins={};pins.mck_io_num=I2S_PIN_NO_CHANGE;pins.bck_io_num=board::AudioBclk;pins.ws_io_num=board::AudioLrclk;pins.data_out_num=board::AudioData;pins.data_in_num=I2S_PIN_NO_CHANGE;
 if(i2s_set_pin(I2S_NUM_0,&pins)!=ESP_OK)return false;
 events=xQueueCreate(1,sizeof(wilds::SoundEvent));if(!events)return false;
 i2s_zero_dma_buffer(I2S_NUM_0);
 return xTaskCreatePinnedToCore(task,"wilds-audio",3072,this,1,nullptr,0)==pdPASS;
}
void Audio::play(wilds::SoundEvent e,bool muted){if(events&&!muted&&e!=wilds::SoundEvent::None)xQueueOverwrite(events,&e);}
void Audio::task(void* ptr){
 auto*self=static_cast<Audio*>(ptr);int16_t samples[256*2];wilds::SoundEvent event;
 for(;;){if(xQueueReceive(self->events,&event,portMAX_DELAY)!=pdTRUE)continue;
  int duration=event==wilds::SoundEvent::Arrive?3600:1500;
  for(int base=0;base<duration;base+=256){int count=std::min(256,duration-base);for(int i=0;i<count;i++){
   int t=base+i;float phase=float(t)/22050.f,progress=float(t)/duration;
   float frequency=event==wilds::SoundEvent::Blocked?220.f:event==wilds::SoundEvent::Arrive?(progress<.5f?660.f:880.f):520.f;
   float envelope=std::sin(3.14159265f*progress);envelope*=envelope;
   int16_t v=int16_t(std::sin(6.2831853f*frequency*phase)*envelope*950.f);samples[i*2]=samples[i*2+1]=v;
  }size_t written;i2s_write(I2S_NUM_0,samples,count*4,&written,portMAX_DELAY);}
  // Silence follows the tone in FIFO order; never overwrite queued tone DMA.
  std::fill(samples,samples+512,int16_t(0));for(int n=0;n<5;n++){size_t written;i2s_write(I2S_NUM_0,samples,sizeof(samples),&written,portMAX_DELAY);}
 }
}

#include "Audio.h"
#include "BoardConfig.h"
#include <driver/i2s.h>
bool Audio::begin() {
  i2s_config_t config = {};
  config.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = 22080;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 3;
  config.dma_buf_len = 128;
  config.tx_desc_auto_clear = true;
  if (i2s_driver_install(I2S_NUM_0, &config, 0, nullptr) != ESP_OK)
    return false;
  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = board::AudioBclk;
  pins.ws_io_num = board::AudioLrclk;
  pins.data_out_num = board::AudioData;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK)
    return false;
  queue = xQueueCreate(1, sizeof(Frame));
  return queue && xTaskCreatePinnedToCore(task, "sms-audio", 4096, this, 2, nullptr, 0) == pdPASS;
}
bool Audio::submit(const int16_t *left, const int16_t *right, int count, bool muted) {
  if (count < 0 || count > 368)
    return false;
  for (int i = 0; i < count; i++) {
    pending.pcm[i * 2] = muted ? 0 : left[i] / 4;
    pending.pcm[i * 2 + 1] = muted ? 0 : right[i] / 4;
  }
  pending.count = count;
  return queue && xQueueOverwrite(queue, &pending) == pdTRUE;
}
void Audio::task(void *ptr) {
  auto *self = static_cast<Audio *>(ptr);
  Frame frame;
  for (;;)
    if (xQueueReceive(self->queue, &frame, portMAX_DELAY) == pdTRUE) {
      size_t written = 0;
      if (i2s_write(I2S_NUM_0, frame.pcm, frame.count * 4, &written, portMAX_DELAY) != ESP_OK ||
          written != size_t(frame.count * 4))
        self->errors.fetch_add(1);
    }
}

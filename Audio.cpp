/**
 * @file Audio.cpp
 * @brief Triển khai module điều khiển âm thanh
 */

#include "Config.h"
#include "Globals.h"
#include "Audio.h"
#include <driver/i2s.h>

// DFPlayer Mini object
DFRobotDFPlayerMini myDFPlayer;

// ==================== DFPLAYER MINI ====================

void MP3_Play(int track) {
  myDFPlayer.play(track);
}

void MP3_Stop(bool pause) {
  if (pause) {
    myDFPlayer.pause();
  } else {
    myDFPlayer.stop();
  }
}

void MP3_SetVolume(int volume) {
  if (volume < 0) volume = 0;
  if (volume > 30) volume = 30;
  myDFPlayer.volume(volume);
}

void Audio_Init() {
  // Khởi tạo Serial cho DFPlayer
  DFPlayerSerial.begin(9600, SERIAL_8N1, RX_MP3_PIN, TX_MP3_PIN);
  delay(100);
  yield();
  
  // Khởi tạo DFPlayer
  if (!myDFPlayer.begin(DFPlayerSerial)) {
    Serial.println(F("DFPlayer Mini ERR!"));
  } else {
    myDFPlayer.volume(15);
    Serial.println(F("DFPlayer Mini initialized"));
  }
}

// ==================== I2S MICROPHONE ====================

void I2S_Init() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // Đã sửa thành 32-bit chuẩn INMP441
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = SCK_INMP441_PIN,
    .ws_io_num = WS_INMP441_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = SD_INMP441_PIN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
  
  Serial.println(F("I2S Microphone initialized"));
}
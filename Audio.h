/**
 * @file Audio.h
 * @brief Module điều khiển âm thanh (DFPlayer Mini và I2S Microphone)
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>

// ==================== DFPLAYER MINI ====================
/**
 * @brief Phát nhạc từ thẻ nhớ
 * @param track Số thứ tự bài hát
 */
void MP3_Play(int track);

/**
 * @brief Dừng hoặc tạm dừng nhạc
 * @param pause true: tạm dừng, false: dừng hẳn
 */
void MP3_Stop(bool pause);

/**
 * @brief Điều chỉnh âm lượng
 * @param volume 0-30
 */
void MP3_SetVolume(int volume);

/**
 * @brief Khởi tạo module âm thanh
 */
void Audio_Init();

// ==================== I2S MICROPHONE ====================
/**
 * @brief Khởi tạo I2S cho microphone
 */
void I2S_Init();

#endif // AUDIO_H
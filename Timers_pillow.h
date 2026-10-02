/**
 * @file Timers.h
 * @brief Module quản lý các bộ định thời
 */

#ifndef TIMERS_PILLOW_H
#define TIMERS_PILLOW_H

#include <Arduino.h>

// ==================== SLEEP TIMER ====================
/**
 * @brief Cập nhật sleep timer
 */
void SleepTimer_Update();

// ==================== AUTO COMFORT ====================
/**
 * @brief Cập nhật chế độ tự động điều chỉnh
 */
void AutoComfort_Update();

/**
 * @brief Xử lý nút nhấn dừng auto comfort
 */
void Button_StopAutoComfort();


#endif // TIMERS_PILLOW_H
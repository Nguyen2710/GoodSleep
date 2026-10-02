/**
 * @file ESPNow.h
 * @brief Module giao tiếp ESP-NOW với đồng hồ thông minh
 */

#ifndef ESPNOW_H
#define ESPNOW_H

#include <Arduino.h>
#include <esp_now.h>

/**
 * @brief Khởi tạo ESP-NOW
 */
void ESPNow_Init();

/**
 * @brief Callback khi nhận dữ liệu
 */
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len);

/**
 * @brief Kiểm tra dữ liệu từ đồng hồ đã được nhận chưa
 */
bool IsWristbandDataReceived();

/**
 * @brief Lấy dữ liệu nhịp tim
 */
int GetHeartRate();

/**
 * @brief Lấy dữ liệu nhiệt độ cơ thể
 */
float GetBodyTemperature();

/**
 * @brief Lấy dữ liệu SpO2
 */
int GetSpO2();

#endif // ESPNOW_H
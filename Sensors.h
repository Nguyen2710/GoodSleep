/**
 * @file Sensors.h
 * @brief Module đọc và xử lý dữ liệu từ các cảm biến
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ==================== FORCE SENSORS ====================
/**
 * @brief Đọc giá trị analog từ cảm biến lực
 */
int Force1_Read();
int Force2_Read();
int Force3_Read();

/**
 * @brief Xác định vị trí đầu người dùng
 * @return 0: Không có, 1: Giữa, 2: Phải, 3: Trái
 */
int Force_Read();

/**
 * @brief In thông tin cảm biến lực ra Serial
 */
void PrintForceSensors();

// ==================== AHT20 ====================
/**
 * @brief Đọc nhiệt độ và độ ẩm từ AHT20
 */
void AHT20_Read(float* temperature, float* humidity);

// ==================== VEML7700 (Light) ====================
/**
 * @brief Đọc cường độ ánh sáng (lux)
 */
float Light_Read();

// ==================== PMS (Dust) ====================
/**
 * @brief Cập nhật dữ liệu bụi
 */
void Dust_Update();

/**
 * @brief Đọc giá trị PM2.5
 */
uint16_t Dust_Read();

// ==================== CO2 ====================
/**
 * @brief Cập nhật giá trị CO2 (simulated)
 */
void CO2_Update();

/**
 * @brief Đọc giá trị CO2
 */
uint16_t CO2_Read();

// ==================== NOISE (I2S Microphone) ====================
/**
 * @brief Đọc mức ồn (dB)
 */
float Noise_Read();

#endif // SENSORS_H
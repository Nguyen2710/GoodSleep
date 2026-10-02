/**
 * @file ESPNow.cpp
 * @brief Triển khai module ESP-NOW
 */

#include "Config.h"
#include "Globals.h"
#include "ESPNow.h"

// ==================== ESP-NOW INITIALIZATION ====================

void ESPNow_Init() {
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("ESP-NOW ERR"));
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  
  Serial.println(F("ESP-NOW initialized"));
}

// ==================== DATA CALLBACK ====================

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  // Thêm dòng này để debug
  if (len != sizeof(wristbandData)) {
    Serial.print("Lỗi Size! Nhận được: "); 
    Serial.print(len);
    Serial.print(" bytes. Nhưng Struct S3 yêu cầu: "); 
    Serial.println(sizeof(wristbandData));
    return;
  }
  
  memcpy(&wristbandData, incomingData, sizeof(wristbandData));
  wristbandDataReceived = true;
  
  Serial.print(F("Wristband Data - HR: "));
  Serial.print(wristbandData.heartRate);
  Serial.print(F(", Temp: "));
  Serial.print(wristbandData.temperature);
  Serial.print(F(", SpO2: "));
  Serial.println(wristbandData.spo2);
}

// ==================== DATA GETTERS ====================

bool IsWristbandDataReceived() {
  return wristbandDataReceived;
}

int GetHeartRate() {
  return wristbandData.heartRate;
}

float GetBodyTemperature() {
  return wristbandData.temperature;
}

int GetSpO2() {
  return wristbandData.spo2;
}
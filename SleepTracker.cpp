/**
 * @file SleepTracker.cpp
 * @brief Triển khai module theo dõi giấc ngủ
 */

#include "Config.h"
#include "Globals.h"
#include "SleepTracker.h"
#include "Sensors.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ==================== SLEEP TRACKING ====================
void SleepTracking_Update() {
  int currentPosition = Force_Read();
  bool isOnPillow = (currentPosition > 0);
  unsigned long currentTime = millis();
  
  if (isOnPillow) {
    if (!wasOnPillow) {
      timeOnPillow = currentTime;
      timeOffPillow = 0;
    }
    wasOnPillow = true;
    if (!isSleeping && (currentTime - timeOnPillow >= 10000)) {
      isSleeping = true;
      Serial.println(F("====> ĐÃ VÀO TRẠNG THÁI NGỦ! BẮT ĐẦU THEO DÕI <===="));
      
      sleepStartTime = timeOnPillow;
      sleepStartTimestamp = getCurrentTimestamp();
      if (sleepStartTimestamp == 0) {
        Serial.println(F("WARNING: sleepStartTimestamp = 0, NTP chua sync!"));
      }
      lastPosition = currentPosition;
      lastPositionTime = currentTime;
      timeLeft = 0;
      timeRight = 0;
      timeCenter = 0;
      positionChangeCount = 0;
      lastDataSendTime = currentTime;
    }
    
    if (isSleeping) {
      if (lastPositionTime > 0) {
        unsigned long timeInPosition = currentTime - lastPositionTime;
        if (lastPosition == 1) {
          timeCenter += timeInPosition;
        } else if (lastPosition == 2) {
          timeRight += timeInPosition;
        } else if (lastPosition == 3) {
          timeLeft += timeInPosition;
        }
      }
      
      if (currentPosition != lastPosition && lastPosition > 0) {
        positionChangeCount++;
      }
      
      lastPosition = currentPosition;
      lastPositionTime = currentTime;
    }
  } else {
    if (wasOnPillow) {
      timeOffPillow = currentTime;
      sleepEndTime = currentTime;
      if (isSleeping) {
        sleepEndTimestamp = getCurrentTimestamp();
        if (sleepEndTimestamp == 0) {
          Serial.println(F("WARNING: sleepEndTimestamp = 0, NTP chua sync!"));
        }
      }
      
      if (isSleeping && lastPositionTime > 0) {
        unsigned long timeInPosition = currentTime - lastPositionTime;
        if (lastPosition == 1) {
          timeCenter += timeInPosition;
        } else if (lastPosition == 2) {
          timeRight += timeInPosition;
        } else if (lastPosition == 3) {
          timeLeft += timeInPosition;
        }
      }
    }
    wasOnPillow = false;
    
    if (isSleeping && timeOffPillow > 0 && (currentTime - timeOffPillow >= 60000)) {
      SaveSleepRecord();
      
      isSleeping = false;
      Serial.println(F("====> ĐÃ KẾT THÚC GIẤC NGỦ! <===="));
      
      PrintSleepStatistics();
      SendSleepStatisticsToServer();
      
      timeOnPillow = 0;
      timeOffPillow = 0;
      lastPosition = -1;
      lastPositionTime = 0;
      timeLeft = 0;
      timeRight = 0;
      timeCenter = 0;
      positionChangeCount = 0;
      sleepStartTime = 0;
      sleepEndTime = 0;
      sleepStartTimestamp = 0;
      sleepEndTimestamp = 0;
      autoComfortActive = false;
      autoComfortStartTime = 0;
    }
    
    if (!isSleeping && timeOnPillow > 0 && timeOffPillow > 0) {
      if (currentTime - timeOffPillow >= 5000) { 
        timeOnPillow = 0;
        timeOffPillow = 0;
        Serial.println(F("Đã reset bộ đếm do mất tín hiệu lực ép quá 5s!"));
      }
    }
  }
}

void PrintSleepStatistics() {
  if (sleepEndTime == 0 || sleepStartTime == 0) {
    return;
  }
  
  unsigned long totalSleepTime = sleepEndTime - sleepStartTime;
  
  Serial.print(F("Sleep:"));
  Serial.print(totalSleepTime / 60000);
  Serial.print(F("m L:"));
  Serial.print(timeLeft / 60000);
  Serial.print(F(" C:"));
  Serial.print(timeCenter / 60000);
  Serial.print(F(" R:"));
  Serial.print(timeRight / 60000);
  Serial.print(F(" Ch:"));
  Serial.println(positionChangeCount);
}

void SaveSleepRecord() {
  if (sleepEndTime == 0 || sleepStartTime == 0) {
    return;
  }
  
  if (sleepStartTimestamp == 0 || sleepEndTimestamp == 0) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("ERROR: Khong the lay thoi gian tu NTP trong SaveSleepRecord!");
      return;
    }
    
    time_t unixTime = mktime(&timeinfo);
    
    if (unixTime < 1577836800UL) {
      Serial.print("ERROR: Unix timestamp (mktime()) khong hop le trong SaveSleepRecord: ");
      Serial.println(unixTime);
      return;
    }
    
    unsigned long currentTimestamp = (unsigned long)unixTime;
    unsigned long totalSleepTime = sleepEndTime - sleepStartTime;
    sleepEndTimestamp = currentTimestamp;
    sleepStartTimestamp = currentTimestamp - (totalSleepTime / 1000);
    
    Serial.print("INFO: Da tinh lai timestamp - startTime: ");
    Serial.print(sleepStartTimestamp);
    Serial.print(", endTime: ");
    Serial.println(sleepEndTimestamp);
  }
  
  if (sleepStartTimestamp < 1577836800UL || sleepEndTimestamp < 1577836800UL) {
    Serial.print("ERROR: Timestamp khong hop le truoc khi luu - startTime: ");
    Serial.print(sleepStartTimestamp);
    Serial.print(" (giay), endTime: ");
    Serial.print(sleepEndTimestamp);
    Serial.println(" (giay)");
    return;
  }
  
  if (sleepRecordCount >= MAX_SLEEP_RECORDS) {
    for (int i = 0; i < MAX_SLEEP_RECORDS - 1; i++) {
      sleepRecords[i] = sleepRecords[i + 1];
    }
    sleepRecordCount = MAX_SLEEP_RECORDS - 1;
  }
  
  unsigned long totalSleepTime = sleepEndTime - sleepStartTime;
  sleepRecords[sleepRecordCount].startTime = sleepStartTimestamp;
  sleepRecords[sleepRecordCount].endTime = sleepEndTimestamp;
  sleepRecords[sleepRecordCount].timeLeft = timeLeft;
  sleepRecords[sleepRecordCount].timeRight = timeRight;
  sleepRecords[sleepRecordCount].timeCenter = timeCenter;
  sleepRecords[sleepRecordCount].positionChanges = positionChangeCount;
  sleepRecords[sleepRecordCount].totalTime = totalSleepTime;
  sleepRecords[sleepRecordCount].isCompleteSleep = (totalSleepTime >= 21600000);
  
  sleepRecordCount++;
}

void PrintSleepHistory() {
  Serial.print(F("Records:"));
  Serial.println(sleepRecordCount);
}

void GetSleepStatistics(unsigned long* totalTime, unsigned long* leftTime, 
                        unsigned long* rightTime, unsigned long* centerTime, 
                        int* changes) {
  if (isSleeping) {
    unsigned long currentTime = millis();
    *totalTime = currentTime - sleepStartTime;
  } else if (sleepEndTime > 0) {
    *totalTime = sleepEndTime - sleepStartTime;
  } else {
    *totalTime = 0;
  }
  *leftTime = timeLeft;
  *rightTime = timeRight;
  *centerTime = timeCenter;
  *changes = positionChangeCount;
}

SleepRecord GetSleepRecord(int index) {
  if (index >= 0 && index < sleepRecordCount) {
    return sleepRecords[index];
  }
  SleepRecord empty = {0, 0, 0, 0, 0, 0, 0, false};
  return empty;
}

void ClearSleepHistory() {
  sleepRecordCount = 0;
}

// ==================== SERVER COMMUNICATION ====================

void SendSleepStatisticsToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi chưa kết nối, không thể gửi thống kê");
    return;
  }
  
  HTTPClient http;
  String url = String(serverURL) + String(sleepStatEndpoint);
  http.begin(url);
  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");
  
  if (sleepRecordCount == 0) {
    Serial.println("Không có dữ liệu thống kê để gửi");
    http.end();
    return;
  }
  
  SleepRecord record = sleepRecords[sleepRecordCount - 1];
  
  StaticJsonDocument<256> doc;
  doc["deviceID"] = 1;
  doc["type"] = "sleep_statistics";
  int64_t startTimeMs = (int64_t)record.startTime * 1000LL;
  int64_t endTimeMs = (int64_t)record.endTime * 1000LL;
  doc["startTime"] = startTimeMs;
  doc["endTime"] = endTimeMs;
  doc["totalTime"] = record.totalTime / 1000;
  doc["totalSleepHours"] = record.totalTime / 3600000.0;
  doc["isCompleteSleep"] = record.isCompleteSleep;
  doc["timeLeft"] = record.timeLeft / 1000;
  doc["timeRight"] = record.timeRight / 1000;
  doc["timeCenter"] = record.timeCenter / 1000;
  doc["positionChanges"] = record.positionChanges;
  
  String jsonString;
  jsonString.reserve(200);
  serializeJson(doc, jsonString);
  
  int httpResponseCode = -1;
  int retryCount = 0;
  const int maxRetries = 2;
  
  while (retryCount < maxRetries && httpResponseCode <= 0) {
    httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      break;
    } else {
      retryCount++;
      if (retryCount < maxRetries) {
        delay(500);
      }
    }
  }
  
  http.end();
}

int GetSleepStage() {
  if (!isSleeping) return 0;
  
  unsigned long currentTime = millis();
  unsigned long sleepDuration = currentTime - sleepStartTime;
  
  if (sleepDuration <= STAGE1_DURATION) {
    return 1;
  }
  
  if (sleepDuration > (STAGE1_DURATION + STAGE2_DURATION)) {
    return 3;
  }
  
  return 2;
}

void SendSleepData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Lỗi gửi Data: WiFi chưa kết nối!"));
    return;
  }
  
  HTTPClient http;
  String url = String(serverURL) + String(sleepDataEndpoint);
  http.begin(url);
  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");
  
  float temp, hum;
  AHT20_Read(&temp, &hum);
  int force = Force_Read();
  uint16_t co2 = CO2_Read();
  uint16_t dust = Dust_Read();
  float light = Light_Read();
  float noise = Noise_Read();
  
  // Tăng lên 512 byte để tránh lỗi tạo chuỗi rỗng
  StaticJsonDocument<512> doc;
  doc["deviceID"] = 1;
  doc["type"] = "sleep_data";
  doc["stage"] = GetSleepStage();
  doc["sleepTime"] = (millis() - sleepStartTime) / 1000;
  
  doc["temperature"] = temp;
  doc["humidity"] = hum;
  doc["co2"] = co2;
  doc["pm25"] = dust;
  doc["light"] = light;
  doc["noise"] = noise;
  doc["position"] = force;
  
  if (wristbandDataReceived) {
    doc["heartRate"] = wristbandData.heartRate;
    doc["bodyTemperature"] = wristbandData.temperature;
    doc["spo2"] = wristbandData.spo2;
  }
  
  String jsonString;
  jsonString.reserve(256);
  serializeJson(doc, jsonString);
  
  Serial.print(F("Đang bắn data lên Server: "));
  Serial.println(jsonString);
  
  int httpResponseCode = -1;
  int retryCount = 0;
  const int maxRetries = 2;
  
  while (retryCount < maxRetries && httpResponseCode <= 0) {
    httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      Serial.print(F("Gửi thành công! Mã HTTP: "));
      Serial.println(httpResponseCode);
      break;
    } else {
      Serial.print(F("Lỗi gửi HTTP, mã: "));
      Serial.println(httpResponseCode);
      retryCount++;
      if (retryCount < maxRetries) {
        delay(500);
      }
    }
  }
  
  http.end();
}

void SleepDataSend_Update() {
  unsigned long currentTime = millis();
  int stage = GetSleepStage();
  unsigned long interval = 0;
  
  switch(stage) {
    case 1:
      interval = STAGE1_INTERVAL;
      break;
    case 2:
      interval = STAGE2_INTERVAL;
      break;
    case 3:
      interval = STAGE3_INTERVAL;
      break;
    default:
      return;
  }
  
  if (currentTime - lastDataSendTime >= interval) {
    SendSleepData();
    lastDataSendTime = currentTime;
  }
}

// ==================== NTP TIMESTAMP ====================

unsigned long getCurrentTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("WARNING: Khong the lay thoi gian tu NTP!");
    return 0;
  }
  
  time_t unixTime = mktime(&timeinfo);
  
  if (unixTime < 1577836800UL) {
    Serial.print("WARNING: Unix timestamp (mktime()) khong hop le: ");
    Serial.print(unixTime);
    Serial.print(" - Thoi gian NTP: ");
    Serial.print(&timeinfo, "%Y-%m-%d %H:%M:%S");
    Serial.print(" - Nam: ");
    Serial.print(timeinfo.tm_year + 1900);
    Serial.print(", Thang: ");
    Serial.print(timeinfo.tm_mon + 1);
    Serial.print(", Ngay: ");
    Serial.println(timeinfo.tm_mday);
    return 0;
  }
  
  return (unsigned long)unixTime;
}
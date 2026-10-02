/**
 * @file Globals.h
 * @brief Khai báo biến toàn cục và cấu trúc dữ liệu
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_AHTX0.h> // Thêm thư viện AHT20 thay cho DHT
#include <Adafruit_VEML7700.h>
#include <PMS.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h> // Đã sửa thành ngoặc nhọn
#include <WiFiClientSecure.h>

// ==================== EXTERNAL OBJECTS ====================
extern Adafruit_AHTX0 aht;
extern Adafruit_VEML7700 veml;
extern WebServer server;
extern WiFiManager wifiManager;
extern HardwareSerial DFPlayerSerial;
extern HardwareSerial DustSerial;
extern PMS pms;
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// ==================== STATUS VARIABLES ====================
extern int currentTrack;
extern int volumeLevel;
extern bool isPlaying;

// ==================== TIMER VARIABLES ====================
// Sleep Timer
extern int sleepTimerMinutes;
extern unsigned long sleepTimerStartTime;
extern bool sleepTimerActive;
extern bool sleepTimerStoppedMusic;

// ==================== AUTO COMFORT ====================
extern bool autoComfortActive;
extern unsigned long autoComfortStartTime;
extern bool autoComfortBlocked;

// ==================== SENSOR DATA ====================
extern bool ahtOK;

extern uint16_t pm25_value;
extern bool pmsWarmingUp;
extern unsigned long pmsWarmStart;
extern unsigned long pmsLastRequest;

extern uint16_t co2_value;
extern bool co2_hasValue;
extern unsigned long co2_lastUpdate;
extern bool vemlOK;

// ==================== SLEEP TRACKING ====================
typedef struct {
  unsigned long startTime;
  unsigned long endTime;
  unsigned long timeLeft;
  unsigned long timeRight;
  unsigned long timeCenter;
  int positionChanges;
  unsigned long totalTime;
  bool isCompleteSleep;
} SleepRecord;

extern SleepRecord sleepRecords[MAX_SLEEP_RECORDS];
extern int sleepRecordCount;

extern bool isSleeping;
extern bool wasOnPillow;
extern unsigned long timeOnPillow;
extern unsigned long timeOffPillow;
extern unsigned long sleepStartTime;
extern unsigned long sleepEndTime;
extern unsigned long sleepStartTimestamp;
extern unsigned long sleepEndTimestamp;
extern unsigned long timeLeft;
extern unsigned long timeRight;
extern unsigned long timeCenter;
extern int lastPosition;
extern int positionChangeCount;
extern unsigned long lastPositionTime;
extern unsigned long lastDataSendTime;

// ==================== WRISTBAND DATA (ESP-NOW) ====================
typedef struct struct_message {
  int heartRate;
  float temperature;
  int spo2;
} struct_message;

extern struct_message wristbandData;
extern bool wristbandDataReceived;
extern PMS::DATA pmsData;

#endif // GLOBALS_H
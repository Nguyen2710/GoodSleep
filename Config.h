/**
 * @file Config.h
 * @brief Định nghĩa chân kết nối và hằng số hệ thống
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include <IPAddress.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>

// ==================== PIN DEFINITIONS ====================
#define Force1_PIN 1
#define Force2_PIN 2
#define Force3_PIN 4

#define btn_PIN 39

#define TX_MP3_PIN 36
#define RX_MP3_PIN 35

#define TX_Dust_PIN 41
#define RX_Dust_PIN 40

#define WS_INMP441_PIN 18
#define SD_INMP441_PIN 16
#define SCK_INMP441_PIN 17

#define SDA_PIN 9
#define SCL_PIN 8

// ==================== THRESHOLDS ====================
#define FORCE_THRESHOLD 200

// ==================== SERVER CONFIG ====================
extern const char* serverURL;
extern const char* sleepDataEndpoint;
extern const char* sleepStatEndpoint;

// ==================== PMS CONFIG ====================
extern const unsigned long PMS_WARMUP_MS;
extern const unsigned long PMS_INTERVAL_MS;

// ==================== CO2 CONFIG ====================
extern const unsigned long CO2_UPDATE_INTERVAL;

// ==================== SLEEP TRACKING CONFIG ====================
#define MAX_SLEEP_RECORDS 20
#define STAGE1_DURATION 7200000
#define STAGE2_DURATION 14400000
#define STAGE1_INTERVAL 5000
#define STAGE2_INTERVAL 300000
#define STAGE3_INTERVAL 60000

// ==================== AUTO COMFORT CONFIG ====================
extern const unsigned long AUTO_COMFORT_DURATION;
extern const int RESTLESS_THRESHOLD;

// ==================== I2S CONFIG ====================
#define I2S_SAMPLE_RATE 16000
#define I2S_SAMPLE_BITS 16
#define I2S_BUFFER_SIZE 512

// ==================== NTP CONFIG ====================
extern const char* ntpServer1;
extern const char* ntpServer2;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// ==================== WIFI CONFIG ====================
extern IPAddress staticIP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress dns1;
extern IPAddress dns2;

#endif
#include "Config.h"

// ==================== SERVER CONFIG ====================
// const char* serverURL = "https://goithongminh.cloud";
const char* serverURL = "http://192.168.1.174:3000";
const char* sleepDataEndpoint = "/api/sleep-data";
const char* sleepStatEndpoint = "/api/sleep-stat";

// ==================== PMS CONFIG ====================
const unsigned long PMS_WARMUP_MS = 30000;
const unsigned long PMS_INTERVAL_MS = 5000;

// ==================== CO2 CONFIG ====================
const unsigned long CO2_UPDATE_INTERVAL = 180000;

// ==================== AUTO COMFORT CONFIG ====================
const unsigned long AUTO_COMFORT_DURATION = 1800000;
const int RESTLESS_THRESHOLD = 10;

// ==================== NTP CONFIG ====================
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

// ==================== WIFI CONFIG ====================
IPAddress staticIP(192, 168, 1, 124);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);
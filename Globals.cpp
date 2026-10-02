/**
 * @file Globals.cpp
 * @brief Định nghĩa các biến toàn cục
 */

#include "Config.h"
#include "Globals.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_VEML7700.h>
#include <PMS.h>
#include <HardwareSerial.h>
#include <U8g2lib.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <DFRobotDFPlayerMini.h>
#include <WiFiClientSecure.h>

// ==================== HARDWARE OBJECTS ====================
Adafruit_AHTX0 aht;
Adafruit_VEML7700 veml;
WebServer server(80);
WiFiManager wifiManager;
HardwareSerial DFPlayerSerial(2);
HardwareSerial DustSerial(1);
PMS pms(DustSerial);
PMS::DATA pmsData;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ==================== STATUS VARIABLES ====================
int currentTrack = 1;
int volumeLevel = 15;
bool isPlaying = false;

// ==================== TIMER VARIABLES ====================
// Sleep Timer
int sleepTimerMinutes = 0;
unsigned long sleepTimerStartTime = 0;
bool sleepTimerActive = false;
bool sleepTimerStoppedMusic = false;

// ==================== AUTO COMFORT ====================
bool autoComfortActive = false;
unsigned long autoComfortStartTime = 0;
bool autoComfortBlocked = false;

// ==================== SENSOR DATA ====================
bool ahtOK = false;

uint16_t pm25_value = 0;
bool pmsWarmingUp = true;
unsigned long pmsWarmStart = 0;
unsigned long pmsLastRequest = 0;

uint16_t co2_value = 0;
bool co2_hasValue = false;
unsigned long co2_lastUpdate = 0;
bool vemlOK = false;

// ==================== SLEEP TRACKING ====================
SleepRecord sleepRecords[MAX_SLEEP_RECORDS];
int sleepRecordCount = 0;

bool isSleeping = false;
bool wasOnPillow = false;
unsigned long timeOnPillow = 0;
unsigned long timeOffPillow = 0;
unsigned long sleepStartTime = 0;
unsigned long sleepEndTime = 0;
unsigned long sleepStartTimestamp = 0;
unsigned long sleepEndTimestamp = 0;
unsigned long timeLeft = 0;
unsigned long timeRight = 0;
unsigned long timeCenter = 0;
int lastPosition = -1;
int positionChangeCount = 0;
unsigned long lastPositionTime = 0;
unsigned long lastDataSendTime = 0;

// ==================== WRISTBAND DATA ====================
struct_message wristbandData;
bool wristbandDataReceived = false;
#include "Config.h"
#include "Globals.h"
#include "Sensors.h"
#include "Audio.h"
#include "SleepTracker.h"
#include "Timers_pillow.h"
#include "WebModule.h"
#include "ESPNow.h"

#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <sntp.h>

// ========== THÊM THƯ VIỆN & CẤU HÌNH WS2812 ==========
#include <Adafruit_NeoPixel.h>

#define LED_BTN_PIN    45
#define WS2812_PIN     5
#define NUM_LEDS       1

Adafruit_NeoPixel strip(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// Biến cho logic nút nhấn và đổi màu
int ledState = 0; 
const int MAX_LED_STATES = 7; // 0: Tắt, 1-6: Các màu khác nhau
bool lastLedBtnReading = HIGH;
bool ledBtnState = HIGH;
unsigned long lastLedBtnDebounceTime = 0;
const unsigned long debounceDelay = 50; // Chống dội nút nhấn 50ms

// Hàm cập nhật màu LED dựa trên ledState
void updateLEDColor() {
  switch(ledState) {
    case 0: strip.setPixelColor(0, strip.Color(0, 0, 0)); break;       // Tắt
    case 1: strip.setPixelColor(0, strip.Color(255, 0, 0)); break;     // Đỏ
    case 2: strip.setPixelColor(0, strip.Color(0, 255, 0)); break;     // Xanh lá (Green)
    case 3: strip.setPixelColor(0, strip.Color(0, 0, 255)); break;     // Xanh dương (Blue)
    case 4: strip.setPixelColor(0, strip.Color(255, 255, 0)); break;   // Vàng (Yellow)
    case 5: strip.setPixelColor(0, strip.Color(0, 255, 255)); break;   // Xanh lơ (Cyan)
    case 6: strip.setPixelColor(0, strip.Color(255, 0, 255)); break;   // Tím (Magenta)
    case 7: strip.setPixelColor(0, strip.Color(255, 255, 255)); break; // Trắng (White)
  }
  strip.show();
}

// Hàm xử lý nút nhấn (gọi trong loop)
void handleLEDButton() {
  bool reading = digitalRead(LED_BTN_PIN);
  
  if (reading != lastLedBtnReading) {
    lastLedBtnDebounceTime = millis();
  }
  
  if ((millis() - lastLedBtnDebounceTime) > debounceDelay) {
    if (reading != ledBtnState) {
      ledBtnState = reading;

      if (ledBtnState == LOW) {
        ledState++;
        if (ledState > MAX_LED_STATES) {
          ledState = 0;
        }
        updateLEDColor();
        Serial.print(F("Đổi màu LED sang trạng thái: "));
        Serial.println(ledState);
      }
    }
  }
  lastLedBtnReading = reading;
}
// =====================================================

// ==================== SETUP ====================
void setup() {
  delay(1000);
  yield();
  
  // Serial
  Serial.begin(9600);
  delay(800);
  yield();

  // ========== GPIO Setup ==========
  pinMode(Force1_PIN, INPUT);
  pinMode(Force2_PIN, INPUT);
  pinMode(Force3_PIN, INPUT);
  pinMode(btn_PIN, INPUT_PULLUP);
  
  // Khởi tạo chân nút nhấn LED
  pinMode(LED_BTN_PIN, INPUT_PULLUP); 
  delay(100);
  yield();

  // ========== WS2812 Setup ==========
  strip.begin();
  strip.setBrightness(255);
  updateLEDColor();
  yield();

  // ========== Audio Setup ==========
  Audio_Init();
  I2S_Init();
  delay(100);
  yield();

  // ========== I2C Setup ==========
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(200);
  yield();

  // ========== AHT20 Setup ==========
  if (!aht.begin()) {
    ahtOK = false;
    Serial.println(F("AHT20 ERR"));
  } else {
    ahtOK = true;
    Serial.println(F("AHT20 OK"));
  }
  delay(50);
  yield();

  // ========== VEML7700 Setup ==========
  if (!veml.begin()) {
    vemlOK = false;
    Serial.println(F("VEML7700 ERR"));
  } else {
    vemlOK = true;
    veml.setGain(VEML7700_GAIN_1);
    veml.setIntegrationTime(VEML7700_IT_100MS);
    Serial.println(F("VEML7700 OK"));
  }
  delay(50);
  yield();

  // ========== Dust Sensor Setup ==========
  DustSerial.begin(9600, SERIAL_8N1, RX_Dust_PIN, TX_Dust_PIN);
  delay(100);
  yield();
  
  pmsWarmingUp = true;
  pmsWarmStart = millis();
  Serial.println(F("ZH03B warming 30s..."));
  yield();

  // ========== OLED Display Setup ==========
  u8g2.begin();
  delay(100);
  yield();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  Serial.println(F("OLED initialized"));

  // ========== WiFi Setup ==========
  WiFi.mode(WIFI_AP_STA);
  yield();

  WiFi.config(staticIP, gateway, subnet, dns1, dns2);
  yield();

  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

  if (!wifiManager.autoConnect("SmartPillow-AP")) {
    Serial.println(F("WiFi timeout"));
    yield();
    delay(3000);
    yield();
    ESP.restart();
  }

  Serial.print(F("WiFi OK IP:"));
  Serial.println(WiFi.localIP());
  yield();

  // ========== AP Mode ==========
  delay(500);
  yield();
  WiFi.softAP("SmartPillow-AP", "", 1, 0);
  delay(100);
  yield();
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  delay(100);
  yield();
  Serial.print(F("AP đã bật - SSID: SmartPillow-AP, IP: "));
  Serial.println(WiFi.softAPIP());
  yield();

  // ========== NTP Time Sync ==========
  Serial.println(F("========================================"));
  Serial.println(F("DANG DONG BO THOI GIAN TU NTP..."));
  Serial.println(F("========================================"));
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  yield();

  struct tm timeinfo;
  int ntpRetryCount = 0;
  const int NTP_MAX_RETRIES = 30;

  Serial.print(F("Dang cho NTP sync"));
  while (!getLocalTime(&timeinfo) && ntpRetryCount < NTP_MAX_RETRIES) {
    delay(500);
    ntpRetryCount++;
    if (ntpRetryCount % 4 == 0) {
      Serial.print(F("."));
    }
    yield();
  }
  Serial.println();

  if (getLocalTime(&timeinfo)) {
    Serial.println(F("========================================"));
    Serial.print(F("NTP DONG BO THANH CONG!"));
    Serial.print(F(" - Thoi gian: "));
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
    
    time_t unixTime = mktime(&timeinfo);
    
    if (unixTime > 1577836800UL) {
      Serial.print(F("Unix timestamp: "));
      Serial.println(unixTime);
      Serial.println(F("========================================"));
    } else {
      Serial.println(F("ERROR: Invalid NTP time!"));
      delay(3000);
      ESP.restart();
    }
  } else {
    Serial.println(F("========================================"));
    Serial.println(F("LOI: KHONG THE DONG BO NTP!"));
    Serial.println(F("ESP32 se restart..."));
    Serial.println(F("========================================"));
    delay(3000);
    ESP.restart();
  }
  yield();

  // ========== ESP-NOW Setup ==========
  ESPNow_Init();
  yield();

  // ========== Web Server Setup ==========
  SetupWebServer();
  yield();

  // ========== Initialize CO2 (Simulated) ==========
  co2_value = random(400, 1000);
  co2_hasValue = true;
  co2_lastUpdate = millis();

  Serial.println(F("========================================"));
  Serial.println(F("SmartPillow Initialized Successfully!"));
  Serial.println(F("========================================"));
  delay(200);
  yield();

  Serial.print("Đang chạy Wi-Fi ở kênh: ");
  Serial.println(WiFi.channel());
}

// ==================== MAIN LOOP ====================
void loop() {
  server.handleClient();
  
  Dust_Update();
  CO2_Update();
  SleepTracking_Update();
  AutoComfort_Update();
  Display_Update();
  Button_StopAutoComfort();
  SleepTimer_Update();
  
  PrintForceSensors();
  handleLEDButton();

  if (isSleeping) {
    SleepDataSend_Update();
  }
  delay(100);
}
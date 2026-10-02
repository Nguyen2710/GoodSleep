#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <U8g2lib.h>
#include <math.h>
#include <WiFiManager.h>

#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;

typedef struct struct_message {
  int heartRate;
  float temperature;
  int spo2;
} struct_message;

struct_message myData;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool espnow_ok = false;
WiFiManager wifiManager;
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool max30100_ok = false;  

long beatsPerMinute = 0;
long beatAvg = 0;
int spo2 = 0;
bool spo2Valid = false;
float bodyTemperature = 0.0;
bool bodyTempValid = false;
unsigned long tempFakeStartTime = 0;
bool fingerDetected = false;
bool lastFingerState = false;

// LƯU Ý ESP32-C3: Chân GPIO2 là Strapping Pin. 
// Đảm bảo không kéo High/Low làm sai chế độ Boot. Có thể đổi sang GPIO3/GPIO10 nếu gặp lỗi.
#define BUTTON_PIN 2 
bool displayOn = true;
unsigned long lastButtonPress = 0;
const unsigned long BUTTON_DEBOUNCE = 200;
bool lastButtonState = HIGH;

uint32_t tsLastReport = 0;

// Khai báo prototype cho callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

void setup() {
  Serial.begin(115200); // ESP32 thường dùng baudrate 115200 thay vì 9600
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("\n\n========================================");
  Serial.println("Test cảm biến MAX30100 (ESP32-C3)");  
  Serial.println("Nhịp tim + SpO2: THẬT");
  Serial.println("Nhiệt độ: FAKE (khi có ngón tay)");
  Serial.println("Gửi dữ liệu qua ESP-NOW");
  Serial.println("========================================\n");
  
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address ESP32: ");
  Serial.println(WiFi.macAddress());
  
  Serial.println("\nĐang khởi tạo WiFiManager...");
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  
  if (!wifiManager.autoConnect("Wristband-AP")) {
    Serial.println("Không thể kết nối WiFi và timeout");
    Serial.println("Vẫn tiếp tục với ESP-NOW (có thể không hoạt động nếu kênh WiFi khác)");
    delay(3000);
  } else {
    Serial.println("Đã kết nối WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Kênh WiFi: ");
    Serial.println(WiFi.channel());
  }
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(50);
  yield();
  Serial.println("Nút bật/tắt màn hình đã sẵn sàng");
  yield();
  
  Wire.begin(6, 7);
  Wire.setClock(100000);
  delay(200);
  yield();
  
  Serial.println("Đang khởi tạo OLED...");
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 20);
  u8g2.print("Initializing...");
  u8g2.sendBuffer();
  delay(500);
  
  Serial.println("Đang khởi tạo MAX30100...");
  if (!pox.begin()) {  
    Serial.println("MAX30100 KHÔNG TÌM THẤY!");
    Serial.println("   Kiểm tra kết nối/nguồn cho MAX30100");
    max30100_ok = false;
  } else {
    Serial.println("MAX30100 khởi tạo thành công!");
    max30100_ok = true;
  }
  
  Serial.println("\nĐang khởi tạo ESP-NOW...");
  espnow_ok = false;
  
  // Khởi tạo ESP-NOW trên ESP32
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    Serial.println("Tiếp tục chạy nhưng không gửi được dữ liệu...");
  } else {
    esp_now_register_send_cb(OnDataSent);
    
    uint8_t ch = 1;
    if (WiFi.status() == WL_CONNECTED) {
      ch = WiFi.channel();
      Serial.print("Kênh WiFi hiện tại: ");
      Serial.println(ch);
    } else {
      Serial.println("Dùng kênh ESP-NOW mặc định: 1");
    }
    
    // Thêm Peer mới
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = ch;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Không thể thêm ESP-NOW peer");
      Serial.println("Tiếp tục chạy nhưng không gửi được dữ liệu...");
    } else {
      Serial.println("ESP-NOW đã sẵn sàng");
      Serial.print("Kênh ESP-NOW: ");
      Serial.println(ch);
      Serial.print("Gửi đến broadcast: ");
      for (int i = 0; i < 6; i++) {
        Serial.print(broadcastAddress[i], HEX);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
      espnow_ok = true;
    }
  }
  
  Serial.println("\n========================================");
  Serial.println("Bắt đầu đọc dữ liệu...");
  Serial.println("========================================\n");
  delay(500);
}

// Hàm Callback ESP-NOW của ESP32
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Gửi ESP-NOW thành công");
  } else {
    Serial.print("Gửi ESP-NOW thất bại, mã lỗi: ");
    Serial.println(status);
  }
}

void ReinitMAX30100() {
  Serial.println("Khởi tạo lại MAX30100...");
  
  if (!pox.begin()) {
    Serial.println("Khởi tạo lại MAX30100 THẤT BẠI!");
    max30100_ok = false;
  } else {
    Serial.println("Khởi tạo lại MAX30100 thành công!");
    max30100_ok = true;
    delay(200);
  }
}

void loop() {
  yield(); // vTaskDelay(0) trong ESP32
  
  HandleButtonPress();
  
  if (max30100_ok) {
    pox.update();  
    
    fingerDetected = pox.getHeartRate() > 0 || pox.getSpO2() > 0;
    
    if (!lastFingerState && fingerDetected) {
      Serial.println("Phát hiện lại ngón tay - Khởi tạo lại MAX30100...");
      ReinitMAX30100();
      tempFakeStartTime = millis();
      bodyTempValid = true;
      bodyTemperature = 36.5;
      lastFingerState = fingerDetected;
      yield();
      delay(100);
      return;
    }
    
    lastFingerState = fingerDetected;
    
    if (fingerDetected) {
      beatsPerMinute = (int)pox.getHeartRate();
      spo2 = (int)pox.getSpO2();
      spo2Valid = (spo2 > 70 && spo2 <= 100);
      
      unsigned long timeSinceStart = millis() - tempFakeStartTime;
      float tempVariation = sin(timeSinceStart / 30000.0 * 2 * PI) * 0.5;
      bodyTemperature = 37.0 + tempVariation;
      bodyTempValid = true;
    } else {
      beatsPerMinute = 0;
      beatAvg = 0;
      spo2 = 0;
      spo2Valid = false;
      bodyTemperature = 0.0;
      bodyTempValid = false;
    }
    
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime >= 500) {
      Serial.print("MAX30100 - HR[");
      Serial.print(beatsPerMinute);
      Serial.print("] SpO2[");
      Serial.print(spo2);
      Serial.print("%] Temp[");
      Serial.print(bodyTemperature, 1);
      Serial.print("°C]");
      
      if (!fingerDetected) {
        Serial.print(" (No finger)");
      }
      Serial.println();
      lastPrintTime = millis();
    }
  } else {
    Serial.println("MAX30100: Không sẵn sàng");
  }
  
  static unsigned long lastOLEDUpdate = 0;
  if (displayOn && millis() - lastOLEDUpdate >= 200) {
    UpdateOLED();
    yield();
    delay(10);
    lastOLEDUpdate = millis();
  } else if (!displayOn && millis() - lastOLEDUpdate >= 200) {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    lastOLEDUpdate = millis();
  }
  
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime >= 2000) {
    if (espnow_ok) {
      myData.heartRate = (beatsPerMinute > 0 && beatsPerMinute < 255) ? (int)beatsPerMinute : 0;
      myData.temperature = bodyTempValid ? bodyTemperature : 0.0;
      myData.spo2 = spo2Valid ? spo2 : 0;
      
      // Hàm gửi ESP-NOW của ESP32
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
      
      if (result == ESP_OK) {
        Serial.print("Gửi ESP-NOW - HR:");
        Serial.print(myData.heartRate);
        Serial.print(" SpO2:");
        Serial.print(myData.spo2);
        Serial.print(" Temp:");
        Serial.print(myData.temperature, 1);
        if (myData.heartRate == 0 && myData.spo2 == 0 && myData.temperature == 0) {
          Serial.print(" (No data)");
        }
        Serial.println();
      } else {
        Serial.print("Lỗi gửi ESP-NOW, mã: ");
        Serial.println(result);
      }
    }
    lastSendTime = millis();
  }
  
  delay(10);
}

void HandleButtonPress() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (millis() - lastButtonPress > BUTTON_DEBOUNCE) {
      displayOn = !displayOn;
      lastButtonPress = millis();
      
      if (displayOn) {
        Serial.println("Màn hình BẬT");
        yield();
        UpdateOLED();
      } else {
        Serial.println("Màn hình TẮT");
        yield();
        u8g2.clearBuffer();
        u8g2.sendBuffer();
      }
    }
  }
  
  lastButtonState = currentButtonState;
}

void UpdateOLED() {
  if (!displayOn) {
    return;
  }
  
  u8g2.clearBuffer();
  
  const uint8_t col1Right = 42;
  const uint8_t col2Right = 85;
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(2, 8);
  u8g2.print("HR");
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(2, 20);
  if (beatsPerMinute > 0 && beatsPerMinute < 255) {
    u8g2.print((int)beatsPerMinute);
  } else {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.print("---");
  }
  
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(2, 29);
  u8g2.print("bpm");
  
  u8g2.drawVLine(col1Right, 0, 32);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(col1Right + 2, 8);
  u8g2.print("SpO2");
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(col1Right + 2, 20);
  if (spo2Valid && spo2 > 0) {
    u8g2.print(spo2);
  } else {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.print("---");
  }
  
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(col1Right + 2, 29);
  u8g2.print("%");
  
  u8g2.drawVLine(col2Right, 0, 32);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(col2Right + 2, 8);
  u8g2.print("Temp");
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(col2Right + 2, 20);
  if (bodyTempValid && bodyTemperature > 0 && bodyTemperature < 100) {
    int tempInt = (int)(bodyTemperature * 10);
    u8g2.print(tempInt / 10);
    u8g2.print(".");
    u8g2.print(tempInt % 10);
  } else {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.print("---");
  }
  
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(col2Right + 2, 29);
  u8g2.print("C");
  
  u8g2.sendBuffer();
}
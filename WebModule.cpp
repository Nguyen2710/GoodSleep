/**
 * @file WebServer.cpp
 * @brief Triển khai module web server
 */

#include "Config.h"
#include "Globals.h"
#include "WebModule.h"
#include "Audio.h"
#include "Sensors.h"
#include "Timers_pillow.h"
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WebServer.h>

// ==================== WEB SERVER SETUP ====================

void SetupWebServer() {
  server.on("/", handleRoot);
  server.on("/wifi", handleWiFiConfig);
  
  server.on("/api/music/play", HTTP_POST, handleMusicPlay);
  server.on("/api/music/stop", HTTP_POST, handleMusicStop);
  server.on("/api/music/pause", HTTP_POST, handleMusicPause);
  server.on("/api/volume", HTTP_POST, handleVolume);
  server.on("/api/sleep-timer", HTTP_POST, handleSleepTimer);
  server.on("/api/status", HTTP_GET, handleStatus);
  
  server.on("/api/wifi/connect", HTTP_POST, handleWiFiConnect);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/status", HTTP_GET, handleWiFiStatus);
  server.on("/api/wifi/reset", HTTP_POST, handleWiFiReset);
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Web Server đã khởi động trên port 80");
}

// ==================== DISPLAY UPDATE ====================

void Display_Update() {

  u8g2.clearBuffer();

  float temp = -1, hum = -1;
  AHT20_Read(&temp, &hum);

  uint16_t co2  = CO2_Read();
  uint16_t dust = Dust_Read();

  float light = Light_Read();
  float noise = Noise_Read();

  // Nếu sensor trả giá trị rác / lỗi thì ép về -1
  if (isnan(temp))  temp  = -1;
  if (isnan(hum))   hum   = -1;
  if (light < 0 || isnan(light)) light = -1;
  if (noise < 0 || isnan(noise)) noise = -1;

  // Nếu CO2 hoặc dust trả 0 bất thường
  if (co2 == 0)  co2  = -1;
  if (dust == 0) dust = -1;

  const uint8_t y1 = 10;
  const uint8_t y2 = 22;
  const uint8_t y3 = 34;
  const uint8_t y4 = 46;
  const uint8_t y5 = 58;

  // ===== Wristband =====
  if (wristbandDataReceived) {
    u8g2.setCursor(0, y1);
    u8g2.print("HR:");
    u8g2.print(wristbandData.heartRate);
    u8g2.print(" bpm   T:");
    u8g2.print(wristbandData.temperature, 1);
    u8g2.print(" C");

    u8g2.setCursor(80, y2);
    u8g2.print(" O2:");
    u8g2.print(wristbandData.spo2);
    u8g2.print("%");
  }

  // ===== CO2 =====
  u8g2.setCursor(0, y2);
  u8g2.print("CO2:");
  u8g2.print(co2);
  u8g2.print("ppm");

  // ===== Dust =====
  u8g2.setCursor(0, y3);
  u8g2.print("PM2.5:");
  u8g2.print(dust);
  u8g2.print("ug/m3");

  // ===== Light & Noise =====
  u8g2.setCursor(0, y4);
  u8g2.print("Lux:");
  u8g2.print(light, 0);
  u8g2.print("  dB:");
  u8g2.print(noise, 0);

  // ===== Temp & Hum =====
  u8g2.setCursor(0, y5);
  u8g2.print("T:");
  u8g2.print(temp, 1);
  u8g2.print("C  H:");
  u8g2.print(hum, 0);
  u8g2.print("%");

  u8g2.sendBuffer();
}

// ==================== HTTP HANDLERS ====================

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"><title>Smart Pillow</title><style>");
  html += F("*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:20px;min-height:100vh}");
  html += F(".c{max-width:600px;margin:0 auto;background:#fff;border-radius:20px;padding:30px;box-shadow:0 10px 40px rgba(0,0,0,.2)}");
  html += F("h1{color:#333;text-align:center;margin-bottom:30px;font-size:28px}");
  html += F(".g{margin-bottom:30px;padding:20px;background:#f8f9fa;border-radius:15px}");
  html += F(".g h2{color:#555;margin-bottom:15px;font-size:20px}");
  html += F(".s{width:100%;height:8px;border-radius:5px;background:#ddd;outline:0;-webkit-appearance:none}.s::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;border-radius:50%;background:#667eea;cursor:pointer}.s::-moz-range-thumb{width:20px;height:20px;border-radius:50%;background:#667eea;cursor:pointer;border:0}");
  html += F(".v{text-align:center;font-size:24px;font-weight:bold;color:#667eea;margin:10px 0}");
  html += F(".b{display:flex;gap:10px;flex-wrap:wrap}button{flex:1;min-width:100px;padding:12px 20px;border:0;border-radius:10px;font-size:16px;font-weight:bold;cursor:pointer}");
  html += F(".p{background:#667eea;color:#fff}.p:hover{background:#5568d3}");
  html += F(".d{background:#e74c3c;color:#fff}.d:hover{background:#c0392b}");
  html += F(".su{background:#27ae60;color:#fff}.su:hover{background:#229954}");
  html += F("input[type=number]{width:100%;padding:10px;border:2px solid #ddd;border-radius:8px;font-size:16px;margin:10px 0}.st{text-align:center;padding:10px;margin:10px 0;border-radius:8px;font-weight:bold}.st.s{background:#d4edda;color:#155724}.st.e{background:#f8d7da;color:#721c24}");
  html += F("</style></head><body><div class=\"c\"><h1>Smart Pillow</h1>");
  html += F("<div style=\"text-align:center;margin-bottom:20px\"><a href=\"/wifi\" style=\"color:#667eea;text-decoration:none;font-weight:bold\">WiFi Config</a></div>");
  
  html += F("<div class=\"g\"><h2>Music</h2><input type=\"number\" id=\"ti\" min=\"1\" max=\"3000\" value=\"1\" placeholder=\"1-3000\">");
  html += F("<div class=\"b\"><button class=\"p\" onclick=\"playM()\">Play</button><button class=\"d\" onclick=\"stopM()\">Stop</button><button class=\"su\" onclick=\"pauseM()\">Pause</button></div></div>");
  
  html += F("<div class=\"g\"><h2>Volume</h2><input type=\"range\" min=\"0\" max=\"30\" value=\"15\" class=\"s\" id=\"vs\" oninput=\"document.getElementById('vv').textContent=this.value;sendV(this.value)\"><div class=\"v\" id=\"vv\">15</div></div>");
  
  html += F("<div class=\"g\"><h2>Sleep Timer</h2><input type=\"range\" min=\"0\" max=\"120\" value=\"0\" class=\"s\" id=\"stimer\" oninput=\"var v=this.value;var d=document.getElementById('stv');d.textContent=v==0?'Off':v+'m';sendST(v)\"><div class=\"v\" id=\"stv\">Off</div><div id=\"stcountdown\" style=\"text-align:center;color:#666;font-size:14px;margin-top:5px\"></div></div>");
  
  html += F("<div id=\"st\" class=\"st\"></div></div><script>");
  
  html += F("function playM(){var t=document.getElementById('ti').value;fetch('/api/music/play',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({track:parseInt(t)})}).then(r=>showS('Playing: '+t,'s'))}");
  html += F("function stopM(){fetch('/api/music/stop',{method:'POST'}).then(r=>showS('Stopped','s'))}");
  html += F("function pauseM(){fetch('/api/music/pause',{method:'POST'}).then(r=>showS('Paused','s'))}");
  html += F("function sendV(l){fetch('/api/volume',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({volume:parseInt(l)})}).then(r=>showS('Volume: '+l,'s'))}");
  html += F("function sendST(m){fetch('/api/sleep-timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({minutes:parseInt(m)})}).then(r=>r.json()).then(d=>{if(d.status=='ok'){showS('Sleep Timer: '+m+'p','s');u()}else{showS('Error','e')}})}");
  
  html += F("function u(){fetch('/api/status').then(r=>r.json()).then(d=>{var cd=document.getElementById('stcountdown');if(d.sleepTimer>0&&d.sleepTimerActive){var x=Math.floor(d.sleepTimerRemaining/60),y=d.sleepTimerRemaining%60;cd.textContent='Còn lại: '+x+':'+(y<10?'0':'')+y;cd.style.color='#667eea'}else{cd.textContent=''}})}");
  
  html += F("function showS(m,t){var s=document.getElementById('st');s.textContent=m;s.className='st '+t;setTimeout(function(){s.textContent=''},2000)}");
  html += F("setInterval(u,1000);u();");
  html += F("</script></body></html>");
  
  server.send(200, "text/html", html);
}

void handleMusicPlay() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    if (doc.containsKey("track")) {
      currentTrack = doc["track"];
      if (currentTrack < 1) currentTrack = 1;
      if (currentTrack > 3000) currentTrack = 3000;
      
      sleepTimerStoppedMusic = false;
      
      MP3_Play(currentTrack);
      isPlaying = true;
      
      server.send(200, "application/json", "{\"status\":\"ok\",\"track\":" + String(currentTrack) + "}");
      Serial.print("Web: Playing track ");
      Serial.println(currentTrack);
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing track\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleMusicStop() {
  MP3_Stop(false);
  isPlaying = false;
  
  server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"stop\"}");
  Serial.println("Web: Music stopped");
}

void handleMusicPause() {
  MP3_Stop(true);
  isPlaying = false;
  
  server.send(200, "application/json", "{\"status\":\"ok\",\"action\":\"pause\"}");
  Serial.println("Web: Music paused");
}

void handleVolume() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    if (doc.containsKey("volume")) {
      volumeLevel = doc["volume"];
      if (volumeLevel < 0) volumeLevel = 0;
      if (volumeLevel > 30) volumeLevel = 30;
      
      MP3_SetVolume(volumeLevel);
      
      server.send(200, "application/json", "{\"status\":\"ok\",\"volume\":" + String(volumeLevel) + "}");
      Serial.print("Web: Volume set to ");
      Serial.println(volumeLevel);
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing volume\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleSleepTimer() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    if (doc.containsKey("minutes")) {
      sleepTimerMinutes = doc["minutes"];
      
      if (sleepTimerMinutes > 0) {
        sleepTimerActive = true;
        sleepTimerStartTime = millis();
        sleepTimerStoppedMusic = false;
        Serial.print(F("Web: Sleep Timer: "));
        Serial.print(sleepTimerMinutes);
        Serial.println(F("m"));
      } else {
        sleepTimerActive = false;
        sleepTimerStartTime = 0;
        sleepTimerStoppedMusic = false;
        Serial.println(F("Web: Sleep Timer off"));
      }
      
      StaticJsonDocument<200> response;
      response["status"] = "ok";
      response["sleepTimer"] = sleepTimerMinutes;
      response["active"] = sleepTimerActive;
      
      String jsonResponse;
      serializeJson(response, jsonResponse);
      server.send(200, "application/json", jsonResponse);
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing minutes\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleStatus() {
  StaticJsonDocument<256> doc;
  doc["track"] = currentTrack;
  doc["volume"] = volumeLevel;
  doc["playing"] = isPlaying;
  doc["ip"] = WiFi.localIP().toString();
  doc["sleepTimer"] = sleepTimerMinutes;
  doc["sleepTimerActive"] = sleepTimerActive;
  
  if (sleepTimerActive && sleepTimerMinutes > 0) {
    unsigned long elapsed = (millis() - sleepTimerStartTime) / 1000;
    unsigned long remaining = (sleepTimerMinutes * 60) - elapsed;
    if (remaining > 0) {
      doc["sleepTimerRemaining"] = remaining;
    } else {
      doc["sleepTimerRemaining"] = 0;
    }
  } else {
    doc["sleepTimerRemaining"] = 0;
  }
  
  String response;
  response.reserve(200);
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWiFiConfig() {
  String html = F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"><title>WiFi Config</title><style>");
  html += F("*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:20px;min-height:100vh}");
  html += F(".c{max-width:500px;margin:0 auto;background:#fff;border-radius:20px;padding:30px;box-shadow:0 10px 40px rgba(0,0,0,.2)}h1{color:#333;text-align:center;margin-bottom:30px}");
  html += F(".f{margin-bottom:20px}label{display:block;margin-bottom:8px;color:#555;font-weight:bold}input[type=text],input[type=password]{width:100%;padding:12px;border:2px solid #ddd;border-radius:8px;font-size:16px}");
  html += F("button{width:100%;padding:12px;background:#667eea;color:#fff;border:0;border-radius:8px;font-size:16px;font-weight:bold;cursor:pointer;margin:5px 0}button:hover{background:#5568d3}.bs{background:#27ae60}.bs:hover{background:#229954}.br{background:#e74c3c}.br:hover{background:#c0392b}");
  html += F(".st{padding:10px;margin:10px 0;border-radius:8px;text-align:center;font-weight:bold}.st.s{background:#d4edda;color:#155724}.st.e{background:#f8d7da;color:#721c24}.st.i{background:#d1ecf1;color:#0c5460}");
  html += F("#nl{max-height:200px;overflow-y:auto;border:1px solid #ddd;border-radius:8px;padding:10px;margin-top:10px}.ni{padding:8px;cursor:pointer;border-radius:5px;margin:5px 0}.ni:hover{background:#f0f0f0}");
  html += F("</style></head><body><div class=\"c\"><h1>WiFi Config</h1>");
  html += F("<div id=\"st\" class=\"st\"></div>");
  html += F("<div class=\"f\"><label>Scan:</label><button class=\"bs\" onclick=\"scanW()\">Scan</button><div id=\"nl\"></div></div>");
  html += F("<form onsubmit=\"connW(event)\"><div class=\"f\"><label>SSID:</label><input type=\"text\" id=\"sid\" required></div>");
  html += F("<div class=\"f\"><label>PWD:</label><input type=\"password\" id=\"pwd\"></div><button type=\"submit\">Connect</button></form>");
  html += F("<button class=\"br\" onclick=\"resetW()\">Reset</button>");
  html += F("<div style=\"text-align:center;margin-top:20px\"><a href=\"/\" style=\"color:#667eea;text-decoration:none;font-weight:bold\">Back</a></div></div>");
  html += F("<script>");
  html += F("function showS(m,t){var s=document.getElementById('st');s.textContent=m;s.className='st '+t;setTimeout(function(){s.textContent=''},5000)}");
  html += F("function scanW(){showS('Scan...','i');fetch('/api/wifi/scan').then(r=>r.json()).then(d=>{var l=document.getElementById('nl');if(d.networks&&d.networks.length>0){l.innerHTML=d.networks.map(n=>'<div class=\"ni\" onclick=\"document.getElementById(\\'sid\\').value=\\'\"+n.ssid+\"\\'\"><strong>'+n.ssid+'</strong> ('+n.rssi+')</div>').join('');showS('Found '+d.networks.length,'s')}else{l.innerHTML='<p>None</p>';showS('None','e')}}).catch(e=>showS('Err','e'))}");
  html += F("function connW(e){e.preventDefault();var s=document.getElementById('sid').value;var p=document.getElementById('pwd').value;showS('Conn...','i');fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,password:p})}).then(r=>r.json()).then(d=>{if(d.success){showS('OK:'+d.ip,'s');setTimeout(function(){window.location.href='/'},3000)}else{showS('Err','e')}}).catch(e=>showS('Err','e'))}");
  html += F("function resetW(){if(confirm('Reset?')){fetch('/api/wifi/reset',{method:'POST'}).then(r=>r.json()).then(d=>{showS('Reset...','i');setTimeout(function(){location.reload()},3000)})}}");
  html += F("fetch('/api/wifi/status').then(r=>r.json()).then(d=>{if(d.ssid){showS('OK:'+d.ssid,'s')}else{showS('None','i')}});");
  html += F("</script></body></html>");
  
  server.send(200, "text/html", html);
}

void handleWiFiConnect() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    if (doc.containsKey("ssid")) {
      String ssid = doc["ssid"].as<String>();
      String password = doc.containsKey("password") ? doc["password"].as<String>() : "";
      
      Serial.print("Web: Kết nối WiFi - SSID: ");
      Serial.println(ssid);
      
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid.c_str(), password.c_str());
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        wifiManager.setConfigPortalTimeout(180);
        wifiManager.autoConnect(ssid.c_str(), password.c_str());
        
        StaticJsonDocument<200> response;
        response["success"] = true;
        response["ip"] = WiFi.localIP().toString();
        response["ssid"] = ssid;
        
        String jsonResponse;
        serializeJson(response, jsonResponse);
        server.send(200, "application/json", jsonResponse);
        
        Serial.print("Kết nối thành công! IP: ");
        Serial.println(WiFi.localIP());
      } else {
        StaticJsonDocument<200> response;
        response["success"] = false;
        response["message"] = "Không thể kết nối. Kiểm tra SSID và mật khẩu.";
        
        String jsonResponse;
        serializeJson(response, jsonResponse);
        server.send(200, "application/json", jsonResponse);
        
        Serial.println("Kết nối WiFi thất bại");
      }
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing SSID\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleWiFiScan() {
  Serial.println("Web: Quét mạng WiFi...");
  
  int n = WiFi.scanNetworks();
  if (n > 15) n = 15;
  
  StaticJsonDocument<512> doc;
  JsonArray networks = doc.createNestedArray("networks");
  
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "O" : "E";
  }
  
  String response;
  response.reserve(400);
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWiFiStatus() {
  StaticJsonDocument<200> doc;
  
  if (WiFi.status() == WL_CONNECTED) {
    doc["connected"] = true;
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
  } else {
    doc["connected"] = false;
    doc["ssid"] = "";
    doc["ip"] = "";
  }
  
  String response;
  response.reserve(150);
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWiFiReset() {
  wifiManager.resetSettings();
  
  StaticJsonDocument<100> doc;
  doc["success"] = true;
  doc["message"] = "Đã xóa cấu hình WiFi";
  
  String response;
  response.reserve(80);
  
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
  
  delay(1000);
  ESP.restart();
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}
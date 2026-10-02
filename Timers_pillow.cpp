/**
 * @file Timers.cpp
 * @brief Triển khai module quản lý bộ định thời
 */

#include "Config.h"
#include "Globals.h"
#include "Timers_pillow.h"
#include "Sensors.h"
#include "Audio.h"
#include "SleepTracker.h"

// ==================== SLEEP TIMER ====================

void SleepTimer_Update() {
  if (sleepTimerActive && sleepTimerMinutes > 0) {
    unsigned long elapsed = (millis() - sleepTimerStartTime) / 1000;
    unsigned long targetSeconds = sleepTimerMinutes * 60;
    
    if (elapsed >= targetSeconds) {
      Serial.println(F("Sleep timer: Stop"));
      MP3_Stop(false);
      isPlaying = false;
      sleepTimerStoppedMusic = true;
      
      sleepTimerActive = false;
      sleepTimerStartTime = 0;
      sleepTimerMinutes = 0;
    }
  }
  
  if (sleepTimerStoppedMusic && isPlaying) {
    Serial.println(F("Auto-play detected - Stop"));
    MP3_Stop(false);
    isPlaying = false;
  }
}

// ==================== AUTO COMFORT ====================

void AutoComfort_Update() {
  if (autoComfortBlocked) {
    return;
  }

  if (!isSleeping || GetSleepStage() != 1) {
    if (autoComfortActive) {
      Serial.println(F("Auto comfort: Tắt (không còn giai đoạn 1)"));
      MP3_Stop(false);
      isPlaying = false;
      autoComfortActive = false;
      autoComfortStartTime = 0;
    }
    return;
  }
  
  if (autoComfortActive) {
    unsigned long elapsed = millis() - autoComfortStartTime;
    if (elapsed >= AUTO_COMFORT_DURATION) {
      Serial.println(F("Auto comfort: Tắt sau 30 phút"));
      MP3_Stop(false);
      isPlaying = false;
      autoComfortActive = false;
      autoComfortStartTime = 0;
    }
    return;
  }
  
  if (positionChangeCount >= RESTLESS_THRESHOLD) {
    Serial.print(F("Auto comfort: Bật (đảo "));
    Serial.print(positionChangeCount);
    Serial.println(F(" lần)"));
    
    MP3_Play(1);
    currentTrack = 1;
    isPlaying = true;
    
    autoComfortActive = true;
    autoComfortStartTime = millis();
  }
}

void Button_StopAutoComfort() {
  static unsigned long lastButtonPress = 0;
  static bool lastButtonState = HIGH;
  const unsigned long debounceDelay = 200;
  
  bool currentButtonState = digitalRead(btn_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (millis() - lastButtonPress > debounceDelay) {
      if (autoComfortActive) {
        Serial.println(F("Nút nhấn: Dừng tự động phát nhạc"));
        MP3_Stop(false);
        isPlaying = false;
        autoComfortActive = false;
        autoComfortStartTime = 0;
        autoComfortBlocked = true;
      }
      lastButtonPress = millis();
    }
  }
  
  lastButtonState = currentButtonState;
}
/**
 * @file WebServer.h
 * @brief Module web server và API handlers
 */

#ifndef WEBMODULE_H
#define WEBMODULE_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

/**
 * @brief Thiết lập web server
 */
void SetupWebServer();

/**
 * @brief Cập nhật hiển thị OLED
 */
void Display_Update();

void handleRoot();
void handleWiFiConfig();
void handleMusicPlay();
void handleMusicStop();
void handleMusicPause();
void handleVolume();
void handleSleepTimer();
void handleStatus();
void handleWiFiConnect();
void handleWiFiScan();
void handleWiFiStatus();
void handleWiFiReset();
void handleNotFound();

#endif // WEBMODULE_H
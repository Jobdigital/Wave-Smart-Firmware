#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

WiFiManager wifiManager;

constexpr unsigned long BAUD_RATE = 115200;

constexpr uint8_t DHT_PIN         = 0x0F;
constexpr uint8_t LED_BUILTIN_PIN = 0x02;
constexpr uint8_t DHT_TYPE        = DHT11;
constexpr uint8_t HW103_PIN       = 0x24;

DHT           dht(DHT_PIN, DHT_TYPE);
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

#endif
#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <WiFiManager.h>

WiFiManager wm;

constexpr unsigned long BAUD_RATE = 115200;

constexpr uint8_t DHT_PIN         = GPIO_NUM_15;
constexpr uint8_t LED_BUILTIN_PIN = GPIO_NUM_2;
constexpr uint8_t DHT_TYPE        = DHT11;
constexpr uint8_t HW103_PIN       = GPIO_NUM_36;

DHT           dht(DHT_PIN, DHT_TYPE);
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

#endif
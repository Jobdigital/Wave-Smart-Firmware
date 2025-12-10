#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <DHT.h>

constexpr unsigned long BAUD_RATE = 115200;

constexpr uint8_t LED_BUILTIN_PIN = 0x02;

constexpr uint8_t DHT_PIN  = 0x0F;
constexpr uint8_t DHT_TYPE = DHT11;

DHT dht(DHT_PIN, DHT_TYPE);

constexpr uint8_t HW103_PIN = 0x24;
#endif
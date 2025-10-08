#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Update.h>
#include <BluetoothSerial.h>

constexpr unsigned long BAUD_RATE        = 115200;
constexpr const char*   DEVICE_NAME      = "Wave Smart";
constexpr const char*   DEVICE_PIN       = "0000";
constexpr uint16_t      FIRMWARE_VERSION = 0;
constexpr uint16_t      HARDWARE_VERSION = 1;

constexpr uint8_t LED_BLUETOOTH_PIN = 2;
constexpr uint8_t CKP1_PIN          = 32;
constexpr uint8_t CMP1_PIN          = 33;
constexpr uint8_t CMP2_PIN          = 25;
constexpr uint8_t CMP3_PIN          = 27;
constexpr uint8_t CMP4_PIN          = 26;
constexpr uint8_t BUZZER_PIN        = 14;

enum Command : uint8_t {
  VERSION_FIRMWARE = 1,
  VERSION_HARDWARE = 2,
  UPDATE_FIRMWARE  = 3,
};

BluetoothSerial BT;

#endif
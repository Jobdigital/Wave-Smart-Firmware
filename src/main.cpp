#include <main.h>

void taskLedBluetooth(void* parameter) {
  while (true) {
    digitalWrite(LED_BLUETOOTH_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED_BLUETOOTH_PIN, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void taskDeviceEnable(void* parameter) {
  while (true) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

xTaskHandle taskLedBluetoothHandle = NULL;
xTaskHandle taskDeviceEnableHandle = NULL;

void setup() {
  // Inicializa la comunicación serial para depuración
  while (!Serial) {
    Serial.begin(BAUD_RATE);
  }
  Serial.printf("%s Firmware v%d starting...\n", DEVICE_NAME, FIRMWARE_VERSION);
  Serial.printf("%s Hardware v%d starting...\n", DEVICE_NAME, HARDWARE_VERSION);

  pinMode(LED_BLUETOOTH_PIN, OUTPUT);
  pinMode(CKP1_PIN, OUTPUT);
  pinMode(CMP1_PIN, OUTPUT);
  pinMode(CMP2_PIN, OUTPUT);
  pinMode(CMP3_PIN, OUTPUT);
  pinMode(CMP4_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  tone(BUZZER_PIN, 4000, 500);
  delay(500);

  digitalWrite(LED_BLUETOOTH_PIN, HIGH);
  delay(200);
  digitalWrite(LED_BLUETOOTH_PIN, LOW);

  tone(BUZZER_PIN, 3000, 100);
  digitalWrite(CKP1_PIN, HIGH);
  delay(100);
  digitalWrite(CKP1_PIN, LOW);

  digitalWrite(CMP1_PIN, HIGH);
  delay(100);
  digitalWrite(CMP1_PIN, LOW);

  digitalWrite(CMP2_PIN, HIGH);
  delay(100);
  digitalWrite(CMP2_PIN, LOW);

  digitalWrite(CMP3_PIN, HIGH);
  delay(100);
  digitalWrite(CMP3_PIN, LOW);

  digitalWrite(CMP4_PIN, HIGH);
  delay(100);
  digitalWrite(CMP4_PIN, LOW);

  digitalWrite(CMP3_PIN, HIGH);
  delay(100);
  digitalWrite(CMP3_PIN, LOW);

  digitalWrite(CMP2_PIN, HIGH);
  delay(100);
  digitalWrite(CMP2_PIN, LOW);

  digitalWrite(CMP1_PIN, HIGH);
  delay(100);
  digitalWrite(CMP1_PIN, LOW);

  digitalWrite(CKP1_PIN, HIGH);
  delay(100);
  digitalWrite(CKP1_PIN, LOW);

  digitalWrite(LED_BLUETOOTH_PIN, HIGH);
  delay(100);
  digitalWrite(LED_BLUETOOTH_PIN, LOW);

  // Inicializa Bluetooth Serial
  taskLedBluetoothHandle = NULL;
  try {
    BT.begin(DEVICE_NAME);  // Nombre del dispositivo Bluetooth
    BT.setPin(DEVICE_PIN);  // PIN para emparejamiento
    Serial.printf("Bluetooth Serial started, MAC: %s.\n", BT.getBtAddressString().c_str());
    xTaskCreatePinnedToCore(taskLedBluetooth, "LED_Bluetooth", 1024, NULL, 1, &taskLedBluetoothHandle, 1);
  } catch (const std::exception& e) {
    Serial.printf("Error al iniciar Bluetooth Serial: %s.\n", e.what());
    while (true);  // Detener la ejecución si hay un error
  }

  BT.onAuthComplete([](bool success) {
    if (success) {
      Serial.println("Dispositivo Bluetooth autenticado.");
    } else {
      Serial.println("Fallo en la autenticación del dispositivo Bluetooth.");
    }
  });

  BT.register_callback([](esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
    if (event == ESP_SPP_CLOSE_EVT) {
      Serial.printf("Dispositivo Bluetooth desconectado, handle: %d. Reiniciando...\n", param->close.handle);
      esp_restart();
    } else if (event == ESP_SPP_SRV_OPEN_EVT) {
      Serial.printf("Dispositivo Bluetooth conectado, handle: %d.\n", param->srv_open.handle);
      tone(BUZZER_PIN, 5000, 200);
      if (taskLedBluetoothHandle != NULL) {
        vTaskDelete(taskLedBluetoothHandle);  // Detener la tarea de parpadeo del LED
        taskLedBluetoothHandle = NULL;
      }
      digitalWrite(LED_BLUETOOTH_PIN, HIGH);  // Encender el LED de Bluetooth
    }
  });
}

void updateFirmware() {
  BT.println();
  if (Update.isRunning()) {
    Update.abort();
  }

  long firmwareLength = BT.parseInt();
  Serial.printf("Tamaño del firmware a actualizar: %d bytes.\n", firmwareLength);

  if (Update.begin(firmwareLength) == false) {
    Serial.println("No hay espacio suficiente para la actualización de firmware.");
    BT.println("No hay espacio suficiente");
    return;
  }
  BT.println();

  uint8_t* firmwareData = new uint8_t[1024];
  size_t   write        = 0;
  while (Update.progress() < firmwareLength) {
    size_t toRead    = std::min(static_cast<size_t>(firmwareLength - Update.progress()), static_cast<size_t>(1024));
    size_t bytesRead = BT.readBytes(firmwareData, toRead);
    if (bytesRead > 0) {
      write = 0;
      do {
        write += Update.write(firmwareData + write, bytesRead - write);
      } while (write < bytesRead);
      BT.println();

      Serial.printf("Progreso de actualización: %d/%d bytes (%.2f%%)\n", Update.progress(), firmwareLength, (Update.progress() * 100.0) / firmwareLength);
    }
  }

  delete[] firmwareData;
  if (Update.end(true)) {
    Serial.println("Actualización de firmware completada. Reiniciando...");
    BT.println();
    ESP.restart();
  } else {
    Serial.printf("Error en la actualización de firmware: %s\n", Update.errorString());
    BT.printf("Update Failed: %s\n", Update.errorString());
  }
}

void loop() {
  if (!BT.available()) return;
  switch (static_cast<Command>(BT.parseInt())) {
    case VERSION_FIRMWARE:
      Serial.println("Comando recibido: Solicitar versión de firmware.");
      BT.printf("%d\n", FIRMWARE_VERSION);
      break;

    case VERSION_HARDWARE:
      Serial.println("Comando recibido: Solicitar versión de hardware.");
      BT.printf("%d\n", HARDWARE_VERSION);
      break;

    case UPDATE_FIRMWARE:
      Serial.println("Comando recibido: Iniciar actualización de firmware.");
      updateFirmware();
      break;

    case UNIQUE_IDENTIFIER:
      Serial.println("Comando recibido: Solicitar identificador único.");
      BT.println(ESP.getEfuseMac());
      break;

    case ENABLE_DEVICE:
      Serial.println("Comando recibido: Habilitar dispositivo.");
      digitalWrite(ENABLE_PIN, HIGH);
      BT.println("");
      break;

    case DISABLE_DEVICE:
      Serial.println("Comando recibido: Deshabilitar dispositivo.");
      digitalWrite(ENABLE_PIN, LOW);
      BT.println("");
      break;
  }
}
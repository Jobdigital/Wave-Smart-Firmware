#include <main.h>

void taskLedBluetooth(void* parameter) {
  while (true) {
    digitalWrite(LED_BLUETOOTH_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED_BLUETOOTH_PIN, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
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

volatile uint16_t DELAY_MS = 0;
String            CKP1     = "";
String            CMP1     = "";
String            CMP2     = "";
String            CMP3     = "";
String            CMP4     = "";

xTaskHandle taskLedBluetoothHandle              = NULL;
xTaskHandle taskLoopBluetoothComunicationHandle = NULL;

void taskLoopBluetoothComunication(void* parameter) {
  while (true) {
    if (!BT.available()) continue;
    switch (static_cast<Command>(BT.readStringUntil('\n').toInt())) {
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
        if (!digitalRead(ENABLE_PIN)) {
          digitalWrite(ENABLE_PIN, HIGH);
          tone(BUZZER_PIN, 1000, 200);
          BT.println("");
        } else {
          BT.println("Ya habilitado");
        }
        break;

      case DISABLE_DEVICE:
        Serial.println("Comando recibido: Deshabilitar dispositivo.");
        if (digitalRead(ENABLE_PIN)) {
          digitalWrite(ENABLE_PIN, LOW);
          tone(BUZZER_PIN, 500, 200);
          BT.println("");
        } else {
          BT.println("Ya deshabilitado");
        }
        break;

      case SET_CKP1:
        BT.println();
        CKP1 = BT.readStringUntil('\n');
        Serial.printf("Comando recibido: Establecer CKP1 a %s.\n", CKP1.c_str());
        BT.println();
        break;

      case SET_CMP1:
        BT.println();
        CMP1 = BT.readStringUntil('\n');
        Serial.printf("Comando recibido: Establecer CMP1 a %s.\n", CMP1.c_str());
        BT.println();
        break;

      case SET_CMP2:
        BT.println();
        CMP2 = BT.readStringUntil('\n');
        Serial.printf("Comando recibido: Establecer CMP2 a %s.\n", CMP2.c_str());
        BT.println();
        break;

      case SET_CMP3:
        BT.println();
        CMP3 = BT.readStringUntil('\n');
        Serial.printf("Comando recibido: Establecer CMP3 a %s.\n", CMP3.c_str());
        BT.println();
        break;

      case SET_CMP4:
        BT.println();
        CMP4 = BT.readStringUntil('\n');
        Serial.printf("Comando recibido: Establecer CMP4 a %s.\n", CMP4.c_str());
        BT.println();
        break;

      case SET_DELAY:
        BT.println();
        try {
          DELAY_MS = BT.readStringUntil('\n').toInt();
          Serial.printf("Comando recibido: Establecer retardo a %d ms.\n", DELAY_MS);
          BT.println();
        } catch (const std::exception& e) {
          Serial.printf("Error al establecer el retardo: %s.\n", e.what());
          BT.printf("Error: %s\n", e.what());
        }
        break;

      case CLEAR_SIGNALS:
        Serial.println("Comando recibido: Limpiar señales.");
        CKP1     = "";
        CMP1     = "";
        CMP2     = "";
        CMP3     = "";
        CMP4     = "";
        DELAY_MS = 0;
        BT.println();
        break;

      default:
        Serial.println("Comando recibido: Comando desconocido.");
        BT.println("Comando desconocido");
        break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

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

  digitalWrite(ENABLE_PIN, LOW);  // Asegura que el dispositivo esté deshabilitado al inicio

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
      tone(BUZZER_PIN, 5000, 200);
      Serial.println("Dispositivo Bluetooth autenticado.");
    } else {
      tone(BUZZER_PIN, 4000, 200);
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

  xTaskCreatePinnedToCore(taskLoopBluetoothComunication, "Loop_Bluetooth", 4096, NULL, 1, &taskLoopBluetoothComunicationHandle, 1);
}

uint16_t signalIndex = 0;
uint64_t lastMicros  = micros();

void loop() {
  if (lastMicros + (DELAY_MS / (CKP1.length() > 0 ? CKP1.length() : 1)) < micros()) {
    digitalWrite(CKP1_PIN, CKP1.length() > 0 ? CKP1.charAt(signalIndex % CKP1.length()) == '1' ? HIGH : LOW : LOW);
    digitalWrite(CMP1_PIN, CMP1.length() > 0 ? CMP1.charAt(signalIndex % CMP1.length()) == '1' ? HIGH : LOW : LOW);
    digitalWrite(CMP2_PIN, CMP2.length() > 0 ? CMP2.charAt(signalIndex % CMP2.length()) == '1' ? HIGH : LOW : LOW);
    digitalWrite(CMP3_PIN, CMP3.length() > 0 ? CMP3.charAt(signalIndex % CMP3.length()) == '1' ? HIGH : LOW : LOW);
    digitalWrite(CMP4_PIN, CMP4.length() > 0 ? CMP4.charAt(signalIndex % CMP4.length()) == '1' ? HIGH : LOW : LOW);
    lastMicros = micros();
    signalIndex++;
  }
}

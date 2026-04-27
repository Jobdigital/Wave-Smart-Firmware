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

// Buffers binarios para señales PWM
#define MAX_SIGNAL_LENGTH 512
uint8_t  CKP1_buf[MAX_SIGNAL_LENGTH] = {0};
uint8_t  CMP1_buf[MAX_SIGNAL_LENGTH] = {0};
uint8_t  CMP2_buf[MAX_SIGNAL_LENGTH] = {0};
uint8_t  CMP3_buf[MAX_SIGNAL_LENGTH] = {0};
uint8_t  CMP4_buf[MAX_SIGNAL_LENGTH] = {0};
uint16_t CKP1_len = 0, CMP1_len = 0, CMP2_len = 0, CMP3_len = 0, CMP4_len = 0;

// Función para convertir String a buffer binario
void parseSignal(const String& str, uint8_t* buf, uint16_t* len) {
  *len = 0;
  for (uint16_t i = 0; i < str.length() && i < MAX_SIGNAL_LENGTH; ++i) {
    buf[i] = (str.charAt(i) == '1') ? 1 : 0;
    (*len)++;
  }
}

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
          // digitalWrite(ENABLE_PIN, HIGH);
          tone(BUZZER_PIN, 1000, 200);
          BT.println("");
        } else {
          BT.println("Ya habilitado");
        }
        break;

      case DISABLE_DEVICE:
        Serial.println("Comando recibido: Deshabilitar dispositivo.");
        if (digitalRead(ENABLE_PIN)) {
          // digitalWrite(ENABLE_PIN, LOW);
          tone(BUZZER_PIN, 500, 200);
          BT.println("");
        } else {
          BT.println("Ya deshabilitado");
        }
        break;

      case SET_CKP1: {
        BT.println();
        String temp = BT.readStringUntil('\n');
        parseSignal(temp, CKP1_buf, &CKP1_len);
        Serial.printf("Comando recibido: Establecer CKP1 a %s.\n", temp.c_str());
        BT.println();
        break;
      }
      case SET_CMP1: {
        BT.println();
        String temp = BT.readStringUntil('\n');
        parseSignal(temp, CMP1_buf, &CMP1_len);
        Serial.printf("Comando recibido: Establecer CMP1 a %s.\n", temp.c_str());
        BT.println();
        break;
      }
      case SET_CMP2: {
        BT.println();
        String temp = BT.readStringUntil('\n');
        parseSignal(temp, CMP2_buf, &CMP2_len);
        Serial.printf("Comando recibido: Establecer CMP2 a %s.\n", temp.c_str());
        BT.println();
        break;
      }
      case SET_CMP3: {
        BT.println();
        String temp = BT.readStringUntil('\n');
        parseSignal(temp, CMP3_buf, &CMP3_len);
        Serial.printf("Comando recibido: Establecer CMP3 a %s.\n", temp.c_str());
        BT.println();
        break;
      }
      case SET_CMP4: {
        BT.println();
        String temp = BT.readStringUntil('\n');
        parseSignal(temp, CMP4_buf, &CMP4_len);
        Serial.printf("Comando recibido: Establecer CMP4 a %s.\n", temp.c_str());
        BT.println();
        break;
      }

      case SET_DELAY:
        BT.println();
        try {
          DELAY_MS = BT.readStringUntil('\n').toInt();
          Serial.printf("Comando recibido: Establecer retardo a %d microsegundos.\n", DELAY_MS);
          BT.println();
        } catch (const std::exception& e) {
          Serial.printf("Error al establecer el retardo: %s.\n", e.what());
          BT.printf("Error: %s\n", e.what());
        }
        break;

      case CLEAR_SIGNALS:
        Serial.println("Comando recibido: Limpiar señales.");
        memset(CKP1_buf, 0, sizeof(CKP1_buf));
        CKP1_len = 0;
        memset(CMP1_buf, 0, sizeof(CMP1_buf));
        CMP1_len = 0;
        memset(CMP2_buf, 0, sizeof(CMP2_buf));
        CMP2_len = 0;
        memset(CMP3_buf, 0, sizeof(CMP3_buf));
        CMP3_len = 0;
        memset(CMP4_buf, 0, sizeof(CMP4_buf));
        CMP4_len = 0;
        DELAY_MS = 0;
        BT.println();
        break;

      default:
        Serial.println("Comando recibido: Comando desconocido.");
        BT.println("Comando desconocido");
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
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

  digitalWrite(ENABLE_PIN, HIGH);  // Asegura que el dispositivo esté habilitado al inicio

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
  if (lastMicros + DELAY_MS < micros()) {
    digitalWrite(CKP1_PIN, CKP1_len > 0 ? CKP1_buf[signalIndex % CKP1_len] ? HIGH : LOW : LOW);
    digitalWrite(CMP1_PIN, CMP1_len > 0 ? CMP1_buf[signalIndex % CMP1_len] ? HIGH : LOW : LOW);
    digitalWrite(CMP2_PIN, CMP2_len > 0 ? CMP2_buf[signalIndex % CMP2_len] ? HIGH : LOW : LOW);
    digitalWrite(CMP3_PIN, CMP3_len > 0 ? CMP3_buf[signalIndex % CMP3_len] ? HIGH : LOW : LOW);
    digitalWrite(CMP4_PIN, CMP4_len > 0 ? CMP4_buf[signalIndex % CMP4_len] ? HIGH : LOW : LOW);
    lastMicros = micros();
    signalIndex++;
  }
}

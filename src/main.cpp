#include <main.h>

void setup() {
  while (!Serial) {
    Serial.begin(BAUD_RATE);
  }

  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP

  dht.begin();
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  pinMode(HW103_PIN, INPUT_PULLDOWN);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Bienvenido");
  lcd.setCursor(0, 1);
  lcd.print("a Ecovermic!");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectate a Ecovermic");
  lcd.setCursor(0, 1);
  lcd.print("para configurar WiFi");

  // wm.setConfigPortalBlocking(false);
  wm.autoConnect("ECOVERMIC");
}

void loop() {
  float    humidity       = dht.readHumidity();
  float    temperature    = dht.readTemperature();
  uint16_t groundHumidity = map(analogRead(HW103_PIN), 4095, 0, 0, 100);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" ºC");
  Serial.print("Ground Humidity: ");
  Serial.print(groundHumidity);
  Serial.println(" %");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("T: %.1fC H: %.1f%%", temperature, humidity);
  lcd.setCursor(0, 1);
  lcd.printf("GH: %d%%", groundHumidity);

  delay(1000 * 60 * 60);  // 1 hour
}

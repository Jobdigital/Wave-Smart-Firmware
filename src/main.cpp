#include <main.h>

void setup() {
  while (!Serial) {
    Serial.begin(BAUD_RATE);
  }

  dht.begin();
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  pinMode(HW103_PIN, INPUT);
}

void loop() {
  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  uint16_t groundHumidity = analogRead(HW103_PIN);
  Serial.print("Ground Humidity: ");
  Serial.println(groundHumidity);

  delay(2000);
}

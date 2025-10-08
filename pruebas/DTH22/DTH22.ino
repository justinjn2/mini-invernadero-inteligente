#include "DHT.h"

DHT dht(27, DHT22);

void setup() {
  Serial.begin(115200);
  delay(2000);            // 2 s para que el monitor sincronice
  Serial.println("Prueba DHT22 en GPIO27...");
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ Error leyendo el sensor DHT22");
  } else {
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print("%  |  Temperatura: ");
    Serial.print(t);
    Serial.println(" °C");
  }

  delay(2000); // 2 segundos entre lecturas
}

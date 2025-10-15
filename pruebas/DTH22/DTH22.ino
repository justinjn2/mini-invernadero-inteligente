/*
  Prueba del sensor DHT22 (temperatura y humedad del aire)
  ---------------------------------------------------------
  Este código realiza la lectura de temperatura y humedad 
  del aire usando el sensor DHT22 conectado al pin GPIO27.
*/

// Inclusión de la librería del sensor DHT
#include "DHT.h"
// Definición de pines y tipo de sensor
#define DHTPIN 27       // Pin de datos del DHT22
#define DHTTYPE DHT22   // Tipo de sensor usado

// Creación del objeto del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);    // Inicializa la comunicación serie
  delay(2000);             // Pausa para sincronizar el monitor
  Serial.println("Prueba DHT22 en GPIO27...");
  dht.begin();             // Inicializa el sensor DHT22
}

void loop() {
  // Lectura de humedad y temperatura desde el sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Verifica si las lecturas son válidas
  if (isnan(h) || isnan(t)) {
    Serial.println("❌ Error leyendo el sensor DHT22");
  } else {
    // Muestra las lecturas en el monitor serie
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print("%  |  Temperatura: ");
    Serial.print(t);
    Serial.println(" °C");
  }

  delay(2000);  // Intervalo de lectura de 2 segundos
}

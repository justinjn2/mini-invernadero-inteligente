/*
  Prueba del sensor DHT22 (temperatura y humedad del aire)
  ---------------------------------------------------------
  Código que lee temperatura y humedad usando el senor DHT22 
  en GPIO27 y muestra los valores en la pantalla LCD 16x2 por 
  I2C (dirección 0x27).
*/

// Librerías
#include <Wire.h>                 // Bus I2C
#include <LiquidCrystal_I2C.h>    // Pantalla LCD por I2C
#include "DHT.h"                  // Sensor DHT

// Definición de pines
#define DHTPIN 27       // Pin de datos del DHT22

// Instancias de las clases
DHT dht(DHTPIN, DHT22);             // Objeto 'dht' para leer T/H aire
LiquidCrystal_I2C lcd(0x27, 16, 2); // Objeto 'lcd' para usar LCD 16x2

// Función que muestra H y T en el LCD. A modo de prueba se muestra en
// la primera columna los mismos datos que en la segunda columna, con 
// nombres diferentes para hacer referencia a los datos de referencia.
void printer(float h, float t) {
  int h_int = (int)h;
  int t_int = (int)t;
  lcd.setCursor(0, 0);  // Columna 0, fila 0
  lcd.print("HR:"); lcd.print(h_int); lcd.print(" H:"); lcd.print(h_int);
  lcd.setCursor(0, 1);  // Segunda línea
  lcd.print("TR:"); lcd.print(t_int); lcd.print(" T:"); lcd.print(t_int);
}

void setup() {
  Serial.begin(115200);   // Inicializa la comunicación serie
  delay(2000);            // Pausa para sincronizar el monitor
  lcd.init();             // Inicializa el LCD
  lcd.backlight();        // Enciende la retroiluminación
  Serial.println("Prueba DHT22 en GPIO27...");
  dht.begin();            // Inicializa el sensor DHT22
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
  // Mostrar en LCD
  printer(h, t);

  delay(2000);  // Intervalo de lectura de 2 segundos
}

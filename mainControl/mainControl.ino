/*
  Código principal del mini-invernadero inteligente
  ------------------------------------------------------
  Este programa controla la humedad del suelo y la temperatura 
  del invernadero usando el sensor DHT22, dos sensores de humedad 
  del suelo AZDelivery V1.2 y potenciómetros como referencias 
  de ajuste.
*/

#include "DHT.h"

// Definición de tipo de sensor DHT
#define DHTTYPE DHT22

// Definición de pines del sistema
#define DHTPIN      27   // Pin de datos del sensor DHT22
#define SENSOR_PIN1 32   // Sensor de humedad del suelo 1
#define SENSOR_PIN2 33   // Sensor de humedad del suelo 2
#define POTE_TEMP   34   // Potenciómetro de referencia de temperatura
#define POTE_HUM    35   // Potenciómetro de referencia de humedad
#define BOMBA       25   // Salida digital que controla la bomba de riego
#define SERVO       26   // Salida para servo o ventilador (control de temperatura futuro)

// Creación del objeto del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// -----------------------------------------------------------------------------
// Funciones de lectura de sensores y potenciómetros
// -----------------------------------------------------------------------------

// Lee temperatura y humedad del aire
void leerAire_DHT(float &temperatura, float &humedad) {
  temperatura = dht.readTemperature();
  humedad     = dht.readHumidity();
}

// Lee la humedad del suelo y la convierte a porcentaje (0–100 %)
float leerHumedadSuelo(int pin, int seco, int humedo) {
  int   valor   = analogRead(pin);
  float humedad = map(valor, seco, humedo, 0, 100);
  return constrain(humedad, 0, 100);
}

// Lee el potenciómetro que ajusta la humedad de referencia (0–100 %)
float leerPoteHumedad(int pin) {
  int valor = analogRead(pin);
  float humedadRef = map(valor, 0, 3000, 0, 100);
  return constrain(humedadRef, 0, 100);
}

// Lee el potenciómetro que ajusta la temperatura de referencia (17–35 °C)
float leerPoteTemp(int pin) {
  int valor = analogRead(pin);
  float tempRef = map(valor, 0, 3000, 17, 35);
  return constrain(tempRef, 0, 100);
}

// -----------------------------------------------------------------------------
// Configuración inicial
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);    // Inicializa la comunicación serie
  dht.begin();             // Inicializa el sensor DHT22

  pinMode(BOMBA, OUTPUT);  // Configura el pin de la bomba como salida      
  digitalWrite(BOMBA, HIGH); // Estado inicial: bomba desactivada
}

// -----------------------------------------------------------------------------
// Bucle principal del programa
// -----------------------------------------------------------------------------
void loop() {
  float temp, humAire;
  leerAire_DHT(temp, humAire);  // Lectura del DHT22

  // Lectura de los dos sensores de humedad del suelo
  float humSuelo1 = leerHumedadSuelo(SENSOR_PIN1, 2750, 1340);
  float humSuelo2 = leerHumedadSuelo(SENSOR_PIN2, 2750, 1270);

  // Promedio de humedad del suelo
  float humSueloProm = (humSuelo1 + humSuelo2) / 2.0;

  // Lecturas de referencia desde los potenciómetros
  float humRef = leerPoteHumedad(POTE_HUM);
  float tempRef = leerPoteTemp(POTE_TEMP);

  // ---------------------------------------------------------------------------
  // Lógica del control de humedad
  // ---------------------------------------------------------------------------
  /*  
    Compara Hum de referencia con Hum 
    real y estado de la bomba ON/OFF 
    para activar o desactivar la bomba  
  */
  if (humSueloProm < (humRef - 10) && digitalRead(BOMBA)) {
    digitalWrite(BOMBA, LOW);
    Serial.print(" BOMBA ACTIVADA ");
  } 
  else if (humSueloProm >= humRef && !digitalRead(BOMBA)) {
    digitalWrite(BOMBA, HIGH);
    Serial.print(" BOMBA DESACTIVADA ");
  }

  // ---------------------------------------------------------------------------
  // Lógica del control de temperatura (en desarrollo)
  // ---------------------------------------------------------------------------
  // if (temp < (tempRef - 1.5) && digitalRead(SERVO)) {
  //   Serial.print(" SERVO ACTIVADO ");
  // }
  // else if (temp >= tempRef && !digitalRead(SERVO)) {
  //   Serial.print(" SERVO DESACTIVADO ");
  // }

  // ---------------------------------------------------------------------------
  // Monitoreo de datos
  // ---------------------------------------------------------------------------
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" °C | Hum Aire: "); Serial.print(humAire);
  Serial.print(" % | Promedio Suelo: "); Serial.print(humSueloProm);
  Serial.print(" % | Referencia Hum: "); Serial.print(humRef);
  Serial.print(" % | Bomba: "); Serial.print(digitalRead(BOMBA));
  Serial.println(" ");

  delay(3000);  // Intervalo de actualización de 3 segundos
}

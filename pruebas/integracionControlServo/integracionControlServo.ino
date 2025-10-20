/*
  Código principal del mini-invernadero inteligente
  ------------------------------------------------------
  Control de humedad del suelo y temperatura con DHT22,
  2 sensores de suelo AZDelivery V1.2 y servo para ventana.
*/

#include "DHT.h"
#include <ESP32Servo.h>

#define DHTTYPE   DHT22

// Pines
#define DHTPIN      27
#define SENSOR_PIN1 32
#define SENSOR_PIN2 33
#define POTE_TEMP   34
#define POTE_HUM    35
#define BOMBA       25
#define SERVO_PIN   26

DHT dht(DHTPIN, DHTTYPE);
Servo servoVentana;

// ---------- Funciones de lectura ----------
void leerAire_DHT(float &temperatura, float &humedad) {
  temperatura = dht.readTemperature();
  humedad     = dht.readHumidity();
}

float leerHumedadSuelo(int pin, int seco, int humedo) {
  int   valor   = analogRead(pin);
  float humedad = map(valor, seco, humedo, 0, 100);
  return constrain(humedad, 0, 100);
}

float leerPoteHumedad(int pin) {
  int valor = analogRead(pin);
  float humedadRef = map(valor, 0, 3000, 0, 100);
  return constrain(humedadRef, 0, 100);
}

float leerPoteTemp(int pin) {
  int valor = analogRead(pin);
  float tempRef = map(valor, 0, 3000, 17, 35);
  return constrain(tempRef, 0, 100);
}

// ---------- Servo (límites y movimiento) ----------
const int ANG_ABIERTO = 80;
const int ANG_CERRADO = 160;
bool ventanaAbierta = false;

void moverVentana(bool abrir) {
  servoVentana.attach(SERVO_PIN);
  servoVentana.write(abrir ? ANG_ABIERTO : ANG_CERRADO);
  delay(500);                  // breve para asegurar llegada
  servoVentana.detach();       // ahorro energía/ruido
  ventanaAbierta = abrir;
}

// ---------- Timers ----------
const unsigned long LECT_MS   = 3000;                 // lecturas cada 3 s
const unsigned long DECIDE_MS = 15UL * 1000UL;  // decisión cada 5 min => 5UL * 60UL * 1000UL
unsigned long tLect = 0, tDecide = 0;

// Lecturas actuales
float temp = NAN, humAire = NAN;
float humSuelo1 = NAN, humSuelo2 = NAN, humSueloProm = NAN;
float humRef = NAN, tempRef = NAN;

// Histeresis temperatura (evita abrir/cerrar seguido)
const float H_T = 2.0;   // abre si T >= tempRef; cierra si T <= tempRef - H_T

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(BOMBA, OUTPUT);
  digitalWrite(BOMBA, HIGH);      // bomba desactivada (lógica activa en LOW)

  // Posición inicial segura del servo
  servoVentana.attach(SERVO_PIN);
  servoVentana.write(ANG_CERRADO);
  delay(300);
  servoVentana.detach();
  ventanaAbierta = false;

  unsigned long now = millis();
  tLect   = now + LECT_MS;
  tDecide = now + DECIDE_MS;
}

void loop() {
  unsigned long now = millis();

  // ---- Lecturas periódicas (cada 3 s) ----
  if (now >= tLect) {
    leerAire_DHT(temp, humAire);
    humSuelo1 = leerHumedadSuelo(SENSOR_PIN1, 2750, 1340);
    humSuelo2 = leerHumedadSuelo(SENSOR_PIN2, 2750, 1270);
    humSueloProm = (humSuelo1 + humSuelo2) / 2.0;

    humRef  = leerPoteHumedad(POTE_HUM);
    tempRef = leerPoteTemp(POTE_TEMP);

    // Lógica de humedad (bomba)
    if (humSueloProm < (humRef - 10) && digitalRead(BOMBA)) {
      digitalWrite(BOMBA, LOW);   // activa bomba
      Serial.println("BOMBA ACTIVADA");
    } 
    else if (humSueloProm >= humRef && !digitalRead(BOMBA)) {
      digitalWrite(BOMBA, HIGH);  // desactiva bomba
      Serial.println("BOMBA DESACTIVADA");
    }

    // Monitoreo breve
    Serial.print("T: "); Serial.print(temp);
    Serial.print(" °C | HAire: "); Serial.print(humAire);
    Serial.print("% | HSuelo: "); Serial.print(humSueloProm);
    Serial.print("% | HRef: "); Serial.print(humRef);
    Serial.print("% | TRef: "); Serial.print(tempRef);
    Serial.print(" °C | Bomba: "); Serial.println(digitalRead(BOMBA));

    tLect += LECT_MS;
  }

  // ---- Decisión de ventana (cada 5 min) con histéresis ----
  if (now >= tDecide) {
    if (!isnan(temp) && !isnan(tempRef)) {
      if (!ventanaAbierta && temp >= tempRef) {
        moverVentana(true);    // abrir
        Serial.println("VENTANA ABIERTA");
      } else if (ventanaAbierta && temp <= (tempRef - H_T)) {
        moverVentana(false);   // cerrar
        Serial.println("VENTANA CERRADA");
      }
    }
    tDecide += DECIDE_MS;
  }

  // sin delay(); el micro queda libre
}

/*
  Código principal del mini-invernadero inteligente
  ------------------------------------------------------
  Control de humedad del suelo y temperatura con DHT22, 2
  sensores de suelo AZDelivery V1.2, servo para ventana y
  bomba para riego. Referencias definidas con potenciómetros.
*/

// Librerías
#include <Wire.h>               // Bus I2C
#include "DHT.h"                // Sensor DHT22 (temperatura y humedad del aire)
#include "esp_system.h"         // Funciones del sistema ESP32 (usada para reinicio)
#include <ESP32Servo.h>         // Control de servomotor
#include <LiquidCrystal_I2C.h>  // Pantalla LCD por I2C

// Pines
#define DHT_PIN     27  // Entrada digital del sensor DHT22
#define SENSOR_PIN1 32  // Entrada ADC de sensor1 de humedad de suelo
#define SENSOR_PIN2 33  // Entrada ADC de sensor2 de humedad de suelo
#define POTE_TEMP   34  // Entrada ADC de referencia de temperatura
#define POTE_HUM    35  // Entrada ADC de referencia de humedad
#define BOMBA_PIN   25  // Salida digital que controla la bomba
#define SERVO_PIN   26  // Salida PWM para control de servomotor 

// Instancias de las clases
LiquidCrystal_I2C lcd(0x27, 16, 2); // Objeto 'lcd' para usar LCD 16x2
DHT dht(DHT_PIN, DHT22);            // Objeto 'dht' para leer T/H aire
Servo servoVentana;                 // Objeto 'servoVentana' para usar servomotor

// ---------- Funciones de lectura ----------

// Lee temperatura y humedad del aire
void leer_DHT(float &temperatura, float &humedad) {
  temperatura = dht.readTemperature();
  humedad     = dht.readHumidity();
}

// Lee la humedad del suelo y la convierte a porcentaje (0–99 %)
float leerHumedadSuelo(int pin, int seco, int humedo) {
  int   valor   = analogRead(pin);
  float humedad = map(valor, seco, humedo, 0, 99);
  return constrain(humedad, 0, 99);
}

// Lee el potenciómetro que ajusta la humedad de referencia (0–99 %)
float leerPoteHumedad(int pin) {
  int valor = analogRead(pin);
  float humedadRef = map(valor, 0, 3000, 0, 99);
  return constrain(humedadRef, 0, 99);
}

// Lee el potenciómetro que ajusta la temperatura de referencia (17–35 °C)
float leerPoteTemp(int pin) {
  int valor = analogRead(pin);
  float tempRef = map(valor, 0, 3000, 17, 35);
  return constrain(tempRef, 17, 35);
}


// Muestra en la pantalla LCD valores de referencia, temperatura, 
// medición de humedad del suelo y aire del sistema.
void printer(float humRef, float humSuelo, float humAire, float tempRef, float temp) {
  // Conversión de valores float a enteros para mostrar sin decimales
  int humSuelo_int = (int)humSuelo;
  int humAire_int  = (int)humAire;
  int tempRef_int  = (int)tempRef;
  int humRef_int   = (int)humRef;
  int temp_int     = (int)temp;
  lcd.setCursor(0, 0);  // Columna 0, fila 0
  lcd.print("HR:"); lcd.print(humRef_int); 
  lcd.setCursor(5, 0);  // Columna 5, fila 0
  lcd.print(" H:"); lcd.print(humSuelo_int); 
  lcd.setCursor(10, 0); // Columna 10, fila 0
  lcd.print(" HA:"); lcd.print(humAire_int);
  lcd.setCursor(0, 1);  // Columna 0, fila 1
  lcd.print("TR:"); lcd.print(tempRef_int); 
  lcd.setCursor(5, 1);  // Columna 5, fila 1
  lcd.print(" T:"); lcd.print(temp_int);
  lcd.setCursor(10, 1); // Columna 10, fila 1
  lcd.print("      ");  // Relleno en blanco
}

// ---------- Servo (límites y movimiento) ----------
const int LIMITE_ABIERTO = 80;
const int LIMITE_CERRADO = 160;
bool ventanaAbierta = false;
int anguloActual = LIMITE_CERRADO;

void moverVentana(bool moverServo) {
  int anguloRetenido = anguloActual;
  if(moverServo && anguloActual > LIMITE_ABIERTO){
    anguloActual -= 10;
    if (anguloActual <= LIMITE_ABIERTO) anguloActual = LIMITE_ABIERTO;
  }else if(!moverServo && anguloActual < LIMITE_CERRADO){
    anguloActual += 10;
    if (anguloActual >= LIMITE_CERRADO) anguloActual = LIMITE_CERRADO;
  }
  servoVentana.attach(SERVO_PIN);
  servoVentana.write(anguloActual);
  delay(500);                  // breve para asegurar llegada
  servoVentana.detach();       // ahorro energía/ruido
  return;
}

// ---------- Timers ----------
const unsigned long TIME_POTE_SS = 600;
const unsigned long TIME_LECT_SENSOR   = 3000;   // lecturas cada 3 s
const unsigned long TIME_SERVO = 15UL * 1000UL;  // decisión cada 5 min => 5UL * 60UL * 1000UL
const unsigned long TIME_REBOOT = 7UL * 24UL * 60UL * 60UL * 1000UL;  // reinicio ESP32 cada 7 días
unsigned long timeLectSensor = 0, timeServo = 0, timeLectPote = 0;

// Lecturas actuales
float temp = NAN, humAire = NAN;
float humSuelo1 = NAN, humSuelo2 = NAN, humSueloProm = NAN;
float humRef = NAN, tempRef = NAN;

// Histeresis temperatura (evita abrir/cerrar seguido)
const float H_T = 2.0;    // cierra si T <= tempRef - H_T y abre si T >= tempRef + H_T;

void setup() {
  Serial.begin(115200);           // Inicializa la comunicación serie
  dht.begin();                    // Inicializa el sensor DHT22

  pinMode(BOMBA_PIN, OUTPUT);     // Configura el pin de la bomba como salida
  digitalWrite(BOMBA_PIN, HIGH);  // Estado inicial: bomba desactivada

  // Posición inicial segura del servo
  servoVentana.attach(SERVO_PIN);
  servoVentana.write(LIMITE_CERRADO);
  delay(2000);
  lcd.init();             // Inicializa el LCD
  delay(1000);
  lcd.backlight();        // Enciende la retroiluminación
  lcd.clear();
  servoVentana.detach();
  ventanaAbierta = false;

  unsigned long now = millis();
  timeLectSensor   = now + TIME_LECT_SENSOR;
  timeServo = now + TIME_SERVO;
  timeLectPote = now + TIME_POTE_SS;
}

void loop() {
  unsigned long now = millis();

  // ---- Lecturas periódicas (cada 3 s) ----
  if (now >= timeLectSensor) {
    // Lectura del DHT22
    leer_DHT(temp, humAire);

    // Lectura y promedio de los dos sensores de humedad del suelo
    humSuelo1 = leerHumedadSuelo(SENSOR_PIN1, 2750, 1340);
    humSuelo2 = leerHumedadSuelo(SENSOR_PIN2, 2750, 1270);
    humSueloProm = (humSuelo1 + humSuelo2) / 2.0;

    /* ------------Lógica de humedad (bomba)------------
      Compara Hum de referencia con Hum real y estado de 
      la bomba ON/OFF para activar o desactivar la bomba.
    */
    if (humSueloProm < (humRef - 10) && digitalRead(BOMBA_PIN)) {
      digitalWrite(BOMBA_PIN, LOW);   // activa bomba
      Serial.println("BOMBA ACTIVADA");
    } 
    else if (humSueloProm >= humRef && !digitalRead(BOMBA_PIN)) {
      digitalWrite(BOMBA_PIN, HIGH);  // desactiva bomba
      Serial.println("BOMBA DESACTIVADA");
    }
  
    // Monitoreo en serial monitor
    Serial.print("T: ");           Serial.print(temp);
    Serial.print(" °C | HAire: "); Serial.print(humAire);
    Serial.print("% | HSuelo: ");  Serial.print(humSueloProm);
    Serial.print("% | HRef: ");    Serial.print(humRef);
    Serial.print("% | TRef: ");    Serial.print(tempRef);
    Serial.print(" °C | Bomba: "); Serial.println(digitalRead(BOMBA_PIN));

    timeLectSensor += TIME_LECT_SENSOR;
  }

  if (now >= timeLectPote) {
    // Lecturas de referencia desde los potenciómetros
    humRef  = leerPoteHumedad(POTE_HUM);
    tempRef = leerPoteTemp(POTE_TEMP);

    // Mostrar variables en LCD
    printer(humRef, humSueloProm, humAire, tempRef, temp);

    timeLectPote += TIME_POTE_SS;
  }

  // ---- Decisión de ventana (cada 5 min) con histéresis ----
  if (now >= timeServo) {
    if (!isnan(temp) && !isnan(tempRef)) {
      if (temp >= (tempRef + H_T)) {
        moverVentana(true);    // abrir
        Serial.print("VENTANA ABIERTA: "); 
        Serial.print(anguloActual);
        Serial.println(" grados |");
      } else if (temp <= (tempRef - H_T)) {
        moverVentana(false);   // cerrar
        Serial.print("VENTANA CERRADA: ");
        Serial.print(anguloActual);
        Serial.println(" grados |");
      }
    }
    timeServo += TIME_SERVO;
  }
  if (now >= TIME_REBOOT) esp_restart();;
}

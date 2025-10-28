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
#define DHT_PIN      27  // Entrada digital del sensor DHT22
#define SENSOR_PIN1  32  // Entrada ADC de sensor1 de humedad de suelo
#define SENSOR_PIN2  33  // Entrada ADC de sensor2 de humedad de suelo
#define POTE_TEMP    34  // Entrada ADC de referencia de temperatura
#define POTE_HUM     35  // Entrada ADC de referencia de humedad
#define BOMBA_PIN    25  // Salida digital que controla la bomba
#define SERVO_PIN    26  // Salida PWM para control de servomotor 
#define SERVO_EN_PIN 14  // Salida digital que alimenta el servomotor

// ========= Servo (límites y movimiento) =========
const int LIMITE_ABIERTO = 80;        // Ángulo mínimo (ventana completamente abierta)
const int LIMITE_CERRADO = 160;       // Ángulo máximo (ventana completamente cerrada)

// ========== Temporizadores del sistema ==========
const unsigned long TIME_POTE_SS = 600;             // Lectura de potenciómetros cada 0.6 s
const unsigned long TIME_LECT    = 3000;            // Lectura de sensores cada 3 s
const unsigned long TIME_SERVO   = 9UL * 1000UL;   // Decisión de servo cada 15 s
const unsigned long TIME_REBOOT  = 7UL * 24UL * 60UL * 60UL * 1000UL; // reinicio ESP32 cada 7 días

// =========== Instancias de las clases ===========
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Objeto 'lcd' para usar LCD 16x2
DHT dht(DHT_PIN, DHT22);              // Objeto 'dht' para leer T/H aire
Servo servoVentana;                   // Objeto 'servoVentana' para usar servomotor

// ========= Variables de estado globales =========
int anguloActual = LIMITE_CERRADO;    // Posición actual del servo (inicia cerrada)
unsigned long timeLectSensor = 0;     // Temporizador de lectura de sensores
unsigned long timeServo      = 0;     // Temporizador de control de ventana
unsigned long timeLectPote   = 0;     // Temporizador de lectura de potenciómetros
// Variables iniciales de medición
float temp      = NAN, humAire   = NAN;                     // Temperatura y humedad del aire
float humSuelo1 = NAN, humSuelo2 = NAN, humSueloProm = NAN; // Valores de humedad de suelo
float humRef    = NAN, tempRef   = NAN;                     // Referencias de potenciómetros

// ============= Funciones de lectura =============
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

// =========== Función de visualización ===========
// Muestra en la pantalla LCD valores de referencia, temperatura, 
// medición de humedad del suelo y aire del sistema.
void printer(float humRef, float humSuelo, float humAire, float tempRef, float temp) {
  // Conversión de valores float a enteros para mostrar sin decimales
  int humSuelo_int = (int)humSuelo;
  int humAire_int  = (int)humAire;
  int tempRef_int  = (int)tempRef;
  int humRef_int   = (int)humRef;
  int temp_int     = (int)temp;

  // Primera fila del LCD: humedades (referencia, suelo, aire)
  lcd.setCursor(0, 0);  // Columna 0, fila 0
  lcd.print("HR:"); lcd.print(humRef_int); 
  lcd.setCursor(5, 0);  // Columna 5, fila 0
  lcd.print(" H:"); lcd.print(humSuelo_int); 
  lcd.setCursor(10, 0); // Columna 10, fila 0
  lcd.print(" HA:"); lcd.print(humAire_int);

  // Segunda fila del LCD: temperaturas (referencia y real)
  lcd.setCursor(0, 1);  // Columna 0, fila 1
  lcd.print("TR:"); lcd.print(tempRef_int); 
  lcd.setCursor(5, 1);  // Columna 5, fila 1
  lcd.print(" T:"); lcd.print(temp_int);
  lcd.setCursor(10, 1); // Columna 10, fila 1
  lcd.print("      ");  // Relleno en blanco
}

// ============ Función de servomotor =============
// Controla la posición de la ventana mediante un servomotor.
void moverVentana(bool moverServo) {
  // Abrir 10 grados si aún no llegó al límite
  if(moverServo && anguloActual > LIMITE_ABIERTO){
    anguloActual -= 10;
    if (anguloActual <= LIMITE_ABIERTO) anguloActual = LIMITE_ABIERTO;
    Serial.print("VENTANA ABIERTA: "); 
    Serial.print(anguloActual);
    Serial.print(" grados |");
  }
  // Cerrar 10 grados si aún no llegó al límite
  else if(!moverServo && anguloActual < LIMITE_CERRADO){
    anguloActual += 10;
    if (anguloActual >= LIMITE_CERRADO) anguloActual = LIMITE_CERRADO;
    Serial.print("VENTANA CERRADA: ");
    Serial.print(anguloActual);
    Serial.print(" grados |");
  }
  // Activación del servomotor
  digitalWrite(SERVO_EN_PIN, LOW);    // Activa relay (alimenta el servo)
  delay(1000);
  servoVentana.attach(SERVO_PIN);     // Activa señal PWM
  servoVentana.write(anguloActual);   // Mueve al nuevo ángulo
  Serial.println("PWM enviado |");
  delay(500);                         // Breve para asegurar llegada
  servoVentana.detach();              // Corta PWM (ahorra energía y ruido)
  digitalWrite(SERVO_EN_PIN, HIGH);   // Apaga relay (corta energía al servo)
}

// ============ Configuración inicial del sistema ============
// Inicializa sensores, pantalla, actuadores y variables de tiempo.
void setup() {
  Serial.begin(115200);   // Inicializa la comunicación serie
  delay(2000);            // Pausa para sincronizar el monitor

  // Sensor DHT22 y Pantalla LCD 16x2 ----
  dht.begin();            // Inicializa el sensor DHT22
  lcd.init();             // Inicializa el LCD
  lcd.backlight();        // Enciende la retroiluminación
  lcd.clear();            // Limpia la pantalla

  // Actuadores y salidas-----------------
  pinMode(BOMBA_PIN, OUTPUT);         // Configura el pin de la bomba como salida
  pinMode(SERVO_EN_PIN, OUTPUT);      // Configura el pin que alimentación del servo como salida
  digitalWrite(BOMBA_PIN, HIGH);      // Estado inicial: bomba desactivada
  digitalWrite(SERVO_EN_PIN, LOW);    // Estado inicial: servo activado
  delay(1000);                        // Espera para que servo servo se alimente
  // Posición inicial segura del servo ---
  servoVentana.attach(SERVO_PIN);     // Habilita PWM del servo
  servoVentana.write(LIMITE_CERRADO); // Posición inicial del servo
  delay(1000);                        // Espera a que servo alcance posición
  servoVentana.detach();              // Deshabilita PWM (ahorro de energía)
  digitalWrite(SERVO_EN_PIN, HIGH);   // Apaga relay (corta energía al servo)

  // Temporizadores iniciales-------------
  unsigned long now = millis();         // Tiempo actual desde el arranque del sistema (ms)
  timeLectSensor = now + TIME_LECT;     // Primera lectura de sensores
  timeLectPote   = now + TIME_POTE_SS;  // Primera lectura de potenciómetros
  timeServo      = now + TIME_SERVO;    // Primera decisión de ventana
}

void loop() {
  unsigned long now = millis();

  // ===== Lecturas de sensores cada 3 s =====
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
      digitalWrite(BOMBA_PIN, LOW);   // Activa bomba
      Serial.println("BOMBA ACTIVADA");
    }
    else if (humSueloProm >= humRef && !digitalRead(BOMBA_PIN)) {
      digitalWrite(BOMBA_PIN, HIGH);  // Desactiva bomba
      Serial.println("BOMBA DESACTIVADA");
    }

    // Monitoreo en serial monitor
    Serial.print("T: ");           Serial.print(temp);
    Serial.print(" °C | HAire: "); Serial.print(humAire);
    Serial.print("% | HSuelo: ");  Serial.print(humSueloProm);
    Serial.print("% | HRef: ");    Serial.print(humRef);
    Serial.print("% | TRef: ");    Serial.print(tempRef);
    Serial.print(" °C | Bomba: "); Serial.println(digitalRead(BOMBA_PIN));

    timeLectSensor += TIME_LECT;  // Tiempo de siguiente lectura de sensores
  }

  // ===== Lecturas de potenciómetros cada 0.6 s =====
  if (now >= timeLectPote) {
    // Lecturas de referencia desde los potenciómetros
    humRef  = leerPoteHumedad(POTE_HUM);
    tempRef = leerPoteTemp(POTE_TEMP);

    // Mostrar variables en LCD
    printer(humRef, humSueloProm, humAire, tempRef, temp);

    timeLectPote += TIME_POTE_SS; // Tiempo de siguiente lectura de referencias
  }

  // ===== Decisión de ventana (cada 5 min) con histéresis =====
  if (now >= timeServo) {
    if (!isnan(temp) && !isnan(tempRef)) {
      if (temp >= (tempRef + 2)) {
        moverVentana(true);    // abrir
      } else if (temp <= (tempRef - 2)) {
        moverVentana(false);   // cerrar
      }
    }
    timeServo += TIME_SERVO;      // Tiempo de siguiente movimiento del servo
  }

  // ===== Reinicio del ESP32 cada 7 días =====
  if (now >= TIME_REBOOT) esp_restart();
}

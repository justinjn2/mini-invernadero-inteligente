#include <ESP32Servo.h>

#define SERVO_PIN 26   // GPIO26

Servo servoVentana;

void setup() {
  Serial.begin(115200);
  servoVentana.attach(SERVO_PIN);  // Inicializa el PWM para el servo
  Serial.println("Iniciando prueba del servomotor...");
}

void loop() {
  // Barrido de 0° a 180°
  for (int angulo = 80; angulo <= 160; angulo += 20) {
    servoVentana.write(angulo);
    Serial.print("Ángulo: ");
    Serial.println(angulo);
    delay(1000);
  }

  // Barrido de 180° a 0°
  for (int angulo = 160; angulo >= 80; angulo -= 20) {
    servoVentana.write(angulo);
    Serial.print("Ángulo: ");
    Serial.println(angulo);
    delay(1000);
  }
}
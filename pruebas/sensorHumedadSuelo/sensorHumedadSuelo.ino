// Prueba de 2 sensores capacitivos de humedad del suelo AZDelivery V1.2
// ESP32: ADC válidos 32–39. Aquí usamos GPIO32 y GPIO33.

#define SENSOR_PIN1 32   // ADC1_CH4
#define SENSOR_PIN2 33   // ADC1_CH5

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Iniciando prueba de 2 sensores de humedad del suelo...");
}

void loop() {
  // Lecturas crudas (0–4095)
  int valor1 = analogRead(SENSOR_PIN1);
  int valor2 = analogRead(SENSOR_PIN2);

  // Mapeando % según calibración
  // Valores altos = más seco
  // Valores bajos = más humedo
  float humedad1 = map(valor1, 2750, 1340, 0, 100);
  float humedad2 = map(valor2, 2750, 1270), 0, 100);

  // Saturar cada una a 0–100
  if (humedad1 < 0) humedad1 = 0;
  if (humedad1 > 100) humedad1 = 100;

  if (humedad2 < 0) humedad2 = 0;
  if (humedad2 > 100) humedad2 = 100;

  // Calcula y satura el promedio
  float humedadProm = (humedad1 + humedad2) / 2.0;
  if (humedadProm < 0) humedadProm = 0;
  if (humedadProm > 100) humedadProm = 100;
  // ================================================

  // Mostrar resultados
  Serial.print("S1 crudo: ");   Serial.print(valor1);
  Serial.print("  | S1 %: ");   Serial.print(humedad1);

  Serial.print("  ||  S2 crudo: "); Serial.print(valor2);
  Serial.print("  | S2 %: ");       Serial.print(humedad2);

  Serial.print("  ||  Promedio %: "); Serial.println(humedadProm);

  delay(1000);
}

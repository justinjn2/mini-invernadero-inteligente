/*
  Prueba de sensores capacitivos de humedad del suelo
  ------------------------------------------------------
  Este código obtiene la humedad promedio del suelo usando 
  dos sensores capacitivos AZDelivery V1.2. Convierte las
  lectura ADC en un porcentaje de 0–100 %
*/

// Definición de pines analógicos usado por el potenciómetro
#define SENSOR_PIN1 32    // GPIO32
#define SENSOR_PIN2 33    // GPIO33

void setup() {
  Serial.begin(115200);   // Inicializa la comunicación serie
  delay(2000);            // Pausa para estabilizar el monitor
  Serial.println("Iniciando prueba de 2 sensores de humedad del suelo...");
}

void loop() {
  // Lecturas analógicas crudas
  int valor1 = analogRead(SENSOR_PIN1);   // rango ADC: 1340–2750
  int valor2 = analogRead(SENSOR_PIN2);   // rango ADC: 1270–2750

  // Mapeo de valores calibrados a porcentaje (%)
  // Valores altos -> suelo seco | Valores bajos -> suelo húmedo
  float humedad1 = map(valor1, 2750, 1340, 0, 100);
  float humedad2 = map(valor2, 2750, 1270, 0, 100);

  // Saturar valores entre 0 y 100 %
  if (humedad1 < 0) humedad1 = 0;
  if (humedad1 > 100) humedad1 = 100;

  if (humedad2 < 0) humedad2 = 0;
  if (humedad2 > 100) humedad2 = 100;

  // Calcula el promedio de humedad de ambos sensores y lo satura
  float humedadProm = (humedad1 + humedad2) / 2.0;
  if (humedadProm < 0) humedadProm = 0;
  if (humedadProm > 100) humedadProm = 100;

  // Muestra los resultados en el monitor serie
  // "crudo" muestra el valor ADC, y "%” muestra la conversión calibrada
  Serial.print("S1 crudo: ");   Serial.print(valor1);
  Serial.print("  | S1 %: ");   Serial.print(humedad1);

  Serial.print("  ||  S2 crudo: "); Serial.print(valor2);
  Serial.print("  | S2 %: ");       Serial.print(humedad2);

  Serial.print("  ||  Promedio %: "); Serial.println(humedadProm);

  delay(1000);                            // Intervalo de muestreo de 1 segundo
}
/*
  Prueba de potenciómetro (referencia de humedad del suelo)
  ------------------------------------------------------------
  Este código obtiene el valor de referencia de humedad del suelo 
  mediante un potenciómetro conectado al pin GPIO35. Convierte 
  la lectura ADC (0–3000 aprox.) en un porcentaje de 0–100 %
*/

#define POTE_PIN1 35      // Pin analógico usado por el potenciómetro (GPIO35)

void setup() {
  Serial.begin(115200);   // Configura la velocidad de transmisión del puerto serie
  delay(2000);            // Pausa para estabilizar el monitor serie
  Serial.println("Iniciando prueba de lectura de potenciómetros...");
}

void loop() {
  // Lee el valor analógico del potenciómetro (rango 0–3000)
  // Tensión en el potenciómetro (0-2.51) V
  int valor1 = analogRead(POTE_PIN1);

  // Convierte el valor leído (aprox. 0–3000) a una escala de 0 a 100 (%)
  float humedad_ref = map(valor1, 0, 3000, 0, 100);

  // Saturar los extremos para evitar lecturas fuera de rango por ruido o error ADC
  if (humedad_ref < 0) humedad_ref = 0;
  if (humedad_ref > 100) humedad_ref = 100;

  // Envía los resultados al monitor serie
  // "crudo" muestra el valor ADC, y "%” muestra la conversión calibrada
  Serial.print("S1 crudo: ");
  Serial.print(valor1);
  Serial.print("  | S1 %: ");
  Serial.print(humedad_ref);
  Serial.println(" %");
  delay(1000);            // Intervalo de muestreo de 1 segundo
}

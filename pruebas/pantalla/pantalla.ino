#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección I2C común del módulo (puede ser 0x27 o 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();           // Inicializa el LCD
  lcd.backlight();      // Enciende la retroiluminación
  lcd.setCursor(0, 0);  // Columna 0, fila 0
  lcd.print("Hola mundo!");
  lcd.setCursor(0, 1);  // Segunda línea
  lcd.print("ESP32 + I2C LCD");
}

void loop() {
  // No hace nada más, texto fijo
}

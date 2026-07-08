#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 17             // GPIO on està connectat el sensor de temperatura i humitat
#define DHTTYPE DHT11         // Tipus de sensor de temperatura i humitat

#define LCD_COLS 16           // Nº de columnes de caràcters disponibles al panell LCD
#define LCD_ROWS 2            // Nº de files disponibles al panell LCD
#define LCD_ADDRESS 0x27      // Adreça del panell LCD on mostrar els resultats.

#define LOOP_DURATION 10000   // Temps de cicle en ms

// Definim variables globals
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void setup() {
  // Inicialització del sensor de Temperatura + Humitat
  dht.begin();

  // Inicialització del panell LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Iniciant...");
}

void loop() {
  // Llegim valors de temperatura i humitat
  float humity = dht.readHumidity();
  float temperature = dht.readTemperature();      // Celsius

  // Comprovar errors
  if (isnan(humity) || isnan(temperature)) {
    lcd.print("Error llegint el DHT11");
    return;
  }

  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("Temp: ");
  lcd.print(temperature,1);
  lcd.print((char)223);   // símbol °
  lcd.print("C");

  lcd.setCursor(0,1);
  lcd.print("Humitat: ");
  lcd.print(humity,0);
  lcd.print("%");

  // Esperem per la següent volta
  delay(LOOP_DURATION);
}

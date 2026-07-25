// Llibreries necessàries, natives i a instal·lar
#include <DHT.h>                  // Per la comunicació amb el sensor DHT11 de temperaturoa i humitat (Requereix INSTAL·LACIÓ)
#include <Wire.h>                 // Gestió de la connectivitat IO amb el pinatge de la placa
#include <Utils.h>                // Llibreria d'utilitats pròpia (gestió de l'hora actual)

// Llibreries locals
#include <BasicLcd.h>

#define DHTPIN 17             // GPIO on està connectat el sensor de temperatura i humitat
#define DHTTYPE DHT11         // Tipus de sensor de temperatura i humitat

#define LCD_COLS 16           // Nº de columnes de caràcters disponibles al panell LCD
#define LCD_ROWS 2            // Nº de files disponibles al panell LCD
#define LCD_ADDRESS 0x27      // Adreça del panell LCD on mostrar els resultats.

#define LOOP_DURATION 20000   // Temps de cicle en ms

// Definim variables globals
DHT dht(DHTPIN, DHTTYPE);
BasicLcd lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void setup() {
  // Inicialització d'instrumental. Comunicació sèrie, sensor i pantalla LCD:
  Serial.begin(115200);
  dht.begin();
  lcd.start();
}

void loop() {
  // Llegim valors de temperatura i humitat
  float humity = dht.readHumidity();
  float temperature = dht.readTemperature();      // Celsius

  // Comprovar errors
  if (isnan(humity) || isnan(temperature)) {
    lcd.write("Error llegint", "DHT11");
    return;
  }

  // Mostrem informació
  lcd.display(temperature, humity, Utils::obtenirHoraActual());
  
  // Esperem per la següent volta
  delay(LOOP_DURATION);
}

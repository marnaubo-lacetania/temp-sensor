// Llibreries necessàries, natives i a instal·lar
#include <DHT.h>                  // Per la comunicació amb el sensor DHT11 de temperaturoa i humitat (Requereix INSTAL·LACIÓ)
#include <Wire.h>                 // Gestió de la connectivitat IO amb el pinatge de la placa
#include <LiquidCrystal_I2C.h>    // Gestió de la comunicació amb panells LCD I2C                     (Requereix INSTAL·LACIÓ)
#include <WiFi.h>                 // Administrar connectivitat WIFI
#include <ThingSpeak.h>           // Llibreria per facilitar l'escriptura de dades a un canal de ThingSpeak (Mathworks)
#include <HTTPClient.h>          // Llibreria per fer peticions HTTP

// Fitxers i llibreries locals a incloure al compilar
#include "config.h"
#include <BasicLcd.h>

#define DHTPIN 17             // GPIO on està connectat el sensor de temperatura i humitat
#define DHTTYPE DHT11         // Tipus de sensor de temperatura i humitat

#define LCD_COLS 16           // Nº de columnes de caràcters disponibles al panell LCD
#define LCD_ROWS 2            // Nº de files disponibles al panell LCD
#define LCD_ADDRESS 0x27      // Adreça del panell LCD on mostrar els resultats.

// Definim variables globals
DHT dht(DHTPIN, DHTTYPE);
BasicLcd lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
WiFiClient client;

bool ready = false;

bool connectWiFi() {
    Serial.print("Connectant a ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int intents = 0;

    while (WiFi.status() != WL_CONNECTED && intents < 20) {
        delay(500);
        Serial.print(".");
        intents++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connectada!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());

        return true;
    } else {
        Serial.println();
        Serial.println("No s'ha pogut connectar a la WiFi.");
        
        return false;
    }
}

bool uploadData(float temperature, float humity) {
  // Assignació als camps de ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humity);

  int resultat = ThingSpeak.writeFields(
    TS_CHANNEL_ID,
    TS_CHANNEL_API_KEY
  );

  if (resultat == 200) {
    Serial.println("Dades enviades correctament");
    return true;
  } else {
    Serial.print("Error enviant dades. Codi HTTP: ");
    Serial.println(resultat);
    return false;
  }
  Serial.println("-------------------------");
}

bool hasInternet() {
    HTTPClient http;

    http.setTimeout(3000);  // 3 segons
    http.begin("http://clients3.google.com/generate_204");

    int code = http.GET();

    http.end();

    return (code == 204);
}

void setup() {
  // Inicialització del sensor de Temperatura + Humitat
  Serial.begin(115200);
  dht.begin();

  // Inicialització del panell LCD
  lcd.start();

  // Connectar a la WiFi
  lcd.write("Connectant WiFi");
  if (! connectWiFi()) {
    lcd.write("Error connexio","WiFi");
    return;
  }

  if (hasInternet()) {
    Serial.println("Internet OK");
  } else {
    Serial.println("Sense Internet");
    lcd.write("Wifi connectada", "sense Internet!");
    return;
  }

  // Habilitar client d'escriptura de dades amb ThingSpeak:
  ThingSpeak.begin(client);

  ready = true;
}

void loop() {
  if (!ready) {
    return;
  }

  // Llegim valors de temperatura i humitat
  float humity = dht.readHumidity();
  float temperature = dht.readTemperature();      // Celsius

  // Comprovar errors
  if (isnan(humity) || isnan(temperature)) {
    lcd.write("Error llegint", "DHT11");
    return;
  }

  lcd.display(temperature, humity);

  if (! uploadData(temperature, humity)) {
    lcd.write("Error carregant", "dades al canal");
    delay(2000);
  }

  // Esperem per la següent volta
  delay(LOOP_DURATION);
}

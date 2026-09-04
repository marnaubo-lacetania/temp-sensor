// =========================================================================
// Importació de llibreries
// =========================================================================
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <esp_wpa2.h>
#include <BasicLcd.h>
#include "config.h"
#include <Utils.h>
#include <ThingSpeak.h>           // Llibreria per facilitar l'escriptura de dades a un canal de ThingSpeak (Mathworks)

// =========================================================================
// Variables globals i configuracions
// =========================================================================
namespace Config {
  constexpr char AQC_API_URL[] = "https://aulesquecremen.cat/api/v1/readings";
  constexpr char CONFIG_FILE[] = "/config.json";
  constexpr uint64_t US_TO_S_FACTOR = 1000000ULL;
  constexpr uint32_t HTTP_TIMEOUT_MS = 30000;
  constexpr uint32_t PORTAL_TIMEOUT_MS = 45000;
  constexpr int RETRY_SLEEP_SECONDS = 60;
  constexpr int MIN_SLEEP_SECONDS = 1;
}  // namespace Config

// Estructura per contenir la configuració del sensor a consultar a la API externa de configuració:
struct SensorConfig {
  String AQC_API_TOKEN;
  int TS_CHANNEL_ID;
  String TS_CHANNEL_API_KEY;
  int LOOP_DURATION;
  int DHT_PIN;
  int LCD_COLS;
  int LCD_ROWS;
  int LCD_ADDRESS;
};

// Funció per entrar en Deep Sleep amb un temps de son definit
void enterDeepSleep(int timeToSleep) {
  // Evita configurar un temporitzador de 0 segons si loop_duration < 1000 ms.
  timeToSleep = max(timeToSleep, Config::MIN_SLEEP_SECONDS);
  Serial.printf("Entrant en Deep Sleep (%d segons)...\n", timeToSleep);
  esp_sleep_enable_timer_wakeup((uint64_t)timeToSleep * Config::US_TO_S_FACTOR);
  esp_deep_sleep_start();
}

// Funció per establir la connexió amb la WiFI
void connectToWifi() {
  // =========================================================================
  // Connexió a la xarxa WIFI configurada
  // =========================================================================
  WiFi.mode(WIFI_STA);
  delay(100);

  if (WIFI_TYPE == "ent") {
    Serial.println("Intentant connexió a xarxa WPA2-Enterprise (Escola)...");

    String anonymous_identity = "";
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)anonymous_identity.c_str(), anonymous_identity.length());
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USER, strlen(WIFI_USER));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));

    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(WIFI_SSID);
  } else {
    Serial.println("Intentant connexió a xarxa WPA2-Personal (Casa/Normal)...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConnexió fallida. Tornant a dormir 60s per reintentar...");
    enterDeepSleep(Config::RETRY_SLEEP_SECONDS);
  }
  Serial.println("\nConnectat correctament!");
}

// Funció per enviar les dades a ThingSpeak
bool uploadDataToThingSpeak(float temperature, float humidity, SensorConfig config) {
  WiFiClient thingSpeakClient;
  ThingSpeak.begin(thingSpeakClient);
  
  // Assignació als camps de ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);

  int resultat = ThingSpeak.writeFields(config.TS_CHANNEL_ID, config.TS_CHANNEL_API_KEY.c_str());

  if (resultat == 200) {
    Serial.println("Dades enviades correctament a ThingSpeak!");
    return true;
  } else {
    Serial.print("Error enviant dades a ThingSpeak. Codi HTTP: ");
    Serial.println(resultat);
    return false;
  }
}

// Funció per enviar les dades a l'API de "Aules que cremen". Extreta del codi públic de l'aplicació propia de AQC
int updateDataToAQC(SensorConfig config, float t, float h) {
  Serial.println("\nEnviant dades a l'API de 'Aules que cremen'...");

  int newTimeToSleep = 0;

  JsonDocument doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  String requestBody;
  serializeJson(doc, requestBody);

  WiFiClientSecure aqcClient;
  aqcClient.setInsecure();  // TODO: instal·lar el certificat CA en producció.
  HTTPClient aqcHttp;
  aqcHttp.begin(aqcClient, Config::AQC_API_URL);
  aqcHttp.setTimeout(Config::HTTP_TIMEOUT_MS);
  aqcHttp.addHeader("Content-Type", "application/json");
  aqcHttp.addHeader("Accept", "application/json");
  String authHeader = "Bearer " + config.AQC_API_TOKEN;
  aqcHttp.addHeader("Authorization", authHeader);

  // Demanem explícitament al client HTTP que reculli la capçalera dinàmica
  const char *headerKeys[] = { "X-RateLimit-Interval" };
  aqcHttp.collectHeaders(headerKeys, 1);
  
  int httpResponseCode = aqcHttp.POST(requestBody);

  // Analitzem la resposta del servidor
  if (httpResponseCode == 200 || httpResponseCode == 201 || httpResponseCode == 202) {
    Serial.print("Dada enregistrada correctament (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");

    if (aqcHttp.hasHeader("X-RateLimit-Interval")) {
      int new_interval = aqcHttp.header("X-RateLimit-Interval").toInt();
      if (new_interval > 0) {
        // La placa obeeix cegament el servidor
        newTimeToSleep = new_interval;
        Serial.printf(" --> El servidor ha fixat el nou interval a: %d segons\n", newTimeToSleep);
      }
    }

  } else if (httpResponseCode == 429) {
    Serial.println("Error 429: Rate Limit excedit. El servidor està saturat.");

    String response = aqcHttp.getString();
    JsonDocument respDoc;
    if (!deserializeJson(respDoc, response)) {
      if (respDoc["retry_after_seconds"].is<int>()) {
        // La placa obeeix cegament el servidor
        newTimeToSleep = respDoc["retry_after_seconds"].as<int>();
        Serial.printf(" --> El servidor demana que ens adormim fins d'aquí a %d segons\n", newTimeToSleep);
      }
    }

  } else if (httpResponseCode > 0) {
    Serial.print("Error de resposta del servidor (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");
  } else {
    Serial.print("Error de xarxa en l'enviament: ");
    Serial.println(aqcHttp.errorToString(httpResponseCode).c_str());
  }

  aqcHttp.end();
  Serial.println("Finalitzat l'enviament a l'API d'Aules que cremen.");

  return newTimeToSleep;
}

// Crear una funció que descarregui la configuració de Supabase i ho retorni en un objecte amb les diferents variables de configuració.
SensorConfig downloadConfiguration() {
  Serial.println("\nConsultant configuració remota del dispositiu a l'API de Supabase...");
  Serial.println("\nAdreça MAC del dispositiu: " + WiFi.macAddress());

  // Definim els valors per defecte dels paràmetres que hem de consultar de l'API:
  SensorConfig config;
  config.AQC_API_TOKEN = "";
  config.TS_CHANNEL_ID = 0;
  config.TS_CHANNEL_API_KEY = "";
  config.LOOP_DURATION = 30000;
  config.DHT_PIN = 17;
  config.LCD_COLS = 16;
  config.LCD_ROWS = 2;
  config.LCD_ADDRESS = 0x27;
  
  // Establim client i sessió de connexió amb l'API de Supabase per descarregar la configuració del dispositiu:
  WiFiClientSecure configClient;
  configClient.setInsecure();  // TODO: instal·lar el certificat CA en producció.
  HTTPClient configHttp;
  const String configUrl = String(SUPABASE_API_URL) +
                           "?mac_address=eq." + WiFi.macAddress() + "&select=*";
  configHttp.begin(configClient, configUrl);
  configHttp.setTimeout(Config::HTTP_TIMEOUT_MS);
  configHttp.addHeader("Accept", "application/json");
  configHttp.addHeader("apikey", SUPABASE_API_TOKEN);
  
  bool validRemoteConfig = false;
  int httpResponseCode = configHttp.GET();

  // Extreure el contingut de la resposta a la petició:
  if (httpResponseCode == 200) {
    String response = configHttp.getString();
    
    // Analitzar la resposta JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("Error en deserialitzar JSON: ");
      Serial.println(error.c_str());
    } else {
      if (!doc.is<JsonArray>()) {
        Serial.println("Error: La resposta de l'API no és un array JSON.");
      } else {
        if (doc.as<JsonArray>().size() == 0) {
          Serial.println("Error: No s'ha trobat la configuració del dispositiu a l'API. Tornant a dormir 60s...");
        } else {
          JsonObject obj = doc.as<JsonArray>()[0];
          if (obj["aqc_api_token"].is<String>()) {
            config.AQC_API_TOKEN = obj["aqc_api_token"].as<String>();
          }
          if (obj["ts_channel_id"].is<int>()) {
            config.TS_CHANNEL_ID = obj["ts_channel_id"].as<int>();
          }
          if (obj["ts_channel_api_key"].is<String>()) {
            config.TS_CHANNEL_API_KEY = obj["ts_channel_api_key"].as<String>();
          }
          if (obj["loop_duration"].is<int>()) {
            config.LOOP_DURATION = obj["loop_duration"].as<int>();
          }
          if (obj["dht_pin"].is<int>()) {
            config.DHT_PIN = obj["dht_pin"].as<int>();
          }
          if (obj["lcd_cols"].is<int>()) {
            config.LCD_COLS = obj["lcd_cols"].as<int>();
          }
          if (obj["lcd_rows"].is<int>()) {
            config.LCD_ROWS = obj["lcd_rows"].as<int>();
          }
          if (obj["lcd_address"].is<int>()) {
            config.LCD_ADDRESS = obj["lcd_address"].as<int>();
          }
          validRemoteConfig = config.LOOP_DURATION > 0 && config.DHT_PIN >= 0 &&
                              config.LCD_COLS > 0 && config.LCD_ROWS > 0;

          //Volquem configuració remota a la sortida sèrie per verificar que s'ha llegit correctament:
          Serial.println("Configuració remota llegida correctament:");
          Serial.printf(" --> AQC_API_TOKEN: %s\n", config.AQC_API_TOKEN.c_str());
          Serial.printf(" --> TS_CHANNEL_ID: %d\n", config.TS_CHANNEL_ID);
          Serial.printf(" --> TS_CHANNEL_API_KEY: %s\n", config.TS_CHANNEL_API_KEY.c_str());
          Serial.printf(" --> LOOP_DURATION: %d s\n", config.LOOP_DURATION);
          Serial.printf(" --> DHTPIN: %d\n", config.DHT_PIN);
          Serial.printf(" --> LCD_COLS: %d\n", config.LCD_COLS);
          Serial.printf(" --> LCD_ROWS: %d\n", config.LCD_ROWS);
          Serial.printf(" --> LCD_ADDRESS: 0x%02X\n", config.LCD_ADDRESS);
        }
      }
    }
  } else {
    Serial.print("Error en consultar l'API (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");
  }
  configHttp.end();

  // Sense configuració remota no convé continuar amb valors parcials o antics.
  if (!validRemoteConfig) {
    Serial.println("Configuració remota absent o no vàlida. Reintent en 60s.");
    enterDeepSleep(Config::RETRY_SLEEP_SECONDS);
  }
  return config;
}


void setup() {
  // Inicialització de la sortida sèrie per a depuració
  Serial.begin(115200);
  delay(1000);

  // Ens connectem a la WiFi
  connectToWifi();

  // Descarreguem la configuració de la API de SupaBase
  SensorConfig sensorConfig = downloadConfiguration();

  // Si tots els parametres són correctes, es pot procedir a llegir el sensor i enviar les dades a les APIs i al panell LCD:  
  DHT dht(sensorConfig.DHT_PIN, DHT11);
  BasicLcd lcd(sensorConfig.LCD_ADDRESS, sensorConfig.LCD_COLS, sensorConfig.LCD_ROWS);

  // =========================================================================
  // Lectura de les dades del sensor DHT11
  // =========================================================================
  dht.begin();
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Error físic del sensor DHT11. Tornant a dormir 60s...");
    enterDeepSleep(Config::RETRY_SLEEP_SECONDS);
  }

  // Mostrem les dades pel LCD. Indicarem l'hora de la lectura per desacoplar la llibreria del tipus
  // de connexió a Internet (WIFI, 4G, etc.) i del tipus de servidor NTP que s'utilitzi. 
  lcd.start();
  lcd.display(t, h, Utils::obtenirHoraActual());

  // Si està correctament definit el token de l'API, enviem les dades a l'API de "Aules que cremen"
  int timeToSleep = max(Config::MIN_SLEEP_SECONDS, sensorConfig.LOOP_DURATION);
  if (sensorConfig.AQC_API_TOKEN != "") {
    int newTimeToSleep = updateDataToAQC(sensorConfig, t, h);
    if (newTimeToSleep > 0) {
      timeToSleep = newTimeToSleep;
    }
  }

  // Si està correctament definit el canal de ThingSpeak, enviem les dades a ThingSpeak
  if (sensorConfig.TS_CHANNEL_ID > 0 && sensorConfig.TS_CHANNEL_API_KEY != "") {
    uploadDataToThingSpeak(t, h, sensorConfig);
  }
  
  enterDeepSleep(timeToSleep);
}

void loop() {}

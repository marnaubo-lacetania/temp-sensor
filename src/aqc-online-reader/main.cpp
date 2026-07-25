// =========================================================================
// [BLOC 1] LLIBRERIES I CONFIGURACIÓ DE MAQUINARI
// =========================================================================
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <esp_wpa2.h>
#include <BasicLcd.h>
#include "config.h"
#include <Utils.h>

DHT dht(DHTPIN, DHT11);

// =========================================================================
// [BLOC 2] VARIABLES GLOBALS I SON PROFUND (DEEP SLEEP)
// =========================================================================
String apiUrl = "https://aulesquecremen.cat/api/v1/readings";
#define uS_TO_S_FACTOR 1000000ULL

// Memòria RTC: No s'esborra quan la placa s'adorm
RTC_DATA_ATTR int time_to_sleep = LOOP_DURATION / 1000;  // Valor en segons
String net_type = WIFI_TYPE;
String wifi_ssid = WIFI_SSID;
String ent_user = WIFI_USER;
String wifi_pass = WIFI_PASSWORD;
String api_token = AQC_API_TOKEN;

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);
BasicLcd lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void enterDeepSleep() {
  Serial.printf("Entrant en Deep Sleep (%d segons)...\n", time_to_sleep);
  esp_sleep_enable_timer_wakeup((uint64_t)time_to_sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}


// =========================================================================
// [BLOC 3] GESTIÓ DE MEMÒRIA INTERNA I CONFIGURACIÓ (LITTLEFS)
// =========================================================================
void loadConfig() {
  if (LittleFS.begin(true)) {
    if (LittleFS.exists("/config.json")) {
      File file = LittleFS.open("/config.json", "r");
      if (file) {
        JsonDocument doc;
        if (!deserializeJson(doc, file)) {
          net_type = doc["net_type"] | "ent";
          wifi_ssid = doc["wifi_ssid"] | "gencat_ENS_EDU";
          ent_user = doc["ent_user"] | "";
          wifi_pass = doc["wifi_pass"] | "";
          api_token = doc["api_token"] | "";
        }
        file.close();
      }
    }
  }
}

void saveConfig() {
  JsonDocument doc;
  doc["net_type"] = net_type;
  doc["wifi_ssid"] = wifi_ssid;
  doc["ent_user"] = ent_user;
  doc["wifi_pass"] = wifi_pass;
  doc["api_token"] = api_token;

  File file = LittleFS.open("/config.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}


// =========================================================================
// [BLOC 4] CODI VISUAL DE LA PÀGINA WEB DEL PORTAL CAPTIU (HTML)
// =========================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ca">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Aules que Cremen - Configuració</title>
  <style>
    body { font-family: -apple-system, sans-serif; background-color: #f4f4f9; padding: 20px; color: #333; }
    .container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
    h2 { text-align: center; color: #2c3e50; }
    label { font-weight: bold; display: block; margin-top: 15px; margin-bottom: 5px; }
    input, select { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    button { width: 100%; padding: 12px; background-color: #f39c12; color: white; border: none; border-radius: 4px; font-size: 16px; font-weight: bold; margin-top: 20px; cursor: pointer; }
    button:hover { background-color: #e67e22; }
    .help { font-size: 0.85em; color: #7f8c8d; margin-top: 4px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>Configuració del Sensor</h2>
    <form action="/save" method="POST">
      <label>Tipus de Xarxa Wi-Fi:</label>
      <select name="net_type" id="net_type" onchange="toggleFields()">
        <option value="ent">Xarxa d'Escola (gencat_ENS_EDU)</option>
        <option value="per">Xarxa de Casa / Normal</option>
      </select>

      <label>Nom de la Xarxa (SSID):</label>
      <input type="text" name="ssid" id="ssid" value="gencat_ENS_EDU" required>

      <div id="ent_group">
        <label>Identitat de Centre (Usuari):</label>
        <input type="text" name="user" id="user" placeholder="Ex: w08012345">
        <div class="help">w (minúscula) + Codi del teu centre. Sense espais.</div>
      </div>

      <label>Contrasenya Wi-Fi:</label>
      <input type="password" name="pass" id="pass" placeholder="Contrasenya de la xarxa" required>

      <label>Token de Sanctum:</label>
      <input type="text" name="token" id="token" placeholder="Enganxa el token del dashboard" required>

      <button type="submit">Guardar i Connectar</button>
    </form>
  </div>
  <script>
    function toggleFields() {
      var type = document.getElementById("net_type").value;
      var entGroup = document.getElementById("ent_group");
      var ssidInput = document.getElementById("ssid");
      if(type === "ent") {
        entGroup.style.display = "block";
        ssidInput.value = "gencat_ENS_EDU";
      } else {
        entGroup.style.display = "none";
        if(ssidInput.value === "gencat_ENS_EDU") ssidInput.value = "";
      }
    }
    toggleFields();
  </script>
</body>
</html>
)rawliteral";


// =========================================================================
// [BLOC 5] FUNCIONS I RUTES DEL SERVIDOR WEB
// =========================================================================
void setupServerRoutes() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/save", HTTP_POST, []() {
    net_type = server.arg("net_type");
    wifi_ssid = server.arg("ssid");
    ent_user = server.arg("user");
    wifi_pass = server.arg("pass");
    api_token = server.arg("token");

    saveConfig();

    String html = "<html><body style='font-family:sans-serif; text-align:center; padding-top:50px;'><h2>Dades desades correctament!</h2><p>El sensor es reiniciar&agrave; ara. Ja pots tancar aquesta finestra.</p></body></html>";
    server.send(200, "text/html", html);

    delay(2000);
    ESP.restart();
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
}


// =========================================================================
// [BLOC 6] INICI DEL SISTEMA (SETUP) I PORTAL CAPTIU
// =========================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  loadConfig();

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  bool isColdBoot = (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER);

  if (isColdBoot) {
    Serial.println("\n[ARRENCADA EN FRED] Iniciant Portal Captiu...");
    lcd.start();
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Configurem_Sensor_Aula");
    delay(500);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    setupServerRoutes();
    server.begin();

    unsigned long startTime = millis();
    bool userWasConnected = false;

    while (true) {
      dnsServer.processNextRequest();
      server.handleClient();

      if (WiFi.softAPgetStationNum() > 0) {
        startTime = millis();
        if (!userWasConnected) {
          Serial.println("Telèfon connectat al portal. Pausant compte enrere...");
          userWasConnected = true;
        }
      } else {
        if (userWasConnected) {
          Serial.println("Telèfon desconnectat. Reprenent compte enrere 45s...");
          userWasConnected = false;
          startTime = millis();
        }
      }

      // Tanca el portal si passen 20 segons sense ningú connectat
      if (millis() - startTime > 45000) {
        Serial.println("Temps esgotat. Tancant portal i provant de connectar...");
        break;
      }
      delay(10);
    }
    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);

    // Solució a l'error de ràdio intern de l'ESP32
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);

  } else {
    Serial.println("\n[ARRENCADA EN CALENT] Despertant del Deep Sleep...");
  }


  // =========================================================================
  // [BLOC 7] CONNEXIÓ WI-FI DEFINITIVA
  // =========================================================================
  WiFi.mode(WIFI_STA);
  delay(100);

  if (net_type == "ent") {
    Serial.println("Intentant connexió a xarxa WPA2-Enterprise (Escola)...");

    String anonymous_identity = "";
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)anonymous_identity.c_str(), anonymous_identity.length());
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)ent_user.c_str(), ent_user.length());
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)wifi_pass.c_str(), wifi_pass.length());

    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(wifi_ssid.c_str());
  } else {
    Serial.println("Intentant connexió a xarxa WPA2-Personal (Casa/Normal)...");
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  }

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConnexió fallida. Tornant a dormir 60s per reintentar...");
    time_to_sleep = 60; // Forcem un reintent ràpid en 1 minut
    enterDeepSleep();
  }
  Serial.println("\nConnectat correctament!");


  // =========================================================================
  // [BLOC 8] LECTURA DEL SENSOR I COMUNICACIÓ AMB L'API
  // =========================================================================
  dht.begin();
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Error físic del sensor DHT11. Tornant a dormir 60s...");
    time_to_sleep = 60; // Forcem un reintent ràpid en 1 minut
    enterDeepSleep();
  }

  // Mostrem les dades pel LCD. Indicarem l'hora de la lectura per desacoplar la llibreria del tipus
  // de connexió a Internet (WIFI, 4G, etc.) i del tipus de servidor NTP que s'utilitzi. 
  lcd.start();
  lcd.display(t, h, Utils::obtenirHoraActual());

  JsonDocument doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  String requestBody;
  serializeJson(doc, requestBody);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, apiUrl);
  http.setTimeout(30000); // Temps màxim d'espera de 30s pactat amb backend
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  String authHeader = "Bearer " + api_token;
  http.addHeader("Authorization", authHeader);

  // Demanem explícitament al client HTTP que reculli la capçalera dinàmica
  const char *headerKeys[] = { "X-RateLimit-Interval" };
  http.collectHeaders(headerKeys, 1);

  Serial.println("Transmetent dades a l'API...");
  
  int httpResponseCode = http.POST(requestBody);

  // Analitzem la resposta del servidor
  if (httpResponseCode == 200 || httpResponseCode == 201 || httpResponseCode == 202) {
    Serial.print("Dada enregistrada correctament (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");

    if (http.hasHeader("X-RateLimit-Interval")) {
      int new_interval = http.header("X-RateLimit-Interval").toInt();
      if (new_interval > 0) {
        // La placa obeeix cegament el servidor
        time_to_sleep = new_interval;
        Serial.printf(" --> El servidor ha fixat el nou interval a: %d segons\n", time_to_sleep);
      }
    }

  } else if (httpResponseCode == 429) {
    Serial.println("Error 429: Rate Limit excedit. El servidor està saturat.");

    String response = http.getString();
    JsonDocument respDoc;
    if (!deserializeJson(respDoc, response)) {
      if (respDoc.containsKey("retry_after_seconds")) {
        // La placa obeeix cegament el servidor
        time_to_sleep = respDoc["retry_after_seconds"].as<int>();
        Serial.printf(" --> El servidor demana que ens adormim fins d'aquí a %d segons\n", time_to_sleep);
      }
    }

  } else if (httpResponseCode > 0) {
    Serial.print("Error de resposta del servidor (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");
  } else {
    Serial.print("Error de xarxa en l'enviament: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  enterDeepSleep();
}

void loop() {}

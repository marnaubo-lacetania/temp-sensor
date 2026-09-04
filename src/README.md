# aqc-online-reader

## Descripció
Aquest projecte administra un sensor DHT11 connectat a una placa ESP32 amb pantalla LCD I2C i envia les lectures a l'API d'`aulesquecremen.cat`.

## Funcionament
El `main.cpp` fa les següents tasques:

### Inicialització de maquinari
- Importa les llibreries per Wi-Fi, servidor web, DNS, sistema de fitxers LittleFS, JSON, HTTP segur, DHT i pantalla LCD.
- Configura el sensor DHT11 i inicialitza variables globals de configuració.
- Defineix el deep sleep i la durada de l'adormida entre lectures.

### Connexió Wi-Fi definitiva
- Després de la configuració o en reinici des de deep sleep, intenta connectar-se a la xarxa Wi-Fi en mode `WIFI_STA`.
- Suporta dos tipus de xarxa:
  - WPA2-Enterprise per a escoles amb autenticació eduroam / gencat.
  - WPA2-Personal per a xarxes de casa o normals.
- Si no connecta en un nombre de reintents, entra novament en deep sleep per 60 segons.

### Descàrrega de la configuració amb SupaBase
- Es connecta a una BD del servei gratuït SupaBase
- Es descarrega la configuració vinculada al dispositiu identificat segons l'adreça MAC de la placa.

### Lectura de sensors i enviament de dades
- Inicia el sensor DHT11 i llegeix temperatura i humitat.
- Si la lectura falla, s'adorm de nou per 60 segons.
- Sincronitza l'hora amb NTP per mostrar l'hora local al LCD.
- Mostra les dades de temperatura, humitat i hora al panell LCD.
- Envia un objecte JSON amb les lectures a l'endpoint `https://aulesquecremen.cat/api/v1/readings`.
- Envia les dades també a un canal de ThingSpeak segons les dades descarregades amb la configuració.
- Inclou les capçaleres `Content-Type`, `Accept` i `Authorization: Bearer <token>`.

### Gestió de la resposta de l'API
- Si la resposta és 200, 201 o 202, considera l'enviament correcte.
- Si el servidor retorna la capçalera `X-RateLimit-Interval`, ajusta el temps de deep sleep a aquest valor.
- Si la resposta és 429, intenta llegir `retry_after_seconds` del JSON de resposta i s'adorm el temps indicat.
- Si hi ha altres errors HTTP o de xarxa, ho registra i entra en deep sleep.

## Flux de funcionament
`setup()` executa tota la lògica de l'aplicació, al finalitzar el codi de "setup" entra en mode "deep_sleep". Per tant no s'executa res de codi en el loop().
1. Connecta a la Wi-Fi amb les credencials configurades.
2. Descarrega la configuració de SupaBase vinculada a la MAC
3. Llegeix el sensor DHT11, mostra dades al LCD
4. Carrega les dades a l'API de "Aules que cremen".
5. Carrega les dades a ThingSpeak.
5. Entra en `esp_deep_sleep` durant l'interval definit pel servidor o per la configuració per estalviar energia.

## Dependències externes
- **DHT.h**: Lectura del sensor DHT11
- **LiquidCrystal_I2C.h**: Control del panell LCD
- **ArduinoJson.h**: Treball amb dades en format JSON.
- **ThingSpeak.h**: Connexió a la API de ThingSpeak.

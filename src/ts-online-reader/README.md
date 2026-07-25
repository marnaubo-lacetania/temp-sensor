# ts-online-reader

## Descripció
Aquest projecte llegeix dades de temperatura i humitat d'un sensor DHT11, les mostra per un panell LCD I2C i les envia a un canal de ThingSpeak per a monitorització remota.

## Funcionament

El `main.cpp` fa les següents tasques:

### Inicialització de components
- Configura la comunicació amb el sensor DHT11 (temperatura i humitat)
- Inicialitza el panell LCD I2C de 16x2 per mostrar informació
- Estableix la connexió WiFi amb les credencials de configuració

### Connexió WiFi
- Connecta la placa a la xarxa WiFi especificada
- Verifica que hi hagi connexió a Internet (ping a Google)
- Mostra missatges d'estat al panell LCD durant el procés

### Lectura de sensors
- Llegeix els valors de temperatura i humitat del sensor DHT11
- Detecta possibles errors en la lectura de dades
- Mostra els valors al panell LCD

### Enviament de dades
- Puja les dades de temperatura i humitat a ThingSpeak
- Utilitza la API de ThingSpeak per escriure en els camps configurats
- Valida que l'enviament s'hagi completat correctament

## Fluxe de funcionament

1. **Setup**: Inicialitza tots els components, connecta a WiFi i verifica Internet
2. **Loop**: Llegeix sensors cada `LOOP_DURATION` ms i envia les dades a ThingSpeak
3. **Visualització**: Mostra l'estat i les lectures al panell LCD

## Dependències

- **DHT.h**: Lectura del sensor DHT11
- **LiquidCrystal_I2C.h**: Control del panell LCD
- **WiFi.h** i **ThingSpeak.h**: Connectivitat i enviament de dades
- **HTTPClient.h**: Verificació de connexió a Internet

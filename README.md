# temp-sensor
Aplicatiu d'arduino per a monitoritzar temperatura i humitat a través d'una placa IoT (ESP32).

## Entorn de desenvolupament
S'utilitza Visual Studio Code amb PlatformIO.

## Maquinari necessari
- Placa ESP32
- Sensor de temperatura i humitat DHT11
- Display LCD per a mostrar les lectures de temperatures

A continuació teniu una llista de dispositius provats i verificats:

### Keyestudio ESP32 Plus Development Board WOROOM 32

#### Informació general
- Fabricant: Keyestudio
- Codi: KS5016
- URL: https://www.keyestudio.com/products/keyestudio-esp32-plus-development-board-woroom-32-module-wifibluetooth-compatible-with-arduino
- Nom: Keyestudio ESP32 Plus Development Board WOROOM 32

#### Configuració controladors
Podem trobar la documentació oficial a:
https://docs.keyestudio.com/projects/KS5016/en/latest/


##### Configuració amb Arduino IDE
Des d'aquí podem acabar accedint a la URL amb el tutorial per la configuració de la IDE i els controladors per Arduino:
https://docs.keyestudio.com/projects/Arduino/en/latest/Arduino%20IDE%20Tutorial.html

Seguint els passos que hi apareixen es pot procedir sense problema. Cal tenir en compte que la interfície documentada d'Arduino és bastant 
obsoleta respecte les últimes versions i pot costar una mica més aconseguir localitzar alguns dels punts que mostren les captures.
No obstant, amb una mica de sentit comú, es pot acabar localitzant tot.

Els passos principals que caldrà seguir:
- Descàrrega i instal·lació de la última versió de l'IDE d'Arduino
- Descàrrega dels controladors. Per aquesta placa pertoca el CH340 (URL de descàrrega per la versió de Windows: https://docs.keyestudio.com/projects/Arduino/en/latest/windowsCH340.html). 
- Com reconèixer i conectar la placa a l'IDE d'Arduino. Cal seguir les instruccions de la URL que apareix en la Wiki: https://docs.keyestudio.com/projects/Arduino/en/latest/win-ESP32.html

**Nota**: En el cas de Windows 11 és probable que el controlador es detecti automàticament i no faci falta actualitzar-lo / instal·lar-lo a través del ZIP
que ens fa descarregar en el tutorial.

Perquè l'IDE reconegui la placa, i seguint les indicacions de l'anterior URL, caldrà:
- Afegir la URL de plaques addicionals a File > Preferences
- Instal·lar el programari per les plaques ESP32 de Espressif Systems (triga una bona estoneta! Take it easy!)

### Freenove I2C LCD 1602 Module
- Fabricant: Freenove
- Codi: FNK0079A
- URL: https://docs.freenove.com/projects/fnk0079/en/latest/
- URL Compra: https://store.freenove.com/products/fnk0079
- Nom: Freenove I2C LCD 1602 Module

#### Configuració de la connexió
En el nostre cas s'ha procedit a connectar el mòdul amb el Bus I2C de la placa ESP32 a través dels 4 PINS corresponents: GND, V, SDA, SCL. Els pins corresponents es poden llegir tant en la llegenda impresa en el mòdul LCD I2C com en la placa principal ESP32.

#### Parametrització
Caldrà configurar adequadament els paràmetres del port escollit per la comunicació en l'arxiu de configuració "config.h":
- LCD_COLS (tipicament 16)
- LCD_ROWS (tipicament 2)
- LCD_ADDRESS (tipicament 0x27)

### AZ-Delivery KY-015 DHT 11 Module
Aquest mòdul permet la lectura de temperatura i humitats a través dels pins de lectura digital de la placa ESP32.
- Fabricant: AZ-Delivery
- Codi: KY-015
- URL: https://docs.freenove.com/projects/fnk0079/en/latest/
- Nom: KY-015 DHT 11 Module

#### Configuració de la connexió
En el nostre cas s'ha procedit a connectar el mòdul amb els pins de comunicació digital (IO 17 p.ex.). En concret els 3 PINS corresponents: G, V, S. Cal tenir en compte que en el mòdul estan en un ordre diferent i per tant cal seguir correctament la interconnexió dels cables dupont.

#### Parametrització
Caldrà configurar adequadament els paràmetres del port escollit per la comunicació en l'arxiu de configuració "config.h":
- DHTPIN (tipiucament 17)

## Llibreries requerides
Caldrà instal·lar les següents llibreries per Arduino, tot i que PlatformIO ja instal·larà les necessàries per cada entorn, tal i com es pot veure en l'arxiu "platformio.ini":
- DHT Sensor Library (Adafruits): https://github.com/adafruit/DHT-sensor-library (verificat amb la versió v1.4.7). També caldrà instal·lar les llibreries que indica com a requisits.
- LiquidCrystal I2C (de Frank de Brabander o compatible): https://github.com/johnrickman/LiquidCrystal_I2C (verificat amb la versió v1.1.2).
- ThingSpeak (de MathWorks): https://thingspeak.mathworks.com/ (verificat amb la versió 2.1.1)
- Arduino Json (de bblanchon): https://github.com/bblanchon/ArduinoJson (verificat amb la versió 7.2.2).

## Entorns
En la configuració de PlatformIO hi ha definits diferents entorns corresponents a diferents versions d'aplicatius amb diverses funcionalitats i utilitats. A la carpeta de cadascún d'ells hi ha la informació detallada.


# temp-sensor
Aplicatiu d'arduino per a monitoritzar temperatura i humitat a través d'una placa IoT (ESP32).

## Maquinari necessari
- Placa ESP32
- Sensor de temperatura i humitat DHT11
- Display LCD per a mostrar les lectures de temperatures

Per informació sobre models específics utilitzats pels diferents components, consulteu a la carpeta [devices](/devices) amb la llista de components utilitzats i verificats.

## Llibreries requerides
Caldrà instal·lar les següents llibreries des de l'entorn IDE d'Arduino:
- DHT Sensor Library (Adafruits): https://github.com/adafruit/DHT-sensor-library (verificat amb la versió v1.4.7). També caldrà instal·lar les llibreries que indica com a requisits.
- LiquidCrystal I2C (de Frank de Brabander o compatible): https://github.com/johnrickman/LiquidCrystal_I2C (verificat amb la versió v1.1.2).


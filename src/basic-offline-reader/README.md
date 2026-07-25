# basic-offline-reader

## Descripció
Aquest codi llegeix temperatura i humitat d’un sensor DHT11 i mostra els valors en una pantalla LCD.

## Funcionament

El `main.cpp` fa les següents tasques:

### Inicialització de components

- Configura la comunicació amb el sensor DHT11 (temperatura i humitat)
- Inicialitza el panell LCD I2C de 16x2 per mostrar informació

### Lectura de sensors

- Llegeix els valors de temperatura i humitat del sensor DHT11
- Detecta possibles errors en la lectura de dades
- Mostra els valors al panell LCD

## Fluxe de funcionament

1. **Setup**: Inicialitza tots els components
2. **Loop**: Llegeix sensors cada `LOOP_DURATION`
3. **Visualització**: Mostra l'estat i les lectures al panell LCD

## Dependències

- **DHT.h**: Lectura del sensor DHT11
- **LiquidCrystal_I2C.h**: Control del panell LCD
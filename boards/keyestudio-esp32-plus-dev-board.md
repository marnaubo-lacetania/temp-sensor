# Informació general
- Fabricant: Keyestudio
- Codi: KS5016
- URL: https://www.keyestudio.com/products/keyestudio-esp32-plus-development-board-woroom-32-module-wifibluetooth-compatible-with-arduino
- Nom: Keyestudio ESP32 Plus Development Board WOROOM 32

# Configuració controladors
Podem trobar la documentació oficial a:
https://docs.keyestudio.com/projects/KS5016/en/latest/

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
- 

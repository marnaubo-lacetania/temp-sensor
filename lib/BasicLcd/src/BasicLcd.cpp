#include "BasicLcd.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <WiFi.h>
#include <time.h>

String obtenirHoraActual() {
  // Calcul de lhora actual:
  configTime(3600, 3600, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    //Serial.println("Esperant sincronització...");
    delay(1000);
  }

  String horaActual =
    String(timeinfo.tm_hour < 10 ? "0" : "") +
    String(timeinfo.tm_hour) + ":" +
    String(timeinfo.tm_min < 10 ? "0" : "") +
    String(timeinfo.tm_min);

  return horaActual;
  //Serial.println("Hora sincronitzada!");

  /*
    time_t now = time(nullptr);
    tm* local = localtime(&now);

    String hora = String(local->tm_hour);
    String minuts = String(local->tm_min);

    if (hora.length() < 2) hora = "0" + hora;
    if (minuts.length() < 2) minuts = "0" + minuts;

    return hora + ":" + minuts;
  */
}

BasicLcd::BasicLcd(int address, int cols, int rows)
	: _lcd(address, cols, rows) 
{
  _count = 0;
}

void BasicLcd::start() {
  _lcd.init();
  _lcd.backlight();
  _lcd.setCursor(0,0);
  _lcd.print("Iniciant...");
}

void BasicLcd::write(String line1, String line2) {
  _lcd.clear();

  if (line1.length() > 0) {
    _lcd.setCursor(0,0);
    _lcd.print(line1);
  }

  if (line2.length() > 0) {
    _lcd.setCursor(0,1);
    _lcd.print(line2);
  }
}

void BasicLcd::display(float temperature, float humity, String time) {
  _count++;
  if (_count > 999) {
    _count = 0;
  }

  String line1 = "T: " + String(temperature, 2) + (char)223 + "C";
  String line2 = "H: " + String(humity, 0) + "%     " + time;
  write(line1, line2);
}

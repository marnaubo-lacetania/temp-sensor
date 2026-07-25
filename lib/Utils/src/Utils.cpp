#include "Utils.h"
#include <Arduino.h>

Utils::Utils() {}

String Utils::obtenirHoraActual() {
  configTime(3600, 3600, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    //Serial.println("Esperant sincronització...");
    delay(1000);
  }

  String ara =
    String(timeinfo.tm_hour < 10 ? "0" : "") +
    String(timeinfo.tm_hour) + ":" +
    String(timeinfo.tm_min < 10 ? "0" : "") +
    String(timeinfo.tm_min);

  return ara;
}
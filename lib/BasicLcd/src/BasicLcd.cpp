#include "BasicLcd.h"

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

void BasicLcd::display(float temperature, float humity) {
  _count++;
  if (_count > 999) {
    _count = 0;
  }

  String line1 = "T: " + String(temperature, 2) + (char)223 + "C";
  String line2 = "H: " + String(humity, 0) + "%       " + String(_count);
  write(line1, line2);
}

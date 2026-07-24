#include <LiquidCrystal_I2C.h>    // Gestió de la comunicació amb panells LCD I2C (Requereix INSTAL·LACIÓ)

class BasicLcd {
	public:
		BasicLcd(int address, int cols, int rows);
		
		void start();
		void write(String line1, String line2 = "");
		void display(float temperature, float humity, String time);
	
	private:
		LiquidCrystal_I2C _lcd;
		int _count;
};

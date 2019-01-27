#include <Wire.h>  // required for LiquidCrystal_I2C
#include <Encoder.h>  // * Paul Stoffregen https://github.com/PaulStoffregen/Encoder
#include <LiquidCrystal_I2C.h>  // * John Rickman https://github.com/johnrickman/LiquidCrystal_I2C
#include <FastX9CXXX.h> // * GitMoDu https://github.com/GitMoDu/FastX9CXXX
#include <Fast.h> //* GitMoDu https://github.com/GitMoDu/Fast

// pins for rotary encoder
#define rotaryPin1 3 
#define rotaryPin2 2
#define buttonPin 4

//pins for digital pot x9c103 (10k) for LM317T voltage regulator
#define X9_CS_PIN_DC 5
#define X9_UD_PIN_DC 6
#define X9_INC_PIN_DC 7

//pins for digital pot x9c103 (10k) for op amp to control AC amplitude
#define X9_CS_PIN_AC 8
#define X9_UD_PIN_AC 9
#define X9_INC_PIN_AC 10

//pins for LiquidCrystal_I2C display
// SDA -> A4
// SCL -> A5

 // Change these two numbers to the pins connected to your encoder.
 //   Best Performance: both pins have interrupt capability
 //   Good Performance: only the first pin has interrupt capability
 //   Low Performance:  neither pin has interrupt capability
Encoder myEnc(rotaryPin1, rotaryPin2);
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

unsigned long lastButtonPress;
const uint8_t step = 4;
//menu 1:home; 2:DC; 4:AC; 5:meters
uint8_t menuLevel;
int8_t menu1CursorPos; //menu 1:home; 2:DC; 4:AC; 5:meters
int8_t menu3CursorPos;
bool menu3ChangeValue;
String acSignal; //0:OFF 1:ON;
String waveForm; //0:SI 1:SQUA;
float amplitude; 
float DCVoltage;



FastX9C103 PotDCVoltage;
FastX9C103 PotACAmp;

void setup() {
	Serial.begin(9600);
	Serial.println("start:");
	pinMode(buttonPin, INPUT_PULLUP);
	lastButtonPress = millis();
	lcd.setBacklight((uint8_t)1);
	lcd.init();

	menuLevel = 1;

	menu1CursorPos = 0;
	DCVoltage = 5;
	PotDCVoltage.Setup(X9_CS_PIN_DC, X9_UD_PIN_DC, X9_INC_PIN_DC);
	PotDCVoltage.JumpToStep(50);

	menu3CursorPos = 0;
	acSignal = "OFF";
	waveForm = "SINE";
	amplitude = 1.0;
	PotACAmp.Setup(X9_CS_PIN_AC, X9_UD_PIN_AC, X9_INC_PIN_AC);
	PotDCVoltage.JumpToStep(50);

	printMenu();
	myEnc.write(0);
	moveCursor(0);
}


void loop() {
	if ((millis() - lastButtonPress) > 500)
	{
		long newPosition = myEnc.read();
		if (newPosition >= step) {
			myEnc.write(0);
			moveCursor(1);

		}
		else if (newPosition <= -step)
		{
			myEnc.write(0);
			moveCursor(-1);
		}


		if (digitalRead(buttonPin) == LOW)
		{
			lastButtonPress = millis();
			buttonPressed();
		}
	}

	delay(10);

}

void buttonPressed()
{
	switch (menuLevel) {
	case 1:
		switch (menu1CursorPos) {
			case 0:
				menuLevel = 2;
				break;
			case 1:
				menuLevel = 3;
				break;
			case 2:
				menuLevel = 4;
				break;
		}
		printMenu();
		break;
	case 2:
		menuLevel = 1;
		printMenu();
		break;
	case 3:
		switch (menu3CursorPos) {
		case 0:
		case 1:
		case 2:
		case 3:
			lcd.setCursor(0, menu3CursorPos);
			if (menu3ChangeValue == false)
			{
				menu3ChangeValue = true;
				lcd.print("-");
			}
			else
			{
				menu3ChangeValue = false;
				lcd.print("*");
			}
			
			break;
		case 4:
			menuLevel = 1;
			printMenu();
			break;
		}
		break;
	case 4:
		menuLevel = 1;
		printMenu();
		break;
	}
}

void moveCursor(int8_t moveNext)
{
	if (menuLevel == 1)
	{
		lcd.setCursor(0, menu1CursorPos);
		lcd.print(" ");
		menu1CursorPos += moveNext;
		if (menu1CursorPos > 2)
		{
			menu1CursorPos = 0;
		}
		else if (menu1CursorPos < 0)
		{
			menu1CursorPos = 2;
		}
		Serial.print("Cursor: ");
		Serial.println(menu1CursorPos);
		lcd.setCursor(0, menu1CursorPos);
		lcd.print("*");
	}
	else if (menuLevel == 2)
	{
		if (moveNext > 0)
		{
			DCVoltage = DCVoltage +0.1;
		}
		else
		{
			DCVoltage = DCVoltage - 0.1;
		}
		printMenu();
	}
	else if (menuLevel == 3)
	{
		if (menu3ChangeValue == false)
		{
			if (menu3CursorPos <= 3)
			{
				lcd.setCursor(0, menu3CursorPos);
			}
			else
			{
				lcd.setCursor(16, menu3CursorPos - 4);
			}

			lcd.print(" ");
			menu3CursorPos += moveNext;
			if (menu3CursorPos > 4)
			{
				menu3CursorPos = 0;
			}
			else if (menu3CursorPos < 0)
			{
				menu3CursorPos = 4;
			}
			Serial.print("Cursor: ");
			Serial.println(menu3CursorPos);
			if (menu3CursorPos <= 3)
			{
				lcd.setCursor(0, menu3CursorPos);
			}
			else
			{
				lcd.setCursor(16, menu3CursorPos - 4);
			}
			lcd.print("*");
		}
		else
		{
			switch (menu3CursorPos) {
			case 0:
				lcd.setCursor(6, 0);
				if (acSignal == "OFF")
				{
					acSignal = "ON";
					lcd.print("ON ");
				}
				else
				{
					acSignal = "OFF";
					lcd.print("OFF");
				}
				break;
			case 1:
				lcd.setCursor(6, 1);
				if (waveForm == "SINE")
				{
					waveForm = "SQUA";
					lcd.print("SQUA");
				}
				else
				{
					waveForm = "SINE";
					lcd.print("SINE");
				}
				break;
			case 2:
				break;
			case 3:
				break;
			}
		}

	}
}

void printMenu()
{
	lcd.clear();
	switch (menuLevel) {
	case 1:
		lcd.setCursor(1, 0);
		lcd.print("DC: ");
		lcd.setCursor(5, 0);
		lcd.print(DCVoltage);
		lcd.setCursor(9, 0);
		lcd.print("v");

		lcd.setCursor(1, 1);
		lcd.print("AC: ");
		lcd.setCursor(5, 1);
		if (acSignal == "ON")
		{
			lcd.print(waveForm);
		}
		else
		{
			lcd.print("OFF ");
		}

		lcd.setCursor(1, 2);
		lcd.print("Voltage Meters");
		lcd.setCursor(0, menu1CursorPos);
		lcd.print("*");
		break;
	case 2:
		lcd.setCursor(0, 0);
		lcd.print("Set DC Voltage:");
		lcd.setCursor(0, 1);
		lcd.print(DCVoltage);
		break;
	case 3:
		menu3ChangeValue = false;
		menu3CursorPos = 0;

		lcd.setCursor(1, 0);
		lcd.print("AC : " + acSignal);

		lcd.setCursor(1, 1);
		lcd.print("FRM: " + waveForm);

		lcd.setCursor(1, 2);
		lcd.print("FRQ: ");
		lcd.setCursor(1, 3);
		lcd.print("AMP: ");

		lcd.setCursor(17, 0);
		lcd.print("Set");

		
		lcd.setCursor(0, 0);
		lcd.print("*");

		break;
	case 4:
		lcd.setCursor(0, 0);
		lcd.print("Measure Voltages:");
		break;
	}
}
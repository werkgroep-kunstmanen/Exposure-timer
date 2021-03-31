/*
Sketch belichtingsklok
*/

#include <Wire.h>
#include <EEPROM.h>
#include "SSD1306Ascii.h" //https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C // 0X3C+SA0 - 0x3C or 0x3D
#define RST_PIN -1 // Define proper RST_PIN if required

#define ENCA 2 // Rotary encoder clockwise on pin 2
#define ENCB 3 // Rotary encoder counterclockwise on pin 3

#define LED 13
#define LAMP 12 // The UV-lamp
#define BUZ 11  // The buzzer

#define SSA 4  // Define switch: start
#define SSO 5  // Define switch: stop
#define S12 6  // Set time for 120 seconds
#define S30 7  // Set time for 300 seconds

SSD1306AsciiWire oled;

volatile int secondsset_org = 0;
volatile int secondsset = 0;
int secondsset_old = 0;
int currentseconds, currentseconds_old = 0;

unsigned long start_millis, current_millis = 0;

int runyesno0 = 0;
String attdisplay;

void setup()
{
  Serial.begin(9600);

  Wire.begin();
  
  // For the rotary encoder
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  attachInterrupt(0, doEncoder, RISING);

  pinMode(SSA, INPUT_PULLUP);
  pinMode(SSO, INPUT_PULLUP);
  pinMode(S12, INPUT_PULLUP);
  pinMode(S30, INPUT_PULLUP);
  
  pinMode(LAMP, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZ, OUTPUT);
  digitalWrite(LAMP,HIGH);
	  
  secondsset_org = EEPROM.read(0);
  secondsset = secondsset_org * 5;

  // Initialize oled-display  
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(fixed_bold10x15);
  oled.clear();
  oled.println(" UV klok");
}

void doEncoder() {
    if (digitalRead(ENCB) == LOW)
    {
	  secondsset_org--;
    }
    else
	{
      secondsset_org++;
    }

    if (secondsset_org < 1)
    {
      secondsset_org = 0;
    }
    
	secondsset = secondsset_org * 5;
}

void soundBuzzer() {
	digitalWrite(BUZ,HIGH);
	delay(1000);
	digitalWrite(BUZ,LOW);
}

void loop() {
	
  // Check if 120 seconds are set
  if (digitalRead(S12) == LOW)
  {
    secondsset = 120;
  }

  // Check if 300 seconds are set
  if (digitalRead(S30) == LOW)
  {
    secondsset = 300;
  }

  // Check if the start-button is pressed
  if (digitalRead(SSA) == LOW)
  {
    runyesno0 = 1;
    start_millis = millis();
    digitalWrite(LAMP,LOW);
    digitalWrite(LED,HIGH);
	EEPROM.write(0, secondsset_org);
  }
 
  if (runyesno0 == 1)
  {
    // Check the exposure time
	current_millis = millis();
	currentseconds = (current_millis - start_millis) / 1000;

	//Serial.print("current_millis: "); Serial.print(current_millis);

    // Check if the stop-button is pressed or exposure time is reached
    if ((digitalRead(SSO) == LOW) || (currentseconds == secondsset))
    {
	  digitalWrite(LAMP,HIGH);
	  digitalWrite(LED,LOW);
	  runyesno0 = 0;
	  secondsset_old = 0;
	  soundBuzzer();
    }

	if (currentseconds != currentseconds_old)
	{
      oled.setCursor(15,3);
      oled.clearToEOL();
      attdisplay = String(secondsset - currentseconds);
      oled.print(attdisplay);
      oled.print(" sec");
	  
	  currentseconds_old = currentseconds;
	}
  }
    
  // Show value on oled
  if ((secondsset_old != secondsset))
  {
      oled.setCursor(15,3);
      oled.clearToEOL();
      attdisplay = String(secondsset);
      oled.print(attdisplay);
      oled.print(" sec");

	  secondsset_old = secondsset;
  }
}

#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* LCD Object*/
rgb_lcd lcd;

/* Global Variables */
int switchStatus = -1;
int OldSwitchStatus = -1;

/* Custom Character */
byte torch1[8] = {
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000
};

byte torch2[8] = {
  0b11000,
  0b11100,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11100,
  0b11000
};

byte lightRay[8] = {
  B00111,
  B00000,
  B00111,
  B00000,
  B00111,
  B00000,
  B00111,
  B00000
};

void setup()
{
  Serial.begin(9600);
  delay(2000);
  Serial.println("DEBUG INFORMATION");

  uint8_t  error = paj7620Init();      // initialize Paj7620 registers
  if (error)
  {
    Serial.print("Gesture Sensor Init ERROR, Code:");
    Serial.println(error);
  }
  else
  {
    Serial.println("Gesture Sensor Init OK");
  }

  // set up the LCD's number of column and rows
  lcd.begin(16, 2);
  // create custom characters
  lcd.createChar(1, torch1);
  lcd.createChar(2, torch2);
  lcd.createChar(3, lightRay);
}

void loop()
{
  // Print to serial terminal after every 1 minute
  if (millis() % 60 == 0) Serial.println("Waiting for Gesture");

  lcd.setCursor(0, 0);
  lcd.print("LIGHT");
  switchStatus = identifyGesture();
  if (switchStatus != OldSwitchStatus)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LIGHT");
    
    if (switchStatus == 0)
    {
      Serial.println("Read ERROR");
    }
    else if (switchStatus == 1)
    {
      Serial.println("ON");

      lcd.setCursor(0, 1);
      lcd.print("ON");
      lcd.setCursor(12, 1);
      lcd.write(3);
      lcd.write(2);
      lcd.write(1);
      lcd.write(1);
    }
    else if (switchStatus == 2)
    {
      Serial.println("OFF");

      lcd.setCursor(0, 1);
      lcd.print("OFF");
      lcd.setCursor(13, 1);
      lcd.write(2);
      lcd.write(1);
      lcd.write(1);
    }
    OldSwitchStatus = switchStatus;
  }
  delay(1000);
}

int identifyGesture()
{
  static int returnVal = 0;
  uint8_t data = 0;
  
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error) return returnVal;

  if (data == GES_RIGHT_FLAG)
  {
    // Turn ON
    returnVal = 1;
  }
  else if (data == GES_LEFT_FLAG)
  {
    // Turn OFF
    returnVal = 2;
  }
  return returnVal;
}

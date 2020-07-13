#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* LCD Object*/
rgb_lcd lcd;

/* Global Variables */
int switchStatus = -1;
int OldSwitchStatus = -1;

/* Custom Character */
byte up[8] = {
  0b00100,
  0b01010,
  0b10001,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
  0b00000
};

byte down[8] = {
  0b00000,
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b10001,
  0b01010,
  0b00100
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
  lcd.createChar(1, up);
  lcd.createChar(2, down);

  // default animation
  stationaryLcdAnimation();
}

void loop()
{
  // Print to serial terminal after every 1 minute
  if (millis() % 60 == 0) Serial.println("Waiting for Gesture");

  // Default Animation
  stationaryLcdAnimation();
  
  switchStatus = identifyGesture();
  if (switchStatus != OldSwitchStatus)
  {
    if (switchStatus == 0)
    {
      Serial.println("Read ERROR");
    }
    else if (switchStatus == 1)
    {
      // Go UP
      Serial.println("UP");
      upLcdAnimation();
      delay(4000);
    }
    else if (switchStatus == 2)
    {
      // Go DOWN
      Serial.println("DOWN");
      downLcdAnimation();
      delay(4000);
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

  if (data == GES_UP_FLAG) returnVal = 1;
  else if (data == GES_DOWN_FLAG) returnVal = 2;
  
  return returnVal;
}

void stationaryLcdAnimation()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Direction:STOP");
  
  lcd.setCursor(15, 0);
  lcd.write(1);
  lcd.setCursor(15, 1);
  lcd.write(2);
}

void upLcdAnimation()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Direction:UP");
  
  lcd.setCursor(15, 0);
  lcd.write(1);
  lcd.setCursor(15, 1);
  lcd.write(1);  
}

void downLcdAnimation()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Direction:DOWN");
  
  lcd.setCursor(15, 0);
  lcd.write(2);
  lcd.setCursor(15, 1);
  lcd.write(2);
}

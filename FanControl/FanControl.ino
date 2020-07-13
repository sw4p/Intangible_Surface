#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* LCD Object*/
rgb_lcd lcd;

/* Variable */
static int inputStatus = -1;
static int OldInputStatus = -1;
static bool fanStatus = false;
static int fanSpeed = 0;

/* Constants */
static const int minFanSpeed = 0;
static const int maxFanSpeed = 10;

byte fanTopRightWing[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B11111,
  B11111,
  B11011,
  B10000
};

byte fanBottomRightWing[8] = {
  B11110,
  B01111,
  B00111,
  B01111,
  B01110,
  B00000,
  B00000,
  B00000
};

byte fanBottomLeftWing[8] = {
  B00001,
  B11011,
  B11111,
  B11111,
  B01110,
  B00000,
  B00000,
  B00000
};

byte fanTopLeftWing[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B11110,
  B11100,
  B11110,
  B01111
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
  lcd.createChar(1, fanTopRightWing);
  lcd.createChar(2, fanBottomRightWing);
  lcd.createChar(3, fanBottomLeftWing);
  lcd.createChar(4, fanTopLeftWing);

  DisplayToLcd(fanStatus, fanSpeed);
}

void loop()
{
  // Print to serial terminal after every 1 minute
  if (millis() % 60 == 0) Serial.println("Waiting for Gesture");

  inputStatus = IdentifyGesture();
  if (inputStatus != 0)
  {    
    if (inputStatus == 0)
    {
      Serial.println("Read ERROR");
    }
    else if (inputStatus == 1)
    {
      // Turn On the fan
      fanStatus = true;
    }
    else if (inputStatus == 2)
    {
      // Turn Off the fan
      fanStatus = false;
    }
    else if ((inputStatus == 3) && fanStatus)
    {
      // increase the fan speed
      if (fanSpeed < maxFanSpeed) ++fanSpeed;
    }
    else if ((inputStatus == 4) && fanStatus)
    {
      // decrease the fan speed
      if (fanSpeed > minFanSpeed) --fanSpeed;
    }
    //OldInputStatus = inputStatus;
    DisplayToLcd(fanStatus, fanSpeed);
  }

  Animation(fanStatus);
  delay(100);
}

int IdentifyGesture()
{
  int returnVal = 0;
  uint8_t data = 0;
  
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error) return returnVal;

  if (data == GES_RIGHT_FLAG) returnVal = 1;
  else if (data == GES_LEFT_FLAG) returnVal = 2;
  else if (data == GES_CLOCKWISE_FLAG) returnVal = 3;
  else if (data == GES_COUNT_CLOCKWISE_FLAG) returnVal = 4;
  else returnVal = 0;
  
  return returnVal;
}

void DisplayToLcd(bool fanStatus, int speed)
{
  String data;
  fanStatus ? data = "ON" : data = "OFF";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FAN");

  // Print fan status
  Serial.println(data);
  lcd.setCursor(0, 1);
  lcd.print(data);

  // Print fan speed
  lcd.setCursor(12, 1);
  fanStatus ? lcd.print(speed) : lcd.print(0);
}

void Animation(bool fanStatus)
{
  if (fanStatus)
  {
    lcd.setCursor(15, 0);
    lcd.write(1);
    lcd.setCursor(15, 1);
    lcd.write(2);
    lcd.setCursor(14, 1);
    lcd.write(3);
    lcd.setCursor(14, 0);
    lcd.write(4);
  }
}

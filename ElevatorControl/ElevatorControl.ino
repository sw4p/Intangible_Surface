#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* LCD Object*/
rgb_lcd lcd;

/* Constants */
static const int groundFloor = 0;
static const int topFloor = 10;
/* Variables */
static int inputStatus = -1;
static bool doorStatus = false; // true: open, false: close
static int floorCount = 0;
static bool emergencyFlag = false;

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
  //  lcd.createChar(1, fanTopRightWing);

  DisplayToLcd();
}

void loop()
{
  // Print to serial terminal after every 1 minute
  if (millis() % 60 == 0) Serial.println("Waiting for Gesture");

  inputStatus = IdentifyGesture();
  if (inputStatus != -1)
  {
    if (inputStatus == 0)
    {
      Serial.println("Read ERROR");
    }
    else if (inputStatus == 1)
    {
      // Increase the floor count by 5
      if (floorCount < topFloor - 5) floorCount += 5;
      else floorCount = topFloor;
    }
    else if (inputStatus == 2)
    {
      // Decrease the floor count by 5
      if (floorCount > groundFloor + 5) floorCount -= 5;
      else floorCount = groundFloor;
    }
    else if (inputStatus == 3)
    {
      // Increase the floor count by 1
      if (floorCount < topFloor) ++floorCount;
    }
    else if (inputStatus == 4)
    {
      // Decrease the floor count by 1
      if (floorCount > groundFloor) --floorCount;
    }
    else if (inputStatus == 5)
    {
      // Open door
      doorStatus = true;
    }
    else if (inputStatus == 6)
    {
      // Close door
      doorStatus = false;
    }
    else if (inputStatus == 8)
    {
      // Emergency
      emergencyFlag = !emergencyFlag;
    }

    DisplayToLcd();
  }

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
  else if (data == GES_UP_FLAG) returnVal = 5;
  else if (data == GES_DOWN_FLAG) returnVal = 6;
  else if (data == GES_FORWARD_FLAG) returnVal = 7;
  else
  {
    paj7620ReadReg(0x44, 1, &data);
    if (data == GES_WAVE_FLAG) returnVal = 8;
    else returnVal = -1;
  }

  return returnVal;
}

void DisplayToLcd()
{
  lcd.clear();
  if (!emergencyFlag)
  {
    lcd.setCursor(0, 0);
    lcd.print("Floor: ");
    lcd.print(floorCount);

    lcd.setCursor(0, 1);
    lcd.print("Door: ");
    doorStatus ? lcd.print("OPEN") : lcd.print("CLOSE");
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print("EMERGENCY");
    lcd.setCursor(2, 1);
    lcd.print("Calling: 911");
  }
}

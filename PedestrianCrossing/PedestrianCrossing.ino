#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* LCD Object*/
rgb_lcd lcd;

/* Constants */
static const int WAIT_DELAY = 5000;
static const int WALK_TIME = 16;

byte walkPos1[8] = {
  0b01100,
  0b01100,
  0b00000,
  0b01110,
  0b11100,
  0b01100,
  0b11010,
  0b10011,
};

byte walkPos2[8] = {
  0b01100,
  0b01100,
  0b00000,
  0b01100,
  0b01100,
  0b01100,
  0b01100,
  0b01110,
};

byte stopHand[8] = {
  0b00100,
  0b01100,
  0b01110,
  0b11110,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
};

byte dangerSkull[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b10101,
  0b01010,
  0b00100,
  0b01010,
  0b10001,
};

void setup()
{  
  Serial.begin(9600);
  delay(2000);
  Serial.println("DEBUG INFORMATION");

  uint8_t error = 0;
  error = paj7620Init();      // initialize Paj7620 registers
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
  lcd.createChar(1, walkPos1);
  lcd.createChar(2, walkPos2);
  lcd.createChar(3, stopHand);
  lcd.createChar(4, dangerSkull);
}

void loop()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("PEDESTRIANS");
  lcd.setCursor(1, 1);
  lcd.print("Wave and Wait");
  
  if (millis() % 60 == 0) Serial.println("Waiting for Gesture");
  
  if (identifyGesture())
  {
    Serial.println("Gesture Recognised");
    /* Wait for the signal */
    lcd.setCursor(0, 1);
    lcd.print("Wait for Signal");
    delay(WAIT_DELAY);
    /* Received signal to WALK*/
    //lcd.clear();
    //lcd.setCursor(2, 0);
    //lcd.print("PEDESTRIANS");
    //lcd.setCursor(0, 0);
    //lcd.print("WALK");
    for (int i = 0; i <= WALK_TIME; i++)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WALK");
      //lcd.setCursor(8, 0);
      //lcd.print("  ");  ///< Clear the timer location
      lcd.setCursor(14, 0);
      lcd.print(WALK_TIME - i);

      for (int j = i; j < WALK_TIME; j++)
      {
        lcd.setCursor(j, 1);
        lcd.print(".");
      }
      lcd.setCursor(i, 1);
      lcd.write(1);
      delay(500);
      lcd.setCursor(i, 1);
      lcd.write(2);
      delay(500);
    }
  }
  delay(1000);
}

bool identifyGesture()
{
  uint8_t data = 0;
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error) return false;

  if ( (data == GES_RIGHT_FLAG) ||
       (data == GES_LEFT_FLAG) ||
       (data == GES_UP_FLAG) ||
       (data == GES_DOWN_FLAG) ||
       (data == GES_FORWARD_FLAG) ||
       (data == GES_BACKWARD_FLAG) ||
       (data == GES_CLOCKWISE_FLAG) ||
       (data == GES_COUNT_CLOCKWISE_FLAG) )
  {
    return true;
  }

  return false;
}

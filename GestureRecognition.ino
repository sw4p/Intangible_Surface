#include <Wire.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/*
  Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
        You also can adjust the reaction time according to the actual circumstance.
*/
#define GES_REACTION_TIME   500       // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME      800       // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s). 
#define GES_QUIT_TIME     2000

/* LCD Object*/
rgb_lcd lcd;

enum Gestures
{
  unknown = 0,
  left,
  right,
  up,
  down,
  forward,
  backward,
  clockWise,
  anticlockWise,
  wave
};

void setup()
{
  uint8_t error = 0;

  Serial.begin(9600);
  delay(2000);
  Serial.println("\nPAJ7620U2 TEST DEMO: Recognize 9 gestures.");

  error = paj7620Init();      // initialize Paj7620 registers
  if (error)
  {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  }
  else
  {
    Serial.println("INIT OK");
  }
  Serial.println("Please input your gestures:\n");

  // set up the LCD's number of column and rows
  lcd.begin(16, 2);
  lcd.print("Input Gesture");
}

void loop()
{
  Gestures gesture = identifyGesture();
  LcdAnimation(gesture);
  delay(100);
}

Gestures identifyGesture()
{
  uint8_t data = 0;
  uint8_t data1 = 0;
  uint8_t error = 0;
  Gestures returnVal = unknown;

  error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error) return returnVal;

  switch (data)                   // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
  {
    case GES_RIGHT_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Right");
        return right;
      }
      break;
    case GES_LEFT_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Left");
        return left;
      }
      break;
    case GES_UP_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Up");
        return up;
      }
      break;
    case GES_DOWN_FLAG:
      delay(GES_ENTRY_TIME);
      paj7620ReadReg(0x43, 1, &data);
      if (data == GES_FORWARD_FLAG)
      {
        Serial.println("Forward");
        delay(GES_QUIT_TIME);
      }
      else if (data == GES_BACKWARD_FLAG)
      {
        Serial.println("Backward");
        delay(GES_QUIT_TIME);
      }
      else
      {
        Serial.println("Down");
        return down;
      }
      break;
    case GES_FORWARD_FLAG:
      Serial.println("Forward");
      delay(GES_QUIT_TIME);
      return forward;
      break;
    case GES_BACKWARD_FLAG:
      Serial.println("Backward");
      delay(GES_QUIT_TIME);
      return backward;
      break;
    case GES_CLOCKWISE_FLAG:
      Serial.println("Clockwise");
      return clockWise;
      break;
    case GES_COUNT_CLOCKWISE_FLAG:
      Serial.println("anti-clockwise");
      return anticlockWise;
      break;
    default:
      paj7620ReadReg(0x44, 1, &data1);
      if (data1 == GES_WAVE_FLAG)
      {
        Serial.println("wave");
        return wave;
      }
      break;
  }
}

void LcdAnimation(Gestures gesture)
{
  switch (gesture)
  {
    case left:
      Left();
      break;
    case right:
      Right();
      break;
    case up:
      Up();
      break;
    case down:
      Down();
      break;
    case forward:
      Forward();
      break;
    case backward:
      Backward();
      break;
    case clockWise:
      ClockWise();
      break;
    case anticlockWise:
      AntiClockWise();
      break;
    case wave:
      Wave();
      break;
    default:
      break;
  }
}

void Left()
{
  lcd.clear();
  lcd.print("Left");
//  lcd.setCursor(0, 1);
//  scrollInFromRight(1, "<-");
}
void Right()
{
  lcd.clear();
  lcd.print("Right");
//  lcd.setCursor(0, 1);
//  scrollInFromLeft(1, "->");
}
void Up()
{
  lcd.clear();
  lcd.print("Up");
}
void Down()
{
  lcd.clear();
  lcd.print("Down");
}
void Forward()
{
  lcd.clear();
  lcd.print("Forward");
}
void Backward()
{
  lcd.clear();
  lcd.print("Backward");
}
void ClockWise()
{
  lcd.clear();
  lcd.print("Clockwise");
}
void AntiClockWise()
{
  lcd.clear();
  lcd.print("Anti Clockwise");
}
void Wave()
{
  lcd.clear();
  lcd.print("Wave");
}

void scrollInFromRight (int line, char str1[])
{
  for (int j = 16; j >= 0; j--) {
    lcd.setCursor(0, line);
    for (int k = 0; k <= 15; k++) {
      lcd.print(" "); // Clear line
    }
    lcd.setCursor(j, line);
    lcd.print(str1);
    delay(200);
  }
}

void scrollInFromLeft (int line, char str1[])
{
  for (int j = 0; j <= 16; j++) {
    lcd.setCursor(0, line);
    for (int k = 0; k <= 15; k++) {
      lcd.print(" "); // Clear line
    }
    lcd.setCursor(j, line);
    lcd.print(str1);
    delay(400);
  }
}

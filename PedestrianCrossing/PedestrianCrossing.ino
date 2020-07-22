#include <Wire.h>
#include <ArduinoBLE.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* Objects */
rgb_lcd lcd;
// Pedestrian Crossing BLE Service
BLEService pedestrianCrossing("19B10010-E8F2-537E-4F6C-D104768A1214");
// Button characteristic.
BLEBoolCharacteristic button("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

/* Constants */
static const int WAIT_DELAY = 5000;
static const int WALK_TIME = 16;

/* Variables */
bool buttonLevel = false;

/* LCD Custom Characters */
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

/***************************************************************************************/
void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial);
  Serial.println("DEBUG INFORMATION");

  // Initialize gesture sensor
  uint8_t error = 0;
  error = paj7620Init();
  if (error)
  {
    Serial.print("Gesture Sensor Init ERROR, Code:");
    Serial.println(error);
  }
  else
  {
    Serial.println("Gesture Sensor Init OK");
  }

  // Initialize LCD
  lcd.begin(16, 2);
  // create custom characters
  lcd.createChar(1, walkPos1);
  lcd.createChar(2, walkPos2);
  lcd.createChar(3, stopHand);
  lcd.createChar(4, dangerSkull);

  // Initialize BLE
  if (!BLE.begin()) Serial.println("BLE Initialization Failed");
  else
  {
    // Set local name for the device
    BLE.setLocalName("PedestrianCrossing");
    // Set advertisement service
    BLE.setAdvertisedService(pedestrianCrossing);
    // Add characteristic to the pedestrian service
    pedestrianCrossing.addCharacteristic(button);
    // Add service to the Device's service list
    BLE.addService(pedestrianCrossing);
    // Set initial value to the button
    button.writeValue(buttonLevel);

    // Start advertising
    BLE.advertise();
    Serial.println("BLE active, waiting for connection...");
  }
}

/***************************************************************************************/
void loop()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("PEDESTRIANS");
  lcd.setCursor(1, 1);
  lcd.print("Wave and Wait");

  if (identifyGesture() || BLEButtonActive())
  {
    Serial.println("Button Active");
    /* Wait for the signal */
    lcd.setCursor(0, 1);
    lcd.print("Wait for Signal");
    delay(WAIT_DELAY);
    /* Received signal to WALK*/
    for (int i = 0; i <= WALK_TIME; i++)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WALK");
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

/***************************************************************************************/
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

/***************************************************************************************/
bool BLEButtonActive()
{
  // Set initial value to button
  button.writeValue(false);
  // Poll for BLE events
  BLE.poll();
  // Read button value
  buttonLevel = button.value();

  // If button is set as TRUE via BLE return true
  if (button.written() && buttonLevel) return true;

  return false;
}

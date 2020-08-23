/*
  Elevator Control

  This code shows how to use gesture sensor and MQTT protocol to interact with the
  elevator contorl panel. You can select floor, open/close door and even call emergency.

  The circuit:
  Please refer https://www.hackster.io/usavswapnil/intangible-surface-dd262b
  for circuit diagram.
  
  By Swapnil Verma
  22/08/2020
*/

#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* Object */
rgb_lcd lcd;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

/* Constants */
static const int groundFloor = 0;
static const int topFloor = 10;
static const char MQTT_BROKER[] = ""; // Your MQTT broker's address
static const int MQTT_PORT = 1883;
static const char MQTT_TOPIC[] = "";  // Topic you want to subscribe/publish
static const char WIFI_SSID[] = "";   // Your WiFi address
static const char WIFI_PASSWORD[] = ""; // Your WiFi password
static const char MQTT_USERNAME[] = ""; // MQTT broker's username if it has one
static const char MQTT_PASSWORD[] = ""; // MQTT broker's password if it has one

/* Variables */
static int inputStatus = -1;


struct elevatorParam
{
  bool doorStatus = false;
  int floorCount = 0;
  bool emergencyFlag = false;
} elevatorParam;

/***************************************************************************************
 * @brief Setup function will be called only once. Use this function to set-up your
 *        device.
 *        
 * @param void
 * @return void
 **************************************************************************************/
void setup()
{
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial);
  Serial.println("DEBUG INFORMATION");

  // Initialize Gesture Sensor
  uint8_t  error = paj7620Init();
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
  DisplayToLcd();

  // Attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED)
  {
    // Failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected to the Wi-Fi network");

  // Attempt to connect to MQTT broker
  Serial.print("Attempting to connect to the MQTT broker: ");
  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  if (!mqttClient.connect(MQTT_BROKER, MQTT_PORT))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");

  // Set the message receive callback
  mqttClient.onMessage(ReceiveMessage);
  
  // Subscribe to a topic
  Serial.print("Subscribing to topic: ");
  Serial.println(MQTT_TOPIC);
  mqttClient.subscribe(MQTT_TOPIC);
}

/***************************************************************************************
 * @brief This function will be called again and again and it will never exit.
 *        It is essentially a while(1) loop. Place your state machine here.
 *        
 * @param void
 * @return void
 **************************************************************************************/
void loop()
{
  // Print to serial terminal after 1 minute
  if (millis() % 60 == 0) Serial.print(".");

  mqttClient.poll();
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
      if (elevatorParam.floorCount < topFloor - 5) elevatorParam.floorCount += 5;
      else elevatorParam.floorCount = topFloor;
    }
    else if (inputStatus == 2)
    {
      // Decrease the floor count by 5
      if (elevatorParam.floorCount > groundFloor + 5) elevatorParam.floorCount -= 5;
      else elevatorParam.floorCount = groundFloor;
    }
    else if (inputStatus == 3)
    {
      // Increase the floor count by 1
      if (elevatorParam.floorCount < topFloor) ++elevatorParam.floorCount;
    }
    else if (inputStatus == 4)
    {
      // Decrease the floor count by 1
      if (elevatorParam.floorCount > groundFloor) --elevatorParam.floorCount;
    }
    else if (inputStatus == 5)
    {
      // Open door
      elevatorParam.doorStatus = true;
    }
    else if (inputStatus == 6)
    {
      // Close door
      elevatorParam.doorStatus = false;
    }
    else if (inputStatus == 8)
    {
      // Emergency
      elevatorParam.emergencyFlag = !elevatorParam.emergencyFlag;
    }
  }

  DisplayToLcd();
  delay(100);
}

/***************************************************************************************
 * @brief This function is responsible for identifying the gestures.
 *        
 * @param void
 * @return int, based on the detected gesture.
 **************************************************************************************/
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
  if (!elevatorParam.emergencyFlag)
  {
    lcd.setCursor(0, 0);
    lcd.print("Floor: ");
    lcd.print(elevatorParam.floorCount);

    lcd.setCursor(0, 1);
    lcd.print("Door: ");
    elevatorParam.doorStatus ? lcd.print("OPEN") : lcd.print("CLOSE");
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print("EMERGENCY");
    lcd.setCursor(2, 1);
    lcd.print("Calling: 911");
  }
}

/***************************************************************************************
 * @brief It is a callback function if any message is received via MQTT broker.
 *        
 * @param messageSize[in] Size of the message
 * @return void
 **************************************************************************************/
void ReceiveMessage(int messageSize)
{
   // mqttClient.read() pops message out of queue.
  char incomingData = mqttClient.read();
  Serial.println(incomingData);
  if ((incomingData >= '0') && (incomingData <= '9'))
  {
    elevatorParam.floorCount = (int)incomingData - 48;
    Serial.println(elevatorParam.floorCount);
  }
  else if (incomingData == 'O') elevatorParam.doorStatus = true;
  else if (incomingData == 'C') elevatorParam.doorStatus = false;
  else if (incomingData == 'E') elevatorParam.emergencyFlag = !elevatorParam.emergencyFlag;
}

/***************************************************************************************
 * @brief This function is responsible for publishing message to a topic.
 *        
 * @param message[in] message in boolean datatype
 * @return void
 **************************************************************************************/
void SendMessage(bool message)
{
  mqttClient.beginMessage(MQTT_TOPIC);
  mqttClient.print(message);
  mqttClient.endMessage();
}

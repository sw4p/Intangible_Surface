/*
  Pedestrian Crossing

  This code shows how to use gesture sensor and MQTT protocol to make
  pedestrian crossing button touch-less.

  The circuit:
  Please refer https://www.hackster.io/usavswapnil/intangible-surface-dd262b
  for circuit diagram.
  
  By Swapnil Verma
  10/08/2020
*/

#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "paj7620.h"
#include "rgb_lcd.h"

/* Objects */
rgb_lcd lcd;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

/* Constants */
static const int WAIT_DELAY = 5000;
static const int WALK_TIME = 16;
static const char MQTT_BROKER[] = ""; // Your MQTT broker's address
static const int MQTT_PORT = 1883;
static const char MQTT_TOPIC[] = "";  // Topic you want to subscribe/publish
static const char WIFI_SSID[] = "";   // Your WiFi address
static const char WIFI_PASSWORD[] = ""; // Your WiFi password
static const char MQTT_USERNAME[] = ""; // MQTT broker's username if it has one
static const char MQTT_PASSWORD[] = ""; // MQTT broker's password if it has one

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

/***************************************************************************************
 * @brief Setup function will be called only once. Use this function to set-up your
 *        device.
 *        
 * @param void
 * @return void
 **************************************************************************************/
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

  // Attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(WIFI_SSID);
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED)
  {
    // Failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected to the network");

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
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("PEDESTRIANS");
  lcd.setCursor(1, 1);
  lcd.print("Wave and Wait");

  mqttClient.poll();
  
  if (IdentifyGesture() || buttonLevel)
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
    buttonLevel = false;
    SendMessage(false);
  }
  delay(1000);
}

/***************************************************************************************
 * @brief This function is responsible for identifying the gestures.
 *        
 * @param void
 * @return true if any gesture is identified else false.
 **************************************************************************************/
bool IdentifyGesture()
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
    SendMessage(true);
    return true;
  }

  return false;
}

/***************************************************************************************
 * @brief It is a callback function if any message is received via MQTT broker.
 *        
 * @param messageSize[in] Size of the message
 * @return void
 **************************************************************************************/
void ReceiveMessage(int messageSize)
{
  if ((char)mqttClient.read() == '1') buttonLevel = true;
  else buttonLevel = false;
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

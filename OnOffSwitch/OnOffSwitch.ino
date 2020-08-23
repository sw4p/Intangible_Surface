/*
  On Off Switch

  This code shows how to use gesture sensor and MQTT protocol to turn a switch
  on and off without physically touching it.

  The circuit:
  Please refer https://www.hackster.io/usavswapnil/intangible-surface-dd262b
  for circuit diagram.
  
  By Swapnil Verma
  17/08/2020
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
static const char MQTT_BROKER[] = ""; // Your MQTT broker's address
static const int MQTT_PORT = 1883;
static const char MQTT_TOPIC[] = "";  // Topic you want to subscribe/publish
static const char WIFI_SSID[] = "";   // Your WiFi address
static const char WIFI_PASSWORD[] = ""; // Your WiFi password
static const char MQTT_USERNAME[] = ""; // MQTT broker's username if it has one
static const char MQTT_PASSWORD[] = ""; // MQTT broker's password if it has one

/* Global Variables */
bool buttonLevel = false;
bool OldButtonLevel = false;

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

/***************************************************************************************
 * @brief Setup function will be called only once. Use this function to set-up your
 *        device.
 *        
 * @param void
 * @return void
 **************************************************************************************/
void setup()
{
  Serial.begin(9600);
  while (!Serial);
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

  lcd.setCursor(0, 0);
  lcd.print("LIGHT");

  mqttClient.poll();  // Poll mqtt client
  identifyGesture();  // Poll Gesture Sensor
  if (buttonLevel != OldButtonLevel)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LIGHT");
    
    if (buttonLevel == false)
    {
      Serial.println("OFF");

      lcd.setCursor(0, 1);
      lcd.print("OFF");
      lcd.setCursor(13, 1);
      lcd.write(2);
      lcd.write(1);
      lcd.write(1);
    }
    else if (buttonLevel == true)
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
    SendMessage(buttonLevel); // Update other nodes subscribed to the same topic
    OldButtonLevel = buttonLevel;
  }
  delay(1000);
}

/***************************************************************************************
 * @brief This function is responsible for identifying the gestures.
 *        
 * @param void
 * @return 1 for Right swipe
 *         0 for Left swipe
 *        -1 for error
 **************************************************************************************/
void identifyGesture()
{
  uint8_t data = 0;
  
  uint8_t error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
  if (error)
  {
    Serial.println("Func: identifyGesture, Error: Sensor read error");
    return;
  }

  if (data == GES_RIGHT_FLAG) buttonLevel = true;
  else if (data == GES_LEFT_FLAG) buttonLevel = false;
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
  char incomingButtonLevel = mqttClient.read();
  if (incomingButtonLevel == '1') buttonLevel = true;
  else if (incomingButtonLevel == '0') buttonLevel = false;
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

#include <HixPinDigitalOutput.h>
#include <HixDS18B20.h>
#include "EspMQTTClient.h"
#include "DS18B20Temperature.h"
#include "secret.h"

//globals
HixPinDigitalOutput  g_beeper(2);
HixPinDigitalOutput  g_relay(14);
HixPinDigitalOutput  g_led(5);
DS18B20Temperature   g_temperature(12);
EspMQTTClient        g_client(
  WIFI_SSID,
  WIFI_PWD,
  MQTT_SERVER,
  MQTT_NAME
);
bool g_bBeeping      = false;
float g_fTemperature = 0.0;

void setup()
{
  //we will be logging at 115200 bps
  Serial.begin(115200);
  Serial.println(F("Starting..."));
  //init pins
  Serial.println(F("Setting up Beeper"));
  g_beeper.begin();
  g_beeper.blink(1, 5, 100);
  Serial.println(F("Setting up Relay"));
  g_relay.begin();
  g_relay.blink(1, 5, 100);
  Serial.println(F("Setting up LED"));
  g_led.begin();
  g_led.blink(1, 5, 100);
  //setup temp sensor
  Serial.println(F("Setting up temperature sensor"));
  g_temperature.begin();
  //configure MQTT
  Serial.println(F("Setting up MQTT"));
  // Optionnal functionnalities of EspMQTTClient :
  g_client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //all done!
  Serial.println(F("All done!"));
}

void loop() {
  //store sensor values
  g_fTemperature = g_temperature.getTemp();
  Serial.println(g_fTemperature);
   g_client.publish("leddisplay/status/temperature", String(g_fTemperature));
  //MQTT client
  g_client.loop();
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{

  //register for beeper
  g_client.subscribe("leddisplay/control/beeper", [](const String & payload) {
    g_bBeeping = toBool(payload);
    g_beeper.digitalWrite(g_bBeeping);
    char szBuf[255];
    sprintf(szBuf, "I set the beeper to %d", g_bBeeping);
    g_client.publish("leddisplay/status/debug", szBuf);
    g_client.publish("leddisplay/status/beeper", toBoolString(g_bBeeping));
  });

  /*  // Publish a message to "mytopic/test"
    client.publish("test/message", "This is a message"); // You can activate the retain flag by setting the third parameter to true

    // Execute delayed instructions
    client.executeDelayed(5 * 1000, []() {
      client.publish("test/message", "This is a message sent 5 seconds later");
    });*/
}

bool toBool(String string) {
  char firstChar = string.charAt(0);
  return firstChar == '1' || firstChar == 't' || firstChar == 'T';
}

String toBoolString(bool bValue) {
  return bValue ? String("true") : String("false");
}

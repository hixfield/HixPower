// This example shows how to connect to Cayenne using an ESP8266 and send/receive sample data.
// Make sure you install the ESP8266 Board Package via the Arduino IDE Board Manager and select the correct ESP8266 board before compiling.

#include <ArduinoOTA.h>
#include <WiFiManager.h>         //refer to : https://github.com/tzapu/WiFiManager
#include "RemoteDebug.h"         //refer to : https://github.com/JoaoLopesF/RemoteDebug
#include "IBMMQTTClient.h"
#include "DS18B20Temperature.h"
#include "HixTimeout.h"
#include "HixPinOutput.h"
#include "HixPinInput.h"
#include "HixGeneral.h"
#include "Status.h"
#include "MySecret.h"

//pins
HixPinOutput         g_Beeper(2);
HixPinOutput         g_LED(5);
HixPinOutput         g_Relay(14);
HixPinInput          g_PIR(13);
HixPinInput          g_Button(4);
DS18B20Temperature   g_Temperature(12);

//other
Status               g_Status;
const char *         g_szHostName = "ESP8266Power";
HixTimeout           m_mqttPublishTimeout(5000, true);
RemoteDebug          g_Debug;

//forward declarations
void mqttCallback(char * topic, unsigned char *payload, unsigned int payloadLength);


WiFiClientSecure g_WIFIClient;

IBMMQTTClient mqttClient(Secret::s_IBM_IoT_Organisation, "ESP8266PowerV3", Secret::s_IBM_IoT_Token, g_WIFIClient, g_Debug, mqttCallback);

void setup() {
  //we will be logging at 115200 bps
  Serial.begin(115200);
  //set pin modes
  g_Beeper.begin();
  g_LED.begin();
  g_Relay.begin();
  g_PIR.begin();
  g_Button.begin();
  //hook up the interrupts
  g_Button.attachInterrupt(isrButton, FALLING);
  g_PIR.attachInterrupt(isrPIR, RISING);
  //setup our wifi
  WiFiManager wifiManager;
  // Set a timeout until configuration is turned off, useful to retry or go to  sleep in n-seconds
  wifiManager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "ESP8266_AP" and waits in a blocking loop for configuration
  if (!wifiManager.autoConnect(g_szHostName)) {
    g_Debug.println("failed to connect and timeout occurred");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  //start and configure sub modules
  g_Temperature.begin();
  g_Debug.begin(g_szHostName);
  g_Debug.setResetCmdEnabled(true);
  g_Debug.setHelpProjectsCmds("any command - simulate switch press");
  g_Debug.setCallBackProjectCmds(&processCmdRemoteDebug);
  //setup OTA
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(g_szHostName);
  //previous pass_$*/@#
  ArduinoOTA.setPassword((const char *)"");
  ArduinoOTA.onStart([]() {
    g_Debug.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    g_Debug.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    g_Debug.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    g_Debug.printf("Error[%u]: ", error);
    if      (error == OTA_AUTH_ERROR)    g_Debug.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)   g_Debug.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) g_Debug.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) g_Debug.println("Receive Failed");
    else if (error == OTA_END_ERROR)     g_Debug.println("End Failed");
  });
  ArduinoOTA.begin();
  //when started and the button is pressed then we do a selftest
  if (!g_Button.digitalRead()) {
    selfTest();
  }
  //signal start
  g_Beeper.blink(true, 4, 250);
  g_Debug.handle();
}

void loop() {
  //store sensor values (round temp to 0.25)
  g_Status.setCurrentTemperature(round(g_Temperature.getTemp() * 2.0) / 2.0);
  g_Status.setHeatingOn(g_Relay.digitalRead());
  //sync status with device
  g_Relay.digitalWrite(g_Status.calculateIfHeatingShouldBeOn());
  g_LED.digitalWrite(g_Status.heatingEnabled());
  //print debugging
  g_Status.printCSV(g_Debug);
  //MQTT stuff
  if (mqttClient.loop()) {
    String payload = g_Status.mqttPayload(m_mqttPublishTimeout.expired(true));
    if (payload != "") {
      m_mqttPublishTimeout.restart();
      mqttClient.publishStatus("status", payload);
    }
  }
  //loop counter
  g_Debug.println("-");
  //different handle functions...
  ArduinoOTA.handle();
  g_Debug.handle();
}

void selfTest() {
  g_Debug.println("toggeling relay...");
  g_Relay.blink(true, 6, 250);
  g_Debug.println("toggeling beeper...");
  g_Beeper.blink(true, 6, 250);
  g_Debug.println("toggeling led...");
  g_LED.blink(true, 6, 250);
  g_Debug.println("print current temperature...");
  for (int i = 0; i < 10; i++) {
    g_Debug.println(g_Temperature.getTemp());
  }
}

void processCmdRemoteDebug() {
  String lastCmd = g_Debug.getLastCommand();
  //g_Debug.printf("****%s****", lastCmd.c_str());
  //g_Debug.println("pressed switch");
  isrButton();
}

void isrButton(void) {
  g_Status.pressedButton();
  g_Relay.digitalWrite(g_Status.calculateIfHeatingShouldBeOn());
  g_LED.digitalWrite(g_Status.heatingEnabled());
}

void isrPIR(void) {

}

void mqttCallback(char * topic, unsigned char *payload, unsigned int payloadLength) {
  String receivedTopic = String(topic);
  g_Debug.print("Message arrived [");
  g_Debug.print(receivedTopic);
  g_Debug.println("] ");
  String receivedPayload;
  for (int i = 0; i < payloadLength; i++) {
    char sz[] = "?";
    sz[0] = payload[i];
    receivedPayload += sz;
  }
  g_Debug.println(receivedPayload);

  if ( mqttClient.topicIsCommand(topic, "pressButton")) {
    isrButton();
  }

  if (mqttClient.topicIsReboot(topic) || mqttClient.topicIsFactoryReset(topic)) {
    mqttClient.disconnect();
    g_Debug.stop();
    ESP.restart();
  }
}



// This example shows how to connect to Cayenne using an ESP8266 and send/receive sample data.
// Make sure you install the ESP8266 Board Package via the Arduino IDE Board Manager and select the correct ESP8266 board before compiling.

#include <WiFiManager.h>       //refer to : https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include "RemoteDebug.h"       //refer to : https://github.com/JoaoLopesF/RemoteDebug
#include "DS18B20Temperature.h"
#include "HixTimeout.h"
#include "HixPinOutput.h"
#include "HixPinInput.h"
#include "HixGeneral.h"

//global vars


//pins
HixPinOutput         g_Beeper(2);
HixPinOutput         g_LED(5);
HixPinOutput         g_Relay(14);
HixPinInput          g_PIR(13);
HixPinInput          g_Switch(4);

//other
const char *         g_szHostName          = "ESP8266Power";
const float          g_tempHisteresis      = 1.0;
const float          g_tempDesired         = 23.0;
const float          g_tempHigh            = g_tempDesired + ( g_tempHisteresis / 2 );
const float          g_tempLow             = g_tempDesired - ( g_tempHisteresis / 2 );
const unsigned long  g_relayDurationSwitch = 60000UL * 25UL;
const unsigned long  g_relayDurationPIR    = 60000UL * 5UL;
const unsigned long  g_durationIgnorePIR   = 10000UL;


float              g_tempCurrent    = 0.0;
DS18B20Temperature g_temp(12);
RemoteDebug        g_Debug;
HixTimeout         g_relayTimeout(g_relayDurationPIR, true);
HixTimeout         g_buttonTimeout(1000, true);
HixTimeout         g_ignorePIRTimeout(g_durationIgnorePIR);
bool               g_movementDetected = false;


void setup() {
  //we will be logging at 115200 bps
  Serial.begin(115200);
  //set pin modes
  g_Beeper.begin();
  g_LED.begin();
  g_Relay.begin();
  g_PIR.begin();
  g_Switch.begin();
  //hook up the interrupts
  g_Switch.attachInterrupt(isrSwitch, FALLING);
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
  g_temp.begin();
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
  if (!g_Switch.digitalRead()) {
    selfTest();
  }
  //signal start
  g_Beeper.blink(true, 2, 250);
}



void loop() {
  //store sensor values
  g_tempCurrent = g_temp.getTemp();
  //calculations
  g_Relay.digitalWrite(heatingAllowed());
  g_LED.digitalWrite(g_relayTimeout.running());
  //print debugging
  g_Debug.print(msToHHMMSS(millis()));
  g_Debug.print("     ");
  g_Debug.print(g_tempDesired);
  g_Debug.print("C     ");
  g_Debug.print(g_tempCurrent);
  g_Debug.print("C     ");
  g_Debug.print(g_relayTimeout.running());
  g_Debug.print("     ");
  g_Debug.print(g_Relay.digitalRead());
  g_Debug.print("     ");
  g_Debug.print(g_relayTimeout.timeLeftMs());
  g_Debug.print("     ");
  g_Debug.print(g_PIR.digitalRead());
  g_Debug.print("     ");
  g_Debug.println(WiFi.macAddress());
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
    g_Debug.println(g_temp.getTemp());
  }
}

//because this is also called in an ISR, we cant use the getTemp() function directly as this requires interrupts!
//therefore we are using the g_tempCurrent stored in the loop
bool heatingAllowed() {
  //if not running (e.g. pressed switch) no heating allowed
  if (g_relayTimeout.expired() ) {
    return false;
  }
  //ok, heating is allowed...
  //if we are currently on, keep heating until high temp reached
  if (g_Relay.digitalRead()) {
    return g_tempCurrent < g_tempHigh;
  }
  //if currently off switch on if low temp reached
  return  g_tempCurrent < g_tempLow;
}

void processCmdRemoteDebug() {
  String lastCmd = g_Debug.getLastCommand();
  //g_Debug.printf("****%s****", lastCmd.c_str());
  //g_Debug.println("pressed switch");
  isrSwitch();
}

void isrSwitch(void) {
  //debouncing
  if (g_buttonTimeout.running() ) {
    return;
  }
  g_buttonTimeout.restart();
  //process command
  if (g_relayTimeout.running() ) {
    g_relayTimeout.invalidate();
  }
  else {
    g_relayTimeout.updateTimeoutAndRestart(g_relayDurationSwitch);
  }
  g_Relay.digitalWrite(heatingAllowed());
  g_LED.digitalWrite(g_relayTimeout.running());
}

void isrPIR(void) {

}


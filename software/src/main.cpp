#include "DS18B20Temperature.h"
#include "HixConfig.h"
#include "HixMQTT.h"
#include "HixWebServer.h"
#include "secret.h"
#include <ArduinoOTA.h>
#include <FS.h>
#include <HixLED.h>
#include <HixPinDigitalInput.h>
#include <HixPinDigitalOutput.h>
#include <HixString.h>
#include <HixTimeout.h>

HixPinDigitalOutput g_relay(14);


//////////////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////////////

void setup() {
    //print startup config
    Serial.begin(115200);
    g_relay.begin();
}

//////////////////////////////////////////////////////////////////////////////////
// Loop
//////////////////////////////////////////////////////////////////////////////////
int counter = 0;
void loop() {
    Serial.print(F("Loop "));
    Serial.println(counter);
    counter++;
    delay(250);
    g_relay.toggle();
}

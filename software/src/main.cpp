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

// runtime global variables
HixConfig    g_config;
HixWebServer g_webServer(g_config);
HixMQTT      g_mqtt(g_config,
               Secret::WIFI_SSID,
               Secret::WIFI_PWD,
               g_config.getMQTTServer(),
               g_config.getDeviceType(),
               g_config.getDeviceVersion(),
               g_config.getRoom(),
               g_config.getDeviceTag());
//hardware related
HixPinDigitalOutput g_beeper(2);
HixPinDigitalOutput g_relay(14);
HixLED              g_led(5);
HixPinDigitalInput  g_motion(13);
HixPinDigitalInput  g_switch(4);
DS18B20Temperature  g_temperature(12);
//software related
HixTimeout    g_sampler(1000, true);
HixTimeout    g_logger(5000, true);
HixTimeout    g_buttonTimeout(500, true);
HixTimeout    g_motionTimeout(5000, true);
HixTimeout    g_relayTimeout(g_config.getAutoSwitchOffSeconds() * 1000, true);
volatile bool g_bLedBlinking = false;
//sensor values
float         g_fTemperature    = 0;
volatile bool g_bDetectedMotion = false;
volatile bool g_bPressedSwitch  = false;

//////////////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////////////

void configureOTA() {
    Serial.println("Configuring OTA, my hostname:");
    Serial.println(g_mqtt.getMqttClientName());
    ArduinoOTA.setHostname(g_mqtt.getMqttClientName());
    ArduinoOTA.setPort(8266);
    //setup handlers
    ArduinoOTA.onStart([]() {
        Serial.println("OTA -> Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("OTA -> End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA -> Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA -> Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println("OTA -> Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("OTA -> Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("OTA -> Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("OTA -> Receive Failed");
        else if (error == OTA_END_ERROR)
            Serial.println("OTA -> End Failed");
    });
    ArduinoOTA.begin();
}

void resetWithMessage(const char * szMessage) {
    Serial.println(szMessage);
    delay(2000);
    ESP.reset();
}

void selfTest(void) {
    g_relay.blink(true, 5, 100);
    g_led.blink(true, 5, 100);
    g_beeper.blink(true, 5, 100);
}

bool heatingAllowed() {
    //if not running no heating allowed
    if (g_relayTimeout.isExpired()) {
        return false;
    }
    //ok, heating is allowed...
    //if we are currently on, keep heating until high temp reached
    if (g_relay.isHigh()) {
        return g_fTemperature < g_config.getSwitchOffTemperature();
    }
    //if currently off switch on if low temp reached
    return g_fTemperature < g_config.getSwitchOnTemperature();
}

bool handlePresseeSwitch(void) {
    if (!g_bPressedSwitch) return false;
    g_bPressedSwitch = false;
    //debouncing
    if (g_buttonTimeout.isRunning()) {
        return false;
    }
    g_buttonTimeout.restart();
    //process command
    if (g_relayTimeout.isRunning()) {
        g_relayTimeout.invalidate();
    } else {
        g_relayTimeout.restart();
    }
    //handled something!
    return true;
}

bool handleDetectedMotion(void) {
    if (!g_bDetectedMotion) return false;
    g_bDetectedMotion = false;
    //debouncing
    if (g_motionTimeout.isRunning()) {
        return false;
    }
    g_motionTimeout.restart();
    //handled something!
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// ISR
//////////////////////////////////////////////////////////////////////////////////

ICACHE_RAM_ATTR void pressedSwitch(void) {
    g_bPressedSwitch = true;
}

ICACHE_RAM_ATTR void detectedMotion(void) {
    g_bDetectedMotion = true;
}

//////////////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////////////

void setup() {
    //print startup config
    Serial.begin(115200);
    Serial.print(F("Startup "));
    Serial.print(g_config.getDeviceType());
    Serial.print(F(" "));
    Serial.println(g_config.getDeviceVersion());
    //disconnect WiFi -> seams to help for bug that after upload wifi does not want to connect again...
    Serial.println(F("Disconnecting WIFI"));
    WiFi.disconnect();
    //init pins
    Serial.println(F("Setting up Beeper"));
    g_beeper.begin();
    Serial.println(F("Setting up Relay"));
    g_relay.begin();
    Serial.println(F("Setting up LED"));
    g_led.begin();
    //setup temp sensor
    Serial.println(F("Setting up temperature sensor"));
    if (!g_temperature.begin()) resetWithMessage("DS18B20 init failed, resetting");
    // configure MQTT
    Serial.println(F("Setting up MQTT"));
    if (!g_mqtt.begin()) resetWithMessage("MQTT allocation failed, resetting");
    //setup SPIFFS
    Serial.println(F("Setting up SPIFFS"));
    if (!SPIFFS.begin()) resetWithMessage("SPIFFS initialization failed, resetting");
    //setup the server
    Serial.println(F("Setting up web server"));
    g_webServer.begin();
    //hookup switch
    Serial.println(F("Hooking up the switch"));
    g_switch.attachInterrupt(pressedSwitch, FALLING);
    //hookup motion detector
    Serial.println(F("Hooking up the motion detector"));
    g_motion.attachInterrupt(detectedMotion, RISING);
    // all done
    Serial.println(F("Setup complete"));
}

//////////////////////////////////////////////////////////////////////////////////
// Loop
//////////////////////////////////////////////////////////////////////////////////

void loop() {
    //other loop functions
    g_mqtt.loop();
    g_webServer.handleClient();
    ArduinoOTA.handle();
    //isr handles & check if should do force publishing
    bool bHandledMotion = handleDetectedMotion();
    bool bHandledSwitch = handlePresseeSwitch();
    if (bHandledSwitch) g_beeper.blink(true, 1, 150);
    bool bForcePublishing = bHandledMotion || bHandledSwitch;
    //now we calculated if we should forcepublishing we also set motion to true if its still high...
    bHandledMotion = bHandledMotion || g_motion.isHigh();
    //beep while motion detected
    //g_beeper.digitalWrite(bHandledMotion);
    //update relay
    g_relay.digitalWrite(heatingAllowed());
    //blink led if not connected
    if (g_mqtt.isConnected()) {
        //if allowed to heat
        if (g_relayTimeout.isRunning()) {
            //and we are actually heating
            if (g_relay.isHigh()) {
                g_led.on();
            }
            //no not heating must be to warm...
            else {
                g_led.fadeInOut();
            }
        }
        //not allow to heat switch off led
        else {
            g_led.off();
        }
    } else {
        g_led.fastBlink();
    }
    //my own processing
    if (g_sampler.isExpired(true) || bForcePublishing) {
        // load sensor values
        g_fTemperature = g_temperature.getTemp();
        // log to serial
        Serial.print(g_fTemperature);
        Serial.print(F(" C ; "));
        Serial.print(bHandledMotion);
        Serial.print(F(" B ; "));
        Serial.print(bHandledSwitch);
        Serial.print(F(" B ; "));
        Serial.print(g_beeper.isHigh());
        Serial.print(F(" B ; "));
        Serial.print(g_relayTimeout.isRunning());
        Serial.print(F(" B ; "));
        Serial.print(g_relay.isHigh());
        Serial.print(F(" B"));
        Serial.println();
    }
    if (g_logger.isExpired(true) || bForcePublishing) {
        g_mqtt.publishStatusValues(g_fTemperature, bHandledMotion, bHandledSwitch, g_beeper.isHigh(), g_relayTimeout.isRunning(), g_relay.isHigh(), g_relayTimeout.timeLeftMs() / 1000);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Required by the MQTT library
//////////////////////////////////////////////////////////////////////////////////

void onConnectionEstablished() {
    //setup OTA
    if (g_config.getOTAEnabled()) {
        configureOTA();
    } else {
        Serial.println("OTA is disabled");
    }

    //publish values
    g_mqtt.publishDeviceValues();
    g_mqtt.publishStatusValues(g_fTemperature, g_bDetectedMotion, g_bPressedSwitch, g_beeper.isHigh(), g_relayTimeout.isRunning(), g_relay.isHigh(), g_relayTimeout.timeLeftMs() / 1000);

    //register for display
    g_mqtt.subscribe(g_mqtt.topicForPath("subscribe/desired_temperature"), [](const String & payload) {
        g_config.setDesiredTemperature(payload.toFloat());
        g_config.commitToEEPROM();
        g_beeper.blink(1, 1, 10);
        g_mqtt.publishDeviceValues();
    });

    //register for beeper
    g_mqtt.subscribe(g_mqtt.topicForPath("subscribe/auto_switchoff_seconds"), [](const String & payload) {
        g_config.setAutoSwitchOffSeconds(payload.toInt());
        g_relayTimeout.updateTimeoutAndRestart(g_config.getAutoSwitchOffSeconds()*1000);
        g_config.commitToEEPROM();
        g_beeper.blink(1, 1, 10);
        g_mqtt.publishDeviceValues();
    });

    //register for enable switching on
    g_mqtt.subscribe(g_mqtt.topicForPath("subscribe/output_toggle"), [](const String & payload) {
        pressedSwitch();
    });
}

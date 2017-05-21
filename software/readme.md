The software is written using the Arduino IDE and there are 2 version availabe, standalone and IBM Bluemix IoT platform.

# General
## ![High voltage](../hardware/images/high-voltage-icon.png) Over the air!

Because the device uses high mains voltage, its not safe to use it and then connect a serial port for debugging to it. Its also very impractical of course.
Therefore debugging and firmware upload is done over the air:

###  Remote debugging via telnet

All versions (standalone, IBM Bluemix...) use remote debugging via a telent conneciton.

Refer to 
[RemoteDebug Library for ESP8266](https://github.com/JoaoLopesF/RemoteDebug) as written by Joao Lopes.

### ArduinoOTA

All versions include the ArduinoOTA functionality so you can upload new firware in the device over the air.

Refer to 
[ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA) as part of the ESP8266 core for Arduino.


## ![](../hardware/images/wifi-icon.png) WiFiManager
When the device is initially booted it will present itself as an access point.
It will then present an nice interface to input your WIFI credentials and then reboot into "normal" operation.

Refer to [WiFiManager](https://github.com/tzapu/WiFiManager) as written by tzapu.

# IBM Bluemix software

Refer to [IBM Bluemix IoT platform](https://www.ibm.com/cloud-computing/bluemix/internet-of-things).

Connects the devices to the IBM Bluemix IoT service. Just subscribe for a 30 day trail (and after this its still free because we consume not nearly enough resources!). After you entrolled and configured the environment enter the secrets (organisation and token) in the secret.h file, compile and upload to the device!

# Standalone software
The device operates in standalone mode, pressing the button on the front panel enables the load for 25 minutes, but operates in thermostat mode with a desired temperature of 23 degrees. The load will be switched off if temp gets higher and will automatically switch on again if its lower, a temp hysteresis of 1 degree is used. 

Todo:

- usage of the PIR to be implemented
- program a nice web ui for setting different parameters
- ...



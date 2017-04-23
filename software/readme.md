The software is written using the Arduino IDE and there are 2 version availabe:

# standalone
The device operates in standalone mode, pressing the button on the front panel enables the load for 25 minutes, but operates in thermostat mode with a desired temperature of 23 degrees. The load will be switched off if temp gets higher and will automatically switch on again if its lower, a temp hysteresis of 1 degree is used. 

A telnet connection to the device is used for streaming debug information.

Todo:

- usage of the PIR to be implemented
- program a nice web ui for setting different parameters


# IBM Bluemix
Connects the devices to the IBM Bluemix IoT service.

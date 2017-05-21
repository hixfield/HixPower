#include <Arduino.h>
#include "Status.h"
#include "HixGeneral.h"

bool operator==(const StatusParam& lhs, const StatusParam& rhs) {
  return
    (lhs.desiredTemperature == rhs.desiredTemperature) &&
    (lhs.currentTemperature == rhs.currentTemperature) &&
    (lhs.heatingOn == rhs.heatingOn) &&
    (lhs.heatingEnabled == rhs.heatingEnabled);
}

Status::Status() : m_timeoutRelay(m_relayDurationSwitch, true), m_timeoutButton(250, true) {

}

void Status::setCurrentTemperature(float temperature) {
  m_current.currentTemperature = temperature;
}

void Status::setHeatingOn(bool on) {
  m_current.heatingOn = on;
}

void Status::pressedButton(void) {
  //debouncing
  if (m_timeoutButton.running() ) {
    return;
  }
  m_timeoutButton.restart();
  //process command
  if (m_timeoutRelay.running() ) {
    m_timeoutRelay.invalidate();
  }
  else {
    m_timeoutRelay.updateTimeoutAndRestart(m_relayDurationSwitch);
  }
}

bool Status::heatingEnabled(void) {
  return m_timeoutRelay.running();
}
unsigned long Status::heatingTimeLeft(void) {
  return m_timeoutRelay.timeLeftMs() / 1000;
}

void Status::printCSV(Print &stream) {
  stream.print(msToHHMMSS(millis()));
  stream.print("     ");
  stream.print(m_current.currentTemperature);  
  stream.print("C     ");
  stream.print(m_current.desiredTemperature);
  stream.print("C     ");
  stream.print(heatingEnabled());
  stream.print("     ");
  stream.print(calculateIfHeatingShouldBeOn());
  stream.print("     ");
  stream.print(heatingTimeLeft());
  stream.print("     ");
  stream.println(macAsFlatString());
}

String Status::mqttPayload(bool forcePublish) {
  m_mqttPayload="";
  //update the current status for the calculated vars
  m_current.heatingEnabled = heatingEnabled();
  //only publish if required
  if ( (m_current == m_lastMqttPublished) && !forcePublish ) {
    return "";
  }
  //start tag
  m_mqttPayload = "{\"d\":{";
  //publish params
  m_mqttPayload += "\"currentTemperature\":";
  m_mqttPayload += m_current.currentTemperature;
  m_mqttPayload += ",";
  m_mqttPayload += "\"desiredTemperature\":";
  m_mqttPayload += m_current.desiredTemperature;
  m_mqttPayload += ",";
  m_mqttPayload += "\"heatingEnabled\":";
  m_mqttPayload += m_current.heatingEnabled ? "true" : "false";
  m_mqttPayload += ",";
  m_mqttPayload += "\"heatingOn\":";
  m_mqttPayload += m_current.heatingOn ? "true" : "false";
  //closing tag
  m_mqttPayload += "}}";
  //sync our publish counters and stuff
  m_lastMqttPublished = m_current;
  //return payload
  return m_mqttPayload;
}

bool Status::calculateIfHeatingShouldBeOn(void) {
  //if not running (e.g. pressed switch) no heating allowed
  if (m_timeoutRelay.expired() ) {
    return false;
  }
  //ok, heating is allowed...
  //if we are currently on, keep heating until high temp reached
  if (m_current.heatingOn) {
    return m_current.currentTemperature < ( m_current.desiredTemperature + ( m_temperatureHysteresis / 2 ) );
  }
  //if currently off switch on if low temp reached
  return  m_current.currentTemperature < ( m_current.desiredTemperature - ( m_temperatureHysteresis / 2 ) );
}

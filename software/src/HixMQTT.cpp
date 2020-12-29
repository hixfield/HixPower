#include "HixMQTT.h"
#include <Arduino.h>
#include <ArduinoJson.h>

HixMQTT::HixMQTT(HixConfig &  config,
                 const char * szWifi_SSID,
                 const char * szWiFi_Password,
                 const char * szMQTT_Server,
                 const char * szDeviceType,
                 const char * szDeviceVersion,
                 const char * szRoom,
                 const char * szDeviceTag,
                 const char * szDeviceBuildTimestamp) : HixMQTTBase(szWifi_SSID, szWiFi_Password, szMQTT_Server, szDeviceType, szDeviceVersion, szRoom, szDeviceTag, szDeviceBuildTimestamp),
                                                        m_config(config) {
}

bool HixMQTT::publishDeviceValues(void) {
    if (!HixMQTTBase::publishDeviceValues()) {
        return false;
    }
    //my custom implementation
    if (isConnected()) {
        publish(topicForPath("device/desired_temperature"), m_config.getDesiredTemperature(), true);
        publish(topicForPath("device/auto_switchoff_seconds"), m_config.getAutoSwitchOffSeconds(), true);
        //return non error
        return true;
    }
    //not connected, return error
    return false;
}

bool HixMQTT::publishStatusValues(float         fTemperature,
                                  bool          bMotionDetected,
                                  bool          bSwitchPressed,
                                  bool          bBeeperOn,
                                  bool          bOutputAllowed,
                                  bool          bOutputOn,
                                  unsigned long nSecondsLeft) {
    //call base implementation
    if (!HixMQTTBase::publishStatusValues()) {
        return false;
    }
    //my custom implementation
    if (isConnected()) {
        //dynamic values are not published with a retain = default value = false
        publish(topicForPath("status/temperature"), fTemperature);
        publish(topicForPath("status/motion_detected"), bMotionDetected);
        publish(topicForPath("status/switch_pressed"), bSwitchPressed);
        publish(topicForPath("status/beeper_on"), bBeeperOn);
        publish(topicForPath("status/output_allowed"), bOutputAllowed);
        publish(topicForPath("status/output_on"), bOutputOn);
        publish(topicForPath("status/auto_switchoff_seconds_left"), nSecondsLeft);
        //publish to influxdb topic
        publish(topicForPath("influxdb"), influxDBJson(fTemperature, bMotionDetected, bSwitchPressed, bBeeperOn, bOutputAllowed, bOutputOn, nSecondsLeft));
        //return non error
        return true;
    }
    //not connected, return error
    return false;
}

String HixMQTT::influxDBJson(float         fTemperature,
                             bool          bMotionDetected,
                             bool          bSwitchPressed,
                             bool          bBeeperOn,
                             bool          bOutputAllowed,
                             bool          bOutputOn,
                             unsigned long nSecondsLeft) {
    DynamicJsonDocument doc(500);

    //create the measurements => fields
    JsonObject doc_0                     = doc.createNestedObject();
    doc_0["temperature"]                 = fTemperature;
    doc_0["motion_detected"]             = bMotionDetected;
    doc_0["switch_pressed"]              = bSwitchPressed;
    doc_0["beeper_on"]                   = bBeeperOn;
    doc_0["output_allowed"]              = bOutputAllowed;
    doc_0["output_on"]                   = bOutputOn;
    doc_0["auto_switchoff_seconds_left"] = nSecondsLeft;
    doc_0["wifi_rssi"]                   = WiFi.RSSI();

    //the device props => tags
    JsonObject doc_1                     = doc.createNestedObject();
    doc_1["device_type"]                 = m_deviceType;
    doc_1["device_version"]              = m_deviceVersion;
    doc_1["device_tag"]                  = m_deviceTag;
    doc_1["device_build_timestamp"]      = m_deviceBuildTimestamp;
    doc_1["room"]                        = m_room;
    doc_1["wifi_mac"]                    = WiFi.macAddress();
    doc_1["wifi_ssid"]                   = WiFi.SSID();
    doc_1["desired_temperature"]         = m_config.getDesiredTemperature();
    doc_1["auto_switchoff_seconds_left"] = m_config.getAutoSwitchOffSeconds();

    //to string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

#include <Arduino.h>
#include "IBMMQTTClient.h"
#include "HixGeneral.h"
#include <ArduinoJson.h>

//note: the order of initialization is the order of declaration in the class itself refer to : http://stackoverflow.com/questions/1242830/constructor-initialization-list-evaluation-order
//this is wy we dont call the PubSubClient(...) with the argument for setting the server because its not yet set at that moment, and we use the setServer() function

const String IBMMQTTClient::s_authMethod           = String("use-token-auth");
const String IBMMQTTClient::s_topicStatusTemplate  = String("iot-2/evt/$$NAME$$/fmt/json");
const String IBMMQTTClient::s_topicCommand         = String("iot-2/cmd/+/fmt/+");
const String IBMMQTTClient::s_topicResponse        = String("iotdm-1/response");
const String IBMMQTTClient::s_topicManage          = String("iotdevice-1/mgmt/manage");
const String IBMMQTTClient::s_topicUpdate          = String("iotdm-1/device/update");
const String IBMMQTTClient::s_topicReboot          = String("iotdm-1/mgmt/initiate/device/reboot");
const String IBMMQTTClient::s_topicFactoryReset    = String("iotdm-1/mgmt/initiate/device/factory_reset");



//The topic is not valid: iot-2/evt/iotdevice-1/mgmt/manage/fmt/json The topic does not match an allowed rule 17 times in the last 5 minutes


IBMMQTTClient::IBMMQTTClient(const char * organisation, const char * deviceType, const char * token, Client& client, Print& debug, MQTT_CALLBACK_SIGNATURE) :
  m_debug(debug),
  m_organisation(organisation),
  m_deviceType(deviceType),
  m_token(token),
  m_server(m_organisation + ".messaging.internetofthings.ibmcloud.com"),
  m_clientId("d:" + m_organisation + ":" + m_deviceType + ":" + macAsFlatString()),
  PubSubClient(client) {
  setServer(m_server.c_str(), 8883);
  setCallback(callback);
}

boolean IBMMQTTClient::loop() {
  boolean returnValue = PubSubClient::loop();
  if (!returnValue) {
    m_debug.print("MQTT establishing with ");
    m_debug.print(m_server);
    m_debug.print(" status:");
    m_debug.println(statusAsString());
    m_debug.println(m_clientId);
    m_debug.println(s_authMethod);
    m_debug.println(m_token);
    if (connect(m_clientId.c_str(), s_authMethod.c_str(), m_token.c_str())) {
      m_debug.println("MQTT connection established");
      subscribeManagedDevice();
      subscribe(s_topicCommand.c_str());      
    }
//    delay(500);
  }
  return returnValue;
}

char * IBMMQTTClient::statusAsString() {
  switch (state() ) {
    case -4 : return "MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time";
    case -3 : return "MQTT_CONNECTION_LOST - the network connection was broken";
    case -2 : return "MQTT_CONNECT_FAILED - the network connection failed";
    case -1 : return "MQTT_DISCONNECTED - the client is disconnected cleanly";
    case  0 : return "MQTT_CONNECTED - the client is connected";
    case  1 : return "MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT";
    case  2 : return "MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier";
    case  3 : return "MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection";
    case  4 : return "MQTT_CONNECT_BAD_CREDENTIALS - the username / password were rejected";
    case  5 : return "MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect";
  }
}

boolean IBMMQTTClient::publishStatus(String topic, String payload) {
  String statusTopic = String(s_topicStatusTemplate);
  statusTopic.replace("$$NAME$$", topic);
  return publishNative(statusTopic.c_str(), payload.c_str());
}

boolean IBMMQTTClient::publishNative(String topic, String payload) {
  boolean returnValue = PubSubClient::publish(topic.c_str(), payload.c_str());
  if (returnValue) {
    m_debug.print("MQTT published payload: ");
  } else {
    m_debug.print("MQTT publish failed");
    m_debug.print(" (");
    m_debug.println(statusAsString());
    m_debug.print(" )");
  }
  m_debug.println(payload);
  return returnValue;
}

boolean IBMMQTTClient::topicIsCommand(const char * topic, const char * command) {
  return String(topic).indexOf(command) != -1;
}

boolean IBMMQTTClient::topicIsReboot(const char * topic) {
  return s_topicReboot == topic;
}
boolean IBMMQTTClient::topicIsFactoryReset(const char * topic) {
  return s_topicFactoryReset == topic;
}


boolean IBMMQTTClient::subscribe(String topic) {
  m_debug.print("MQTT subscribe to [");
  m_debug.print(topic);
  m_debug.print("] ");
  if (PubSubClient::subscribe(topic.c_str())) {
    m_debug.println("OK");
  }
  else {
    m_debug.print("FAILED [");
    m_debug.print(statusAsString());
    m_debug.println("]");
  }
}

void IBMMQTTClient::subscribeManagedDevice(void) {
  //subscribe(s_topicResponse);
  subscribe(s_topicReboot);
  subscribe(s_topicFactoryReset);
  subscribe(s_topicUpdate);
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root     = jsonBuffer.createObject();
  JsonObject& d        = root.createNestedObject("d");
  JsonObject& metadata =      d.createNestedObject("metadata");
  metadata["testmetadata"] = "123456789";
  JsonObject& supports =      d.createNestedObject("supports");
  supports["deviceActions"] = true;
  char buff[300];
  root.printTo(buff, sizeof(buff));
  publishNative(s_topicManage, buff);
}



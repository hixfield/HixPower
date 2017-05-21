#ifndef IBMMQTTClient_h
#define IBMMQTTClient_h

#include "pubsubclient.h"
#include "HixTimeout.h"
#include "Print.h"

class IBMMQTTClient : public PubSubClient {

  private :
    //used for debugging
    Print & m_debug;
    //constants
    static const String s_authMethod;
    static const String s_topicStatusTemplate;
    static const String s_topicCommand;
    static const String s_topicResponse;
    static const String s_topicManage;
    static const String s_topicUpdate;
    static const String s_topicReboot;
    static const String s_topicFactoryReset;

    //input params during construction
    const String m_organisation;
    const String m_deviceType;
    const String m_token;

    //params calculated during initalisation
    const String m_server;
    const String m_clientId;


    void subscribeManagedDevice(void);

  public:

    IBMMQTTClient(const char * organisation, const char * deviceType, const char * token, Client& client, Print& debug, MQTT_CALLBACK_SIGNATURE);
    boolean loop();
    char * statusAsString();
    boolean publishNative(String topic, String payload);
    boolean publishStatus(String status, String payload);
    boolean topicIsCommand(const char * topic, const char * command);
    boolean topicIsReboot(const char * topic);
    boolean topicIsFactoryReset(const char * topic);
    boolean subscribe(String topic);
};

#endif

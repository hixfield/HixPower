#ifndef Status_h
#define Status_h

#include "HixTimeout.h"
#include "Print.h"


struct StatusParam {
  float          desiredTemperature;
  float          currentTemperature;
  bool           heatingOn;
  bool           heatingEnabled;
  StatusParam() {
    desiredTemperature = 23.0;
    currentTemperature = 0;
    heatingOn          = false;
    heatingEnabled     = false;
  }
};


class Status {

  private:
    //working variables
    String         m_mqttPayload;
    StatusParam    m_lastMqttPublished;
    //hardware linked parameters
    StatusParam    m_current;
    //soft parameters
    float          m_temperatureHysteresis = 0.90;
    unsigned long  m_relayDurationSwitch   = 60000UL * 25UL;
    HixTimeout     m_timeoutRelay;
    HixTimeout     m_timeoutButton;

  public:

    Status();
    //sync physical world with status functions
    void          setCurrentTemperature(float temperature);
    void          setHeatingOn(bool on);
    void          pressedButton();
    //calculated status parameters
    bool          heatingEnabled(void);
    bool          calculateIfHeatingShouldBeOn(void);
    unsigned long heatingTimeLeft(void);
    //exporting of status data
    void          printCSV(Print &stream);
    String        mqttPayload(bool forcePublish);
};

#endif

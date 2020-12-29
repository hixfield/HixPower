#ifndef HixConfig_h
#define HixConfig_h

#include <arduino.h>

class HixConfig {
  private:
    //base address for epprom
    const int m_nEPPROMBaseAddress = 0;
    //histeresis fDesiredTemperature
    const float m_fTemperatureHysteresis = 1.0f;
    //hilds the data
    struct {
        int           nNumberOfBootUps;
        char          szMQTTServer[50];
        char          szRoom[50];
        char          szDeviceTag[50];
        float         fDesiredTemperature;
        unsigned long ulAutoSwithcOffSecconds;
        bool          bOTAEnabled;
        unsigned long crc;
    } data;
    //determine crc
    unsigned long calculateCRC(void);
    //store defaults
    void commitDefaults(void);

  public:
    HixConfig();
    //getters
    int           getNumberOfBootUps(void);
    const char *  getDeviceType(void) { return "HixPower"; };
    const char *  getDeviceVersion(void) { return "2.3.5"; };
    const char *  getDeviceBuildTimestamp(void);
    const char *  getMQTTServer(void);
    const char *  getRoom(void);
    const char *  getDeviceTag(void);
    bool          getOTAEnabled(void);
    float         getDesiredTemperature(void);
    float         getSwitchOnTemperature(void);
    float         getSwitchOffTemperature(void);
    unsigned long getAutoSwitchOffSeconds(void);
    //setters
    void setMQTTServer(const char * szValue);
    void setRoom(const char * szValue);
    void setDeviceTag(const char * szValue);
    void setOTAEnabled(bool bValue);
    void setDesiredTemperature(float fValue);
    void setAutoSwitchOffSeconds(unsigned long ulValue);
    //save to eeprom
    void commitToEEPROM(void);
    //replaces placeholders in string with config values
    void replacePlaceholders(String & contents);
};

#endif

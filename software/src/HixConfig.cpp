#include "HixConfig.h"
#include "secret.h"
#include <EEPROM.h>

HixConfig::HixConfig() {
    EEPROM.begin(512);
    EEPROM.get(m_nEPPROMBaseAddress, data);
    if (calculateCRC() != data.crc) {
        commitDefaults();
    }
}

int HixConfig::getNumberOfBootUps(void) {
    return data.nNumberOfBootUps;
}

const char * HixConfig::getDeviceBuildTimestamp(void) {
    return __DATE__ " " __TIME__;
}

const char * HixConfig::getMQTTServer(void) {
    return data.szMQTTServer;
};

const char * HixConfig::getRoom(void) {
    return data.szRoom;
};

const char * HixConfig::getDeviceTag(void) {
    return data.szDeviceTag;
};

bool HixConfig::getOTAEnabled(void) {
    return data.bOTAEnabled;
}

float HixConfig::getDesiredTemperature(void) {
    return data.fDesiredTemperature;
}

float HixConfig::getSwitchOnTemperature(void) {
    return getDesiredTemperature() - (m_fTemperatureHysteresis / 2.0f);
}

float HixConfig::getSwitchOffTemperature(void) {
    return getDesiredTemperature() + (m_fTemperatureHysteresis / 2.0f);
}

unsigned long HixConfig::getAutoSwitchOffSeconds(void) {
    return data.ulAutoSwithcOffSecconds;
};

void HixConfig::setMQTTServer(const char * szValue) {
    memset(data.szMQTTServer, 0, sizeof(data.szMQTTServer));
    strncpy(data.szMQTTServer, szValue, sizeof(data.szMQTTServer) - 1);
}

void HixConfig::setRoom(const char * szValue) {
    memset(data.szRoom, 0, sizeof(data.szRoom));
    strncpy(data.szRoom, szValue, sizeof(data.szRoom) - 1);
}

void HixConfig::setDeviceTag(const char * szValue) {
    memset(data.szDeviceTag, 0, sizeof(data.szDeviceTag));
    strncpy(data.szDeviceTag, szValue, sizeof(data.szDeviceTag) - 1);
}

void HixConfig::setOTAEnabled(bool bValue) {
    data.bOTAEnabled = bValue;
}

void HixConfig::setDesiredTemperature(float fValue) {
    data.fDesiredTemperature = fValue;
}

void HixConfig::setAutoSwitchOffSeconds(unsigned long ulValue) {
    data.ulAutoSwithcOffSecconds = ulValue;
}

void HixConfig::commitToEEPROM(void) {
    data.crc = calculateCRC();
    EEPROM.put(m_nEPPROMBaseAddress, data);
    EEPROM.commit();
}

unsigned long HixConfig::calculateCRC(void) {
    unsigned char * crcData = (unsigned char *)&data;
    size_t          size    = sizeof(data) - sizeof(data.crc);
    unsigned long   crc     = 1234;
    for (int index = 0; index < size; index++) {
        crc += crcData[index];
    }
    return crc;
}

void HixConfig::commitDefaults(void) {
    setMQTTServer(Secret::MQTT_SERVER);
    setRoom("test_room");
    setDeviceTag("test_tag");
    setOTAEnabled(true);
    setDesiredTemperature(22.0f);
    setAutoSwitchOffSeconds(60 * 20);
    commitToEEPROM();
}

void HixConfig::replacePlaceholders(String & contents) {
    contents.replace("||NBR_BOOTUPS||", String(getNumberOfBootUps()));
    contents.replace("||MQTT_SERVER||", getMQTTServer());
    contents.replace("||ROOM||", getRoom());
    contents.replace("||DEVICE_TAG||", getDeviceTag());
    contents.replace("||DEVICE_TYPE||", getDeviceType());
    contents.replace("||DEVICE_VERSION||", getDeviceVersion());
    contents.replace("||OTA_ENABLED||", getOTAEnabled() ? "checked" : "");
    contents.replace("||RESET_REASON||", ESP.getResetReason());
    contents.replace("||RESET_INFO||", ESP.getResetInfo());
    contents.replace("||FREE_HEAP||", String(ESP.getFreeHeap()));
    contents.replace("||FREE_STACK||", String(ESP.getFreeContStack()));
    contents.replace("||BUILD_TIMESTAMP||", getDeviceBuildTimestamp());
}
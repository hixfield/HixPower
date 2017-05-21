#include "DS18B20Temperature.h"

DS18B20Temperature::DS18B20Temperature(int nPinOneWire) {
    m_pOneWire = new OneWire(nPinOneWire);
    m_pDallasTemperature = new DallasTemperature((OneWire *)m_pOneWire);
}

void DS18B20Temperature::begin(void) {
    m_pDallasTemperature->begin();
}

DS18B20Temperature::~DS18B20Temperature() {
    if (m_pOneWire) delete m_pOneWire;
    if (m_pDallasTemperature) delete m_pDallasTemperature;
}

float DS18B20Temperature::getTemp(void) {
    m_pDallasTemperature->requestTemperatures();
    return m_pDallasTemperature->getTempCByIndex(0);
}



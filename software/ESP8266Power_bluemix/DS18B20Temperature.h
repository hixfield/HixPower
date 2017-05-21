#ifndef DS18B20Temperature_h
#define DS18B20Temperature_h

#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20Temperature {
    
    OneWire *           m_pOneWire;
    DallasTemperature * m_pDallasTemperature;
    
public:
    
    //lifecycle
    DS18B20Temperature(int nPinOneWire);
    void begin(void);
    
    //public api
    float getTemp(void);
    ~DS18B20Temperature();
};


#endif

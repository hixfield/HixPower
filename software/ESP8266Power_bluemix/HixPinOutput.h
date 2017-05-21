#ifndef HixPinOutput_h
#define HixPinOutput_h

#include "HixPin.h"

class HixPinOutput : HixPin {

  public:

    HixPinOutput(int nPinNumber);
    void begin();
    void digitalWrite(bool value);
    bool digitalRead(void);
    void blink(bool bOn, int nNumberOfBlinks, int nDelayMs);
};

#endif

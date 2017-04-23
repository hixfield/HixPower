#ifndef HixPinInput_h
#define HixPinInput_h

#include "HixPin.h"

class HixPinInput : public HixPin {

  public:

    HixPinInput(int nPinNumber);
    void begin();
    void attachInterrupt( void (isr)(void), int mode);
};

#endif

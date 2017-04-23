#ifndef HixPin_h
#define HixPin_h


//*virtual* class for inherited HixPinInput and HixPinOutput

class HixPin {

  protected:

    const int m_nPinNumber;

  public:
    int getPinNumber();
    HixPin(int nPinNumber);
    bool digitalRead(void);
    virtual void begin() = 0;
};

#endif

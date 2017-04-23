#include "HixPinOutput.h"
#include <Arduino.h>

HixPinOutput::HixPinOutput(int nPinNumber): HixPin(nPinNumber) {
}

void HixPinOutput::begin() {
  pinMode(m_nPinNumber, OUTPUT);
}

void HixPinOutput::digitalWrite(bool value) {
  ::digitalWrite(m_nPinNumber, value);
}

bool HixPinOutput::digitalRead(void) {
  return ::digitalRead(m_nPinNumber);
}

void HixPinOutput::blink(bool bOn, int nNumberOfBlinks, int nDelayMs) {
  for (int i = 0; i < nNumberOfBlinks; i++) {
    //led on
    digitalWrite(bOn);
    delay(nDelayMs);
    //led off
    digitalWrite(!bOn);
    delay(nDelayMs);
  }
}

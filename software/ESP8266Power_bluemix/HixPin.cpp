#include "HixPinOutput.h"
#include <Arduino.h>

HixPin::HixPin(int nPinNumber): m_nPinNumber(nPinNumber) {
}

int HixPin::getPinNumber() {
  return m_nPinNumber;
}

bool HixPin::digitalRead(void) {
  return ::digitalRead(m_nPinNumber);
}

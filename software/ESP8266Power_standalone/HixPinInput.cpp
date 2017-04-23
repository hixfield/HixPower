#include "HixPinInput.h"
#include <Arduino.h>

HixPinInput::HixPinInput(int nPinNumber): HixPin(nPinNumber) {
}

void HixPinInput::begin() {
  pinMode(m_nPinNumber, INPUT);
}

void HixPinInput::attachInterrupt(void (isr)(void), int mode) {
::attachInterrupt(digitalPinToInterrupt(m_nPinNumber),isr, mode); 
}


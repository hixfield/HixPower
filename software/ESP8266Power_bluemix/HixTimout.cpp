#include "HixTimeout.h"
#include <Arduino.h>


HixTimeout::HixTimeout(unsigned long ulDelayMs, bool invalidated) {
  updateTimeoutAndRestart(ulDelayMs);
  m_bInvalidated = invalidated;
}

bool HixTimeout::expired(bool restartIfExpired) {
  //if I am invalidated return expired immediatly
  if (m_bInvalidated) {
    if  (restartIfExpired) {
      restart();
    }
    return true;
  }
  //depending on overflow do different calculations
  bool ret = currentDiff() > ((float)m_ulDelayMs);
  //restart if requested
  if (ret && restartIfExpired) {
    restart();
  }
  //return our value
  return ret;
}

bool HixTimeout::running(void) {
  return !expired(false);
}

void HixTimeout::restart(void) {
  m_bInvalidated = false;
  m_ulStart = millis();
}

void HixTimeout::updateTimeoutAndRestart(unsigned long ulDelayMs) {
  m_ulDelayMs = ulDelayMs;
  restart();
}

void HixTimeout::invalidate() {
  m_bInvalidated = true;
}

float HixTimeout::currentDiff(void) {
  float fStart   = m_ulStart;
  float fCurrent = millis();
  float fDiff    = fCurrent - fStart;
  float fMaxUL   = (unsigned long) 0xFFFFFFFF;
  if (fDiff < 0) {
    fDiff += fMaxUL;
  }
  return fDiff;
}

unsigned long HixTimeout::timeLeftMs(void) {
  return m_ulDelayMs - timePastMs();
}

unsigned long HixTimeout::timePastMs(void) {
  return ( expired(false) ) ? 0 : currentDiff();
}

unsigned long HixTimeout::timeoutMs(void) {
  return m_ulDelayMs;
}


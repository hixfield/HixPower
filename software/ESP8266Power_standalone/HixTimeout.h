#ifndef HixTimeout_h
#define HixTimeout_h

class HixTimeout {

  private:

    unsigned long m_ulDelayMs;
    unsigned long m_ulStart;
    bool          m_bInvalidated;

    float currentDiff(void);

  public:

    HixTimeout(unsigned long ulDelayMs, bool invalidated = false);
    bool expired(bool restartIfExpired = false);
    bool running(void);
    void restart(void);
    void invalidate(void);
    void updateTimeoutAndRestart(unsigned long ulDelayMslyMs);
    unsigned long timeLeftMs(void);
    unsigned long timePastMs(void);
    unsigned long timeoutMs(void);

};

#endif

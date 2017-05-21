#ifndef HixGeneral_h
#define HixGeneral_h
#include <arduino.h>

char * msToHHMMSS(unsigned long ms);
String macToFlatString(const uint8_t* mac);
String macAsFlatString(void);

#endif

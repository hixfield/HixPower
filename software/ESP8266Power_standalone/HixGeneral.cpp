#include "HixGeneral.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

char * msToHHMMSS(unsigned long ms) {
  static char str[12];
  ms /= 1000;
  long h = ms / 3600;
  ms = ms % 3600;
  int m = ms / 60;
  int s = ms % 60;
  sprintf(str, "%04ld:%02d:%02d", h, m, s);
  return str;
}

String macToFlatString(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
  }
  return result;
}

String macAsFlatString(void) {
  String str = WiFi.macAddress();
  str.replace(":","");
  return str;
}


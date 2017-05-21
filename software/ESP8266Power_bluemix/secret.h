#ifndef Secret_h
#define Secret_h

class Secret { 

  public:
  
  static const char * s_IBM_IoT_Organisation;
  static const char * s_IBM_IoT_Token;

};


const char * Secret::s_IBM_IoT_Organisation = "ENTER YOUR IBM ORGANISATION HERE";
const char * Secret::s_IBM_IoT_Token        = "ENTER THE IBM IOT TOKEN HERE";

#endif

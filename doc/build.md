# Build
This project can be build using Arduino IDE with installed dependencies (listed above).

## Dependencies
Following libraries were used:
 - ESPAsyncWebSrv
 - FS
 - WiFi
 - AsyncTCP
 - DNSServer
 - ArduinoJson
 - EEPROM
 - StreamUtils
 - ESPmDNS
 - ESPDateTime
 - PubSubClient
 
## Required changes in libraries
You have to change include in ElegantOTA.h header:
    #include "ESPAsyncWebServer.h" -> #include "ESPAsyncWebSrv.h"

[Main page](../README.md)
# Louver control for ESP based devices
This contains a firmware for louver control for Tasmota-compatible ESP based devices like Sonoff DUAL R3. It was created because I was not satisfied with Tasmota configuration specific to louvers (not the same as blinds and curtains). Louvers require specific control of lamellas.

## Function description
There are 3 GPIO buttons:
 - key up
 - key down
 - reset

And 2 GPIO relays:
 - motor up
 - motor down

Short key press controls relays directly, long press (more than 2 seconds) makes louver to fully open/close (time can be configured).

5 second long press of reset button causes loading defaults and module reboot to AP mode.

There is also web interface to control louver and configure firmware.

## Web interface
![Main page](doc/main_page.png)
![Settings page](doc/settings_page.png)

## Supported devices
 - ESP8266
 - ESP32
 
## What is implemented
 - AP and client mode
 - Web interface
 - Captive server
 - OTA firmware update
 - mDNS for friendly access (xxx.local instead of IP) 
  
## Dependencies
 - ESPAsyncWebSrv
 - FS
 - WiFi
 - AsyncTCP
 - AsyncElegantOTA
 - DNSServer
 - ArduinoJson
 - EEPROM
 - StreamUtils
 - ESPmDNS
 - ESPDateTime

## How to build
This project can be build using Arduino IDE with installed dependencies (listed above).

## Important notes
This project is under heavy development and it's still not finished.

To do:
 - Export / import configuration
 - MQTT support


# Quick setup tutorial
This page describes quick setup after first firmware flash.
## Network configuration
Module is configured in AP mode. You should be able to see it as Louver_xxxxxx
network (where xxxxxx is unique module ID). Default IP address in AP mode is 192.168.4.1.

Required:
 1. Connect wifi to module
 2. Enter 'Network config' page
 3. Change Wifi mode to Client
 4. Set client SSID (network you want module to connect to)
 5. Set client password
 6. Click 'Save'. Module will reboot and connect to local wifi.
    
Optional:
 1. Change AP SSID from Louver_xxxxxx to anything you like
 2. Change mDNS host name (default is louver). You can than connect to module
    using DNS name (e.g. louver.local)
 3. Enable telnet logging (you can see logs using teltet client)

*Note: Module is trying to connect to local wifi in client mode for 1 minute. It will automatically switch
to AP mode when connection was not successfull. It remains in AP mode for 5 minutes (you have a chance to change settings without need to reset module).*

## Module name configuration
You can set module frienldy name that will be displayed in module web interface heading
 1. Enter 'Module config' page
 2. Change module name
 3. Click 'Save'

 Optional:
 You can change default log levels by entering JSON configuration. Default is (all modules has INFO log level):
```json
{"*":2}
 ```

Example (HTTP module se to VERBOSE):
```json
{"*":2,"HTTP":4}
```

Supported modules:
 - '*' = all modules (default log level)
 - ADE7953
 - BL0939
 - Config
 - HTTP
 - Log
 - Louver
 - MDNS
 - Module
 - MQTT
 - PowerMeas
 - OTA

 ### GPIO configuration
1. Enter 'GPIO config' page
2. Set all settings for your module
3. Click 'Save'

Defaults for Sonoff DUAL R3: 
Up key pin = 32, up key level = Active low, up key pull resistor = Enabled, down key pin = 33, down key level = Active low, down key pull resistor = Enabled, relay up pin = 27, relay up active level = Active high, relay down pin = 14, relay down active level = Active high, reset key pin = 0, reset key level = Active low, reset key pull resistor = Enabled

## Movement config
You can set up, down, short and open lamellas movement durations.

 1. Enter 'Movement config' page
 2. Configure times
 3. Click 'Save'

Optional:
You can enable stop movement conditions when your module supports power measurement. This simulates end switches - it can stop full open and full close movements when motor stops.

## Power measurement config
Power measurement can be used to simulate end switches and stop long movements.

Example for Sonoff DUAL R3 - movement up (stops when Power 1 is below 20W for 1500ms):
```json
{"index":3,"threshold":20.00,"comparator":">","duration_milli":1500}
```

Example for Sonoff DUAL R3 - movement down(stops when Power 2 is below 20W for 1500ms):
```json
{"index":4,"threshold":20.00,"comparator":">","duration_milli":1500}
```

## MQTT configuration
Module supports MQTT. It can connect to MQTT broker. 

 1. Enter 'MQTT config' page
 2. Set MQTT to Enabled
 3. Fill all required settings
 4. Click 'Save'

*Note: Leave MQTT disabled when you do not use it. Module responsiveness is better.*

# MQTT config and usage

## Config page
MQTT config page provides module MQTT configuration.

![MQTT config](mqtt_config.png)

### MQTT enabled
Enable or disable MQTT

### Client ID
MQTT client ID to be used to connect to broker

### Broker IP
MQTT broker IP

### Broker port
MQTT broker port

### Username
Username to be used to connect to MQTT broker

### Password
Password to be used to connect to MQTT broker

### Power meas. publish period
Power measurement topics publish perion in milliseconds (1000 is minimum)

## Usage
### Subscribe topics
Following subscribe topics are implemented:
 - CLIENT_ID/movement
 
#### CLIENT_ID/movement
Performs louver movement. Following values are supported:
 - up - short open movement
 - down - short close movement
 - open - full open movement
 - close - full close movement
 - close_open_lamellas - full close and open lamellas movement
 
mosquitto example:
```
mosquitto_pub.exe -t "louver/movement" -m "down"
```

### Publish topics
Following publish topics are implemented:
 - CLIENT_ID/movement/status
 - CLIENT_ID/key/up
 - CLIENT_ID/key/down
 - CLIENT_ID/power_meas/[depends on driver]
 
#### CLIENT_ID/movement/status
Current louver movement status. Following values are reported:
 - stop - no movement
 - up - short open movement
 - down - short close movement
 - open - full open movement
 - close - full close movement
 - close_open_lamellas - full close and open lamellas movement
 
#### CLIENT_ID/key/up and CLIENT_ID/key/down
Key press status. Following values are reported:
 - active - key pressed
 - inactive - key released
 - hold - key long pressed
 
#### CLIENT_ID/power_meas/[depends on driver]
These topics are used by power measurement. You can configure publish period.
Topic list is dependent on driver. Values are float converted to string.

[Main page](../README.md)

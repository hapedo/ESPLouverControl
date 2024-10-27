#pragma once

#define PROFILE PROFILE_ATHOM_KM01

#define PROFILE_ESP8266_DEV_KIT 0
#define PROFILE_ESP32_DEV_KIT 1
#define PROFILE_SONOFF_DUAL_POW_R3 2
#define PROFILE_SHELLY_PLUS_2PM 3
#define PROFILE_ATHOM_KM01          4

#if (PROFILE == PROFILE_ESP8266_DEV_KIT)
    #define PROFILE_DEFAULT_PIN_KEY_UP 2
    #define PROFILE_DEFAULT_PIN_KEY_DOWN 14
    #define PROFILE_DEFAULT_PIN_RELAY_UP 5
    #define PROFILE_DEFAULT_PIN_RELAY_DOWN 4
    #define PROFILE_DEFAULT_KEY_ACTIVE_HIGH false
    #define PROFILE_DEFAULT_KEY_PULL_ACTIVE true
    #define PROFILE_DEFAULT_RELAY_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_KEY_RESET 0
    #define PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_LED 6

    #define PROFILE_DEFAULT_BL0939_SERIAL 1
    #define PROFILE_DEFAULT_BL0939_TX_GPIO 25
    #define PROFILE_DEFAULT_BL0939_RX_GPIO 26
    #define PROFILE_DEFAULT_BL0939_PERIOD_MILLI 500
#endif

#if (PROFILE == PROFILE_ESP32_DEV_KIT)
    #define PROFILE_DEFAULT_PIN_KEY_UP 32
    #define PROFILE_DEFAULT_PIN_KEY_DOWN 33
    #define PROFILE_DEFAULT_PIN_RELAY_UP 27
    #define PROFILE_DEFAULT_PIN_RELAY_DOWN 14
    #define PROFILE_DEFAULT_KEY_ACTIVE_HIGH false
    #define PROFILE_DEFAULT_KEY_PULL_ACTIVE true
    #define PROFILE_DEFAULT_RELAY_ACTIVE_HIGH true

    #define PROFILE_DEFAULT_PIN_KEY_RESET 0
    #define PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_LED 6
#endif

#if (PROFILE == PROFILE_SONOFF_DUAL_POW_R3)
    #define PROFILE_DEFAULT_PIN_KEY_UP 32
    #define PROFILE_DEFAULT_PIN_KEY_DOWN 33
    #define PROFILE_DEFAULT_PIN_RELAY_UP 27
    #define PROFILE_DEFAULT_PIN_RELAY_DOWN 14
    #define PROFILE_DEFAULT_KEY_ACTIVE_HIGH false
    #define PROFILE_DEFAULT_KEY_PULL_ACTIVE true
    #define PROFILE_DEFAULT_RELAY_ACTIVE_HIGH true

    #define PROFILE_DEFAULT_PIN_KEY_RESET 0
    #define PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_LED 13

    #define PROFILE_DEFAULT_BL0939_SERIAL 2
    #define PROFILE_DEFAULT_BL0939_TX_GPIO 25
    #define PROFILE_DEFAULT_BL0939_RX_GPIO 26
    #define PROFILE_DEFAULT_BL0939_PERIOD_MILLI 500

    #define PROFILE_DEFAULT_CSE7761_SERIAL 2
    #define PROFILE_DEFAULT_CSE7761_TX_GPIO 25
    #define PROFILE_DEFAULT_CSE7761_RX_GPIO 26
    #define PROFILE_DEFAULT_CSE7761_PERIOD_MILLI 500
#endif

#if (PROFILE == PROFILE_SHELLY_PLUS_2PM)
    #define PROFILE_DEFAULT_PIN_KEY_UP 5
    #define PROFILE_DEFAULT_PIN_KEY_DOWN 18
    #define PROFILE_DEFAULT_PIN_RELAY_UP 13
    #define PROFILE_DEFAULT_PIN_RELAY_DOWN 12
    #define PROFILE_DEFAULT_KEY_ACTIVE_HIGH true
    #define PROFILE_DEFAULT_KEY_PULL_ACTIVE true
    #define PROFILE_DEFAULT_RELAY_ACTIVE_HIGH true

    #define PROFILE_DEFAULT_PIN_KEY_RESET 4
    #define PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_LED 0

    #define PROFILE_DEFAULT_ADE7953_MODE 1
    #define PROFILE_DEFAULT_ADE7953_PERIPHERAL 0
    #define PROFILE_DEFAULT_ADE7953_PIN0_GPIO 26
    #define PROFILE_DEFAULT_ADE7953_PIN1_GPIO 25
    #define PROFILE_DEFAULT_ADE7953_RESET_GPIO 33
    #define PROFILE_DEFAULT_ADE7953_PERIOD_MILLI 500
#endif

#if (PROFILE == PROFILE_ATHOM_KM01)
    #define PROFILE_DEFAULT_PIN_KEY_UP 3
    #define PROFILE_DEFAULT_PIN_KEY_DOWN 4
    #define PROFILE_DEFAULT_PIN_RELAY_UP 12
    #define PROFILE_DEFAULT_PIN_RELAY_DOWN 14
    #define PROFILE_DEFAULT_KEY_ACTIVE_HIGH false
    #define PROFILE_DEFAULT_KEY_PULL_ACTIVE true
    #define PROFILE_DEFAULT_RELAY_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_KEY_RESET 5
    #define PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH false

    #define PROFILE_DEFAULT_PIN_LED 13
#endif
// Defaults when not used by profile

//BL0939
#ifndef PROFILE_DEFAULT_BL0939_SERIAL
    #define PROFILE_DEFAULT_BL0939_SERIAL 1
#endif

#ifndef PROFILE_DEFAULT_BL0939_TX_GPIO
    #define PROFILE_DEFAULT_BL0939_TX_GPIO 25
#endif

#ifndef PROFILE_DEFAULT_BL0939_RX_GPIO
    #define PROFILE_DEFAULT_BL0939_RX_GPIO 26
#endif

#ifndef PROFILE_DEFAULT_BL0939_PERIOD_MILLI
    #define PROFILE_DEFAULT_BL0939_PERIOD_MILLI 500    
#endif

// ADE7953
#ifndef PROFILE_DEFAULT_ADE7953_MODE
    #define PROFILE_DEFAULT_ADE7953_MODE 0
#endif
#ifndef PROFILE_DEFAULT_ADE7953_PERIPHERAL
    #define PROFILE_DEFAULT_ADE7953_PERIPHERAL 1
#endif

#ifndef PROFILE_DEFAULT_ADE7953_PIN0_GPIO
    #define PROFILE_DEFAULT_ADE7953_PIN0_GPIO 25
#endif

#ifndef PROFILE_DEFAULT_ADE7953_PIN1_GPIO
    #define PROFILE_DEFAULT_ADE7953_PIN1_GPIO 26
#endif

#ifndef PROFILE_DEFAULT_ADE7953_RESET_GPIO
    #define PROFILE_DEFAULT_ADE7953_RESET_GPIO 33
#endif

#ifndef PROFILE_DEFAULT_ADE7953_PERIOD_MILLI
    #define PROFILE_DEFAULT_ADE7953_PERIOD_MILLI 500    
#endif

//CSE7761
#ifndef PROFILE_DEFAULT_CSE7761_SERIAL
    #define PROFILE_DEFAULT_CSE7761_SERIAL 1
#endif

#ifndef PROFILE_DEFAULT_CSE7761_TX_GPIO
    #define PROFILE_DEFAULT_CSE7761_TX_GPIO 25
#endif

#ifndef PROFILE_DEFAULT_CSE7761_RX_GPIO
    #define PROFILE_DEFAULT_CSE7761_RX_GPIO 26
#endif

#ifndef PROFILE_DEFAULT_CSE7761_PERIOD_MILLI
    #define PROFILE_DEFAULT_CSE7761_PERIOD_MILLI 500    
#endif

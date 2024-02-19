#pragma once
#include <Arduino.h>

class Module
{
public:

    static constexpr const char* DEFAULT_NAME = "Unknown room";

    static void loadConfig();

    static String getName();

    static void setName(const char* name); 

    static uint32_t getChipId();

    static void getResetGpioConfig(uint8_t& pinKey, bool& keyActiveHigh, bool& keyPullEnabled);

    static void setResetGpioConfig(uint8_t pinKey, bool keyActiveHigh, bool keyPullEnabled);

    static uint8_t getLedGpioConfig();

    static void setLedGpioConfig(uint8_t pinLed);

    static void process();

    static void reboot();

    static bool isRebootRequested();

private:

    static constexpr uint32_t RESET_HOLD_TIME_MILLI = 5000;
    static constexpr uint32_t REBOOT_TIME_MILLI = 3000;

    Module();

    static inline Module& getInstance()
    {
        static Module module;
        return module;
    }

    void initPins();

    String m_name;
    uint8_t m_pinKeyReset;
    bool m_pinKeyResetActiveHigh;
    bool m_pinKeyResetPullEnabled;
    bool m_lastKeyResetState;
    uint64_t m_lastKeyResetChangeTime;
    uint8_t m_pinLed;
    bool m_reboot;
    uint64_t m_rebootTimeout;
    uint64_t m_lastLedChangeTime;
};

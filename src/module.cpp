#include "module.h"
#include "config.h"
#include "log.h"
#include "profiles.h"
#include "time.h"
#include "http_server.h"

Module::Module() :
    m_name(DEFAULT_NAME),
    m_pinKeyReset(PROFILE_DEFAULT_PIN_KEY_RESET),
    m_pinKeyResetActiveHigh(PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH),
    m_pinKeyResetPullEnabled(PROFILE_DEFAULT_KEY_PULL_ACTIVE),
    m_lastKeyResetState(false),
    m_lastKeyResetChangeTime(0),
    m_pinLed(PROFILE_DEFAULT_PIN_LED),
    m_reboot(false),
    m_rebootTimeout(0),
    m_lastLedChangeTime(0)
{
    initPins();
}

void Module::loadConfig()
{
    Module& inst = getInstance();
    inst.m_name = Config::getString("module/name", DEFAULT_NAME);
    inst.m_pinKeyReset = Config::getInt("gpio/key_reset", PROFILE_DEFAULT_PIN_KEY_RESET);
    inst.m_pinKeyResetActiveHigh = Config::getBool("gpio/key_reset_active_high", PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH);
    inst.m_pinKeyResetPullEnabled = Config::getBool("gpio/key_reset_pull_enabled", PROFILE_DEFAULT_KEY_PULL_ACTIVE);
    inst.m_pinLed = Config::getInt("gpio/led", PROFILE_DEFAULT_PIN_LED);
    inst.initPins();
    Log::info("Module", "Configuration loaded, name=\"%s\"", inst.m_name.c_str());
}

String Module::getName()
{
    return getInstance().m_name;
}

void Module::setName(const char* name)
{
    Log::info("Module", "Module name set to \"%s\"", name);
    Config::setString("module/name", String(name));
    Config::flush();
    getInstance().m_name = String(name);
}

uint32_t Module::getChipId()
{
#ifdef ESP32
    uint32_t chipId = 0;
	for(int i=0; i<17; i=i+8) 
    {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
    return chipId;
#else
    return ESP.getChipId();
#endif
}

void Module::getResetGpioConfig(uint8_t& pinKey, bool& keyActiveHigh, bool& keyPullEnabled)
{
    Module& inst = getInstance();
    pinKey = inst.m_pinKeyReset;
    keyActiveHigh = inst.m_pinKeyResetActiveHigh;
    keyPullEnabled = inst.m_pinKeyResetPullEnabled;
}

void Module::setResetGpioConfig(uint8_t pinKey, bool keyActiveHigh, bool keyPullEnabled)
{
    Module& inst = getInstance();
    inst.m_pinKeyReset = pinKey;
    Config::setInt("gpio/key_reset", inst.m_pinKeyReset);
    inst.m_pinKeyResetActiveHigh = keyActiveHigh;
    Config::setBool("gpio/key_reset_active_high", inst.m_pinKeyResetActiveHigh);
    inst.m_pinKeyResetPullEnabled = keyPullEnabled;
    Config::setBool("gpio/key_reset_pull_enabled", inst.m_pinKeyResetPullEnabled);
    Config::flush();
    inst.initPins();
    Log::info("Module", "Reset key configuration, pin = %d", pinKey);
}

uint8_t Module::getLedGpioConfig()
{
    return getInstance().m_pinLed;
}

void Module::setLedGpioConfig(uint8_t pinLed)
{
    Module& inst = getInstance();
    getInstance().m_pinLed = pinLed;
    Config::setInt("gpio/led", inst.m_pinLed);
    Config::flush();
    inst.initPins();
    Log::info("Module", "LED configuration, pin = %d", pinLed);
}

void Module::initPins()
{
    uint8_t pull = INPUT_PULLUP;
#ifdef ESP32
    if (m_pinKeyResetActiveHigh)
        pull = INPUT_PULLDOWN;
#else
    // No pulldown for ESP8266
    if (m_pinKeyResetActiveHigh)
        pull = 0;
#endif
    if (!m_pinKeyResetPullEnabled)
        pull = 0;
    pinMode(m_pinKeyReset, INPUT | pull);
    pinMode(m_pinLed, OUTPUT);
}

void Module::reboot()
{
    Log::info("Module", "Reboot request - preparing reboot in %d ms", REBOOT_TIME_MILLI);
    getInstance().m_reboot = true;
    getInstance().m_rebootTimeout = Time::nowRelativeMilli() + REBOOT_TIME_MILLI;
}

bool Module::isRebootRequested()
{
    return getInstance().m_reboot;
}

void Module::process()
{
    Module& inst = getInstance();
    bool isKeyResetActive = digitalRead(inst.m_pinKeyReset) == HIGH;
    if (!inst.m_pinKeyResetActiveHigh)
        isKeyResetActive = !isKeyResetActive;
    uint64_t now = Time::nowRelativeMilli();
    if ((isKeyResetActive != inst.m_lastKeyResetState) && (now >= inst.m_lastKeyResetChangeTime + 100))
    {
        inst.m_lastKeyResetState = isKeyResetActive;
        inst.m_lastKeyResetChangeTime = now;
        Log::debug("Module", "Reset key state changed, active=%d", isKeyResetActive);
    }
    if ((isKeyResetActive) && !inst.m_reboot && (now >= inst.m_lastKeyResetChangeTime + RESET_HOLD_TIME_MILLI))
    {
        Log::info("Module", "Reset hold detected, clearing configuration and rebooting...");
        Config::clearAll();
        reboot();
    }
    // Reboot check
    if ((inst.m_reboot) && (Time::nowRelativeMilli() >= inst.m_rebootTimeout))
    {
        Log::info("Module", "Reboot request - rebooting");
        delay(1000);
        ESP.restart();
    }
    uint32_t blinkPeriod = 2000;
    if (HttpServer::isApMode())
        blinkPeriod = 500;
    if (now >= inst.m_lastLedChangeTime + blinkPeriod)
    {
        inst.m_lastLedChangeTime = now;
        if (digitalRead(inst.m_pinLed) == HIGH)
            digitalWrite(inst.m_pinLed, LOW);
        else
            digitalWrite(inst.m_pinLed, HIGH);
    }
}
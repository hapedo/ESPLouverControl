#include "module.h"
#include "config.h"
#include "log.h"
#include "profiles.h"
#include "time.h"

Module::Module() :
    m_name(DEFAULT_NAME),
    m_pinKeyReset(PROFILE_DEFAULT_PIN_KEY_RESET),
    m_pinKeyResetActiveHigh(PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH),
    m_lastKeyResetState(false),
    m_lastKeyResetChangeTime(0),
    m_reboot(false),
    m_rebootTimeout(0)
{
    initPins();
}

void Module::loadConfig()
{
    Module& inst = getInstance();
    inst.m_name = Config::getString("module/name", DEFAULT_NAME);
    inst.m_pinKeyReset = Config::getInt("gpio/key_reset", PROFILE_DEFAULT_PIN_KEY_RESET);
    inst.m_pinKeyResetActiveHigh = Config::getBool("gpio/key_reset_active_high", PROFILE_DEFAULT_PIN_KEY_RESET_ACTIVE_HIGH);
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

void Module::getResetGpioConfig(uint8_t& pinKey, bool& keyActiveHigh)
{
    Module& inst = getInstance();
    pinKey = inst.m_pinKeyReset;
    keyActiveHigh = inst.m_pinKeyResetActiveHigh;
}

void Module::setResetGpioConfig(uint8_t pinKey, bool keyActiveHigh)
{
    Module& inst = getInstance();
    inst.m_pinKeyReset = pinKey;
    Config::setInt("gpio/key_reset", inst.m_pinKeyReset);
    inst.m_pinKeyResetActiveHigh = keyActiveHigh;
    Config::setBool("gpio/key_reset_active_high", inst.m_pinKeyResetActiveHigh);
    Config::flush();
    inst.initPins();
    Log::info("Module", "Reset key configuration, pin = %d", pinKey);
}

void Module::initPins()
{
    pinMode(m_pinKeyReset, INPUT);
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
    if ((isKeyResetActive) && (now >= inst.m_lastKeyResetChangeTime + RESET_HOLD_TIME_MILLI))
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
}
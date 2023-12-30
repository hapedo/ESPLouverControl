#include "config.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <StreamUtils.h>
#include "log.h"

Config::Config() :
    m_json(MAX_JSON_SIZE)
{
    EEPROM.begin(MAX_JSON_SIZE);
    Log::info("Config", "Trying to read configuration file");
    EepromStream eepromStream(0, MAX_JSON_SIZE);
    DeserializationError error = deserializeJson(m_json, eepromStream);
    if (error) 
    {
        Log::error("Config", "Error while parsing JSON: %s", error.c_str());
    }
}

void Config::flush()
{
    Log::info("Config", "Trying to write configuration file");
    EepromStream eepromStream(0, MAX_JSON_SIZE);
    if (serializeJson(getInstance().m_json, eepromStream) == 0) 
    {
        Log::error("Config", "Failed to write configuration file");
    }
    eepromStream.flush();
}

void Config::setInt(const char* key, int value)
{
    getInstance().m_json[key] = value;
    Log::debug("Config", "Set key %s to %d", key, value);
}

void Config::setString(const char* key, String value)
{
    getInstance().m_json[key] = value;
    Log::debug("Config", "Set key %s to \"%s\"", key, value.c_str());
}

void Config::setFloat(const char* key, float value)
{
    getInstance().m_json[key] = value;
    Log::debug("Config", "Set key %s to %f", key, value);
}

void Config::setBool(const char* key, bool value)
{
    getInstance().m_json[key] = value;
    Log::debug("Config", "Set key %s to %d", key, value);
}

int Config::getInt(const char* key, int defaultValue)
{
    int value = defaultValue;
    if (getInstance().m_json.containsKey(key))
        value = getInstance().m_json[key].as<int>();
    Log::debug("Config", "Key %s=%d", key, value);
    return value;
}

String Config::getString(const char* key, String defaultValue)
{
    String value = defaultValue;
    if (getInstance().m_json.containsKey(key))
        value = getInstance().m_json[key].as<String>();
    Log::debug("Config", "Key %s=%s", key, value.c_str());
    return value;
}

float Config::getFloat(const char* key, float defaultValue)
{
    float value = defaultValue;
    if (getInstance().m_json.containsKey(key))
        value = getInstance().m_json[key].as<float>();
    Log::debug("Config", "Key %s=%f", key, value);
    return value;
}

bool Config::getBool(const char* key, bool defaultValue)
{
    bool value = defaultValue;
    if (getInstance().m_json.containsKey(key))
        value = getInstance().m_json[key].as<bool>();
    Log::debug("Config", "Key %s=%d", key, value);
    return value;
}
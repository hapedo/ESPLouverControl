#include "log.h"
#include "time.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include <TelnetPrint.h>
#include "config.h"

Log::Log() :
    m_telnetInitialized(false),
    m_telnetEnabled(false),
    m_logLevel(INFO),
    m_json(MAX_JSON_SIZE)
{
    Serial.begin(115200);
}

void Log::loadConfig()
{
    getInstance().m_telnetEnabled = Config::getBool("network/telnet_logging", false);
    getInstance().setLoggingLevelOverridePrivate(Config::getString("module/logging_level_override", "{}"), false);
    info("Log", "Telnet logging enabled = %d", getInstance().m_telnetEnabled);
}

bool Log::getTelnetLoggingEnabled()
{
    return getInstance().m_telnetEnabled;
}

void Log::setTelnetLoggingEnabled(bool enabled)
{
    info("Log", "Set telnet logging enabled to %d", enabled);
    Config::setBool("network/telnet_logging", enabled);
    Config::flush();
    getInstance().m_telnetEnabled = enabled;
}

String Log::getLoggingLevelOverride()
{
    Log& inst = getInstance();
    String result = "{";
    result = result + "\";*\":" + String(inst.m_logLevel);
    if (inst.m_levelOverride.size())
        result = result + ",";
    for(size_t i = 0; i < inst.m_levelOverride.size(); i++)
    {
        result = result + "\"" + String(inst.m_levelOverride[i].module) + "\":" + String(inst.m_levelOverride[i].maximalLevel); 
        if (i < (inst.m_levelOverride.size() - 1))
            result = result + ",";
    }
    result = result + "}";
    return result;
}

void Log::setLoggingLevelOverride(String config)
{
    getInstance().setLoggingLevelOverridePrivate(config, true);
}

void Log::setLoggingLevelOverridePrivate(String config, bool save)
{
    DeserializationError error = deserializeJson(m_json, config);
    if (error) 
    {
        Log::error("Log", "Error while parsing config JSON: %s", error.c_str());
    }
    else
    {
        m_levelOverride.clear();
        for (JsonPair elem : m_json.as<JsonObject>()) 
        {
            LevelOverride over;
            if (elem.key().size() <= MODULE_NAME_LENGTH)
            {
                strcpy(over.module, elem.key().c_str());
                over.maximalLevel = (Level)elem.value().as<int>();
                if (strcmp(over.module, "*") == 0)
                {
                    m_logLevel = over.maximalLevel;
                    Log::info("Log", "Set default level to %d", m_logLevel);
                }
                else
                {
                    m_levelOverride.push_back(over);
                    Log::info("Log", "Set maximal level for module %s to %d", over.module, over.maximalLevel);
                }
            }
        }
        if (save)
        {
            Config::setString("module/logging_level_override", config);
            Config::flush();
        }
    }
}

Log::LevelOverride* Log::getOverride(const char* module)
{
    for(size_t i = 0; i < m_levelOverride.size(); i++)
    {
        if (strcmp(module, m_levelOverride[i].module) == 0)
            return &m_levelOverride[i];
    }
    return nullptr;
}

void Log::performLog(Level level, const char* module, const char* format, va_list arg)
{
	s_logMsg[0] = 0;

    LevelOverride* override = getOverride(module);
    if (override && (override->maximalLevel < level))
        return;
    if ((override == nullptr) && (m_logLevel < level))
        return;

    int status = WiFi.status();

    if (status != WL_CONNECTED)
    {
        // Reset init flag
        m_telnetInitialized = false;
    }

    if (!m_telnetInitialized && (status == WL_CONNECTED))
    {
        // Initialize telnet on wifi connect
        m_telnetInitialized = true;
        TelnetPrint.begin();
    }

    strcat(s_logMsg, Time::getTimeLog().c_str());
    size_t len = strlen(s_logMsg);
    if (len > TIME_LENGTH)
    {
        s_logMsg[TIME_LENGTH] = 0;
    }
    else
    {
        for(size_t i = len; i < TIME_LENGTH; i++)
            strcat(s_logMsg, " ");
    }

    switch(level)
    {
    case ERROR:
        strcat(s_logMsg, "ERR  ");
        break;
    case WARNING:
    	strcat(s_logMsg, "WARN ");
        break;
    case INFO:
    	strcat(s_logMsg, "INFO ");
        break;
    case DEBUG:
    	strcat(s_logMsg, "DEBG ");
        break;
    case VERBOSE:
    	strcat(s_logMsg, "VERB ");
        break;
    }

    strcat(s_logMsg, module);

    len = strlen(s_logMsg);
    if (len > HEADER_LENGTH)
    {
        s_logMsg[HEADER_LENGTH] = 0;
    }
    else
    {
        for(size_t i = len; i < HEADER_LENGTH; i++)
            strcat(s_logMsg, " ");
    }
    len = HEADER_LENGTH;

    vsnprintf(s_logMsg + len, sizeof(s_logMsg) - len - 1, format, arg);
    len = strlen(s_logMsg);
    if (len >= sizeof(s_logMsg) - 2)
    	len -= 2;
    s_logMsg[len] = '\r';
    s_logMsg[len+1] = '\n';
    s_logMsg[len+2] = 0;
    len += 2;
    Serial.print(s_logMsg);
    if (m_telnetEnabled)
        TelnetPrint.print(s_logMsg);
}

void Log::error(const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(ERROR, module, format, args);
    va_end(args);
}

void Log::warning(const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(WARNING, module, format, args);
    va_end(args);
}

void Log::debug(const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(DEBUG, module, format, args);
    va_end(args);
}

void Log::info(const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(INFO, module, format, args);
    va_end(args);
}

void Log::verbose(const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(VERBOSE, module, format, args);
    va_end(args);
}

void Log::log(Level level, const char* module, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    getInstance().performLog(level, module, format, args);
    va_end(args);
}

#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>

class Log
{
public:

    enum Level
    {
        ERROR = 0,
        WARNING,
        INFO,
        DEBUG, 
        VERBOSE
    };

    static void loadConfig();

    static bool getTelnetLoggingEnabled();

    static void setTelnetLoggingEnabled(bool enabled);

    static String getLoggingLevelOverride();

    static void setLoggingLevelOverride(String config);

    static void error(const char* module, const char* format, ...);

    static void warning(const char* module, const char* format, ...);

    static void debug(const char* module, const char* format, ...);

    static void info(const char* module, const char* format, ...);

    static void verbose(const char* module, const char* format, ...);

    static void log(Level level, const char* module, const char* format, ...);

private:

    Log();

    static inline Log& getInstance()
    {
        static Log log;
        return log;
    }

    static constexpr size_t MAX_JSON_SIZE = 1024;
    static constexpr size_t LOG_BUFFER_SIZE = 512;
    static constexpr size_t TIME_LENGTH = 20;
    static constexpr size_t LEVEL_LENGTH = 5;
    static constexpr size_t MODULE_NAME_LENGTH = 10;
    static constexpr size_t HEADER_LENGTH = (TIME_LENGTH + LEVEL_LENGTH + MODULE_NAME_LENGTH);

    struct LevelOverride
    {
        char module[MODULE_NAME_LENGTH + 1];
        Level maximalLevel;
    };

    void setLoggingLevelOverridePrivate(String config, bool save);

    LevelOverride* getOverride(const char* module);

    void performLog(Level level, const char* module, const char* format, va_list arg);

    bool m_telnetInitialized;
    bool m_telnetEnabled;
    Level m_logLevel;
    char s_logMsg[LOG_BUFFER_SIZE];
    ::std::vector<LevelOverride> m_levelOverride;
    DynamicJsonDocument m_json;
};
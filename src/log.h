#pragma once
#include <stdlib.h>
#include <stdarg.h>

class Log
{
public:

    enum Level
    {
        ERROR = 0,
        WARNING,
        DEBUG, 
        INFO,
        VERBOSE
    };

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

    static constexpr size_t LOG_BUFFER_SIZE = 512;
    static constexpr size_t TIME_LENGTH = 20;
    static constexpr size_t LEVEL_LENGTH = 5;
    static constexpr size_t MODULE_NAME_LENGTH = 10;
    static constexpr size_t HEADER_LENGTH = (TIME_LENGTH + LEVEL_LENGTH + MODULE_NAME_LENGTH);

    void performLog(Level level, const char* module, const char* format, va_list arg);

    char s_logMsg[LOG_BUFFER_SIZE];
};
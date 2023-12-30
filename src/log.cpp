#include "log.h"
#include "time.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>

Log::Log()
{
    Serial.begin(115200);
}

void Log::performLog(Level level, const char* module, const char* format, va_list arg)
{
	s_logMsg[0] = 0;

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

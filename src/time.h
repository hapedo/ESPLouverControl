#pragma once
#include <Arduino.h>
#include <stdint.h>

class Time
{
public:

    static String getTimeLog();

    static uint64_t nowRelativeMilli();

    static void process();

private:

    Time();

    static inline Time& getInstance()
    {
        static Time time;
        return time;
    }
};
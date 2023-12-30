#include "time.h"
#include <ESPDateTime.h>

using namespace std;

Time::Time()
{
    DateTime.setTimeZone("GMT+1");
    DateTime.setServer("europe.pool.ntp.org");
}

uint64_t Time::nowRelativeMilli()
{
    return millis();
}

String Time::getTimeLog()
{
    return DateTime.toString();
}
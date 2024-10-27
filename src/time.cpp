#include "time.h"
#include <ESPDateTime.h>
#include "http_server.h"

using namespace std;

Time::Time()
{
    DateTime.setTimeZone("GMT+1");
    DateTime.setServer("europe.pool.ntp.org");
}

uint64_t Time::nowRelativeMilli()
{
    static uint64_t epoch = 0;
    static uint32_t lastMillis = 0;
    uint32_t now = millis();
    if (now < lastMillis)
    {
        epoch++;
    }
    lastMillis = now;
    return (epoch << 32) | now;
}

String Time::getTimeLog()
{
    return DateTime.toString();
}

void Time::process()
{
    static bool lastWifiConnected = false;
    static uint64_t timer = 0;
    uint64_t now = nowRelativeMilli();

    // Lets check wifi status
    if (timer < now)
    {
        timer = now + 1000;
        bool isWifiConnected = HttpServer::isWifiConnected();
        if ((lastWifiConnected != isWifiConnected) && (isWifiConnected))
        {
            // Wifi gets connected - init NTP
            DateTime.begin();
        }
        lastWifiConnected = isWifiConnected;
    }
}
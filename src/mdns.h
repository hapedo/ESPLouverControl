#pragma once
#include <Arduino.h>

class Mdns
{
public:

    static constexpr const char* DEFAULT_HOST = "louver";

    static void loadConfig();

    static void init();

    static void configure(String host);

    static String getHost();

    static void process();

private:

    Mdns();

    static inline Mdns& getInstance()
    {
        static Mdns dns;
        return dns;
    }

    String m_host;

};
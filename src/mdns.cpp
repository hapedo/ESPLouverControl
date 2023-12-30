#include "mdns.h"
#ifdef ESP32
#include <ESPmDNS.h>
#else
#include <ESP8266mDNS.h>
#endif
#include "log.h"
#include "config.h"

Mdns::Mdns() :
    m_host(DEFAULT_HOST)
{
}

void Mdns::loadConfig()
{
    getInstance().m_host = Config::getString("network/mdns_host", DEFAULT_HOST);
    Log::info("MDNS", "Configuration loaded, host=\"%s\"", getInstance().m_host.c_str());
}

void Mdns::init()
{
    Log::info("MDNS", "Configuring MDNS to host=\"%s\"", getInstance().m_host.c_str());
    if (!MDNS.begin(getInstance().m_host))
        Log::error("MDNS", "Unable to configure, host=\"%s\"", getInstance().m_host.c_str());
}

void Mdns::configure(String host)
{
    getInstance().m_host = host;
    Log::info("MDNS", "Host set to \"%s\"", getInstance().m_host.c_str());
    Config::setString("network/mdns_host", host);
    init();    
}

String Mdns::getHost()
{
    return getInstance().m_host;
}

void Mdns::process()
{
#ifndef ESP32
    MDNS.update();
#endif
}
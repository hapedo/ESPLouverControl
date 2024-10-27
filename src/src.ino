#include "log.h"
#include "time.h"
#include "louver.h"
#include "http_server.h"
#include "module.h"
#include "mdns.h"
#include "config.h"
#include "mqtt.h"
#include "power_meas.h"

void setup() {
    // put your setup code here, to run once:
    Log::loadConfig();
    Log::info("main", "Louver control, firmware version %s", Config::VERSION);
    Module::loadConfig();
    Louver::loadConfig();
    HttpServer::loadConfig();
    HttpServer::init();
    Mdns::loadConfig();
    Mdns::init();
    Mqtt::loadConfig();
    PowerMeas::loadConfig();
}

void loop() 
{
    Time::process();
    Module::process();
    Louver::process();
    HttpServer::process();
    Mdns::process();
    Mqtt::process();
    PowerMeas::process();
}
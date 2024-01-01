#include <Preferences.h>
#include "log.h"
#include "louver.h"
#include "http_server.h"
#include "module.h"
#include "mdns.h"
#include "config.h"

void setup() {
    // put your setup code here, to run once:
    Log::info("main", "Louver control, firmware version %s", Config::VERSION);
    Module::loadConfig();
    Louver::loadConfig();
    HttpServer::loadConfig();
    HttpServer::init();
    Mdns::loadConfig();
    Mdns::init();
}

void loop() 
{
    Louver::process();
    HttpServer::process();
    Mdns::process();
}
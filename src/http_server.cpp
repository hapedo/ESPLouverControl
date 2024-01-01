#include "http_server.h"
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include "log.h"
#include "http_pages.h"
#include "louver.h"
#include "time.h"
#include "config.h"
#include "mdns.h"
#include "module.h"

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}
    bool canHandle(AsyncWebServerRequest *request)
    {
        Log::debug("HTTP", "Captive server request, can handle");
        request->addInterestingHeader("ANY");
        return true;
    }
    void handleRequest(AsyncWebServerRequest *request)
    {
        Log::debug("HTTP", "Captive server request, /");
        request->send_P(200, "text/html", getHttpIndex());
    }
};

HttpServer::HttpServer() :
    m_wifiConfig(WIFI_CONF_AP),
    m_wifiClientBehavior(WIFI_CLIENT_BEH_1MCLIENT_5MAP),
    m_wifiClientModeSwap(false),
    m_server(80),
    m_wifiSsidAp(DEFAULT_SSID_AP),
    m_wifiPasswordAp(DEFAULT_PASSWORD_AP),
    m_wifiSsid(DEFAULT_SSID),
    m_wifiPassword(DEFAULT_PASSWORD),
    m_lastWifiStatus(WL_CONNECTED)
{

}

void HttpServer::loadConfig()
{
    getInstance().loadConfigPrivate();
}

void HttpServer::init()
{
    getInstance().initPrivate();
}

void HttpServer::setWifiConfig(WifiConfig config)
{
    bool changed = config != getInstance().m_wifiConfig;
    getInstance().m_wifiConfig = config;
    Config::setInt("network/wifi_mode", config);
    Config::setBool("network/wifi_client_mode_swap", false);
    Log::info("HTTP", "WIFI mode set to %d", config);
    Config::flush();
}

void HttpServer::setWifiClientBehavior(WifiClientBehavior behavior)
{
    getInstance().m_wifiClientBehavior = behavior;
    Config::setInt("network/wifi_client_behavior", behavior);
    Log::info("HTTP", "WIFI client behavior set to %d", behavior);
    Config::flush();
}

void HttpServer::configureAP(const char* ssid, const char* password)
{
    getInstance().m_wifiSsidAp = String(ssid);
    Config::setString("network/ssid_ap", getInstance().m_wifiSsidAp);
    getInstance().m_wifiPasswordAp = String(password);
    Config::setString("network/password_ap", getInstance().m_wifiPasswordAp);
    Config::flush();
}

void HttpServer::configureClient(const char* ssid, const char* password)
{
    getInstance().m_wifiSsid = String(ssid);
    Config::setString("network/ssid", getInstance().m_wifiSsid);
    getInstance().m_wifiPassword = String(password);
    Config::setString("network/password", getInstance().m_wifiPassword);
    Config::flush();
}

void HttpServer::getConfig(WifiConfig& wifiConfig, String& ssidAp, String& passAp, String& ssidClient, String& passClient)
{
    HttpServer& inst = getInstance();
    wifiConfig = inst.m_wifiConfig;
    ssidAp = inst.m_wifiSsidAp;
    passAp = inst.m_wifiPasswordAp;
    ssidClient = inst.m_wifiSsid;
    passClient = inst.m_wifiPassword;
}

void HttpServer::initPrivate()
{
    if (m_wifiConfig == WIFI_CONF_CLIENT)
    {
        if (m_wifiClientModeSwap)
        {
            Log::info("HTTP", "WIFI mode swap requested (client mode to AP mode)");
            initAsAccessPoint();
            m_dnsServer.start(53, "*", WiFi.softAPIP());
            // Set timeout to switch back to client
            m_wifiClientSwapTimeout = Time::nowRelativeMilli() + 5000 * 60;
        }
        else
            initAsClient();
    }
    else
    {
        initAsAccessPoint();
        m_dnsServer.start(53, "*", WiFi.softAPIP());
    }
    m_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpIndex(), defaultProcessor);
        Log::debug("HTTP", "GET request, /");
    });
    m_server.on("/default.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/css", getHttpDefaultCss());
        Log::debug("HTTP", "GET request, /default.css");
    });
    m_server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpSettings(), defaultProcessor);
        Log::debug("HTTP", "GET request, /settings");
    });
    m_server.on("/moduleConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpModuleConfig(), moduleConfigProcessor);
        Log::debug("HTTP", "GET request, /moduleConfig");
    });
    m_server.on("/moduleConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /moduleConfigSave");
        String value;
        String name = Module::getName();
        if (request->hasParam("name", true))
        {
            name = request->getParam("name", true)->value();
        }
        Module::setName(name.c_str());
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
    });
    m_server.on("/gpioConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpGpioConfig(), gpioConfigProcessor);
        Log::debug("HTTP", "GET request, /gpioConfig");
    });
    m_server.on("/gpioConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /gpioConfigSave");
        uint8_t pinKeyUp;
        uint8_t relayUp;
        bool highKeyUp;
        bool highRelayUp;
        uint8_t pinKeyDown;
        uint8_t relayDown;
        bool highKeyDown;
        bool highRelayDown;
        Louver::getGpioConfig(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp);
        Louver::getGpioConfig(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown);
        uint8_t pinKeyReset;
        bool highKeyReset;
        Module::getResetGpioConfig(pinKeyReset, highKeyReset);
        String value;
        if (request->hasParam("downGpio", true))
        {
            pinKeyDown = request->getParam("downGpio", true)->value().toInt();
        }
        if (request->hasParam("downInvert", true))
        {
            if (request->getParam("downInvert", true)->value() == "1")
                highKeyDown = false;
            else
                highKeyDown = true;
        }
        if (request->hasParam("upGpio", true))
        {
            pinKeyUp = request->getParam("upGpio", true)->value().toInt();
        }
        if (request->hasParam("upInvert", true))
        {
            if (request->getParam("upInvert", true)->value() == "1")
                highKeyUp = false;
            else
                highKeyUp = true;
        }
        if (request->hasParam("rdownGpio", true))
        {
            relayDown = request->getParam("rdownGpio", true)->value().toInt();
        }
        if (request->hasParam("rdownInvert", true))
        {
            if (request->getParam("rdownInvert", true)->value() == "1")
                highRelayDown = false;
            else
                highRelayDown = true;
        }
        if (request->hasParam("rupGpio", true))
        {
            relayUp = request->getParam("rupGpio", true)->value().toInt();
        }
        if (request->hasParam("rupInvert", true))
        {
            if (request->getParam("rupInvert", true)->value() == "1")
                highRelayUp = false;
            else
                highRelayUp = true;
        }
        if (request->hasParam("resetGpio", true))
        {
            pinKeyReset = request->getParam("resetGpio", true)->value().toInt();
        }
        if (request->hasParam("resetInvert", true))
        {
            if (request->getParam("resetInvert", true)->value() == "1")
                highKeyReset = false;
            else
                highKeyReset = true;
        }
        Louver::configureGpio(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp);
        Louver::configureGpio(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown);
        Module::setResetGpioConfig(pinKeyReset, highKeyReset);
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
    });
    m_server.on("/timingConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpTimingConfig(), timingConfigProcessor);
        Log::debug("HTTP", "GET request, /timingConfig");
    });
    m_server.on("/timingConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /timingConfigSave");
        String value;
        float timeFullOpenSecs;
        float timeFullCloseSecs;
        float timeOpenLamellasSecs;
        float shortMovementSecs;
        Louver::getTimesConfig(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
        if (request->hasParam("timeFullOpen", true))
        {
            timeFullOpenSecs = request->getParam("timeFullOpen", true)->value().toFloat();
        }
        if (request->hasParam("timeFullClose", true))
        {
            timeFullCloseSecs = request->getParam("timeFullClose", true)->value().toFloat();
        }
        if (request->hasParam("timeShort", true))
        {
            shortMovementSecs = request->getParam("timeShort", true)->value().toFloat();
        }
        if (request->hasParam("timeOpenLamellas", true))
        {
            timeOpenLamellasSecs = request->getParam("timeOpenLamellas", true)->value().toFloat();
        }
        Louver::configureTimes(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
    });
    m_server.on("/networkConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpNetworkConfig(), networkConfigProcessor);
        Log::debug("HTTP", "GET request, /networkConfig");
    });
    m_server.on("/networkConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /networkConfigSave");
        String value;
        WifiConfig oldWifiConfig;
        WifiConfig wifiConfig;
        String ssidAp;
        String passAp;
        String ssid;
        String pass;
        getInstance().getConfig(oldWifiConfig, ssidAp, passAp, ssid, pass);
        wifiConfig = oldWifiConfig;
        String mdnsHost = Mdns::getHost();
        WifiClientBehavior clientBehavior = getInstance().m_wifiClientBehavior;
        if (request->hasParam("wifiMode", true)) 
        {
            value = request->getParam("wifiMode", true)->value();
            if (value == "client")
                wifiConfig = WIFI_CONF_CLIENT;
            else
                wifiConfig = WIFI_CONF_AP;
        }
        if (request->hasParam("apSSID", true)) 
        {
            ssidAp = request->getParam("apSSID", true)->value();
        }
        if (request->hasParam("apPass", true)) 
        {
            passAp = request->getParam("apPass", true)->value();
        }
        if (request->hasParam("clientSSID", true)) 
        {
            ssid = request->getParam("clientSSID", true)->value();
        }
        if (request->hasParam("clientPass", true)) 
        {
            pass = request->getParam("clientPass", true)->value();
        }
        if (request->hasParam("mDNSHost", true)) 
        {
            mdnsHost = request->getParam("mDNSHost", true)->value();
        }
        if (request->hasParam("clientBehavior", true))
        {
            if (request->getParam("clientBehavior", true)->value() == "1")
                clientBehavior = WIFI_CLIENT_BEH_STILL_CLIENT;
            else
                clientBehavior = WIFI_CLIENT_BEH_1MCLIENT_5MAP;
        }
        Module::reboot();
        configureAP(ssidAp.c_str(), passAp.c_str());
        configureClient(ssid.c_str(), pass.c_str());
        setWifiConfig(wifiConfig);
        setWifiClientBehavior(clientBehavior);
        Mdns::configure(mdnsHost);
        Log::info("HTTP", "Network config changed - requesting reboot");    
        request->send_P(200, "text/html", getHttpNetworkConfigSaved(), defaultProcessor);
    });
    m_server.on("/portal", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpIndex(), defaultProcessor);
        Log::debug("HTTP", "GET request, /portal");
    });
    // Send a GET request to <ESP_IP>/update?state=<inputMessage>
    m_server.on("/command", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String buttonId;
        // GET input1 value on <ESP_IP>/update?state=<inputMessage>
        Log::debug("HTTP", "GET request, /command");
        if (request->hasParam("button")) {
            buttonId = request->getParam("button")->value();
            Log::info("HTTP", "Command from button %s received", buttonId.c_str());
            if (buttonId == "shortUp")
                Louver::shortOpen(Louver::getShortMovementSecs());
            else if (buttonId == "shortDown")
                Louver::shortClose(Louver::getShortMovementSecs());
            else if (buttonId == "fullOpen")
                Louver::fullOpen();
            else if (buttonId == "fullClose")
                Louver::fullClose();
            else if (buttonId == "fullCloseAndOpenLamellas")
                Louver::fullCloseAndOpenLamellas();
            else if (buttonId == "stop")
                Louver::stop();
        }
        else {
            Log::error("HTTP", "Command received but no button info provided");
        }
        request->send(200, "text/plain", "OK");
    });
    m_server.onNotFound([&](AsyncWebServerRequest *request){
        Log::debug("HTTP", "GET request, not found, redirecting");
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "redirect to captive portal");
        response->addHeader("Location", "/portal");
        request->send(response);
    });
    m_server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    AsyncElegantOTA.begin(&m_server);
/*    AsyncElegantOTA.onStart([]{
        Log::info("HTTP", "Firmware update started");
    });*/
    m_server.begin();
}

void HttpServer::initAsAccessPoint()
{
    //WiFi.mode(WIFI_AP);
    WiFi.enableAP(true);
    delay(100);
    WiFi.softAP(m_wifiSsidAp, m_wifiPasswordAp);
    Log::info("HTTP", "Initializing as AP, IP = %s, SSID = %s", WiFi.softAPIP().toString().c_str(), m_wifiSsidAp.c_str());
    delay(500);
}

void HttpServer::initAsClient()
{
    Log::info("HTTP", "Initializing as client, trying to connect to %s", m_wifiSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_wifiSsid, m_wifiPassword);
}

void HttpServer::loadConfigPrivate()
{
    m_wifiConfig = (WifiConfig)Config::getInt("network/wifi_mode", WIFI_CONF_AP);
    m_wifiClientBehavior = (WifiClientBehavior)Config::getInt("network/wifi_client_behavior", WIFI_CLIENT_BEH_1MCLIENT_5MAP);
    m_wifiClientModeSwap = Config::getBool("network/wifi_client_mode_swap", false);
    m_wifiSsidAp = Config::getString("network/ssid_ap", String(DEFAULT_SSID_AP) + "_" + String(Module::getChipId()));
    m_wifiPasswordAp = Config::getString("network/password_ap", DEFAULT_PASSWORD_AP);
    m_wifiSsid = Config::getString("network/ssid", DEFAULT_SSID);
    m_wifiPassword = Config::getString("network/password", DEFAULT_PASSWORD);
    Log::info("HTTP", "Configuration loaded");
}

String HttpServer::defaultProcessor(const String& var)
{
    if (var == "MODULE_NAME")
        return Module::getName();
    else if (var == "VERSION")
        return String(Config::VERSION);
    return String();
}

String HttpServer::moduleConfigProcessor(const String& var)
{
    if (var == "MODULE_NAME")
        return Module::getName();
    return defaultProcessor(var);
}

String HttpServer::gpioConfigProcessor(const String& var)
{
    uint8_t pinKeyUp;
    uint8_t relayUp;
    bool highKeyUp;
    bool highRelayUp;
    uint8_t pinKeyDown;
    uint8_t relayDown;
    bool highKeyDown;
    bool highRelayDown;
    Louver::getGpioConfig(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp);
    Louver::getGpioConfig(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown);
    uint8_t pinKeyReset;
    bool highKeyReset;
    Module::getResetGpioConfig(pinKeyReset, highKeyReset);
    if (var == "KEY_DOWN_GPIO")
        return String(pinKeyDown);
    if (var == "KEY_UP_GPIO")
        return String(pinKeyUp);
    if (var == "RELAY_DOWN_GPIO")
        return String(relayDown);
    if (var == "RELAY_UP_GPIO")
        return String(relayUp);
    if (var == "KEY_RESET_GPIO")
        return String(pinKeyReset);
    if ((var == "SELECTED_UP_INVERTED_YES") && (!highKeyUp))
        return "selected";
    if ((var == "SELECTED_UP_INVERTED_NO") && (highKeyUp))
        return "selected";
    if ((var == "SELECTED_DOWN_INVERTED_YES") && (!highKeyDown))
        return "selected";
    if ((var == "SELECTED_DOWN_INVERTED_NO") && (highKeyDown))
        return "selected";
    if ((var == "SELECTED_RUP_INVERTED_YES") && (!highRelayUp))
        return "selected";
    if ((var == "SELECTED_RUP_INVERTED_NO") && (highRelayUp))
        return "selected";
    if ((var == "SELECTED_RDOWN_INVERTED_YES") && (!highRelayDown))
        return "selected";
    if ((var == "SELECTED_RDOWN_INVERTED_NO") && (highRelayDown))
        return "selected";
    if ((var == "SELECTED_KEY_RESET_INVERTED_YES") && (!highKeyReset))
        return "selected";
    if ((var == "SELECTED_KEY_RESET_INVERTED_NO") && (highKeyReset))
        return "selected";
    return defaultProcessor(var);
}

String HttpServer::timingConfigProcessor(const String& var)
{
    float timeFullOpenSecs;
    float timeFullCloseSecs;
    float timeOpenLamellasSecs;
    float shortMovementSecs;
    Louver::getTimesConfig(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
    if (var == "TIME_FULL_OPEN")
        return String(timeFullOpenSecs);
    if (var == "TIME_FULL_CLOSE")
        return String(timeFullCloseSecs);
    if (var == "TIME_OPEN_LAMELLAS")
        return String(timeOpenLamellasSecs);
    if (var == "TIME_SHORT")
        return String(shortMovementSecs);
    return defaultProcessor(var);
}

String HttpServer::networkConfigProcessor(const String& var)
{
    if ((var == "SELECTED_WIFI_MODE_AP") && (getInstance().m_wifiConfig == WIFI_CONF_AP))
        return "selected";
    if ((var == "SELECTED_WIFI_MODE_CLIENT") && (getInstance().m_wifiConfig == WIFI_CONF_CLIENT))
        return "selected";
    if (var == "NETWORK_SSID_AP")
        return getInstance().m_wifiSsidAp;
    if (var == "NETWORK_PASS_AP")
        return getInstance().m_wifiPasswordAp;
    if (var == "NETWORK_SSID")
        return getInstance().m_wifiSsid;
    if (var == "NETWORK_PASS")
        return getInstance().m_wifiPassword;
    if (var == "NETWORK_MDNS_HOST")
        return Mdns::getHost();
    if ((var == "SELECTED_CLIENT_BEHAVIOR_0") && (getInstance().m_wifiClientBehavior == WIFI_CLIENT_BEH_1MCLIENT_5MAP))
        return "selected";
    if ((var == "SELECTED_CLIENT_BEHAVIOR_1") && (getInstance().m_wifiClientBehavior == WIFI_CLIENT_BEH_STILL_CLIENT))
        return "selected";
    return defaultProcessor(var);
}

void HttpServer::process()
{
    HttpServer& inst = getInstance();
    int status = WiFi.status();
    if (!Module::isRebootRequested())
    {
        if (status != inst.m_lastWifiStatus)
        {
            if ((inst.m_lastWifiStatus == WL_CONNECTED) && (inst.m_wifiConfig == WIFI_CONF_CLIENT) && (!inst.m_wifiClientModeSwap))
            {
                // Set client mode switch timeout
                Log::info("HTTP", "WIFI not connected, setting mode swap timeout");
                inst.m_wifiClientSwapTimeout = Time::nowRelativeMilli() + 1000 * 60;
            }
            if (status == WL_CONNECTED)
            {
                Log::info("HTTP", "Connected to wifi, IP = %s", WiFi.localIP().toString().c_str());
                Mdns::init();
            }
            else if (status == WL_CONNECT_FAILED)
            {
                Log::error("HTTP", "Connecting to wifi failed");
                inst.m_nextReconnectTime = Time::nowRelativeMilli() + (RECONNECT_PERIOD_SECS * 1000);
            }
            else if (status == WL_NO_SSID_AVAIL)
            {
                Log::error("HTTP", "Trying to connect to SSID = %s, but not available", inst.m_wifiSsid.c_str());
                inst.m_nextReconnectTime = Time::nowRelativeMilli() + (RECONNECT_PERIOD_SECS * 1000);
            }
            else if (status == WL_DISCONNECTED)
            {
                Log::info("HTTP", "Disconnected from wifi");
                inst.m_nextReconnectTime = Time::nowRelativeMilli() + (RECONNECT_PERIOD_SECS * 1000);
            }
            inst.m_lastWifiStatus = status;
        }
        if ((inst.m_wifiConfig == WIFI_CONF_CLIENT) && (status != WL_CONNECTED))
        {
            if ((!inst.m_wifiClientModeSwap) && (Time::nowRelativeMilli() >= inst.m_nextReconnectTime))
            {
                Log::error("HTTP", "Still not connected to wifi, trying to reconnect...");
                inst.m_nextReconnectTime = Time::nowRelativeMilli() + (RECONNECT_PERIOD_SECS * 1000);
                // Reinit client
                inst.initAsClient();
            }
            if ((inst.m_wifiClientBehavior == WIFI_CLIENT_BEH_1MCLIENT_5MAP) && (Time::nowRelativeMilli() >= inst.m_wifiClientSwapTimeout))
            {
                Log::info("HTTP", "WIFI client mode swap timeout");
                if (inst.m_wifiClientModeSwap)
                {
                    Log::info("HTTP", "WIFI client mode swap timeout - swapping back to client mode");
                    Config::setBool("network/wifi_client_mode_swap", false);
                    Config::flush();
                    Module::reboot();
                }
                else
                {
                    Log::info("HTTP", "WIFI client mode swap timeout - swapping to AP mode");
                    Config::setBool("network/wifi_client_mode_swap", true);
                    Config::flush();
                    Module::reboot();
                }
            }
        }
    }
}
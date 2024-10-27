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
#include "mqtt.h"
#include "power_meas.h"

static String htmlEscape(String str)
{
    String result;
    for(size_t i = 0; i < str.length(); i++)
    {
        if (str.charAt(i) == '\"')
            result = result + "&quot;";
        else
            result = result + str.charAt(i);
    }
    return result;
}

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
        request->send_P(200, "text/html", getHttpIndex(), HttpServer::defaultProcessor);
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
    m_hostname("Hostname"),
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

void HttpServer::configureAP(const char* ssid, const char* password, const char* hostname)
{
    getInstance().m_wifiSsidAp = String(ssid);
    Config::setString("network/ssid_ap", getInstance().m_wifiSsidAp);
    getInstance().m_wifiPasswordAp = String(password);
    Config::setString("network/password_ap", getInstance().m_wifiPasswordAp);
    getInstance().m_hostname = String(hostname);
    Config::setString("network/hostname", getInstance().m_hostname);
    Config::flush();
}

void HttpServer::configureClient(const char* ssid, const char* password, const char* hostname)
{
    getInstance().m_wifiSsid = String(ssid);
    Config::setString("network/ssid", getInstance().m_wifiSsid);
    getInstance().m_wifiPassword = String(password);
    Config::setString("network/password", getInstance().m_wifiPassword);
    getInstance().m_hostname = String(hostname);
    Config::setString("network/hostname", getInstance().m_hostname);
    Config::flush();
}

void HttpServer::getConfig(WifiConfig& wifiConfig, String& ssidAp, String& passAp, String& ssidClient, String& passClient, String& hostname)
{
    HttpServer& inst = getInstance();
    wifiConfig = inst.m_wifiConfig;
    ssidAp = inst.m_wifiSsidAp;
    passAp = inst.m_wifiPasswordAp;
    ssidClient = inst.m_wifiSsid;
    passClient = inst.m_wifiPassword;
    hostname = inst.m_hostname;
}

bool HttpServer::isApMode(bool respectClientModeSwap)
{
    HttpServer& inst = getInstance();
    if (respectClientModeSwap)
    {
        return (inst.m_wifiConfig == WIFI_CONF_AP) || ((inst.m_wifiConfig == WIFI_CONF_CLIENT) && (inst.m_wifiClientModeSwap));
    }
    return inst.m_wifiConfig == WIFI_CONF_AP;
}

bool HttpServer::isWifiConnected()
{
    HttpServer& inst = getInstance();
    return (inst.m_wifiConfig == WIFI_CONF_CLIENT) && (inst.m_lastWifiStatus == WL_CONNECTED);
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
    m_server.on("/moduleInfo", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpModuleInfo(), defaultProcessor);
        Log::debug("HTTP", "GET request, /moduleInfo");
    });
    m_server.on("/moduleConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpModuleConfig(), moduleConfigProcessor);
        Log::debug("HTTP", "GET request, /moduleConfig");
    });
    m_server.on("/moduleConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /moduleConfigSave");
        String value;
        String name = Module::getName();
        String logLevelOverride = Log::getLoggingLevelOverride();
        if (request->hasParam("name", true))
        {
            name = request->getParam("name", true)->value();
        }
        if (request->hasParam("logLevelOverride", true))
        {
            logLevelOverride = request->getParam("logLevelOverride", true)->value();
        }
        Module::setName(name.c_str());
        Log::setLoggingLevelOverride(logLevelOverride);
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
        bool pinKeyUpPullEnabled;
        bool pinKeyDownPullEnabled;
        Louver::getGpioConfig(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp, pinKeyUpPullEnabled);
        Louver::getGpioConfig(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown, pinKeyDownPullEnabled);
        uint8_t pinKeyReset;
        bool highKeyReset;
        bool keyResetPullEnabled;
        Module::getResetGpioConfig(pinKeyReset, highKeyReset, keyResetPullEnabled);
        uint8_t pinLed = Module::getLedGpioConfig();
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
        if (request->hasParam("downPull", true))
        {
            if (request->getParam("downPull", true)->value() == "1")
                pinKeyDownPullEnabled = true;
            else
                pinKeyDownPullEnabled = false;
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
        if (request->hasParam("upPull", true))
        {
            if (request->getParam("upPull", true)->value() == "1")
                pinKeyUpPullEnabled = true;
            else
                pinKeyUpPullEnabled = false;
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
        if (request->hasParam("resetPull", true))
        {
            if (request->getParam("resetPull", true)->value() == "1")
                keyResetPullEnabled = true;
            else
                keyResetPullEnabled = false;
        }
        if (request->hasParam("ledGpio", true))
        {
            pinLed = request->getParam("ledGpio", true)->value().toInt();
        }
        Louver::configureGpio(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp, pinKeyUpPullEnabled);
        Louver::configureGpio(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown, pinKeyDownPullEnabled);
        Module::setResetGpioConfig(pinKeyReset, highKeyReset, keyResetPullEnabled);
        Module::setLedGpioConfig(pinLed);
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
    });
    m_server.on("/movementConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpMovementConfig(), movementConfigProcessor);
        Log::debug("HTTP", "GET request, /movementConfig");
    });
    m_server.on("/movementConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /movementConfigSave");
        String value;
        float timeFullOpenSecs;
        float timeFullCloseSecs;
        float timeOpenLamellasSecs;
        float shortMovementSecs;
        Louver::getTimesConfig(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
        bool stopCond1;
        bool stopCond2;
        Louver::getPowerCondStop(stopCond1, stopCond2);
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
        if (request->hasParam("stopOpenOnPowerCond1", true))
        {
            if (request->getParam("stopOpenOnPowerCond1", true)->value() == "1")
                stopCond1 = true;
            else
                stopCond1 = false;
        }
        if (request->hasParam("stopCloseOnPowerCond2", true))
        {
            if (request->getParam("stopCloseOnPowerCond2", true)->value() == "1")
                stopCond2 = true;
            else
                stopCond2 = false;
        }
        Louver::configureTimes(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
        Louver::configurePowerCondStop(stopCond1, stopCond2);
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
        String host;
        getInstance().getConfig(oldWifiConfig, ssidAp, passAp, ssid, pass, host);
        wifiConfig = oldWifiConfig;
        String mdnsHost = Mdns::getHost();
        WifiClientBehavior clientBehavior = getInstance().m_wifiClientBehavior;
        bool telnetLoggingEnabled = Log::getTelnetLoggingEnabled();
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
        if (request->hasParam("Host", true)) 
        {
            host = request->getParam("Host", true)->value();
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
        if (request->hasParam("telnetLoggingEnabled", true))
        {
            telnetLoggingEnabled = request->getParam("telnetLoggingEnabled", true)->value() == "1";
        }
        Module::reboot();
        configureAP(ssidAp.c_str(), passAp.c_str(), host.c_str());
        configureClient(ssid.c_str(), pass.c_str(), host.c_str());
        setWifiConfig(wifiConfig);
        setWifiClientBehavior(clientBehavior);
        Mdns::configure(mdnsHost);
        WiFi.setHostname(host.c_str());
        Log::setTelnetLoggingEnabled(telnetLoggingEnabled);
        Log::info("HTTP", "Network config changed - requesting reboot");    
        request->send_P(200, "text/html", getHttpNetworkConfigSaved(), defaultProcessor);
    });
    m_server.on("/mqttConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpMqttConfig(), mqttConfigProcessor);
        Log::debug("HTTP", "GET request, /mqttConfig");
    });
    m_server.on("/mqttConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /mqttConfigSave");
        String value;
        bool enabled = Mqtt::getEnabled();
        String brokerIp = Mqtt::getBrokerIp();
        uint16_t brokerPort = Mqtt::getBrokerPort();
        String clientId = Mqtt::getClientId();
        String user = Mqtt::getAuthenticationUser();
        String pass = Mqtt::getAuthenticationPassword();
        uint32_t powerMeasPeriod = Mqtt::getPowerPublishPeriod();
        if (request->hasParam("enabled", true))
        {
            if (request->getParam("enabled", true)->value() == "1")
                enabled = true;
            else
                enabled = false;
        }
        if (request->hasParam("clientId", true)) 
        {
            clientId = request->getParam("clientId", true)->value();
        }
        if (request->hasParam("brokerIp", true)) 
        {
            brokerIp = request->getParam("brokerIp", true)->value();
        }
        if (request->hasParam("brokerPort", true)) 
        {
            brokerPort = request->getParam("brokerPort", true)->value().toInt();
        }
        if (request->hasParam("brokerUser", true)) 
        {
            user = request->getParam("brokerUser", true)->value();
        }
        if (request->hasParam("brokerPass", true)) 
        {
            pass = request->getParam("brokerPass", true)->value();
        }        
        if (request->hasParam("powerMeasPeriod", true)) 
        {
            powerMeasPeriod = (uint32_t)request->getParam("powerMeasPeriod", true)->value().toInt();
        }
        Mqtt::setEnabled(enabled);
        Mqtt::setBrokerIp(brokerIp.c_str());
        Mqtt::setBrokerPort(brokerPort);
        Mqtt::setClientId(clientId.c_str());
        Mqtt::setAuthentication(user.c_str(), pass.c_str());
        Mqtt::setPowerPublishPeriod(powerMeasPeriod);
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
    });
    m_server.on("/powerMeasConfig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", getHttpPowerMeasConfig(), powerMeasConfigProcessor);
        Log::debug("HTTP", "GET request, /powerMeasConfig");
    });
    m_server.on("/powerMeasConfigSave", HTTP_POST, [](AsyncWebServerRequest *request){
        Log::debug("HTTP", "POST request, /powerMeasConfigSave");
        String value;
        PowerMeas::DeviceType deviceType = PowerMeas::getActiveDeviceType();
        String bl0939Config = PowerMeas::getConfiguration(PowerMeas::DEV_BL0939);
        String ade7953Config = PowerMeas::getConfiguration(PowerMeas::DEV_ADE7953);
        String cse7761Config = PowerMeas::getConfiguration(PowerMeas::DEV_CSE7761);
        String stopCond1 = PowerMeas::getConditionConfig(0);
        String stopCond2 = PowerMeas::getConditionConfig(1);
        if (request->hasParam("deviceType", true))
        {
            deviceType = (PowerMeas::DeviceType)request->getParam("deviceType", true)->value().toInt();
        }
        if (request->hasParam("bl0939Config", true))
        {
            bl0939Config = request->getParam("bl0939Config", true)->value();
        }
        if (request->hasParam("ade7953Config", true))
        {
            ade7953Config = request->getParam("ade7953Config", true)->value();
        }
        if (request->hasParam("cse7761Config", true))
        {
            cse7761Config = request->getParam("cse7761Config", true)->value();
        }
        if (request->hasParam("powerMeasStopCond1", true))
        {
            stopCond1 = request->getParam("powerMeasStopCond1", true)->value();
        }
        if (request->hasParam("powerMeasStopCond2", true))
        {
            stopCond2 = request->getParam("powerMeasStopCond2", true)->value();
        }
        PowerMeas::setActiveDeviceType(deviceType);
        PowerMeas::setConfiguration(PowerMeas::DEV_BL0939, bl0939Config, deviceType == PowerMeas::DEV_BL0939);
        PowerMeas::setConfiguration(PowerMeas::DEV_ADE7953, ade7953Config, deviceType == PowerMeas::DEV_ADE7953);
        PowerMeas::setConfiguration(PowerMeas::DEV_CSE7761, cse7761Config, deviceType == PowerMeas::DEV_CSE7761);
        PowerMeas::setConditionConfig(0, stopCond1);
        PowerMeas::setConditionConfig(1, stopCond2);
        request->send_P(200, "text/html", getHttpConfigSaved(), defaultProcessor);
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
    m_server.on("/powerMeasurementExport", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/json", PowerMeas::exportActiveDescriptorsToJSON());
        Log::debug("HTTP", "GET request, /powerMeasurementExport");
    });
    m_httpUpdater.setup(&m_server, "/update", "", "", getHttpOTA(), defaultProcessor);
    m_server.onNotFound([&](AsyncWebServerRequest *request){
        Log::debug("HTTP", "GET request, not found, redirecting");
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "redirect to captive portal");
        response->addHeader("Location", "/portal");
        request->send(response);
    });
    m_server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    m_server.begin();
}

void HttpServer::initAsAccessPoint()
{
    //WiFi.mode(WIFI_AP);
    //WiFi.enableAP(true);
    WiFi.setHostname(m_hostname.c_str());
    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.softAP(m_wifiSsidAp, m_wifiPasswordAp);
    Log::info("HTTP", "Initializing as AP, IP = %s, SSID = %s", WiFi.softAPIP().toString().c_str(), m_wifiSsidAp.c_str());
    delay(500);
}

void HttpServer::initAsClient()
{
    Log::info("HTTP", "Initializing as client, trying to connect to %s", m_wifiSsid.c_str());
    WiFi.setHostname(m_hostname.c_str());
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
    m_hostname = Config::getString("network/hostname", WiFi.getHostname());
    Log::info("HTTP", "Configuration loaded");
}

String HttpServer::defaultProcessor(const String& var)
{
    if (var == "MODULE_NAME")
        return htmlEscape(Module::getName());
    else if (var == "VERSION")
        return String(Config::VERSION);
    return String();
}

String HttpServer::moduleConfigProcessor(const String& var)
{
    if (var == "MODULE_LOG_LEVEL_OVERRIDE")
        return htmlEscape(Log::getLoggingLevelOverride());
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
    bool pinKeyUpPullEnabled;
    bool pinKeyDownPullEnabled;
    Louver::getGpioConfig(Louver::DIR_UP, pinKeyUp, relayUp, highKeyUp, highRelayUp, pinKeyUpPullEnabled);
    Louver::getGpioConfig(Louver::DIR_DOWN, pinKeyDown, relayDown, highKeyDown, highRelayDown, pinKeyDownPullEnabled);
    uint8_t pinKeyReset;
    bool highKeyReset;
    bool keyResetPullEnabled;
    Module::getResetGpioConfig(pinKeyReset, highKeyReset, keyResetPullEnabled);
    uint8_t pinLed = Module::getLedGpioConfig();
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
    if (var == "LED_GPIO")
        return String(pinLed);
    if ((var == "SELECTED_UP_INVERTED_YES") && (!highKeyUp))
        return "selected";
    if ((var == "SELECTED_UP_INVERTED_NO") && (highKeyUp))
        return "selected";
    if ((var == "SELECTED_UP_PULL_YES") && (pinKeyUpPullEnabled))
        return "selected";
    if ((var == "SELECTED_UP_PULL_NO") && (!pinKeyUpPullEnabled))
        return "selected";
    if ((var == "SELECTED_DOWN_INVERTED_YES") && (!highKeyDown))
        return "selected";
    if ((var == "SELECTED_DOWN_INVERTED_NO") && (highKeyDown))
        return "selected";
    if ((var == "SELECTED_DOWN_PULL_YES") && (pinKeyDownPullEnabled))
        return "selected";
    if ((var == "SELECTED_DOWN_PULL_NO") && (!pinKeyDownPullEnabled))
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
    if ((var == "SELECTED_KEY_RESET_PULL_YES") && (keyResetPullEnabled))
        return "selected";
    if ((var == "SELECTED_KEY_RESET_PULL_NO") && (!keyResetPullEnabled))
        return "selected";
    return defaultProcessor(var);
}

String HttpServer::movementConfigProcessor(const String& var)
{
    float timeFullOpenSecs;
    float timeFullCloseSecs;
    float timeOpenLamellasSecs;
    float shortMovementSecs;
    Louver::getTimesConfig(timeFullOpenSecs, timeFullCloseSecs, timeOpenLamellasSecs, shortMovementSecs);
    bool upCond;
    bool downCond;
    Louver::getPowerCondStop(upCond, downCond);
    if (var == "TIME_FULL_OPEN")
        return String(timeFullOpenSecs);
    if (var == "TIME_FULL_CLOSE")
        return String(timeFullCloseSecs);
    if (var == "TIME_OPEN_LAMELLAS")
        return String(timeOpenLamellasSecs);
    if (var == "TIME_SHORT")
        return String(shortMovementSecs);
    if ((var == "SELECTED_OPEN_STOP_PWRCOND1_NO") && (!upCond))
        return "selected";
    if ((var == "SELECTED_OPEN_STOP_PWRCOND1_YES") && (upCond))
        return "selected";
    if ((var == "SELECTED_CLOSE_STOP_PWRCOND2_NO") && (!downCond))
        return "selected";
    if ((var == "SELECTED_CLOSE_STOP_PWRCOND2_YES") && (downCond))
        return "selected";
    return defaultProcessor(var);
}

String HttpServer::networkConfigProcessor(const String& var)
{
    if ((var == "SELECTED_WIFI_MODE_AP") && (getInstance().m_wifiConfig == WIFI_CONF_AP))
        return "selected";
    if ((var == "SELECTED_WIFI_MODE_CLIENT") && (getInstance().m_wifiConfig == WIFI_CONF_CLIENT))
        return "selected";
    if (var == "NETWORK_SSID_AP")
        return htmlEscape(getInstance().m_wifiSsidAp);
    if (var == "NETWORK_PASS_AP")
        return htmlEscape(getInstance().m_wifiPasswordAp);
    if (var == "NETWORK_SSID")
        return htmlEscape(getInstance().m_wifiSsid);
    if (var == "NETWORK_PASS")
        return htmlEscape(getInstance().m_wifiPassword);
    if (var == "NETWORK_HOST")
        return WiFi.getHostname();
    if (var == "NETWORK_MDNS_HOST")
        return Mdns::getHost();
    if ((var == "SELECTED_CLIENT_BEHAVIOR_0") && (getInstance().m_wifiClientBehavior == WIFI_CLIENT_BEH_1MCLIENT_5MAP))
        return "selected";
    if ((var == "SELECTED_CLIENT_BEHAVIOR_1") && (getInstance().m_wifiClientBehavior == WIFI_CLIENT_BEH_STILL_CLIENT))
        return "selected";
    if ((var == "SELECTED_TELNET_LOG_NO") && (!Log::getTelnetLoggingEnabled()))
        return "selected";
    if ((var == "SELECTED_TELNET_LOG_YES") && (Log::getTelnetLoggingEnabled()))
        return "selected";
    return defaultProcessor(var);
}

String HttpServer::mqttConfigProcessor(const String& var)
{
    if ((var == "MQTT_ENABLED_NO") && (!Mqtt::getEnabled()))
        return "selected";
    if ((var == "MQTT_ENABLED_YES") && (Mqtt::getEnabled()))
        return "selected";
    if (var == "MQTT_CLIENT_ID")
        return htmlEscape(Mqtt::getClientId());
    if (var == "MQTT_BROKER_IP")
        return Mqtt::getBrokerIp();
    if (var == "MQTT_BROKER_PORT")
        return String(Mqtt::getBrokerPort());
    if (var == "MQTT_BROKER_USER")
        return String(Mqtt::getAuthenticationUser());
    if (var == "MQTT_BROKER_PASS")
        return String(Mqtt::getAuthenticationPassword());
    if (var == "MQTT_POWER_MEAS_PERIOD")
        return htmlEscape(String(Mqtt::getPowerPublishPeriod()));
    return defaultProcessor(var);
}

String HttpServer::powerMeasConfigProcessor(const String& var)
{
    if ((var == "POWER_MEAS_DRIVER_0") && (PowerMeas::getActiveDeviceType() == PowerMeas::DEV_NONE))
        return "selected";
    if ((var == "POWER_MEAS_DRIVER_1") && (PowerMeas::getActiveDeviceType() == PowerMeas::DEV_BL0939))
        return "selected";
    if ((var == "POWER_MEAS_DRIVER_2") && (PowerMeas::getActiveDeviceType() == PowerMeas::DEV_ADE7953))
        return "selected";
    if ((var == "POWER_MEAS_DRIVER_3") && (PowerMeas::getActiveDeviceType() == PowerMeas::DEV_CSE7761))
        return "selected";
    if (var == "POWER_MEAS_BL0939_CONFIG")
        return htmlEscape(String(PowerMeas::getConfiguration(PowerMeas::DEV_BL0939)));
    if (var == "POWER_MEAS_ADE7953_CONFIG")
        return htmlEscape(String(PowerMeas::getConfiguration(PowerMeas::DEV_ADE7953)));
    if (var == "POWER_MEAS_CSE7761_CONFIG")
        return htmlEscape(String(PowerMeas::getConfiguration(PowerMeas::DEV_CSE7761)));
    if (var == "POWER_MEAS_STOP_COND_1")
        return htmlEscape(String(PowerMeas::getConditionConfig(0)));
    if (var == "POWER_MEAS_STOP_COND_2")
        return htmlEscape(String(PowerMeas::getConditionConfig(1)));
    return defaultProcessor(var);
}

void HttpServer::process()
{
    HttpServer& inst = getInstance();
    inst.m_dnsServer.processNextRequest();
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
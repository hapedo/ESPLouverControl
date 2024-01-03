#pragma once
// To avoid clashes with already declared constants add WEBSERVER_H directive
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>
#include <DNSServer.h>

class HttpServer
{
public:

    static constexpr const char* DEFAULT_SSID_AP = "Louver";

    static constexpr const char* DEFAULT_PASSWORD_AP = "";

    static constexpr const char* DEFAULT_SSID = "MyNetwork";

    static constexpr const char* DEFAULT_PASSWORD = "password";

    enum WifiConfig
    {
        WIFI_CONF_AP = 0,
        WIFI_CONF_CLIENT
    };

    enum WifiClientBehavior
    {
        WIFI_CLIENT_BEH_1MCLIENT_5MAP = 0,
        WIFI_CLIENT_BEH_STILL_CLIENT
    };

    static void loadConfig();

    static void init();

    static void setWifiConfig(WifiConfig config);

    static void setWifiClientBehavior(WifiClientBehavior behavior);

    static void configureAP(const char* ssid, const char* password);

    static void configureClient(const char* ssid, const char* password);

    static void getConfig(WifiConfig& wifiConfig, String& ssidAp, String& passAp, String& ssidClient, String& passClient);

    static void process();

    static String defaultProcessor(const String& var);

private:

    static constexpr uint32_t RECONNECT_PERIOD_SECS = 20;

    HttpServer();

    static inline HttpServer& getInstance()
    {
        static HttpServer server;
        return server;
    }

    void initPrivate();

    void initAsAccessPoint();

    void initAsClient();
    
    void loadConfigPrivate();

    static String moduleConfigProcessor(const String& var);

    static String gpioConfigProcessor(const String& var);

    static String timingConfigProcessor(const String& var);

    static String networkConfigProcessor(const String& var);

    static String mqttConfigProcessor(const String& var);

    WifiConfig m_wifiConfig;
    WifiClientBehavior m_wifiClientBehavior;
    bool m_wifiClientModeSwap;
    AsyncWebServer m_server;
    DNSServer m_dnsServer;
    String m_wifiSsidAp;
    String m_wifiPasswordAp;
    String m_wifiSsid;
    String m_wifiPassword;
    int m_lastWifiStatus;
    uint64_t m_nextReconnectTime;
    uint64_t m_wifiClientSwapTimeout;
};
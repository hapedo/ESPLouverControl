#pragma once
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>

class Mqtt
{
public:

    static constexpr const char* DEFAULT_CLIENT_ID = "louver";

    static constexpr const char* DEFAULT_BROKER_IP = "192.168.1.1";

    static constexpr uint16_t DEFAULT_BROKER_PORT = 1883;

    static constexpr uint32_t DEFAULT_POWER_PUBLISH_PERIOD_MILLI = 2000;

    static void loadConfig();

    static bool getEnabled();

    static void setEnabled(bool enabled);

    static String getBrokerIp();

    static void setBrokerIp(const char* brokerIp);

    static uint16_t getBrokerPort();

    static void setBrokerPort(uint16_t port);

    static String getClientId();

    static void setClientId(const char* id);

    static void setAuthentication(const char* user, const char* password);

    static String getAuthenticationUser();

    static String getAuthenticationPassword();

    static void setPowerPublishPeriod(uint32_t periodMilli);

    static uint32_t getPowerPublishPeriod();

    static void publishMovement(const char* value);

    static void publishKey(const char* key, const char* value);

    static void publishPosition(uint8_t position);

    static void process();

private:

    static constexpr uint32_t RECONNECT_PERIOD_MILLI = (30 * 1000);

    Mqtt();

    static inline Mqtt& getInstance()
    {
        static Mqtt mqtt;
        return mqtt;
    }

    void reconnect();

    static void mqttCallback(char* topic, byte* message, unsigned int length);

    WiFiClient m_wifiClient;
    PubSubClient m_client;
    bool m_enabled;
    String m_brokerIp;
    uint16_t m_brokerPort;
    String m_clientId;
    String m_user;
    String m_password;
    uint32_t m_powerPublishPeriod;
    uint64_t m_lastReconnectTime;
    uint64_t m_lastPowerPublishTime;
};
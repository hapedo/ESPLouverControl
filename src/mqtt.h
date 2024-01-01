#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

class Mqtt
{
public:

    static constexpr const char* DEFAULT_CLIENT_ID = "louver";

    static constexpr const char* DEFAULT_BROKER_IP = "192.168.1.1";

    static constexpr uint16_t DEFAULT_BROKER_PORT = 1883;

    static void loadConfig();

    static String getBrokerIp();

    static void setBrokerIp(const char* brokerIp);

    static uint16_t getBrokerPort();

    static void setBrokerPort(uint16_t port);

    static String getClientId();

    static void setClientId(const char* id);

    static void process();

private:

    static constexpr uint32_t RECONNECT_PERIOD_MILLI = 5000;

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
    String m_brokerIp;
    uint16_t m_brokerPort;
    String m_clientId;
    uint64_t m_lastReconnectTime;
};
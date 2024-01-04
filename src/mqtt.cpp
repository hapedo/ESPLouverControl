#include "mqtt.h"
#include "config.h"
#include "time.h"
#include "log.h"
#include "louver.h"

Mqtt::Mqtt() :
    m_client(m_wifiClient),
    m_enabled(false),
    m_brokerIp(DEFAULT_BROKER_IP),
    m_brokerPort(DEFAULT_BROKER_PORT),
    m_clientId(DEFAULT_CLIENT_ID),
    m_lastReconnectTime(0)
{

}

void Mqtt::loadConfig()
{
    Mqtt& inst = getInstance();
    inst.m_enabled = Config::getBool("mqtt/enabled", false);
    inst.m_brokerIp = Config::getString("mqtt/broker_ip", DEFAULT_BROKER_IP);
    inst.m_brokerPort = Config::getInt("mqtt/broker_port", DEFAULT_BROKER_PORT);
    inst.m_clientId = Config::getString("mqtt/client_id", DEFAULT_CLIENT_ID);
    Log::info("MQTT", "Configuration loaded, broker=%s:%d, client ID=\"%s\"", inst.m_brokerIp.c_str(), inst.m_brokerPort, inst.m_clientId.c_str());
}

bool Mqtt::getEnabled()
{
    return getInstance().m_enabled;
}

void Mqtt::setEnabled(bool enabled)
{
    Mqtt& inst = getInstance();
    Log::info("MQTT", "MQTT enabled set to \"%d\"", enabled);
    Config::setBool("mqtt/enabled", enabled);
    Config::flush();
    inst.m_enabled = enabled;
    if (!enabled && inst.m_client.connected())
        inst.m_client.disconnect();
}

String Mqtt::getBrokerIp()
{
    return getInstance().m_brokerIp;
}

void Mqtt::setBrokerIp(const char* brokerIp)
{
    Log::info("MQTT", "Broker IP set to \"%s\"", brokerIp);
    Config::setString("mqtt/broker_ip", String(brokerIp));
    Config::flush();
    getInstance().m_brokerIp = String(brokerIp);
}

uint16_t Mqtt::getBrokerPort()
{
    return getInstance().m_brokerPort;
}

void Mqtt::setBrokerPort(uint16_t brokerPort)
{
    Log::info("MQTT", "Broker port set to \"%d\"", brokerPort);
    Config::setInt("mqtt/broker_port", brokerPort);
    Config::flush();
    getInstance().m_brokerPort = brokerPort;
}

String Mqtt::getClientId()
{
    return getInstance().m_clientId;
}

void Mqtt::setClientId(const char* id)
{
    Log::info("MQTT", "Client ID set to \"%s\"", id);
    Config::setString("mqtt/client_id", String(id));
    Config::flush();
    getInstance().m_clientId = String(id);
}

void Mqtt::reconnect()
{
    IPAddress ip;
    if (!ip.fromString(m_brokerIp))
    {
        Log::error("MQTT", "Broker IP address format error, please check IP=\"%s\"", m_brokerIp.c_str());
        ip.fromString(String(DEFAULT_BROKER_IP));
    }
    if (m_client.connected())
        m_client.disconnect();
    m_client.setServer(ip, m_brokerPort);
    m_client.setCallback(Mqtt::mqttCallback);
    if (m_client.connect(m_clientId.c_str()))
    {
        Log::info("MQTT", "Connected to broker, IP=%s", m_brokerIp.c_str());
        String topic;
        topic = m_clientId + "/movement";
        m_client.subscribe(topic.c_str());
    }
}

void Mqtt::mqttCallback(char* topic, byte* message, unsigned int length) 
{
    Mqtt& inst = getInstance();
    String messageStr;
    for (int i = 0; i < length; i++) 
    {
        messageStr += (char)message[i];
    }
    Log::info("MQTT", "Message arrived, topic=\"%s\", message=\"%s\"", topic, messageStr.c_str());
    String requiredTopic;
    requiredTopic = inst.m_clientId + "/movement";
    if (String(topic) == requiredTopic)
    {
        if (messageStr == "up")
        {
            Log::info("MQTT", "Short up movement requested");
            Louver::shortOpen();
        }
        else if (messageStr == "down")
        {
            Log::info("MQTT", "Short down movement requested");
            Louver::shortClose();
        }
        else if (messageStr == "open")
        {
            Log::info("MQTT", "Full open movement requested");
            Louver::fullOpen();
        }
        else if (messageStr == "close")
        {
            Log::info("MQTT", "Full close movement requested");
            Louver::fullClose();
        }
        else if (messageStr == "close_open_lamellas")
        {
            Log::info("MQTT", "Full close and open lamellas movement requested");
            Louver::fullCloseAndOpenLamellas();
        }
    }
}

void Mqtt::process()
{
    Mqtt& inst = getInstance();
    inst.m_client.loop();
    uint64_t now = Time::nowRelativeMilli();
    if (inst.m_enabled)
    {
        if (!inst.m_client.connected() && (now >= inst.m_lastReconnectTime + RECONNECT_PERIOD_MILLI))
        {
            inst.m_lastReconnectTime = now;
            Log::info("MQTT", "Trying to reconnect");
            inst.reconnect();
        }
    }
}

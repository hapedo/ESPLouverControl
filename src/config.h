#pragma once
#include <ArduinoJson.h>

class Config
{
public:

    static constexpr const char* VERSION = "0.0.1";

    static constexpr size_t MAX_JSON_SIZE = 2048;

    static void flush();

    static void clearAll();

    static void setInt(const char* key, int value);

    static void setString(const char* key, String value);

    static void setFloat(const char* key, float value);

    static void setBool(const char* key, bool value);

    static int getInt(const char* key, int defaultValue = 0);

    static String getString(const char* key, String defaultValue = "");

    static float getFloat(const char* key, float defaultValue = 0);

    static bool getBool(const char* key, bool defaultValue = false);

private:

    Config();

    static inline Config& getInstance()
    {
        static Config config;
        return config;
    }

    DynamicJsonDocument m_json;
};
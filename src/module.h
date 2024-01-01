#pragma once
#include <Arduino.h>

class Module
{
public:

    static constexpr const char* DEFAULT_NAME = "Unknown room";

    static void loadConfig();

    static String getName();

    static void setName(const char* name); 

    static uint32_t getChipId();

private:

    Module();

    static inline Module& getInstance()
    {
        static Module module;
        return module;
    }

    String m_name;

};

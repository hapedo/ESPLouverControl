#include "module.h"
#include "config.h"
#include "log.h"

Module::Module() :
    m_name(DEFAULT_NAME)
{

}

void Module::loadConfig()
{
    Module& inst = getInstance();
    inst.m_name = Config::getString("module/name", DEFAULT_NAME);
    Log::info("Module", "Configuration loaded, name=\"%s\"", inst.m_name.c_str());
}

String Module::getName()
{
    return getInstance().m_name;
}

void Module::setName(const char* name)
{
    Log::info("Module", "Module name set to \"%s\"", name);
    Config::setString("module/name", String(name));
    getInstance().m_name = String(name);
}

uint32_t Module::getChipId()
{
#ifdef ESP32
    uint32_t chipId = 0;
	for(int i=0; i<17; i=i+8) 
    {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
    return chipId;
#else
    return ESP.getChipId();
#endif
}

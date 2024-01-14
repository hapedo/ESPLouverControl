#include "power_meas.h"
#include <ArduinoJson.h>
#include "bl0939.h"
#include "ade7953.h"
#include "time.h"
#include "log.h"
#include "config.h"

PowerMeas::PowerMeas() :
    m_activeDevice(DEV_NONE)
{
    PowerMeasDevice* device = new PowerMeasDevice;
    m_devices.push_back(device);
    BL0939* bl0939 = new BL0939;
    m_devices.push_back(bl0939); 
    ADE7953* ade7953 = new ADE7953;
    m_devices.push_back(ade7953); 

    // Stop conditions defaults
    for(size_t i = 0; i < STOP_CONDITION_COUNT; i++)
    {
        m_stopConditions[i].valueIndex = 0;
        m_stopConditions[i].comparator = CMP_EQUAL;
        m_stopConditions[i].valueToCheck = 0;
        m_stopConditions[i].durationMilli = 100;
        m_stopConditions[i].result = false;
        m_stopConditions[i].__timer = 0;
        m_stopConditions[i].__resetFlag = false;
        m_stopConditions[i].__lastCompare = false;
    }
}

void PowerMeas::loadConfig()
{
    PowerMeas& inst = getInstance();

    setActiveDeviceType((PowerMeas::DeviceType)Config::getInt("power_meas/device", DEV_NONE));

    inst.m_stopConditions[0].valueIndex = Config::getInt("power_meas/cond1_index", 0);
    inst.m_stopConditions[0].valueToCheck = Config::getFloat("power_meas/cond1_threshold", 0);
    inst.m_stopConditions[0].comparator = (Comparator)Config::getInt("power_meas/cond1_comparator", CMP_EQUAL);
    inst.m_stopConditions[0].durationMilli = (uint32_t)Config::getInt("power_meas/cond1_duration", 100);
    inst.m_stopConditions[1].valueIndex = Config::getInt("power_meas/cond2_index", 0);
    inst.m_stopConditions[1].valueToCheck = Config::getFloat("power_meas/cond2_threshold", 0);
    inst.m_stopConditions[1].comparator = (Comparator)Config::getInt("power_meas/cond2_comparator", CMP_EQUAL);
    inst.m_stopConditions[1].durationMilli = (uint32_t)Config::getInt("power_meas/cond2_duration", 100);
    Log::info("PowerMeas", "Configuration loaded, driver=%d", getInstance().m_activeDevice);
    Log::info("PowerMeas", "Stop condition 1 set, value index=%d, threshold=%f, comparator=%d, duration=%dms", 
        inst.m_stopConditions[0].valueIndex,
        inst.m_stopConditions[0].valueToCheck,
        inst.m_stopConditions[0].comparator,
        inst.m_stopConditions[0].durationMilli);
    Log::info("PowerMeas", "Stop condition 2 set, value index=%d, threshold=%f, comparator=%d, duration=%dms", 
        inst.m_stopConditions[1].valueIndex,
        inst.m_stopConditions[1].valueToCheck,
        inst.m_stopConditions[1].comparator,
        inst.m_stopConditions[1].durationMilli);
}

::std::vector<PowerMeas::Device> PowerMeas::getDevices()
{
    ::std::vector<Device> devices;
    for(size_t i = 0; i < getInstance().m_devices.size(); i++)
    {
        Device device;
        device.index = (uint8_t)i;
        device.chipId = getInstance().m_devices[i]->getChipInfo();
    }
    return devices;
}

PowerMeas::Device PowerMeas::getActiveDevice()
{
    PowerMeas& inst = getInstance();
    if (inst.m_activeDevice < inst.m_devices.size())
    {
        Device device;
        device.index = inst.m_activeDevice;
        device.chipId = inst.m_devices[inst.m_activeDevice]->getChipInfo();
        return device;
    }
    Device device;
    device.index = 0;
    device.chipId = "Invalid";
    return device;
}

void PowerMeas::setActiveDeviceType(DeviceType deviceType)
{
    PowerMeas& inst = getInstance();
    uint8_t last = inst.m_activeDevice;
    if (deviceType >= inst.m_devices.size())
        deviceType = (PowerMeas::DeviceType)(inst.m_devices.size() - 1);
    inst.m_activeDevice = deviceType;
    if (last != deviceType)
    {
        inst.m_devices[deviceType]->init();
        Config::setInt("power_meas/device", deviceType);
        Log::info("PowerMeas", "Active device set to driver %d", getInstance().m_activeDevice);
    }
}

PowerMeas::DeviceType PowerMeas::getActiveDeviceType()
{
    return getInstance().m_activeDevice;
}

String PowerMeas::exportActiveDescriptorsToJSON()
{
    PowerMeas& inst = getInstance();
    if (inst.m_activeDevice < inst.m_devices.size())
        return inst.m_devices[inst.m_activeDevice]->exportDescriptorsToJSON();
    return "{\"power_meas\":[]}";
}

String PowerMeas::getActiveConfiguration()
{
    PowerMeas& inst = getInstance();
    if (inst.m_activeDevice < inst.m_devices.size())
        return inst.m_devices[inst.m_activeDevice]->getConfiguration();
    return "{}";
}

void PowerMeas::setActiveConfiguration(String config)
{
    PowerMeas& inst = getInstance();
    if (inst.m_activeDevice < inst.m_devices.size())
        return inst.m_devices[inst.m_activeDevice]->setConfiguration(config);
}


String PowerMeas::getConfiguration(DeviceType deviceType)
{
    PowerMeas& inst = getInstance();
    if (deviceType < inst.m_devices.size())
        return inst.m_devices[deviceType]->getConfiguration();
    return "{}";
}

void PowerMeas::setConfiguration(DeviceType deviceType, String config, bool performInit)
{
    PowerMeas& inst = getInstance();
    if (deviceType < inst.m_devices.size())
        return inst.m_devices[deviceType]->setConfiguration(config, true, performInit);
}

bool PowerMeas::getConditionResult(uint8_t conditionIndex)
{
    if (conditionIndex < STOP_CONDITION_COUNT)
    {
        if (getInstance().m_stopConditions[conditionIndex].__resetFlag)
            return false;
        return getInstance().m_stopConditions[conditionIndex].result;
    }
    return false;
}

void PowerMeas::resetAllConditions()
{
    uint64_t now = Time::nowRelativeMilli();
    for(size_t i = 0; i < STOP_CONDITION_COUNT; i++)
    {
        getInstance().m_stopConditions[i].__resetFlag = true;
    }
    Log::debug("PowerMeas", "All conditions reset");
}

String PowerMeas::getConditionConfig(uint8_t conditionIndex)
{
    PowerMeas& inst = getInstance();
    if (conditionIndex < STOP_CONDITION_COUNT)
    {
        String result = "{";
        result = result + "\"index\":" + String(inst.m_stopConditions[conditionIndex].valueIndex) + ",";
        result = result + "\"threshold\":" + String(inst.m_stopConditions[conditionIndex].valueToCheck) + ",";
        result = result + "\"comparator\":\"" + comparatorToString(inst.m_stopConditions[conditionIndex].comparator) + "\",";
        result = result + "\"duration_milli\":" + String(inst.m_stopConditions[conditionIndex].durationMilli);
        result = result + "}";
        return result;
    }
    return "{}";
}

void PowerMeas::setConditionConfig(uint8_t conditionIndex, String config)
{
    PowerMeas& inst = getInstance();
    if (conditionIndex < STOP_CONDITION_COUNT)
    {
        DynamicJsonDocument json(512);
        DeserializationError error = deserializeJson(json, config);
        if (error) 
        {
            Log::error("PowerMeas", "Error while parsing config JSON, condition %d: %s", conditionIndex, error.c_str());
        }
        else
        {
            if (json.containsKey("index"))
                inst.m_stopConditions[conditionIndex].valueIndex = json["index"].as<uint8_t>();
            if (json.containsKey("threshold"))
                inst.m_stopConditions[conditionIndex].valueToCheck = json["threshold"].as<float>();
            if (json.containsKey("comparator"))
                inst.m_stopConditions[conditionIndex].comparator = stringToComparator(json["comparator"]);
            if (json.containsKey("duration_milli"))
                inst.m_stopConditions[conditionIndex].durationMilli = json["duration_milli"].as<uint32_t>();
            if (conditionIndex == 0)
            {
                Config::setInt("power_meas/cond1_index", inst.m_stopConditions[conditionIndex].valueIndex);
                Config::setFloat("power_meas/cond1_threshold", inst.m_stopConditions[conditionIndex].valueToCheck);
                Config::setInt("power_meas/cond1_comparator", inst.m_stopConditions[conditionIndex].comparator);
                Config::setInt("power_meas/cond1_duration", inst.m_stopConditions[conditionIndex].durationMilli);
            }
            else if (conditionIndex == 1)
            {
                Config::setInt("power_meas/cond2_index", inst.m_stopConditions[conditionIndex].valueIndex);
                Config::setFloat("power_meas/cond2_threshold", inst.m_stopConditions[conditionIndex].valueToCheck);
                Config::setInt("power_meas/cond2_comparator", inst.m_stopConditions[conditionIndex].comparator);
                Config::setInt("power_meas/cond2_duration", inst.m_stopConditions[conditionIndex].durationMilli);
            }
            Log::info("PowerMeas", "Stop condition %d set, value index=%d, threshold=%f, comparator=%d, duration=%dms", 
                conditionIndex, 
                inst.m_stopConditions[conditionIndex].valueIndex,
                inst.m_stopConditions[conditionIndex].valueToCheck,
                inst.m_stopConditions[conditionIndex].comparator,
                inst.m_stopConditions[conditionIndex].durationMilli);
        }
    }
}

bool PowerMeas::compare(float value1, float value2, Comparator cmp)
{
    switch(cmp)
    {
        case CMP_LESS:
            return value1 < value2; 
        case CMP_LESS_OR_EQUAL:
            return value1 <= value2;
        case CMP_EQUAL:
            return value1 == value2;
        case CMP_GREATER:
            return value1 > value2;
        case CMP_GREATER_OR_EQUAL:
            return value1 >= value2;
        case CMP_NOT_EQUAL:
            return value1 != value2;
    }
    return false;
}

String PowerMeas::comparatorToString(Comparator cmp)
{
    switch(cmp)
    {
        case CMP_LESS:
            return "<"; 
        case CMP_LESS_OR_EQUAL:
            return "<="; 
        case CMP_EQUAL:
            return "=="; 
        case CMP_GREATER:
            return ">"; 
        case CMP_GREATER_OR_EQUAL:
            return ">="; 
        case CMP_NOT_EQUAL:
            return "!="; 
    }
    return "==";
}

PowerMeas::Comparator PowerMeas::stringToComparator(String cmp)
{
    if (cmp == "<")
        return CMP_LESS;
    else if (cmp == "<=")
        return CMP_LESS_OR_EQUAL;
    else if (cmp == "==")
        return CMP_EQUAL;
    else if (cmp == ">")
        return CMP_GREATER;
    else if (cmp == ">=")
        return CMP_GREATER_OR_EQUAL;
    else if (cmp == "!=")
        return CMP_NOT_EQUAL;
    return CMP_EQUAL;
}

void PowerMeas::process()
{
    PowerMeas& inst = getInstance();
    if (inst.m_activeDevice < inst.m_devices.size())
    {
        uint64_t now = Time::nowRelativeMilli();
        inst.m_devices[inst.m_activeDevice]->process();
        for(size_t i = 0; i < STOP_CONDITION_COUNT; i++)
        {
            if (inst.m_stopConditions[i].__resetFlag)
            {
                    inst.m_stopConditions[i].result = false;
                    inst.m_stopConditions[i].__timer = now;
                    inst.m_stopConditions[i].__lastCompare = false;
                    inst.m_stopConditions[i].__resetFlag = false;
            }
            if (inst.m_stopConditions[i].valueIndex < inst.m_devices[inst.m_activeDevice]->getDescriptorCount())
            {
                PowerMeasDevice::ValueDescriptor desc = inst.m_devices[inst.m_activeDevice]->getDescriptor(inst.m_stopConditions[i].valueIndex);
                bool cmpResult = compare(inst.m_stopConditions[i].valueToCheck, desc.lastValue, inst.m_stopConditions[i].comparator);
                if (!cmpResult)
                {
                    inst.m_stopConditions[i].result = false;
                    inst.m_stopConditions[i].__timer = now;
                }
                else
                {
                    if (cmpResult != inst.m_stopConditions[i].__lastCompare)
                    {
                        inst.m_stopConditions[i].__timer = now;
                    }
                    if (!inst.m_stopConditions[i].result && (now > (inst.m_stopConditions[i].__timer + inst.m_stopConditions[i].durationMilli)))
                    {
                        inst.m_stopConditions[i].result = true;
                        Log::info("PowerMeas", "Condition %d met", i);
                    }
                }
                inst.m_stopConditions[i].__lastCompare = cmpResult;
            }
        }
    }
}
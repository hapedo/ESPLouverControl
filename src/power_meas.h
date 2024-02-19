#pragma once
#include "power_meas_device.h"

class PowerMeas
{
public:

    static constexpr uint8_t STOP_CONDITION_COUNT = 2;

    enum DeviceType
    {
        DEV_NONE = 0,
        DEV_BL0939,
        DEV_ADE7953
    };

    enum Comparator
    {
        CMP_LESS = 0,
        CMP_LESS_OR_EQUAL,
        CMP_EQUAL,
        CMP_GREATER,
        CMP_GREATER_OR_EQUAL,
        CMP_NOT_EQUAL
    };

    struct Device
    {
        uint8_t index;
        String chipId;
    };

    static void loadConfig();

    static ::std::vector<Device> getDevices();

    static Device getActiveDevice();

    static void setActiveDeviceType(DeviceType deviceType);

    static DeviceType getActiveDeviceType();

    static String exportActiveDescriptorsToJSON();

    static ::std::vector<PowerMeasDevice::ValueDescriptor> getActiveDescriptors();

    static String getActiveConfiguration();

    static void setActiveConfiguration(String config);

    static String getConfiguration(DeviceType deviceType);

    static void setConfiguration(DeviceType deviceType, String config, bool performInit = false);

    static bool getConditionResult(uint8_t conditionIndex);

    static void resetAllConditions();

    static String getConditionConfig(uint8_t conditionIndex);

    static void setConditionConfig(uint8_t conditionIndex, String config);

    static void process();

private:

    struct StopCondition
    {
        uint8_t valueIndex;
        Comparator comparator;
        float valueToCheck;
        uint32_t durationMilli;
        bool result;
        bool __resetFlag;
        uint64_t __timer;
        bool __lastCompare;
    };

    PowerMeas();

    static bool compare(float value1, float value2, Comparator cmp);

    static String comparatorToString(Comparator cmp);

    static Comparator stringToComparator(String cmp);

    static inline PowerMeas& getInstance()
    {
        static PowerMeas powMeas;
        return powMeas;
    }

    ::std::vector<PowerMeasDevice*> m_devices;
    DeviceType m_activeDevice;
    StopCondition m_stopConditions[STOP_CONDITION_COUNT];

};
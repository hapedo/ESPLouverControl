#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <vector>

class PowerMeasDevice
{
public:

    struct ValueDescriptor
    {
        uint8_t index;
        String description;
        String unit;
        String valueFormat;
        float lastValue;
        float maxValue;
        float minValue;
    };

    PowerMeasDevice();

    virtual void init();

    virtual String getChipInfo() const;

    uint8_t getDescriptorCount();

    ValueDescriptor getDescriptor(uint8_t index);

    ::std::vector<ValueDescriptor> getDescriptors();

    String exportDescriptorsToJSON();

    void resetMinMax();

    virtual String getConfiguration();

    virtual void setConfiguration(String config, bool save = true, bool performInit = false);

    virtual void process();

protected:

    ValueDescriptor createEmptyDescriptor();

    void appendDescriptor(String description, String unit, String valueFormat);

    void setLastValue(uint8_t index, float lastValue, bool updateMinMax = true);

    ::std::vector<ValueDescriptor> m_descriptors;

private:

    bool m_enabled;

};
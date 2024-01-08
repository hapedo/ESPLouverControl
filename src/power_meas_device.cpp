#include "power_meas.h"
#include "log.h"

PowerMeasDevice::PowerMeasDevice() :
    m_enabled(false)
{

}

String PowerMeasDevice::getChipInfo() const
{
    return "Unknown";
}

uint8_t PowerMeasDevice::getDescriptorCount()
{
    return m_descriptors.size();
}

PowerMeasDevice::ValueDescriptor PowerMeasDevice::getDescriptor(uint8_t index)
{
    if (index > getDescriptorCount())
        return createEmptyDescriptor();
    else
        return m_descriptors[index];
}

::std::vector<PowerMeasDevice::ValueDescriptor> PowerMeasDevice::getDescriptors()
{
    return m_descriptors;
}

String PowerMeasDevice::exportDescriptorsToJSON()
{
    String result = "{\"power_meas\":[";
    for(size_t i = 0; i < m_descriptors.size(); i++)
    {
        result = result + "{";
        result = result + "\"description\":\"" + m_descriptors[i].description + "\",";
        result = result + "\"unit\":\"" + m_descriptors[i].unit + "\",";
        result = result + "\"valueFormat\":\"" + m_descriptors[i].valueFormat + "\",";
        result = result + "\"lastValue\":" + String(m_descriptors[i].lastValue) + ",";
        result = result + "\"minValue\":" + String(m_descriptors[i].minValue) + ",";
        result = result + "\"maxValue\":" + String(m_descriptors[i].maxValue);
        result = result + "}";
        if (i < (m_descriptors.size() - 1))
            result = result + ",";
    }
    result = result + "]}";
    return result;
}

void PowerMeasDevice::resetMinMax()
{
    for(size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptors[i].maxValue = 0;
        m_descriptors[i].minValue = 0;
    }
}

String PowerMeasDevice::getConfiguration()
{
    return "{}";
}

void PowerMeasDevice::setConfiguration(String config, bool save)
{

}

PowerMeasDevice::ValueDescriptor PowerMeasDevice::createEmptyDescriptor()
{
    ValueDescriptor desc;
    desc.index = 0;
    desc.description = "Invalid";
    desc.unit = "-";
    desc.valueFormat = "";
    desc.lastValue = 0;
    desc.minValue = 0;
    desc.maxValue = 0;
    return desc;
}

void PowerMeasDevice::init()
{

}

void PowerMeasDevice::appendDescriptor(String description, String unit, String valueFormat)
{
    ValueDescriptor desc;
    desc.index = m_descriptors.size();
    desc.description = description;
    desc.unit = unit;
    desc.valueFormat = valueFormat;
    desc.lastValue = 0;
    desc.minValue = 0;
    desc.maxValue = 0;
    m_descriptors.push_back(desc);
}

void PowerMeasDevice::setLastValue(uint8_t index, float lastValue, bool updateMinMax)
{
    if (index < m_descriptors.size())
    {
        m_descriptors[index].lastValue = lastValue;
        if (updateMinMax)
        {
            if (lastValue > m_descriptors[index].maxValue)
                m_descriptors[index].maxValue = lastValue;
            if (lastValue < m_descriptors[index].minValue)
                m_descriptors[index].minValue = lastValue;
        }
    }
}

void PowerMeasDevice::process()
{

}
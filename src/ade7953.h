#pragma once
#include <Wire.h>
#include "power_meas_device.h"

class ADE7953 : public PowerMeasDevice
{
public:

    enum Mode
    {
        M_UART = 0,
        M_I2C
    };

    ADE7953(Mode mode = M_I2C);

    void init() override;

    String getChipInfo() const override;

    String getConfiguration() override;

    void setConfiguration(String config, bool save = true, bool performInit = false) override;

    void process() override;

private:

    void write8(uint16_t reg, uint8_t value);
    void write16(uint16_t reg, uint16_t value);
    void write32(uint16_t reg, uint32_t value);
    bool read16(uint16_t reg, uint16_t& value);
    bool read32(uint16_t reg, uint32_t& value);

    Mode m_mode;
    uint8_t m_peripheralIndex;
    uint8_t m_pin0Index;
    uint8_t m_pin1Index;
    uint8_t m_pinResetIndex;
    uint32_t m_refreshPeriod;
    TwoWire* m_i2c;
    HardwareSerial* m_serial;
    uint64_t m_lastReadTimestamp;
};
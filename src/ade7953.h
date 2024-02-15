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

    enum PgaGain 
    {
        PGA_GAIN_1 = 0x00,
        PGA_GAIN_2 = 0x01,
        PGA_GAIN_4 = 0x02,
        PGA_GAIN_8 = 0x03,
        PGA_GAIN_16 = 0x04,
        PGA_GAIN_22 = 0x05
    };

    struct AdeConfig
    {
        float voltageScale;
        float voltageOffset;
        float currentScale0;
        float currentScale1;
        float currentOffset0;
        float currentOffset1;
        float apowerScale0;
        float apowerScale1;
        float aenergyScale0;
        float aenergyScale1;
        PgaGain voltagePgaGain;
        PgaGain currentPgaGain0;
        PgaGain currentPgaGain1;
    };

    uint8_t getRegSize(uint16_t reg);
    void write(uint16_t reg, int32_t value);
    bool read(uint16_t reg, bool isSigned, int32_t& value);

    Mode m_mode;
    uint8_t m_peripheralIndex;
    uint8_t m_pin0Index;
    uint8_t m_pin1Index;
    uint8_t m_pinResetIndex;
    uint32_t m_refreshPeriod;
    TwoWire* m_i2c;
    HardwareSerial* m_serial;
    uint64_t m_lastReadTimestamp;
    AdeConfig m_config;
};
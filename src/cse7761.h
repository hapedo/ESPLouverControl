#pragma once
#include "power_meas_device.h"

class CSE7761 : public PowerMeasDevice
{
public:
    CSE7761();

    void init() override;

    String getChipInfo() const override;

    String getConfiguration() override;

    void setConfiguration(String config, bool save = true, bool performInit = false) override;

    void process() override;

protected:

private:

    enum State
    {
        CSE_ST_IDLE = 0,
        CSE_ST_DETECT,
        CSE_ST_INIT,
        CSE_ST_READ
    };

    enum Coefficient
    {
        COEF_RMS_IAC = 0, 
        COEF_RMS_IBC, 
        COEF_RMS_UC, 
        COEF_POWER_PAC, 
        COEF_POWER_PBC, 
        COEF_POWER_SC, 
        COEF_ENERGY_AC, 
        COEF_ENERGY_BC
    };

    struct Data
    {
        uint32_t frequency = 0;
        uint32_t voltage_rms = 0;
        uint32_t current_rms[2] = { 0 };
        uint32_t energy[2] = { 0 };
        uint32_t active_power[2] = { 0 };
        uint16_t coefficient[8] = { 0 };
        uint8_t energy_update[2] = { 0 };
        uint8_t init = 4;
        uint8_t ready = 0;
        uint8_t model;
    };    

    uint8_t calculateChecksum(uint8_t size);

    uint8_t getRegisterSize(uint8_t reg);

    bool writeRegister(uint8_t reg, uint32_t value);

    bool readRegister(uint8_t reg, uint32_t& result);

    bool readRegister(uint8_t reg, uint16_t& result);

    bool readRegister(uint8_t reg, uint8_t& result);

    HardwareSerial* m_serial;
    State m_state;
    uint8_t m_serialIndex;
    uint8_t m_rxGpio;
    uint8_t m_txGpio;
    uint32_t m_refreshPeriod;
    uint8_t m_packet[32];
    uint64_t m_lastReadTimestamp;
    uint8_t m_registerIndex;
    Data m_data;
};
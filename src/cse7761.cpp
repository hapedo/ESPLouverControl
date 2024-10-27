#include "cse7761.h"
#include "log.h"
#include "time.h"
#include "config.h"
#include "profiles.h"


#define CSE7761_UREF                  42563        // RmsUc
#define CSE7761_IREF                  52241        // RmsIAC
#define CSE7761_PREF                  44513        // PowerPAC
#define CSE7761_FREF                  3579545      // System clock (3.579545MHz) as used in frequency calculation

static const uint8_t CSE_PACKET_HEADER = 0xa5;
static const uint8_t CSE_CMD_READ = 0x00;
static const uint8_t CSE_CMD_WRITE = 0x80;

static const uint8_t CSE_REG_SYSCON = 0x00;
static const uint8_t CSE_REG_EMUCON = 0x01;
static const uint8_t CSE_REG_HFCONST = 0x02;
static const uint8_t CSE_REG_PSTARTPA = 0x03;
static const uint8_t CSE_REG_PSTARTPB = 0x04;
static const uint8_t CSE_REG_PAGAIN = 0x05;
static const uint8_t CSE_REG_PBGAIN = 0x06;
static const uint8_t CSE_REG_PHASEA = 0x07;
static const uint8_t CSE_REG_PHASEB = 0x08;
static const uint8_t CSE_REG_PAOS = 0x0a;
static const uint8_t CSE_REG_PBOS = 0x0b;
static const uint8_t CSE_REG_RMSIAOS = 0x0e;
static const uint8_t CSE_REG_RMSIBOS = 0x0f;
static const uint8_t CSE_REG_IBGAIN = 0x10;
static const uint8_t CSE_REG_PSGAIN = 0x11;
static const uint8_t CSE_REG_PSOS = 0x12;
static const uint8_t CSE_REG_EMUCON2 = 0x13;
static const uint8_t CSE_REG_SAGCYC = 0x17;
static const uint8_t CSE_REG_SAGLVL = 0x18;
static const uint8_t CSE_REG_OVLVL = 0x19;
static const uint8_t CSE_REG_OIALVL = 0x1a;
static const uint8_t CSE_REG_OIBLVL = 0x1b;
static const uint8_t CSE_REG_OPLVL = 0x1c;
static const uint8_t CSE_REG_PULSE1SEL = 0x1d;
static const uint8_t CSE_REG_PFCNT_PA = 0x20;
static const uint8_t CSE_REG_PFCNT_PB = 0x21;
static const uint8_t CSE_REG_ANGLE = 0x22;
static const uint8_t CSE_REG_UFREQ = 0x23;
static const uint8_t CSE_REG_RMSIA = 0x24;
static const uint8_t CSE_REG_RMSIB = 0x25;
static const uint8_t CSE_REG_RMSU = 0x26;
static const uint8_t CSE_REG_POWERFACTOR = 0x27;
static const uint8_t CSE_REG_ENERGY_PA = 0x28;
static const uint8_t CSE_REG_ENERGY_PB = 0x29;
static const uint8_t CSE_REG_POWERPA = 0x2c;
static const uint8_t CSE_REG_POWERPB = 0x2d;
static const uint8_t CSE_REG_POWERS = 0x2e;
static const uint8_t CSE_REG_EMUSTATUS = 0x2f;
static const uint8_t CSE_REG_PEAKIA = 0x30;
static const uint8_t CSE_REG_PEAKIB = 0x31;
static const uint8_t CSE_REG_PEAKU = 0x32;
static const uint8_t CSE_REG_INSTANTIA = 0x33;
static const uint8_t CSE_REG_INSTANTIB = 0x34;
static const uint8_t CSE_REG_INSTANTU = 0x35;
static const uint8_t CSE_REG_WAVEIA = 0x36;
static const uint8_t CSE_REG_WAVEIB = 0x37;
static const uint8_t CSE_REG_WAVEU = 0x38;
static const uint8_t CSE_REG_INSTANTP = 0x3c;
static const uint8_t CSE_REG_INSTANTS = 0x3d;
static const uint8_t CSE_REG_IE = 0x40;
static const uint8_t CSE_REG_IF = 0x41;
static const uint8_t CSE_REG_RIF = 0x42;
static const uint8_t CSE_REG_SYSSTATUS = 0x43;
static const uint8_t CSE_REG_RDATA = 0x44;
static const uint8_t CSE_REG_WDATA = 0x45;
static const uint8_t CSE_REG_COEFF_CHKSUM = 0x6f;
static const uint8_t CSE_REG_RMSIAC = 0x70;
static const uint8_t CSE_REG_RMSIBC = 0x71;
static const uint8_t CSE_REG_RMSUC = 0x72;
static const uint8_t CSE_REG_POWERPAC = 0x73;
static const uint8_t CSE_REG_POWERPBC = 0x74;
static const uint8_t CSE_REG_POWERSC = 0x75;
static const uint8_t CSE_REG_ENERGYAC = 0x76;
static const uint8_t CSE_REG_ENERGYBC = 0x77;
static const uint8_t CSE_REG_DEVICEID = 0x7f;
static const uint8_t CSE_REG_SPECIAL = 0xea;

static const uint32_t CSE_DEVICEID = 0x776110;

static const uint64_t CSE_RECEIVE_TIMEOUT_MILLI = 100;
static const uint64_t CSE_DETECT_PERIOD_MILLI = 1000;

CSE7761::CSE7761() :
    PowerMeasDevice(),
    m_serial(nullptr),
    m_state(CSE_ST_IDLE),
    m_serialIndex(PROFILE_DEFAULT_CSE7761_SERIAL),
    m_rxGpio(PROFILE_DEFAULT_CSE7761_RX_GPIO),
    m_txGpio(PROFILE_DEFAULT_CSE7761_TX_GPIO),
    m_refreshPeriod(PROFILE_DEFAULT_CSE7761_PERIOD_MILLI)
{
    appendDescriptor("Voltage RMS", "V", ".0f", "voltage");
    appendDescriptor("Frequency", "Hz", ".0f", "frequency");
    appendDescriptor("Current 1 RMS", "A", ".3f", "current1");
    appendDescriptor("Power 1", "W", ".3f", "power1");
    appendDescriptor("Current 2 RMS", "A", ".3f", "current2");
    appendDescriptor("Power 2", "W", ".3f", "power2");
    //appendDescriptor("Energy 1", "Wh", ".0f", "energy1");
    //appendDescriptor("Energy 2", "Wh", ".0f", "energy2");
    //appendDescriptor("Total energy", "Wh", ".0f", "total_energy");
}

String CSE7761::getChipInfo() const
{
    return "CSE7761";
}

void CSE7761::init()
{
    PowerMeasDevice::init();
    m_serial = nullptr;
    m_serialIndex = Config::getInt("cse7761/serial", PROFILE_DEFAULT_CSE7761_SERIAL);
    m_rxGpio = Config::getInt("cse7761/rx_gpio", PROFILE_DEFAULT_CSE7761_RX_GPIO);
    m_txGpio = Config::getInt("cse7761/tx_gpio", PROFILE_DEFAULT_CSE7761_TX_GPIO);
    m_refreshPeriod = Config::getInt("cse7761/refresh_milli", PROFILE_DEFAULT_CSE7761_PERIOD_MILLI);

    if (m_serialIndex == 0)
        m_serial = &Serial;
    else if (m_serialIndex == 1)
        m_serial = &Serial1;
#ifdef ESP32
    else if (m_serialIndex == 2)
        m_serial = &Serial2;
#endif
    m_lastReadTimestamp = 0;
    m_state = CSE_ST_DETECT;
    if (m_serial)
    {
#ifdef ESP32
        m_serial->begin(38400, SERIAL_8E1, m_rxGpio, m_txGpio);
#else
        m_serial->begin(38400, SERIAL_8E1);
#endif
        Log::info("CSE7761", "Initialized, serial=%d, rx=%d, tx=%d, refresh period=%d ms", m_serialIndex, m_rxGpio, m_txGpio, m_refreshPeriod);
    }
    else
    {
        Log::error("CSE7761", "Unable to init serial");
    }
}

String CSE7761::getConfiguration()
{
    String result = "{";
    result = result + "\"serial\":" + String(m_serialIndex) + ",";
    result = result + "\"rx_gpio\":" + String(m_rxGpio) + ",";
    result = result + "\"tx_gpio\":" + String(m_txGpio) + ",";
    result = result + "\"refresh_period_milli\":" + String(m_refreshPeriod);
    result = result + "}";
    return result;
}

void CSE7761::setConfiguration(String config, bool save, bool performInit)
{
    DynamicJsonDocument json(512);
    DeserializationError error = deserializeJson(json, config);
    if (error) 
    {
        Log::error("CSE7761", "Error while parsing config JSON: %s", error.c_str());
    }
    else
    {
        if (json.containsKey("serial"))
            m_serialIndex = json["serial"].as<uint8_t>();
        if (json.containsKey("rx_gpio"))
            m_rxGpio = json["rx_gpio"].as<uint8_t>();
        if (json.containsKey("tx_gpio"))
            m_txGpio = json["tx_gpio"].as<uint8_t>();
        if (json.containsKey("refresh_period_milli"))
        {
            m_refreshPeriod = json["refresh_period_milli"].as<uint32_t>();
            if (m_refreshPeriod < PROFILE_DEFAULT_CSE7761_PERIOD_MILLI)
                m_refreshPeriod = PROFILE_DEFAULT_CSE7761_PERIOD_MILLI;
        }
        if (save)
        {
            Config::setInt("cse7761/serial", m_serialIndex);
            Config::setInt("cse7761/rx_gpio", m_rxGpio);
            Config::setInt("cse7761/tx_gpio", m_txGpio);
            Config::setInt("cse7761/refresh_milli", m_refreshPeriod);
            Config::flush();
            Log::info("CSE7761", "Configuration saved");
        }
        if (performInit)
            init();
    }
}

uint8_t CSE7761::calculateChecksum(uint8_t size)
{
    uint8_t chk = 0;
    for(uint8_t i = 0; i < size; i++)
    {
        chk += m_packet[i];
    }
    chk = ~chk;
    return chk;
}

uint8_t CSE7761::getRegisterSize(uint8_t reg)
{
    if ((reg == CSE_REG_PHASEA) || (reg == CSE_REG_PHASEB) || (reg == CSE_REG_SPECIAL))
        return 1;
    else if ((reg >= CSE_REG_RMSIA) && (reg <= CSE_REG_ENERGY_PB))
        return 3;
    else if ((reg >= CSE_REG_POWERPA) && (reg <= CSE_REG_POWERS))
        return 4;
    else if ((reg >= CSE_REG_EMUSTATUS) && (reg <= CSE_REG_WAVEU))
        return 3;
    else if ((reg >= CSE_REG_INSTANTP) && (reg <= CSE_REG_INSTANTS))
        return 4;
    else if (reg == CSE_REG_SYSSTATUS)
        return 1;
    else if (reg == CSE_REG_RDATA)
        return 4;
    else if (reg == CSE_REG_DEVICEID)
        return 3;
    return 2;
}

bool CSE7761::writeRegister(uint8_t reg, uint32_t value)
{
    if (!m_serial)
        return false;
    uint8_t size = getRegisterSize(reg);
    m_packet[0] = CSE_PACKET_HEADER;
    m_packet[1] = CSE_CMD_WRITE | reg;
    Log::verbose("CSE7761", "Writing register 0x%x = 0x%x, size %d", reg, value, size);
    for(uint8_t i = 0; i < size; i++)
    {
        m_packet[1 + size - i] = value & 0xff;
        value >>= 8;
    }
    m_packet[2 + size] = calculateChecksum(2 + size);
    m_serial->flush();
    m_serial->write(m_packet, 3 + size);
    return true;
}

bool CSE7761::readRegister(uint8_t reg, uint32_t& result)
{
    if (!m_serial)
        return false;
    uint8_t size = getRegisterSize(reg);
    m_packet[0] = CSE_PACKET_HEADER;
    m_packet[1] = reg;
    m_serial->flush();
    m_serial->write(m_packet, 2);
    uint8_t packetIndex = 2;
    uint32_t readStartTimestamp = Time::nowRelativeMilli();
    while (1)
    {
        if (Time::nowRelativeMilli() > (readStartTimestamp + CSE_RECEIVE_TIMEOUT_MILLI))
        {
            Log::error("CSE7761", "Receive timeout, register 0x%x, received %d bytes", reg, packetIndex - 2);
            return false;
        }
        if (m_serial->available())
        {
            uint8_t d = m_serial->read();
            m_packet[packetIndex] = d;
            packetIndex++;
            if (packetIndex == size + 3)
            {
                result = 0;
                uint8_t chk = calculateChecksum(size + 2);
                if (chk != m_packet[size + 2])
                {
                    Log::error("CSE7761", "Receive error - bad checksum (0x%x != 0x%x)", chk, m_packet[size + 2]);
                    return false;
                }
                for(uint8_t i = 0; i < size; i++)
                {
                    result <<= 8;
                    result |= m_packet[2 + i];
                }
                Log::verbose("CSE7761", "Register 0x%x = 0x%x (size %d)", reg, result, size);
                return true;
            }
        }
    }
    return false;
}

bool CSE7761::readRegister(uint8_t reg, uint16_t& result)
{
    uint32_t value;
    bool ret = readRegister(reg, value);
    result = value & 0xffff;
    return ret;
}

bool CSE7761::readRegister(uint8_t reg, uint8_t& result)
{
    uint32_t value;
    bool ret = readRegister(reg, value);
    result = value & 0xff;
    return ret;
}

void CSE7761::process()
{
    if (m_serial)
    {
        uint64_t now = Time::nowRelativeMilli();

        switch(m_state)
        {
            case CSE_ST_IDLE:
                break;
            case CSE_ST_DETECT:
                if (now > m_lastReadTimestamp + CSE_DETECT_PERIOD_MILLI)
                {
                    m_lastReadTimestamp = now;
                    Log::verbose("CSE7761", "Sending detect request");
                    uint32_t value;
                    if (readRegister(CSE_REG_DEVICEID, value))
                    {
                        Log::verbose("CSE7761", "Device detected, ID = 0x%x", value);
                        // Reset
                        if (!writeRegister(CSE_REG_SPECIAL, 0x96))
                        {
                            m_state = CSE_ST_DETECT;
                            break;
                        }
                        m_registerIndex = 0;
                        m_state = CSE_ST_INIT;
                    }
                }
                break;
            case CSE_ST_INIT:
                if (now > m_lastReadTimestamp + 1000)
                {
                    uint16_t calcChksum = 0xFFFF;
                    for (uint32_t i = 0; i < 8; i++) 
                    {
                        uint32_t value;
                        if (!readRegister(CSE_REG_RMSIAC + i, m_data.coefficient[i]))
                        {
                            m_state = CSE_ST_DETECT;
                        }
                        calcChksum += m_data.coefficient[i];
                    }
                    calcChksum = ~calcChksum;
                    //  uint16_t dummy = Cse7761Read(CSE7761_REG_COEFFOFFSET, 2);
                    uint16_t coeffChksum;
                    if (!readRegister(CSE_REG_COEFF_CHKSUM, coeffChksum))
                    {
                        m_state = CSE_ST_DETECT;
                    }
                    //if ((calcChksum != coeffChksum) || (!calcChksum)) 
                    {
                        Log::debug("CSE7761", "Default calibration");
                        m_data.coefficient[COEF_RMS_IAC] = CSE7761_IREF;
                        m_data.coefficient[COEF_RMS_IBC] = CSE7761_IREF;
                    //    CSE7761Data.coefficient[RmsIBC] = 0xCC05;
                        m_data.coefficient[COEF_RMS_UC] = CSE7761_UREF;
                        m_data.coefficient[COEF_POWER_PAC] = CSE7761_PREF;
                        m_data.coefficient[COEF_POWER_PBC] = CSE7761_PREF;
                    //    CSE7761Data.coefficient[PowerPBC] = 0xADD7;
                    }
                    // Enable writing
                    if (!writeRegister(CSE_REG_SPECIAL, 0xe5))
                        m_state = CSE_ST_DETECT;
                    uint8_t sysStatus;
                    if (readRegister(CSE_REG_SYSSTATUS, sysStatus) && (sysStatus & 0x10))
                    {
                        if (!writeRegister(CSE_REG_SYSCON, 0xFF04))
                            m_state = CSE_ST_DETECT;
                        uint16_t syscon;
                        if (!readRegister(CSE_REG_SYSCON, syscon))
                        {
                            m_state = CSE_ST_DETECT;
                            break;
                        }

                        Log::verbose("CSE7761", "SYSCON = 0x%x", syscon);

                        if (!writeRegister(CSE_REG_EMUCON, 0x1183))
                            m_state = CSE_ST_DETECT;
                        if (!writeRegister(CSE_REG_EMUCON2, 0x0FE5))
                            m_state = CSE_ST_DETECT;
                        if (!writeRegister(CSE_REG_PULSE1SEL, 0x3290))
                            m_state = CSE_ST_DETECT;
                        // Disable writing
                        if (!writeRegister(CSE_REG_SPECIAL, 0xdc))
                            m_state = CSE_ST_DETECT;
                        m_state = CSE_ST_INIT;
                    }
                    else
                    {
                        Log::error("CSE7761", "Registers are write protected");
                        m_state = CSE_ST_DETECT;
                    }
                    if (m_state == CSE_ST_INIT)
                    {
                        m_state = CSE_ST_READ;
                        Log::verbose("CSE7761", "Init finished");
                    }
                    else
                        Log::error("CSE7761", "Cannot initialize");
                }
                break;
            case CSE_ST_READ:
                if (now >= m_lastReadTimestamp + m_refreshPeriod)
                {
                    m_lastReadTimestamp = now;
                    Log::verbose("CSE7761", "Reading data");
                    uint32_t value;
                    if (readRegister(CSE_REG_RMSU, value))
                    {
                        m_data.voltage_rms = (value >= 0x800000) ? 0 : value;
                        double val = 0.01 * (double)m_data.voltage_rms * (double)m_data.coefficient[COEF_RMS_UC] / (double)0x400000;
                        setLastValue(0, val);
                    }
                    if (readRegister(CSE_REG_UFREQ, value))
                    {
                        m_data.frequency = (value >= 0x8000) ? 0 : value;
                        double val;
                        if (m_data.frequency == 0)
                            val = 0;
                        else 
                            val = 3579545./8./m_data.frequency;
                        setLastValue(1, val);
                    }
                    if (readRegister(CSE_REG_RMSIA, value))
                    {
                        m_data.current_rms[0] = ((value >= 0x800000) || (value < 1600)) ? 0 : value;  // No load threshold of 10mA
                        //double val = 0.001 * (double)m_data.current_rms[0] * (double)m_data.coefficient[COEF_RMS_IAC] / (double)0x800000;
                        double val = (double)m_data.current_rms[0] / ((0x800000 * 100 / this->m_data.coefficient[COEF_RMS_IAC]) * 10);
                        setLastValue(2, val);
                    }
                    if (readRegister(CSE_REG_POWERPA, value))
                    {
                        m_data.active_power[0] = (0 == value) ? 0 : (value & 0x80000000) ? (~value) + 1 : value;
                        double val = (double)m_data.active_power[0] * (double)m_data.coefficient[COEF_POWER_PAC] / (double)0x80000000;
                        setLastValue(3, val);
                    }
                    if (readRegister(CSE_REG_RMSIB, value))
                    {
                        m_data.current_rms[1] = ((value >= 0x800000) || (value < 1600)) ? 0 : value;  // No load threshold of 10mA
                        double val = 0.001 * (double)m_data.current_rms[1] * (double)m_data.coefficient[COEF_RMS_IBC] / (double)0x800000;
                        setLastValue(4, val);
                    }
                    if (readRegister(CSE_REG_POWERPB, value))
                    {
                        m_data.active_power[1] = (0 == value) ? 0 : (value & 0x80000000) ? (~value) + 1 : value;
                        double val = (double)m_data.active_power[1] * (double)m_data.coefficient[COEF_POWER_PBC] / (double)0x80000000;
                        setLastValue(5, val);
                    }
                }
                break;
        }
/*            if ((m_packetIndex == 0) && (d == BL0939_PACKET_HEADER))
            {
                m_packet.raw[0] = d;
                m_packetIndex++;
            }
            else if (m_packetIndex > 0)
            {
                m_packet.raw[m_packetIndex] = d;
                m_packetIndex++;
                if (m_packetIndex >= sizeof(DataPacket::raw))
                {
                    // Whole packet received
                    if (validateChecksum(&m_packet))
                    {
                        Log::verbose("BL0939", "Valid packet received");
                        float v_rms = (float) to_uint32_t(m_packet.v_rms) / BL0939_UREF;
                        float ia_rms = (float) to_uint32_t(m_packet.ia_rms) / BL0939_IREF;
                        float ib_rms = (float) to_uint32_t(m_packet.ib_rms) / BL0939_IREF;
                        float a_watt = (float) to_int32_t(m_packet.a_watt) / BL0939_PREF;
                        float b_watt = (float) to_int32_t(m_packet.b_watt) / BL0939_PREF;
                        int32_t cfa_cnt = to_int32_t(m_packet.cfa_cnt);
                        int32_t cfb_cnt = to_int32_t(m_packet.cfb_cnt);
                        float a_energy_consumption = (float) cfa_cnt / BL0939_EREF;
                        float b_energy_consumption = (float) cfb_cnt / BL0939_EREF;
                        float total_energy_consumption = a_energy_consumption + b_energy_consumption;
                        setLastValue(0, v_rms);
                        setLastValue(1, ia_rms);
                        setLastValue(2, ib_rms);
                        setLastValue(3, a_watt);
                        setLastValue(4, b_watt);
                        setLastValue(5, a_energy_consumption);
                        setLastValue(6, b_energy_consumption);
                        setLastValue(7, total_energy_consumption);
                    }
                    else
                    {
                        Log::verbose("BL0939", "Invalid checksum");
                    }
                    m_packetIndex = 0;
                }
            }
            */
    }
}

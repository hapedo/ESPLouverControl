#include "bl0939.h"
#include "log.h"
#include "time.h"
#include "config.h"
#include "profiles.h"

#define TXD2 25
#define RXD2 26

static const uint8_t BL0939_READ_COMMAND = 0x55;  // 0x5{A4,A3,A2,A1}
static const uint8_t BL0939_FULL_PACKET = 0xAA;
static const uint8_t BL0939_PACKET_HEADER = 0x55;

static const uint8_t BL0939_WRITE_COMMAND = 0xA5;  // 0xA{A4,A3,A2,A1}
static const uint8_t BL0939_REG_IA_FAST_RMS_CTRL = 0x10;
static const uint8_t BL0939_REG_IB_FAST_RMS_CTRL = 0x1E;
static const uint8_t BL0939_REG_MODE = 0x18;
static const uint8_t BL0939_REG_SOFT_RESET = 0x19;
static const uint8_t BL0939_REG_USR_WRPROT = 0x1A;
static const uint8_t BL0939_REG_TPS_CTRL = 0x1B;

const uint8_t BL0939_INIT[6][6] = {
    // Reset to default
    {BL0939_WRITE_COMMAND, BL0939_REG_SOFT_RESET, 0x5A, 0x5A, 0x5A, 0x33},
    // Enable User Operation Write
    {BL0939_WRITE_COMMAND, BL0939_REG_USR_WRPROT, 0x55, 0x00, 0x00, 0xEB},
    // 0x0100 = CF_UNABLE energy pulse, AC_FREQ_SEL 50Hz, RMS_UPDATE_SEL 800mS
    {BL0939_WRITE_COMMAND, BL0939_REG_MODE, 0x00, 0x10, 0x00, 0x32},
    // 0x47FF = Over-current and leakage alarm on, Automatic temperature measurement, Interval 100mS
    {BL0939_WRITE_COMMAND, BL0939_REG_TPS_CTRL, 0xFF, 0x47, 0x00, 0xF9},
    // 0x181C = Half cycle, Fast RMS threshold 6172
    {BL0939_WRITE_COMMAND, BL0939_REG_IA_FAST_RMS_CTRL, 0x1C, 0x18, 0x00, 0x16},
    // 0x181C = Half cycle, Fast RMS threshold 6172
    {BL0939_WRITE_COMMAND, BL0939_REG_IB_FAST_RMS_CTRL, 0x1C, 0x18, 0x00, 0x08}};


BL0939::BL0939() :
    PowerMeasDevice(),
    m_serial(nullptr),
    m_serialIndex(PROFILE_DEFAULT_BL0939_SERIAL),
    m_rxGpio(PROFILE_DEFAULT_BL0939_RX_GPIO),
    m_txGpio(PROFILE_DEFAULT_BL0939_TX_GPIO),
    m_refreshPeriod(PROFILE_DEFAULT_BL0939_PERIOD_MILLI),
    m_packetIndex(0)
{
    appendDescriptor("Voltage RMS", "V", ".0f");
    appendDescriptor("Current 1 RMS", "A", ".3f");
    appendDescriptor("Current 2 RMS", "A", ".3f");
    appendDescriptor("Power 1", "W", ".3f");
    appendDescriptor("Power 2", "W", ".3f");
    appendDescriptor("Energy 1", "Wh", ".0f");
    appendDescriptor("Energy 2", "Wh", ".0f");
    appendDescriptor("Total energy", "Wh", ".0f");
}

String BL0939::getChipInfo() const
{
    return "BL0939";
}

void BL0939::init()
{
    PowerMeasDevice::init();
    m_serial = nullptr;
    m_serialIndex = Config::getInt("bl0939/serial", PROFILE_DEFAULT_BL0939_SERIAL);
    m_rxGpio = Config::getInt("bl0939/rx_gpio", PROFILE_DEFAULT_BL0939_RX_GPIO);
    m_txGpio = Config::getInt("bl0939/tx_gpio", PROFILE_DEFAULT_BL0939_TX_GPIO);
    m_refreshPeriod = Config::getInt("bl0939/refresh_milli", PROFILE_DEFAULT_BL0939_PERIOD_MILLI);

    if (m_serialIndex == 0)
        m_serial = &Serial;
    else if (m_serialIndex == 1)
        m_serial = &Serial1;
    else if (m_serialIndex == 2)
        m_serial = &Serial2;
    m_serial = &Serial2;
    m_packetIndex = 0;
    m_lastReadTimestamp = 0;
    if (m_serial)
    {
        m_serial->begin(4800, SERIAL_8N1, m_rxGpio, m_txGpio);
        for(uint8_t i = 0; i < 6; i++)
        {
            for (uint8_t ii = 0; ii < 6; ii++)
            {
                m_serial->write(BL0939_INIT[i][ii]);
            }
            delay(10);
        }
        m_serial->flush();
        Log::info("BL0939", "Initialized, serial=%d, rx=%d, tx=%d, refresh period=%d ms", m_serialIndex, m_rxGpio, m_txGpio, m_refreshPeriod);
    }
    else
    {
        Log::error("BL0939", "Unable to init serial");
    }
}

String BL0939::getConfiguration()
{
    String result = "{";
    result = result + "\"serial\":" + String(m_serialIndex) + ",";
    result = result + "\"rx_gpio\":" + String(m_rxGpio) + ",";
    result = result + "\"tx_gpio\":" + String(m_txGpio) + ",";
    result = result + "\"refresh_period_milli\":" + String(m_refreshPeriod);
    result = result + "}";
    return result;
}

void BL0939::setConfiguration(String config, bool save)
{
    DynamicJsonDocument json(512);
    DeserializationError error = deserializeJson(json, config);
    if (error) 
    {
        Log::error("BL0939", "Error while parsing config JSON: %s", error.c_str());
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
            if (m_refreshPeriod < PROFILE_DEFAULT_BL0939_PERIOD_MILLI)
                m_refreshPeriod = PROFILE_DEFAULT_BL0939_PERIOD_MILLI;
        }
        if (save)
        {
            Config::setInt("bl0939/serial", m_serialIndex);
            Config::setInt("bl0939/rx_gpio", m_rxGpio);
            Config::setInt("bl0939/tx_gpio", m_txGpio);
            Config::setInt("bl0939/refresh_milli", m_refreshPeriod);
            Config::flush();
            Log::info("BL0939", "Configuration saved");
        }
        init();
    }
}

bool BL0939::validateChecksum(const DataPacket *data) 
{
    uint8_t checksum = BL0939_READ_COMMAND;
    // Whole package but checksum
    for (uint32_t i = 0; i < sizeof(data->raw) - 1; i++) 
    {
        checksum += data->raw[i];
    }
    checksum ^= 0xFF;
    return checksum == data->checksum;
}

uint32_t BL0939::to_uint32_t(ube24_t input) 
{ 
    return input.h << 16 | input.m << 8 | input.l; 
}

int32_t BL0939::to_int32_t(sbe24_t input) 
{ 
    return input.h << 16 | input.m << 8 | input.l; 
}

void BL0939::process()
{
    if (m_serial)
    {
        uint64_t now = Time::nowRelativeMilli();
        if (now >= m_lastReadTimestamp + m_refreshPeriod)
        {
            m_lastReadTimestamp = now;
            m_serial->flush();
            m_serial->write(BL0939_READ_COMMAND);
            m_serial->write(BL0939_FULL_PACKET);
            Log::verbose("BL0939", "Sending request");
        }
        while(m_serial->available())
        {
            uint8_t d = m_serial->read();
            if ((m_packetIndex == 0) && (d == BL0939_PACKET_HEADER))
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
        }
    }
}

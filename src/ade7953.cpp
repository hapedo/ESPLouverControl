#include "ade7953.h"
#include "profiles.h"
#include "config.h"
#include "log.h"
#include "time.h"

static constexpr uint8_t ADE_ADDRESS = 56; 
static constexpr uint8_t PGA_V_8 =
    0x007;  // PGA_V,  (R/W) Default: 0x00, Unsigned, Voltage channel gain configuration (Bits[2:0])
static constexpr uint8_t PGA_IA_8 =
    0x008;  // PGA_IA, (R/W) Default: 0x00, Unsigned, Current Channel A gain configuration (Bits[2:0])
static constexpr uint8_t PGA_IB_8 =
    0x009;  // PGA_IB, (R/W) Default: 0x00, Unsigned, Current Channel B gain configuration (Bits[2:0])

static constexpr uint32_t AIGAIN_32 =
    0x380;  // AIGAIN, (R/W)   Default: 0x400000, Unsigned,Current channel gain (Current Channel A)(32 bit)
static constexpr uint32_t AVGAIN_32 = 0x381;  // AVGAIN, (R/W)   Default: 0x400000, Unsigned,Voltage channel gain(32 bit)
static constexpr uint32_t AWGAIN_32 =
    0x382;  // AWGAIN, (R/W)   Default: 0x400000, Unsigned,Active power gain (Current Channel A)(32 bit)
static constexpr uint32_t AVARGAIN_32 =
    0x383;  // AVARGAIN, (R/W) Default: 0x400000, Unsigned, Reactive power gain (Current Channel A)(32 bit)
static constexpr uint32_t AVAGAIN_32 =
    0x384;  // AVAGAIN, (R/W)  Default: 0x400000, Unsigned,Apparent power gain (Current Channel A)(32 bit)

static constexpr uint32_t BIGAIN_32 =
    0x38C;  // BIGAIN, (R/W)   Default: 0x400000, Unsigned,Current channel gain (Current Channel B)(32 bit)
static constexpr uint32_t BVGAIN_32 = 0x38D;  // BVGAIN, (R/W)   Default: 0x400000, Unsigned,Voltage channel gain(32 bit)
static constexpr uint32_t BWGAIN_32 =
    0x38E;  // BWGAIN, (R/W)   Default: 0x400000, Unsigned,Active power gain (Current Channel B)(32 bit)
static constexpr uint32_t BVARGAIN_32 =
    0x38F;  // BVARGAIN, (R/W) Default: 0x400000, Unsigned, Reactive power gain (Current Channel B)(32 bit)
static constexpr uint32_t BVAGAIN_32 =
    0x390;  // BVAGAIN, (R/W)  Default: 0x400000, Unsigned,Apparent power gain (Current Channel B)(32 bit)

ADE7953::ADE7953(Mode mode) :
    m_mode((Mode)PROFILE_DEFAULT_ADE7953_MODE),
    m_peripheralIndex(0),
    m_pin0Index(PROFILE_DEFAULT_ADE7953_PIN0_GPIO),
    m_pin1Index(PROFILE_DEFAULT_ADE7953_PIN1_GPIO),
    m_pinResetIndex(PROFILE_DEFAULT_ADE7953_RESET_GPIO),
    m_refreshPeriod(PROFILE_DEFAULT_ADE7953_PERIOD_MILLI),
    m_i2c(nullptr),
    m_serial(nullptr),
    m_lastReadTimestamp(0)
{
    appendDescriptor("Power factor 1", "%", ".0f");
    appendDescriptor("Power factor 2", "%", ".0f");
    appendDescriptor("Apparent power 1", "W", ".0f");
    appendDescriptor("Apparent power 2", "W", ".0f");
    appendDescriptor("Active power 1", "W", ".0f");
    appendDescriptor("Active power 2", "W", ".0f");
    appendDescriptor("Reactive power 1", "W", ".0f");
    appendDescriptor("Reactive power 2", "W", ".0f");
    appendDescriptor("Current 1", "A", ".3f");
    appendDescriptor("Current 2", "A", ".3f");
    appendDescriptor("Voltage", "V", ".0f");
    appendDescriptor("Frequency", "Hz", ".0f");
}

void ADE7953::init()
{
    m_i2c = nullptr;
    m_serial = nullptr;
    bool ok = false;
    m_mode = (Mode)Config::getInt("ade7953/mode", (Mode)PROFILE_DEFAULT_ADE7953_MODE);
    m_peripheralIndex = Config::getInt("ade7953/peripheral", PROFILE_DEFAULT_ADE7953_PERIPHERAL);
    m_pin0Index = Config::getInt("ade7953/pin0_gpio", PROFILE_DEFAULT_ADE7953_PIN0_GPIO);
    m_pin1Index = Config::getInt("ade7953/pin1_gpio", PROFILE_DEFAULT_ADE7953_PIN1_GPIO);
    m_pinResetIndex = Config::getInt("ade7953/reset_gpio", PROFILE_DEFAULT_ADE7953_RESET_GPIO);
    m_refreshPeriod = Config::getInt("ade7953/refresh_milli", PROFILE_DEFAULT_ADE7953_PERIOD_MILLI);

    pinMode(m_pinResetIndex, OUTPUT);
    digitalWrite(m_pinResetIndex, 1);

    if (m_mode == M_UART)
    {
        if (m_peripheralIndex == 0)
            m_serial = &Serial;
        else if (m_peripheralIndex == 1)
            m_serial = &Serial1;
#ifdef ESP32
        else if (m_peripheralIndex == 2)
            m_serial = &Serial2;
#endif
        if (m_serial)
        {
#ifdef ESP32
            m_serial->begin(4800, SERIAL_8N1, m_pin0Index, m_pin1Index);
#else
            m_serial->begin(4800, SERIAL_8N1);
#endif
            m_serial->flush();
            ok = true;
            Log::info("ADE7953", "Initialized, serial=%d, rx=%d, tx=%d, reset=%d, refresh period=%d ms", m_peripheralIndex, m_pin0Index, m_pin1Index, m_pinResetIndex, m_refreshPeriod);
        }
    }
    else if (m_mode == M_I2C)
    {
        if (m_peripheralIndex == 0)
            m_i2c = &Wire;
        else if (m_peripheralIndex == 1)
            m_i2c = &Wire1;
        if (m_i2c)
        {
            m_i2c->begin(m_pin0Index, m_pin1Index);
            ok = true;
            Log::info("ADE7953", "Initialized, i2c=%d, sda=%d, scl=%d, refresh period=%d ms", m_peripheralIndex, m_pin0Index, m_pin1Index, m_refreshPeriod);
        }
        else
        {
            Log::error("ADE7953", "Unable to initialize, bad configuration");
        }
    }
    if (ok)
    {
        // this->ade_write_8(0x0010, 0x04);
        write8(0x00FE, 0xAD);
        write16(0x0120, 0x0030);
        uint32_t val32;
        if (!read32(AWGAIN_32, val32))
        {
            Log::error("ADE7953", "Unable to read register - power measurement disabled");
            m_serial = nullptr;
            m_i2c = nullptr;
        }
        // Set gains
/*        write8(PGA_V_8, pga_v_);
        write8(PGA_IA_8, pga_ia_);
        write8(PGA_IB_8, pga_ib_);
        write32(AVGAIN_32, vgain_);
        write32(AIGAIN_32, aigain_);
        write32(BIGAIN_32, bigain_);
        write32(AWGAIN_32, awgain_);
        write32(BWGAIN_32, bwgain_);*/
        // Read back gains for debugging
/*
        this->ade_read_8(PGA_V_8, &pga_v_);
        this->ade_read_8(PGA_IA_8, &pga_ia_);
        this->ade_read_8(PGA_IB_8, &pga_ib_);
        this->ade_read_32(AVGAIN_32, &vgain_);
        this->ade_read_32(AIGAIN_32, &aigain_);
        this->ade_read_32(BIGAIN_32, &bigain_);
        this->ade_read_32(AWGAIN_32, &awgain_);
        this->ade_read_32(BWGAIN_32, &bwgain_);
        */
    }
}

String ADE7953::getChipInfo() const
{
    return "ADE7953";
}

String ADE7953::getConfiguration()
{
    String result = "{";
    if (m_mode == M_UART)
    {
        result = result + "\"mode\":\"uart\",";
        result = result + "\"serial\":" + String(m_peripheralIndex) + ",";
        result = result + "\"rx_gpio\":" + String(m_pin0Index) + ",";
        result = result + "\"tx_gpio\":" + String(m_pin1Index) + ",";
        result = result + "\"reset_gpio\":" + String(m_pinResetIndex) + ",";
        result = result + "\"refresh_period_milli\":" + String(m_refreshPeriod);
    }
    else if (m_mode == M_I2C)
    {
        result = result + "\"mode\":\"i2c\",";
        result = result + "\"i2c\":" + String(m_peripheralIndex) + ",";
        result = result + "\"sda_gpio\":" + String(m_pin0Index) + ",";
        result = result + "\"scl_gpio\":" + String(m_pin1Index) + ",";
        result = result + "\"reset_gpio\":" + String(m_pinResetIndex) + ",";
        result = result + "\"refresh_period_milli\":" + String(m_refreshPeriod);
    }
    result = result + "}";
    return result;
}

void ADE7953::setConfiguration(String config, bool save)
{
    DynamicJsonDocument json(512);
    DeserializationError error = deserializeJson(json, config);
    if (error) 
    {
        Log::error("ADE7953", "Error while parsing config JSON: %s", error.c_str());
    }
    else
    {
        if (json.containsKey("mode") && (json["mode"] == "uart"))
            m_mode = M_UART;
        if (json.containsKey("mode") && (json["mode"] == "i2c"))
            m_mode = M_I2C;
        if (json.containsKey("reset_gpio"))
            m_pinResetIndex = json["reset_gpio"].as<uint8_t>();
        if (m_mode == M_UART)
        {
            if (json.containsKey("serial"))
                m_peripheralIndex = json["serial"].as<uint8_t>();
            if (json.containsKey("rx_gpio"))
                m_pin0Index = json["rx_gpio"].as<uint8_t>();
            if (json.containsKey("tx_gpio"))
                m_pin1Index = json["tx_gpio"].as<uint8_t>();
            if (json.containsKey("refresh_period_milli"))
            {
                m_refreshPeriod = json["refresh_period_milli"].as<uint32_t>();
                if (m_refreshPeriod < PROFILE_DEFAULT_ADE7953_PERIOD_MILLI)
                    m_refreshPeriod = PROFILE_DEFAULT_ADE7953_PERIOD_MILLI;
            }
        }
        else if (m_mode == M_I2C)
        {
            if (json.containsKey("i2c"))
                m_peripheralIndex = json["i2c"].as<uint8_t>();
            if (json.containsKey("sda_gpio"))
                m_pin0Index = json["sda_gpio"].as<uint8_t>();
            if (json.containsKey("scl_gpio"))
                m_pin1Index = json["scl_gpio"].as<uint8_t>();
            if (json.containsKey("refresh_period_milli"))
            {
                m_refreshPeriod = json["refresh_period_milli"].as<uint32_t>();
                if (m_refreshPeriod < PROFILE_DEFAULT_ADE7953_PERIOD_MILLI)
                    m_refreshPeriod = PROFILE_DEFAULT_ADE7953_PERIOD_MILLI;
            }
        }
        if (save)
        {
            Config::setInt("ade7953/mode", m_mode);
            Config::setInt("ade7953/peripheral", m_peripheralIndex);
            Config::setInt("ade7953/pin0_gpio", m_pin0Index);
            Config::setInt("ade7953/pin1_gpio", m_pin1Index);
            Config::setInt("ade7953/reset_gpio", m_pinResetIndex);
            Config::setInt("ade7953/refresh_milli", m_refreshPeriod);
            Config::flush();
            Log::info("ADE7953", "Configuration saved");
        }
        init();
    }
}

void ADE7953::write8(uint16_t reg, uint8_t value)
{
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0xca);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        m_serial->write(value);
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        m_i2c->write(value);
        if (m_i2c->endTransmission() != 0)
            Log::error("ADE7953", "Unable to write register %x, no ACK?", reg);
    }
}

void ADE7953::write16(uint16_t reg, uint16_t value)
{
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0xca);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        m_serial->write(value >> 8);
        m_serial->write(value & 0xff);
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        m_i2c->write(value >> 8);
        m_i2c->write(value & 0xff);
        if (m_i2c->endTransmission() != 0)
            Log::error("ADE7953", "Unable to write register %x, no ACK?", reg);

    }
}

void ADE7953::write32(uint16_t reg, uint32_t value)
{
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0xca);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        m_serial->write(value >> 24);
        m_serial->write((value >> 16) & 0xff);
        m_serial->write((value >> 8) & 0xff);
        m_serial->write(value & 0xff);
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        m_i2c->write(value >> 24);
        m_i2c->write((value >> 16) & 0xff);
        m_i2c->write((value >> 8) & 0xff);
        m_i2c->write(value & 0xff);
        if (m_i2c->endTransmission() != 0)
            Log::error("ADE7953", "Unable to write register %x, no ACK?", reg);

    }
}

bool ADE7953::read16(uint16_t reg, uint16_t& value)
{
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0x35);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        delay(1);
        value = 0;
        uint8_t cnt = 0;
        uint64_t start = Time::nowRelativeMilli();
        while(Time::nowRelativeMilli() < start + 500)
        {
            while(m_serial->available())
            {
                value <<= 8;
                value |= m_serial->read();
                cnt++;
                if (cnt == 2)
                    return true;
            }
        }
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        if (m_i2c->endTransmission() == 0)
        {
            m_i2c->requestFrom(ADE_ADDRESS, (uint8_t)1);
            value = 0;
            if(m_i2c->available() >= 1)
            {
                value |= (uint16_t)m_i2c->read() << 8;
                value |= m_i2c->read();
                return true;
            }
        }
    }
    Log::error("ADE7953", "Error reading 16 bit register %x", reg);
    return false;
}

bool ADE7953::read32(uint16_t reg, uint32_t& value)
{
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0x35);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        delay(1);
        value = 0;
        uint8_t cnt = 0;
        uint64_t start = Time::nowRelativeMilli();
        while(Time::nowRelativeMilli() < start + 500)
        {
            while(m_serial->available())
            {
                value <<= 8;
                value |= m_serial->read();
                cnt++;
                if (cnt == 4)
                    return true;
            }
        }
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        if (m_i2c->endTransmission() == 0)
        {
            m_i2c->requestFrom(ADE_ADDRESS, (uint8_t)4);
            value = 0;
            if(m_i2c->available() >= 4)
            {
                value |= (uint32_t)m_i2c->read() << 24;
                value |= (uint32_t)m_i2c->read() << 16;
                value |= (uint32_t)m_i2c->read() << 8;
                value |= (uint32_t)m_i2c->read();
                return true;
            }
        }
    }
    Log::error("ADE7953", "Error reading 32 bit register %x", reg);
    return false;
}

void ADE7953::process()
{
    if ((m_i2c) || (m_serial))
    {
        uint64_t now = Time::nowRelativeMilli();
        if (now >= m_lastReadTimestamp + m_refreshPeriod)
        {
            m_lastReadTimestamp = now;
            Log::verbose("ADE7953", "Values update");
            uint16_t val16;
            uint32_t val32;
            if (read16(0x010A, val16))
                setLastValue(0, (float)val16 / (0x7FFF / 100.0f));
            if (read16(0x010B, val16))
                setLastValue(1, (float)val16 / (0x7FFF / 100.0f));
            if (read32(0x0310, val32))
                setLastValue(2, (float)val32 / 154.0f);
            if (read32(0x0311, val32))
                setLastValue(3, (float)val32 / 154.0f);
            if (read32(0x0312, val32))
                setLastValue(4, (float)val32 / 154.0f);
            if (read32(0x0313, val32))
                setLastValue(5, (float)val32 / 154.0f);
            if (read32(0x0314, val32))
                setLastValue(6, (float)val32 / 154.0f);
            if (read32(0x0315, val32))
                setLastValue(7, (float)val32 / 154.0f);
            if (read32(0x031A, val32))
                setLastValue(8, (float)val32 / 100000.0f);
            if (read32(0x031B, val32))
                setLastValue(9, (float)val32 / 100000.0f);
            if (read32(0x031C, val32))
                setLastValue(10, (float)val32 / 26000.0f);
            if (read32(0x010E, val32))
                setLastValue(11, 223750.0f / (1 + (float)val32));
        }
    }
}
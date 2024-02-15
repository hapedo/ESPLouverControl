#include "ade7953.h"
#include <stdlib.h>
#include "profiles.h"
#include "config.h"
#include "log.h"
#include "time.h"

#define ADE_ADDRESS 0x38
// Registers
//
#define ADE_REG_LCYCMODE 0x004
#define ADE_REG_PGA_V 0x007
#define ADE_REG_PGA_IA 0x008
#define ADE_REG_PGA_IB 0x009
#define ADE_REG_UNNAMED 0x0FE

#define ADE_REG_LINECYC 0x101
#define ADE_REG_CONFIG 0x102
#define ADE_REG_PERIOD 0x10E
#define ADE_REG_RESERVED 0x120

#define ADE_REG_AIRMSOS 0x386
#define ADE_REG_VRMSOS 0x388
#define ADE_REG_BIRMSOS 0x392
#define ADE_REG_AVA 0x310
#define ADE_REG_BVA 0x311
#define ADE_REG_AWATT 0x312
#define ADE_REG_BWATT 0x313
#define ADE_REG_AVAR 0x314
#define ADE_REG_BVAR 0x315
#define ADE_REG_IA 0x31A
#define ADE_REG_IB 0x31B
#define ADE_REG_V 0x31C
#define ADE_REG_ANENERGYA 0x31E
#define ADE_REG_ANENERGYB 0x31F
#define ADE_REG_PFA 0x10A
#define ADE_REG_PFB 0x10B

#define ADE_REG_IRQSTATA 0x32D
#define ADE_REG_IRQSTATA_RESET (1 << 20)

#define ADE_REG_VERSION 0x702
#define ADE_REG_EX_REF 0x800

#define ADE_REG_CONFIG_HPFEN (1 << 2)
#define ADE_REG_CONFIG_SWRST (1 << 7)

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
    appendDescriptor("Active power 1", "W", ".0f");
    appendDescriptor("Active power 2", "W", ".0f");
    appendDescriptor("Current 1", "A", ".3f");
    appendDescriptor("Current 2", "A", ".3f");
    appendDescriptor("Energy 1", "Wh", ".3f");
    appendDescriptor("Energy 2", "Wh", ".3f");
    appendDescriptor("Voltage", "V", ".0f");
    appendDescriptor("Frequency", "Hz", ".0f");

    // Default config
    m_config.voltageScale = .0000382602;
    m_config.voltageOffset = -0.068;
    m_config.currentScale0 = 0.00000949523;
    m_config.currentScale1 = 0.00000949523;
    m_config.currentOffset0 = -0.017;
    m_config.currentOffset1 = -0.017;
    m_config.apowerScale0 = (1 / 164.0);
    m_config.apowerScale1 = (1 / 164.0);
    m_config.aenergyScale0 = (1 / 25240.0);
    m_config.aenergyScale1 = (1 / 25240.0);
    m_config.voltagePgaGain = PGA_GAIN_1;
    m_config.currentPgaGain0 = PGA_GAIN_1;
    m_config.currentPgaGain1 = PGA_GAIN_1;
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
#ifdef ESP32
        else if (m_peripheralIndex == 1)
            m_i2c = &Wire1;
#endif
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
        int32_t value;
        if (read(ADE_REG_VERSION, false, value))
        {
            Log::info("ADE7953", "Silicon version: 0x%02x (%d)", (int)value, (int)value);
            write(ADE_REG_CONFIG, ADE_REG_CONFIG_SWRST);
            delay(10);
            int32_t val = 0;
            uint64_t timeout = Time::nowRelativeMilli() + 2000;
            do 
            {
                delay(10);
                if (Time::nowRelativeMilli() > timeout)
                {
                    Log::error("ADE7953", "Unable to initialize - communication timeout");
                    m_serial = nullptr;
                    m_i2c = nullptr;
                    return;
                }
            } while (!read(ADE_REG_IRQSTATA, false, value) || !(value & ADE_REG_IRQSTATA_RESET));
            // Lock comms interface, enable high pass filter
            write(ADE_REG_CONFIG, 0x04);
            // Unlock unnamed (!) register 0x120 (see datasheet, page 18)
            write(ADE_REG_UNNAMED, 0xAD);
            // Set "optimal setting" (see datasheet, page 18)
            write(ADE_REG_RESERVED, 0x30);
            // Program measurement offsets.
            if (m_config.voltageOffset != 0) 
            {
                write(ADE_REG_VRMSOS, (int32_t)(m_config.voltageOffset / m_config.voltageScale));
            }
            if (m_config.currentOffset0 != 0) 
            {
                write(ADE_REG_AIRMSOS, (int32_t)(m_config.currentOffset0 / m_config.currentScale0));
            }
            if (m_config.currentOffset1 != 0) 
            {
                write(ADE_REG_BIRMSOS, (int32_t)(m_config.currentOffset1 / m_config.currentScale1));
            }

            // Set PGA gains.
            if (m_config.voltagePgaGain != 0) 
            {
                write(ADE_REG_PGA_V, m_config.voltagePgaGain);
            }
            if (m_config.currentPgaGain0 != 0) 
            {
                write(ADE_REG_PGA_IA, m_config.currentPgaGain0);
            }
            if (m_config.currentPgaGain1 != 0) 
            {
                write(ADE_REG_PGA_IB, m_config.currentPgaGain1);
            }

            write(ADE_REG_LCYCMODE, 0x40);
        }
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

void ADE7953::setConfiguration(String config, bool save, bool performInit)
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
        if (performInit)
            init();
    }
}

uint8_t ADE7953::getRegSize(uint16_t reg) 
{
    uint8_t size = 1;
    if (reg != ADE_REG_VERSION && reg != ADE_REG_EX_REF) 
    {
        if (reg >= 0x300) size++;
        if (reg >= 0x200) size++;
        if (reg >= 0x100) size++;
    }
    return size;
}

void ADE7953::write(uint16_t reg, int32_t value)
{
    uint8_t size = getRegSize(reg);
    if ((size > 4) || (size < 0))
        return;
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0xca);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        while(size--)
        {
            m_serial->write((value >> (8 * size)) & 0xff);
        }
    }
    if ((m_mode == M_I2C) && (m_i2c))
    {
        m_i2c->beginTransmission(ADE_ADDRESS);
        m_i2c->write(reg >> 8);
        m_i2c->write(reg & 0xff);
        while(size--)
        {
            m_i2c->write((value >> (8 * size)) & 0xff);
        }
        if (m_i2c->endTransmission() != 0)
            Log::error("ADE7953", "Unable to write register %x, no ACK?", reg);
    }
}

bool ADE7953::read(uint16_t reg, bool isSigned, int32_t& value)
{
    int32_t v = 0;
    uint8_t size = getRegSize(reg);
    if ((m_mode == M_UART) && (m_serial))
    {
        m_serial->flush();
        m_serial->write(0x35);
        m_serial->write(reg >> 8);
        m_serial->write(reg & 0xff);
        delay(1);
        uint8_t cnt = 0;
        uint64_t start = Time::nowRelativeMilli();
        while(Time::nowRelativeMilli() < start + 500)
        {
            while(m_serial->available())
            {
                v = (v << 8) | m_serial->read();
                cnt++;
                if (cnt == size)
                    break;
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
            m_i2c->requestFrom((int)ADE_ADDRESS, (int)size);
            value = 0;
            if(m_i2c->available() >= size)
            {
                for(uint8_t i = 0; i < size; i++)
                {
                    v = (v << 8) | m_i2c->read();
                }
            }
            else
            {
                Log::error("ADE7953", "Error reading 16 bit register %x", reg);
                return false;
            }
        }
    }
    if (isSigned) 
    {
        uint32_t signMask = 0;
        if (size == 1) signMask = (1 << 7);
        if (size == 2) signMask = (1 << 15);
        if (size == 3) signMask = (1 << 23);
        if (size == 4) signMask = (1 << 31);
        if ((v & signMask) != 0) 
        {
            v &= ~signMask;
            v |= (1 << 31);
        }
    }
    value = (int32_t)v;    
    return true;
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
            int32_t value;
            // Power factor A
            if (read(ADE_REG_PFA, false, value))
            {
                  // bit 15 is indicationg the sign and is part of the calculation
                float pf = (value & (1 << 15)) ? /*negative sign*/ -(32767.0 / value) : /*positive sign*/ (value * 0.000030518);
                setLastValue(0, pf);
            }
            // Power factor B
            if (read(ADE_REG_PFB, false, value))
            {
                  // bit 15 is indicationg the sign and is part of the calculation
                float pf = (value & (1 << 15)) ? /*negative sign*/ -(32767.0 / value) : /*positive sign*/ (value * 0.000030518);
                setLastValue(1, pf);
            }
            // Active power A
            if (read(ADE_REG_AWATT, true, value))
            {
                float watts = abs((float)value * m_config.apowerScale0);
                setLastValue(2, watts);
            }
            // Active power B
            if (read(ADE_REG_BWATT, true, value))
            {
                float watts = abs((float)value * m_config.apowerScale1);
                setLastValue(3, watts);
            }
            // Current A
            if (read(ADE_REG_IA, true, value))
            {
                float amperes = abs((float)value * m_config.currentScale0);
                setLastValue(4, amperes);
            }
            // Current B
            if (read(ADE_REG_IB, true, value))
            {
                float amperes = abs((float)value * m_config.currentScale1);
                setLastValue(5, amperes);
            }
            // Energy A
            if (read(ADE_REG_ANENERGYA, true, value)) 
            {
                float wh = (float)value * m_config.aenergyScale0;
                setLastValue(6, wh);
            }
            // Energy B
            if (read(ADE_REG_ANENERGYB, true, value)) 
            {
                float wh = (float)value * m_config.aenergyScale1;
                setLastValue(7, wh);
            }
            // Voltage
            if (read(ADE_REG_V, false, value))
                setLastValue(8, (float)value * m_config.voltageScale);
            // Frequency
            if (read(ADE_REG_PERIOD, false, value))
            {
                float hertz = 223750.0f / ((float) value + 1);
                setLastValue(9, hertz);
            }
        }
    }
}
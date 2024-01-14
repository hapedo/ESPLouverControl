#pragma once
#include "power_meas_device.h"

class BL0939 : public PowerMeasDevice
{
public:
    BL0939();

    void init() override;

    String getChipInfo() const override;

    String getConfiguration() override;

    void setConfiguration(String config, bool save = true, bool performInit = false) override;

    void process() override;

protected:

private:

    static constexpr float BL0939_IREF = 324004 * 1 / 1.218;
    static constexpr float BL0939_UREF = 79931 * 0.51 * 1000 / (1.218 * (5 * 390 + 0.51));
    static constexpr float BL0939_PREF = 4046 * 1 * 0.51 * 1000 / (1.218 * 1.218 * (5 * 390 + 0.51));
    static constexpr float BL0939_EREF = 3.6e6 * 4046 * 1 * 0.51 * 1000 / (1638.4 * 256 * 1.218 * 1.218 * (5 * 390 + 0.51));

    struct ube24_t {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
        uint8_t l;
        uint8_t m;
        uint8_t h;
    } __attribute__((packed));

    struct ube16_t {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
        uint8_t l;
        uint8_t h;
    } __attribute__((packed));

    struct sbe24_t {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
        uint8_t l;
        uint8_t m;
        int8_t h;
    } __attribute__((packed));

    // Caveat: All these values are big endian (low - middle - high)

    union DataPacket {  // NOLINT(altera-struct-pack-align)
        uint8_t raw[35];
        struct {
            uint8_t frame_header;  // 0x55 according to docs
            ube24_t ia_fast_rms;
            ube24_t ia_rms;
            ube24_t ib_rms;
            ube24_t v_rms;
            ube24_t ib_fast_rms;
            sbe24_t a_watt;
            sbe24_t b_watt;
            sbe24_t cfa_cnt;
            sbe24_t cfb_cnt;
            ube16_t tps1;
            uint8_t RESERVED1;  // value of 0x00
            ube16_t tps2;
            uint8_t RESERVED2;  // value of 0x00
            uint8_t checksum;   // checksum
        };
    } __attribute__((packed));

    static bool validateChecksum(const DataPacket *data);

    uint32_t to_uint32_t(ube24_t input);

    int32_t to_int32_t(sbe24_t input);

    HardwareSerial* m_serial;
    uint8_t m_serialIndex;
    uint8_t m_rxGpio;
    uint8_t m_txGpio;
    uint32_t m_refreshPeriod;
    uint8_t m_packetIndex;
    DataPacket m_packet;
    uint64_t m_lastReadTimestamp;
};
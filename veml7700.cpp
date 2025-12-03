#include "veml7700.h"

//
// VEML7700 Register map
//
#define VEML7700_REG_ALS_CONF    0x00
#define VEML7700_REG_ALS_WH      0x01
#define VEML7700_REG_ALS_WL      0x02
#define VEML7700_REG_ALS_PS      0x03
#define VEML7700_REG_ALS_DATA    0x04
#define VEML7700_REG_WHITE_DATA  0x05
#define VEML7700_REG_ALS_INT     0x06

// Default config: 100ms IT, gain x1, auto-modes disabled
#define VEML7700_DEFAULT_CONF    0x0000

// sensitivity table (lux per count) based on datasheet
static float CalcLuxCoefficient(u16 als_conf)
{
    // bits:
    // gain: ALS_CONF[12:11]
    // IT:   ALS_CONF[ 9: 6]
    u16 gain = (als_conf >> 11) & 0x03;
    u16 it   = (als_conf >> 6) & 0x0F;

    float gain_factor;
    switch (gain)
    {
        case 0: gain_factor = 1.0f;   break;  // x1
        case 1: gain_factor = 2.0f;   break;  // x2
        case 2: gain_factor = 0.125f; break;  // x1/8
        case 3: gain_factor = 0.25f;  break;  // x1/4
        default: gain_factor = 1.0f;  break;
    }

    float it_factor;
    switch (it)
    {
        case 0: it_factor = 25.0f;  break;
        case 1: it_factor = 50.0f;  break;
        case 2: it_factor = 100.0f; break;
        case 3: it_factor = 200.0f; break;
        case 8: it_factor = 400.0f; break;
        case 12: it_factor = 800.0f; break;
        default: it_factor = 100.0f; break;   // fallback
    }

    // Datasheet: lux = raw * 0.0576 * (2/gain) * (100ms/IT)
    // Adjusted for our interpretation
    float coeff = 0.0576f * (1.0f / gain_factor) * (100.0f / it_factor);
    return coeff;
}

CVEML7700::CVEML7700(CI2CMaster &pI2C)
: m_pI2C(pI2C)
{
}

bool CVEML7700::Initialize()
{
    CLogger::Get()->Write ("VEML7700", LogNotice, "Initializing VEML7700 sensor...");
    return WriteReg(VEML7700_REG_ALS_CONF, VEML7700_DEFAULT_CONF);
}

float CVEML7700::GetLux()
{
    u16 raw = 0;
    if (!ReadReg(VEML7700_REG_ALS_DATA, raw))
        return -1.0f;

    u16 conf = 0;
    if (!ReadReg(VEML7700_REG_ALS_CONF, conf))
        return -1.0f;

    float coeff = CalcLuxCoefficient(conf);
    return raw * coeff;
}

bool CVEML7700::WriteReg(u8 reg, u16 value)
{
    int res = 0;
    u8 data[3];
    data[0] = reg;
    data[1] = value & 0xFF;         // LSB
    data[2] = (value >> 8) & 0xFF;  // MSB

    res = m_pI2C.Write(VEML7700_I2C_ADDR, data, sizeof(data));
    if (res != sizeof(data)){
        CLogger::Get()->Write ("VEML7700", LogNotice, "i2c error: %d", res);
        return false;
    }
    return true;
}

bool CVEML7700::ReadReg(u8 reg, u16 &value)
{
    int res = 0;
    // Write register pointer
    res = m_pI2C.Write(VEML7700_I2C_ADDR, &reg, 1);
    if (res != 1){
        CLogger::Get()->Write ("VEML7700", LogNotice, "i2c error: %d", res);
        return false;
    }

    u8 buf[2];
    res = m_pI2C.Read(VEML7700_I2C_ADDR, buf, sizeof(buf));
    if (res != sizeof(buf)){
        CLogger::Get()->Write ("VEML7700", LogNotice, "i2c error: %d", res);
        return false;
    }

    // Received LSB first
    value = buf[0] | (buf[1] << 8);
    return true;
}

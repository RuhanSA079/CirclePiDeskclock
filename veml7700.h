#include <circle/i2cmaster.h>
#include <circle/types.h>
#include <circle/logger.h>
#include <circle/util.h>

class CVEML7700
{
    public:
        CVEML7700(CI2CMaster &pI2C);

        bool Initialize();      // returns false on I2C/device error
        float GetLux();         // read ALS registers and convert to lux

    private:
        bool WriteReg(u8 reg, u16 value);
        bool ReadReg(u8 reg, u16 &value);

    private:
        CI2CMaster &m_pI2C;
        static const u8 VEML7700_I2C_ADDR = 0x10;   // 7-bit
};
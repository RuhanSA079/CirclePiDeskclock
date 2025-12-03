#include <circle/i2cmaster.h>
#include <circle/types.h>
#include <circle/time.h>
#include <circle/timer.h>
#include <circle/logger.h>

class CDS1307RTC
{
public:
    CDS1307RTC(CI2CMaster &I2CMaster);

    bool Init();
    time_t GetTimeFromRTC();
    bool SetTimeToRTC(time_t);

private:
    CI2CMaster &m_I2CMaster;
    CTime t;
    u8 DecToBcd(u8 val) { return ((val / 10) << 4) | (val % 10); }
    u8 BcdToDec(u8 val) { return ((val >> 4) * 10) + (val & 0x0F); }
};

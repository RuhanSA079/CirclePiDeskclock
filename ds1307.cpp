#include "ds1307.h"

//TODO: Use logger in constructor for debugging

const u8 RTC_I2C_ADDR = 0x68;
CTime t;

CDS1307RTC::CDS1307RTC(CI2CMaster &I2CMaster): 
m_I2CMaster(I2CMaster)
{
}

//See if we can talk to the RTC.
bool CDS1307RTC::Init()
{
    int res = 0;
    // Just attempt a read of the seconds register
    u8 reg = 0x00;
    u8 sec = 0;

    res = m_I2CMaster.Write(RTC_I2C_ADDR, &reg, 1);
    if (res != 1){
        CLogger::Get()->Write ("RTC", LogNotice, "i2c error: %d", res);
        return false;
    }

    res = m_I2CMaster.Read(RTC_I2C_ADDR, &sec, 1);
    if (res != 1){
        CLogger::Get()->Write ("RTC", LogNotice, "i2c error: %d", res);
        return false;
    }

    //RuhanvdB -> Get the OSC bit and set it if not set.
    reg = 0x0F;
    res = m_I2CMaster.Write(RTC_I2C_ADDR, &reg, 1);
    if (res != 1){
        CLogger::Get()->Write ("RTC", LogNotice, "i2c error: %d", res);
        return false;
    }

    res = m_I2CMaster.Read(RTC_I2C_ADDR, &sec, 1);
    if (res != 1){
        CLogger::Get()->Write ("RTC", LogNotice, "i2c error: %d", res);
        return false;
    }

    // If (EOSC) bit is set, clear it, to start the RTC ticking.
    if (sec & 0x80)
    {
        CLogger::Get()->Write("RTC", LogNotice, "RTC Clock halt set, resetting...");
        sec &= 0x7F;
        u8 data[1] = {sec};
        res = m_I2CMaster.Write(RTC_I2C_ADDR, data, 1);
        if (res != 1){
            CLogger::Get()->Write ("RTC", LogNotice, "i2c error: %d, unable to start the OSC", res);
        }
    }

    return true;
}

bool CDS1307RTC::SetTimeToRTC(time_t timeToSet)
{
    t.Set(timeToSet);

    CLogger::Get()->Write("RTC", LogNotice, "Setting RTC time: %02u:%02u:%02u, date: %02u-%02u-%04u", t.GetHours(), t.GetMinutes(), t.GetSeconds(), t.GetMonthDay(), t.GetMonth(), t.GetYear());

    u8 data[8];

    data[0] = 0x00;  // register pointer
    data[1] = DecToBcd(t.GetSeconds());
    data[2] = DecToBcd(t.GetMinutes());
    data[3] = DecToBcd(t.GetHours());
    data[4] = DecToBcd(t.GetWeekDay() + 1);   // convert 0–6 → 1–7
    data[5] = DecToBcd(t.GetMonthDay());
    data[6] = DecToBcd(t.GetMonth());     // convert 0–11 → 1–12
    data[7] = DecToBcd(t.GetYear() - 2000);  // store year offset from 2000

    int res = m_I2CMaster.Write(RTC_I2C_ADDR, data, sizeof(data));
    if (res != 7){
        return false;
    }
    return true;
}

//RuhanvdB -> Fetch the time from the RTC using i2c and then create the time_t object based on those values.
time_t CDS1307RTC::GetTimeFromRTC()
{
    int res = 0;
    u8 reg = 0x00;
    u8 buf[7];

    //RuhanvdB -> Set Address pointer to 0x00
    res = m_I2CMaster.Write(RTC_I2C_ADDR, &reg, 1);
    if (res != 1){
        return 0;
    }

    //RuhanvdB -> Start reading from 0x00 (mentioned above) until 7 registers are filled.
    res = m_I2CMaster.Read(RTC_I2C_ADDR, buf, sizeof(buf));

    if (res != 7){
        return 0;
    }

    t.SetTime(
        BcdToDec(buf[2] & 0x3F),   // hours
        BcdToDec(buf[1]),          // minutes
        BcdToDec(buf[0] & 0x7F)    // seconds
    );

    t.SetDate(
        BcdToDec(buf[4]),          // day of month
        BcdToDec(buf[5]),          // month 1–12
        2000 + BcdToDec(buf[6])    // year
    );

    CLogger::Get()->Write("RTC", LogNotice, "Getting RTC time: %02u:%02u:%02u, date: %02u-%02u-%04u", t.GetHours(), t.GetMinutes(), t.GetSeconds(), t.GetMonthDay(), t.GetMonth(), t.GetYear());
    return t.Get();
}
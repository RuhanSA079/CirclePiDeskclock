#include "kernel.h"

static const char FromKernel[] = "kernel";
#define REFRESH_FROM_RTC_INTERVAL 300
#define PWM_CLOCK_RATE	1000000
#define PWM_RANGE	1024
#define CHAN1	PWM_CHANNEL1
const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//RuhanvdB -> CKernel constructor
CKernel::CKernel (void): 
m_Timer (&m_Interrupt), 
m_Logger (m_Options.GetLogLevel (), &m_Timer), 
m_SPIMaster (15000000),
m_I2CMaster(1), 
m_PWMPin(18, GPIOModeAlternateFunction5),
m_PWMOutput(PWM_CLOCK_RATE, PWM_RANGE, TRUE)
{
    //m_ActLED.Blink(2); //Show kernel started.
}

CKernel::~CKernel (void)
{
	//RuhanvdB -> Nothing here in destructor
}

boolean CKernel::Initialize (void)
{
    boolean bOK = TRUE;
	if (bOK)
	{
		bOK = m_Serial.Initialize(115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Serial;
		}

		bOK = m_Logger.Initialize(pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize();
	}

	if (bOK)
	{
		bOK = m_SPIMaster.Initialize();
	}

	if (bOK)
	{
		bOK = m_I2CMaster.Initialize();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	int brightness = 0;
	int rtcRefreshCounter = 0;
	CTime systemTime;
	CString *dateString = new CString;
	CString *timeHour = new CString;
	CString *timeMin = new CString;
	u8 min = 0;
	bool updateTimeonLCD = false;

	bool PWM_OK = false;
	bool LUX_OK = false;

	m_Logger.Write (FromKernel, LogNotice, "ST7735 DeskClock v0.5 compiled on: " __DATE__ " " __TIME__);

	m_Logger.Write (FromKernel, LogNotice, "Configuring heartbeat LED...");
	//Start heartbeat LED task.
	//new CLEDTaskHB (&m_ActLED);
	
	//RuhanvdB -> Setup GPIO pins
	m_Logger.Write (FromKernel, LogNotice, "Configuring GPIO...");
	CGPIOPin PinDC(22, GPIOModeOutput);
	CGPIOPin PinRST(27, GPIOModeOutput);

	//RuhanvdB -> Initialise the display library
	m_Logger.Write (FromKernel, LogNotice, "Starting display driver...");
	CST7735SDisplay Display(m_SPIMaster, PinDC, PinRST);

	//RuhanvdB -> TODO: if PWM fails to start, reassign backlight pin to normal GPIO and drive high.
	m_Logger.Write (FromKernel, LogNotice, "Setting up display PWM output...");
	PWM_OK = m_PWMOutput.Start();
	if (!PWM_OK)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot start PWM output!");
	}
	else
	{
		m_Logger.Write (FromKernel, LogNotice, "PWM setup OK...");
		m_PWMOutput.Write(CHAN1, 1024);
	}

	//RuhanvdB -> detect/use RTC clock stuff 	
	m_Logger.Write (FromKernel, LogNotice, "Starting RTC driver...");
	CDS1307RTC RTC(m_I2CMaster);

	bool rtc_OK = RTC.Init();
	if (rtc_OK){
		m_Logger.Write (FromKernel, LogNotice, "RTC OK, getting time...");
		systemTime.Set(RTC.GetTimeFromRTC());
		if (systemTime.GetYear() <= 2000){
			m_Logger.Write (FromKernel, LogNotice, "RTC time out of sync, syncing RTC to last known compile time...");
			systemTime.SetDate(30, 11, 2025);
			systemTime.SetTime(13, 00, 00);
			//SetTimeFromCompile(systemTime);
			RTC.SetTimeToRTC(systemTime.Get());
		}
	}else{
		m_Logger.Write (FromKernel, LogNotice, "Failed to talk to the RTC, using compiled time...");
		//SetTimeFromCompile(systemTime);
		systemTime.SetDate(29, 11, 2025);
		systemTime.SetTime(13, 00, 00);
	}

	//RuhanvdB -> Fetch the datetime and set it to the global clock.
	m_Logger.Write (FromKernel, LogNotice, "Setting up system time...");
	m_Timer.SetTime(systemTime.Get());

	m_Logger.Write (FromKernel, LogNotice, "Starting VEML7700 driver...");
	CVEML7700 LUX(m_I2CMaster);

	LUX_OK = LUX.Initialize();
	if (LUX_OK && PWM_OK){
		m_Logger.Write (FromKernel, LogNotice, "System able to use display dimming...");
		//m_PWMOutput.Write(CHAN1, BrightnessFromLux(LUX.GetLux()));
	}

	m_Logger.Write (FromKernel, LogNotice, "Initializing display...");
	Display.Init();

	m_Logger.Write (FromKernel, LogNotice, "App startup complete, now running...");

	Display.FillScreen(0x0000);
	Display.DrawString(0, 0, "RPiZ Desk Clock", 0xFFFF, 0x0000);
	Display.DrawString(0, 18, "by RuhanSA079", 0xFFFF, 0x0000);
	Display.DrawString(0, 36, "C++ baremetal!", 0xFFFF, 0x0000);
	Display.DrawString(0, 54, "github.com/rsta2/circle", 0xFFFF, 0x0000);
	Display.Refresh();
	m_Timer.MsDelay(8000);
    
	//RuhanvdB -> On first boot, set the variables, this is to avoid 1 second screen flicker when drawn over.
	systemTime.Set(m_Timer.GetTime());
	min = systemTime.GetMinutes();
	updateTimeonLCD = true;

	Display.FillScreen(0x0000);

	while (1) {
		//RuhanvdB -> Get the time from the timer to CTime
		if (rtc_OK){
			rtcRefreshCounter++;
			if(rtcRefreshCounter >= REFRESH_FROM_RTC_INTERVAL){
				rtcRefreshCounter = 0;
				if (rtc_OK){
					systemTime.Set(RTC.GetTimeFromRTC());
					m_Timer.SetTime(systemTime.Get());
				}else{
					rtc_OK = RTC.Init();
					if (rtc_OK){
						systemTime.Set(RTC.GetTimeFromRTC());
						m_Timer.SetTime(systemTime.Get());
					}
				}
			}
			systemTime.Set(m_Timer.GetTime());
		}else{
			systemTime.Set(m_Timer.GetTime());
		}
		
		// if (PWM_OK && LUX_OK){
		// 	float lux = LUX.GetLux();
		// 	m_Logger.Write (FromKernel, LogNotice, "Lux value: %f", lux);

		// 	if (brightness != BrightnessFromLux(lux)){
		// 		brightness = BrightnessFromLux(lux);
		// 		m_PWMOutput.Write(CHAN1, brightness);
		// 	}
		// }

		if (min != systemTime.GetMinutes()){
			min = systemTime.GetMinutes();
			updateTimeonLCD = true;
		}

		if (updateTimeonLCD){
			dateString->Format("%2u %s %04u", systemTime.GetMonthDay(), monthNames[systemTime.GetMonth()-1], systemTime.GetYear());
			timeHour->Format("%02u", systemTime.GetHours());
			timeMin->Format("%02u", min);

			Display.DrawStringScaled(18, -5, timeHour->c_str(), 0xFFFF, 0x0000, 6);
			Display.DrawStringScaled(18, 80, timeMin->c_str(), 0xFFFF, 0x0000, 6);
			Display.DrawString(19, 72, dateString->c_str(), 0xFFFF, 0x0000);
			Display.Refresh();
			updateTimeonLCD = false;
		}

		m_Timer.MsDelay(1000);
	}
	return ShutdownHalt;
}

int CKernel::MonthFromString(const char* mon)
{
    if (strcmp(mon, "Jan") == 0) return 1;
    if (strcmp(mon, "Feb") == 0) return 2;
    if (strcmp(mon, "Mar") == 0) return 3;
    if (strcmp(mon, "Apr") == 0) return 4;
    if (strcmp(mon, "May") == 0) return 5;
    if (strcmp(mon, "Jun") == 0) return 6;
    if (strcmp(mon, "Jul") == 0) return 7;
    if (strcmp(mon, "Aug") == 0) return 8;
    if (strcmp(mon, "Sep") == 0) return 9;
    if (strcmp(mon, "Oct") == 0) return 10;
    if (strcmp(mon, "Nov") == 0) return 11;
    if (strcmp(mon, "Dec") == 0) return 12;
    return 1;
}

void CKernel::SetTimeFromCompile(CTime &systemTime)
{
    const char* date = __DATE__;   // "Nov 30 2025"
    const char* time = __TIME__;   // "14:34:21"

    //
    // Parse date
    //
    char monStr[4];
    monStr[0] = date[0];
    monStr[1] = date[1];
    monStr[2] = date[2];
    monStr[3] = '\0';

    int day = (date[4] == ' ' ? date[5] - '0' : (date[4]-'0')*10 + (date[5]-'0'));

    int year =
        (date[7]-'0')*1000 +
        (date[8]-'0')*100 +
        (date[9]-'0')*10 +
        (date[10]-'0');

    int month = MonthFromString(monStr);

    //
    // Parse time "hh:mm:ss"
    //
    int hour = (time[0]-'0')*10 + (time[1]-'0');
    int min  = (time[3]-'0')*10 + (time[4]-'0');
    int sec  = (time[6]-'0')*10 + (time[7]-'0');

    //
    // Set system time
    //
    systemTime.SetTime(hour, min, sec);
    systemTime.SetDate(year, month, day);
}

int CKernel::BrightnessFromLux(float lux)
{
	const int MIN_PWM = 80;
	const int MAX_PWM = 900;

	int pwm;

	if (lux < 1.0f)
		pwm = MIN_PWM;                       // dark room
	else if (lux < 10.0f)
		pwm = MIN_PWM + (lux * 10);          // dim → indoor
	else if (lux < 100.0f)
		pwm = 180 + (lux * 1.2f);            // indoor → bright indoor
	else if (lux < 1000.0f)
		pwm = 300 + (lux * 0.2f);            // bright indoor → shade
	else
		pwm = MAX_PWM;                       // daylight

	if (pwm < MIN_PWM) pwm = MIN_PWM;
	if (pwm > MAX_PWM) pwm = MAX_PWM;

	return pwm;
}
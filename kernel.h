#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/types.h>
#include <circle/spimaster.h>
#include <circle/i2cmaster.h>
#include <circle/debug.h>
#include <circle/pwmoutput.h>
#include <circle/gpiopin.h>
#include <circle/sched/task.h>
#include <circle/sched/scheduler.h>
#include "st7735s.h"
#include "ds1307.h"
#include "taskHB.h"
#include "veml7700.h"

enum TShutdownMode
{
        ShutdownNone,
        ShutdownHalt,
        ShutdownReboot
};

class CKernel
{
        public:
                CKernel (void);
                ~CKernel (void);
                boolean Initialize (void);
                TShutdownMode Run (void);

        private:
                CActLED                 m_ActLED;
                CKernelOptions          m_Options;
                CDeviceNameService      m_DeviceNameService;
                CSerialDevice           m_Serial;
                CExceptionHandler       m_ExceptionHandler;
                CInterruptSystem        m_Interrupt;
                CTimer                  m_Timer;
                CLogger                 m_Logger;
                CSPIMaster              m_SPIMaster;
                CI2CMaster		m_I2CMaster;
                CGPIOPin		m_PWMPin;
                CPWMOutput		m_PWMOutput;
                //CScheduler		m_Scheduler;
                void SetTimeFromCompile(CTime &systemTime);
                int MonthFromString(const char* mon);
                int BrightnessFromLux(float lux);
};

#endif

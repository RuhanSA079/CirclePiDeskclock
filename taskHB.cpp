#include "taskHB.h"
#include <circle/sched/scheduler.h>

CLEDTaskHB::CLEDTaskHB (CActLED *pActLED)
:	m_pActLED (pActLED)
{
}

CLEDTaskHB::~CLEDTaskHB (void)
{
}

void CLEDTaskHB::Run (void)
{
	while (1)
	{
        m_pActLED->On();
        CScheduler::Get()->MsSleep(50);
        m_pActLED->Off();
        CScheduler::Get()->MsSleep(200);
        m_pActLED->On();
        CScheduler::Get()->MsSleep(50);
        m_pActLED->Off();
		CScheduler::Get()->MsSleep(800);
	}
}
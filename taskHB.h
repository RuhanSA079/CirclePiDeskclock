#include <circle/actled.h>
#include <circle/sched/task.h>
 
class CLEDTaskHB : public CTask
{
public:
	CLEDTaskHB (CActLED *pActLED);
	~CLEDTaskHB (void);

	void Run (void);

private:
	CActLED *m_pActLED;
};
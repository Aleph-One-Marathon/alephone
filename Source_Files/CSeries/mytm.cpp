// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
/*
	Changes:

Jan 30, 2000 (Loren Petrich)
	Did some typecasts
*/

#include <stdlib.h>

#include <Timer.h>

#include "cstypes.h"
#include "csmisc.h"
#include "mytm.h"


struct myTMTask {
	TMTask task;
#ifdef env68k
	long a5;
#endif
	bool (*func)(void);
	long time;
	Boolean insX;
	Boolean primed;
};

#ifdef env68k
#pragma parameter timer_proc(__A1)
#endif

static pascal void timer_proc(
	TMTaskPtr task)
{
	myTMTaskPtr mytask=(myTMTaskPtr)task;
#ifdef env68k
	long savea5;

	savea5=set_a5(mytask->a5);
#endif
	mytask->primed=false;
	if ((*mytask->func)()) {
		mytask->primed=true;
		PrimeTime((QElemPtr)mytask,mytask->time);
	} else {
		mytask->task.tmWakeUp=0;
	}
#ifdef env68k
	set_a5(savea5);
#endif
}

#ifdef env68k
#define timer_upp ((TimerUPP)timer_proc);
#else
static RoutineDescriptor timer_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppTimerProcInfo,timer_proc);
#define timer_upp (&timer_desc)
#endif

myTMTaskPtr myTMSetup(
	long time,
	bool (*func)(void))
{
	myTMTaskPtr result;

	result= new myTMTask;
	if (!result)
		return result;
	result->task.tmAddr=timer_upp;
	result->task.tmWakeUp=0;
	result->task.tmReserved=0;
#ifdef env68k
	result->a5=get_a5();
#endif
	result->func=func;
	result->time=time;
	result->insX=false;
	result->primed=true;
	InsTime((QElemPtr)result);
	PrimeTime((QElemPtr)result,time);
	return result;
}

myTMTaskPtr myXTMSetup(
	long time,
	bool (*func)(void))
{
	myTMTaskPtr result;

	result= new myTMTask;
	if (!result)
		return result;
	result->task.tmAddr=timer_upp;
	result->task.tmWakeUp=0;
	result->task.tmReserved=0;
#ifdef env68k
	result->a5=get_a5();
#endif
	result->func=func;
	result->time=time;
	result->insX=false;
	result->primed=true;
	InsXTime((QElemPtr)result);
	PrimeTime((QElemPtr)result,time);
	return result;
}

myTMTaskPtr myTMRemove(
	myTMTaskPtr task)
{
	if (!task)
		return NULL;
	RmvTime((QElemPtr)task);
	delete task;
	return NULL;
}

void myTMReset(
	myTMTaskPtr task)
{
	if (task->primed) {
		RmvTime((QElemPtr)task);
		task->task.tmWakeUp=0;
		task->task.tmReserved=0;
		if (task->insX) {
			InsXTime((QElemPtr)task);
		} else {
			InsTime((QElemPtr)task);
		}
	}
	task->primed=true;
	PrimeTime((QElemPtr)task,task->time);
}


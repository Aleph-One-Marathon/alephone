#include "timer.h"
#include <MacMemory.h>
#include <Timer.h>
#include <Gestalt.h>

uint32 globalTime;

uint32 Timer::Clicks() {
	if (startTime != 0) {
		return (totalTime+(globalTime-startTime));
	} else {
		return totalTime;
	}
}

class TimerInstaller {
	TMTask task;
	static pascal void TimerCallback(TMTaskPtr tmTaskPtr);
public:
	TimerInstaller();
	~TimerInstaller();
} gTimerInstaller;


TimerInstaller::TimerInstaller() {
	long         gestaltResponse;
	
	OSErr err = HoldMemory(&globalTime, 4);
	
	if (err) return;
	
	if (Gestalt(gestaltTimeMgrVersion, &gestaltResponse) != noErr
		|| (gestaltResponse < gestaltExtendedTimeMgr))
			return;

	task.qLink = NULL;
	task.qType = 0;
	task.tmAddr = NewTimerUPP(TimerCallback);
	task.tmCount = 0;
	task.tmWakeUp = 0;
	task.tmReserved = 0;

	InsXTime((QElemPtr) &task);
 	PrimeTime((QElemPtr) &task, MSECS_PER_CLICK);
}

TimerInstaller::~TimerInstaller() {
	RmvTime((QElemPtr) &task);
	UnholdMemory(&globalTime, 4);
}

pascal void TimerInstaller::TimerCallback(TMTaskPtr tmTaskPtr) {
	globalTime += 1;
	PrimeTime((QElemPtr) tmTaskPtr, MSECS_PER_CLICK);
}

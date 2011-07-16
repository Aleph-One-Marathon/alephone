/*
 * mytm_macintosh.cpp (was mytm.cpp)
 
	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html


	Changes:

Jan 30, 2000 (Loren Petrich)
	Did some typecasts
        
Oct 15, 2001 (Woody Zenfell)
        Renamed this to mytm_macintosh.cpp (from mytm.cpp), so I can also make a mytm_sdl.cpp

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	timer_proc allocated as a NewTimerUPP under carbon
*/

#include <stdlib.h>

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>

#else
#include <Timer.h>

#endif

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
	bool insX;
	bool primed;
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

#if defined(TARGET_API_MAC_CARBON)
static TimerUPP timer_upp = NewTimerUPP(timer_proc);
#else
#ifdef env68k
#define timer_upp ((TimerUPP)timer_proc);
#else
static RoutineDescriptor timer_desc =
	BUILD_ROUTINE_DESCRIPTOR(uppTimerProcInfo,timer_proc);
#define timer_upp (&timer_desc)
#endif
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

// WZ's dummy functions
// ZZZ thought: the Carbon version should probably use a mutex (or maybe just use
// the mytm_sdl stuff wholesale) since I'm not sure that TMTasks, Open Transport
// notifiers, etc. are guaranteed to execute atomically (with respect to one another).
// For Classic, each such task is mutually exclusive with the others, so no problem.
void myTMCleanup(bool) {}
bool take_mytm_mutex() { return true; }	// we LIE so our callers don't freak out.
bool release_mytm_mutex() { return true; }
void mytm_initialize() {}

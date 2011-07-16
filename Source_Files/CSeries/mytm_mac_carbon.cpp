/*
 * mytm_mac_carbon.cpp
 
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

Jan 29, 2002 (Br'fin (Jeremy Parsons)):
	Forked from mytm_macintosh.cpp to use CarbonEvents timer
*/

#include <stdlib.h>

#include <Carbon/Carbon.h>

#include "cstypes.h"
#include "csmisc.h"
#include "mytm.h"


struct myTMTask {
	EventLoopTimerRef task;
	bool (*func)(void);
	long time;
	bool primed;
};

static pascal void timer_proc(
	EventLoopTimerRef inTimer,
	void *task)
{
	myTMTaskPtr mytask=(myTMTaskPtr)task;

	mytask->primed=false;
	if ((*mytask->func)()) {
		mytask->primed=true;
	} else {
		RemoveEventLoopTimer(inTimer);
	}
}

static EventLoopTimerUPP timer_upp = NewEventLoopTimerUPP(timer_proc);

myTMTaskPtr myTMSetup(
	int32 time,
	bool (*func)(void))
{
	myTMTaskPtr result;

	result= new myTMTask;
	if (!result)
		return result;
	result->func=func;
	result->time=time;
	result->primed=true;
	
	InstallEventLoopTimer(GetMainEventLoop(),
		time * kEventDurationMillisecond,
		time * kEventDurationMillisecond,
		timer_upp,
		result,
		&result->task);
	return result;
}

myTMTaskPtr myXTMSetup(
	int32 time,
	bool (*func)(void))
{
	return myTMSetup(time, func);
}

myTMTaskPtr myTMRemove(
	myTMTaskPtr task)
{
	if (!task)
		return NULL;
	if(task->primed)
	{
		RemoveEventLoopTimer(task->task);
	}
	delete task;
	return NULL;
}

void myTMReset(
	myTMTaskPtr task)
{
	if (task->primed) {
		SetEventLoopTimerNextFireTime(task->task,
			task->time * kDurationMillisecond);
	}
	else
	{
		task->primed=true;
		InstallEventLoopTimer(GetMainEventLoop(),
			task->time * kEventDurationMillisecond,
			task->time * kEventDurationMillisecond,
			timer_upp,
			task,
			&task->task);
	}
}

// WZ's dummy function
void myTMCleanup(bool) {}

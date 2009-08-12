#pragma once

#ifndef _TIMER_
#define _TIMER_

/* WARNING!!!

You MUST set the terminate routine of your app to __terminate or your mac will
become very unstable if your app crashes.

*/

	extern uint32 globalTime;

#define MSECS_PER_CLICK 1

class Timer {
uint32 totalTime;
uint32 startTime;
public:
	Timer() {Clear();}
	void Clear() { totalTime=0; startTime=0;}
	void Start() { startTime=globalTime; }
	void Stop() { totalTime += globalTime-startTime; startTime=0;}
	uint32 Clicks();
	float Seconds() {return Clicks()/(1000.0/MSECS_PER_CLICK);}
};

#endif

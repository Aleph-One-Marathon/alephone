/*
 *  mytm_sdl.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  The point of this file is to let us (networking code, in particular) use timing services
 *  with the same source-level API wrapper that Bungie used to access the Mac's Time Manager.
 *
 *  Created by woody on Mon Oct 15 2001.
 *
 *  3 December 2001 (Woody Zenfell): changed dependence on SDL_Threadx's SetRelativeThreadPriority
 *	to simply BoostThreadPriority(), a simpler function with a simpler interface.
 */

// The implementation is built on SDL_thread, and approximates the Time Manager behavior.
// Obviously, it's not a perfect emulation.  :)

// I probably would have made life easier for myself by using SDL_timer instead, but frankly
// the documentation does not inspire me to trust it.  I'll do things on my own.

#include "cseries.h"
#include "thread_priority_sdl.h"
#include "mytm.h"

#include <vector>
#if defined(__MACH__) && defined(__APPLE__)
#include <SDL/SDL_thread.h>
#include <SDL/SDL_timer.h>
#else
#include    <SDL_thread.h>
#include    <SDL_timer.h>
#endif

#ifndef NO_STD_NAMESPACE
using std::vector;
#endif

#ifdef DEBUG
struct myTMTask_profile {
    uint32		mStartTime;
    uint32		mFinishTime;
    uint32		mNumCallsThisReset;
    uint32		mNumCallsTotal;
    int32		mDriftMin;
    int32		mDriftMax;
    uint32		mNumLateCalls;
    uint32		mNumWarmResets;
    uint32		mNumResuscitations;
};
#endif

// Housekeeping structure used in setup, teardown, and execution
struct myTMTask {
    SDL_Thread*		mThread;
    uint32		mPeriod;
    bool 		(*mFunction)(void);
    volatile bool	mKeepRunning;	// set true by myTMSetup; set false by thread or by myTMRemove.
    volatile bool	mIsRunning;	// set true by myTMSetup; set false by thread when about to exit.
    volatile uint32	mResetTime;	// set positive by myTMReset; set to 0 by myTMSetup or by thread.
#ifdef DEBUG
    myTMTask_profile	mProfilingData;
#endif
};



// Function that threads execute - does housekeeping and calls user callback
// Tries to be drift-free.
static int
thread_loop(void* inData) {
    myTMTask*	theTMTask	= (myTMTask*) inData;
    
    uint32	theLastRunTime	= SDL_GetTicks();
    uint32	theCurrentRunTime;
    int32	theDrift	= 0;

	// Added a little later: if we missed a period altogether, we might be pregnant.
	// No really, we should avoid calling the function twice in a row, and here's why.
	bool	missedDeadline	= false;

	// If we miss a period, but the ring packet comes around, the packet-handler "smears" -
	// that is, it effectively manufactures a queue entry since we didn't have one ready for it.
	// If we then try to compensate for missing a period by queueing some action_flags for the
	// chance we missed, we have effectively injected an extra element into the action_flags queue,
	// That means, from then on, it will take an extra GAME_TICK for our inputs to be acted on...
	// in essence, we've added latency.  Yuck.  Who would want to be responsible for that??

	// So, we keep track of whether we missed a deadline this time through, and if so, we don't
	// call the function we're supposed to be calling.  We just go through the motions, and let the
	// (nearly immediate) next pass through the loop call the function.

/* taken out again even later still, modified the queueing behavior in network.cpp instead.
The new wisdom: the other way kept latency to a bare minimum, but left no room for things to jitter, I think.
The "faux queue" approach will, I hope, correctly strike up a balance between low latency and smooth gameplay.
We'll see!
*/

#ifdef DEBUG
    theTMTask->mProfilingData.mStartTime	= theLastRunTime;
#endif

    while(theTMTask->mKeepRunning) {
        // Delay, unless we're at least a period behind schedule
        // Originally, I didn't compute theDelay explicitly as a signed quantity, which
        // made for some VERY long waits if we were running late... :)
        int32	theDelay 	= theTMTask->mPeriod - theDrift;
        if(theDelay > 0)
            SDL_Delay(theDelay);
        else {
			// We missed a deadline!
#ifdef DEBUG
            theTMTask->mProfilingData.mNumLateCalls++;
#endif                
			missedDeadline = true;
		}
        
        // If a reset was requested, pretend we were last called at the reset time, clear the reset,
        // and delay some more if needed.
        // Note: this is a "while" so, in case another reset comes while we are in the Delay() inside
        // this block, we wait longer.
        while(theTMTask->mResetTime > 0) {
			// Whoops, heh, maybe we didn't miss a deadline after all - we were reset.
			missedDeadline = false;
            
			theLastRunTime	= theTMTask->mResetTime;
            theTMTask->mResetTime = 0;
            
#ifdef DEBUG
            theTMTask->mProfilingData.mNumWarmResets++;
            theTMTask->mProfilingData.mStartTime		= theLastRunTime;
            theTMTask->mProfilingData.mNumCallsThisReset	= 0;
#endif

            theCurrentRunTime	= SDL_GetTicks();
            theDrift		+= theCurrentRunTime - theLastRunTime - theTMTask->mPeriod;
            theLastRunTime	= theCurrentRunTime;

#ifdef DEBUG
            if(theDrift < theTMTask->mProfilingData.mDriftMin)
                theTMTask->mProfilingData.mDriftMin	= theDrift;

            if(theDrift > theTMTask->mProfilingData.mDriftMax)
                theTMTask->mProfilingData.mDriftMax	= theDrift;
#endif
            
            theDelay = theTMTask->mPeriod - theDrift;
            
            if(theDelay > 0)
                SDL_Delay(theDelay);
			else {
				// We did miss a deadline!
				missedDeadline = true;
#ifdef DEBUG
                theTMTask->mProfilingData.mNumLateCalls++;
#endif                
			}
        }
    
	theCurrentRunTime	= SDL_GetTicks();
	theDrift		+= theCurrentRunTime - theLastRunTime - theTMTask->mPeriod;
        theLastRunTime		= theCurrentRunTime;

#ifdef DEBUG
        if(theDrift < theTMTask->mProfilingData.mDriftMin)
            theTMTask->mProfilingData.mDriftMin	= theDrift;

        if(theDrift > theTMTask->mProfilingData.mDriftMax)
            theTMTask->mProfilingData.mDriftMax	= theDrift;
#endif

        // Since we've been delayed for a while, double-check that we still want to run.
        if(theTMTask->mKeepRunning == false)
            break;
        
        // NOTE: since we could be preempted between checking for termination and actually calling the
        // callback, there is a VERY small chance that mFunction could be called (at most once) after 
        // myTMRemoveTask() completes.  This is a BUG, but to avoid expensive synchronization (making
        // myTMRemoveTask() block until this thread finishes, protecting mKeepRunning with a mutex, etc.)
        // we take our chances.  This bug could only bite anyway (in the current IPring) while making the
        // transition from a normal player to the gatherer (in drop_upring_player()) as a result of the
        // gatherer becoming netdead - not terribly likely to begin with!
        
        // Call the function.  If it doesn't want to be rescheduled, stop ourselves.
#ifdef DEBUG
        theTMTask->mProfilingData.mNumCallsThisReset++;
        theTMTask->mProfilingData.mNumCallsTotal++;
#endif

		// Don't call the function if we're already at least a period behind.
        // later change: DO call the function extra times to catch up.  network.cpp does the right thing
        // now that it has the "faux queue".
//		if(!missedDeadline) {
	        if(theTMTask->mFunction() != true)
		        break;
//		}
//		else
			missedDeadline = false;
	}
    
    theTMTask->mIsRunning = false;
    
#ifdef DEBUG
    theTMTask->mProfilingData.mFinishTime	= SDL_GetTicks();
#endif
    
    return 0;
}


static vector<myTMTaskPtr> sOutstandingTasks;


// Set up a periodic callout with no anti-drift mechanisms.  (We don't support that,
// but it's unlikely that anyone is counting on NOT having drift-correction?)
myTMTaskPtr
myTMSetup(long time, bool (*func)(void)) {
    return myXTMSetup(time, func);
}

// Set up a periodic callout, with what tries to be a fairly drift-free period.
myTMTaskPtr
myXTMSetup(long time, bool (*func)(void)) {
    myTMTaskPtr	theTask = new myTMTask;
    
    theTask->mPeriod		= time;
    theTask->mFunction		= func;
    theTask->mKeepRunning	= true;
    theTask->mIsRunning		= true;
    theTask->mResetTime		= 0;
    
#ifdef DEBUG
    obj_clear(theTask->mProfilingData);
#endif
    
    theTask->mThread		= SDL_CreateThread(thread_loop, theTask);

    // Set thread priority a little higher
    BoostThreadPriority(theTask->mThread);
    
    sOutstandingTasks.push_back(theTask);
    
    return theTask;
}

// Stop an existing callout from executing.
myTMTaskPtr
myTMRemove(myTMTaskPtr task) {
    if(task != NULL)
        task->mKeepRunning	= false;
    
    return NULL;
}

// Reset an existing callout's delay to the original value.
// This is similar to myTMRemove() followed by another myTMSetup() with the same task and period.
void
myTMReset(myTMTaskPtr task) {
    if(task != NULL) {
        // If the thread has not exited, we can message it.  NOTE: there is a small possibility
        // that the thread has already broken its loop, but got preempted before it could set
        // mIsRunning to false.  I'm going to take the easy lazy evil way out and just hope that
        // doesn't happen.
        if(task->mIsRunning)
            task->mResetTime	= SDL_GetTicks();
        
        // Otherwise, we need to start a new thread for the task.
        else {
            // This is our only chance to clean up that zombie thread.  This should not block.
            SDL_WaitThread(task->mThread, NULL);
            
            task->mKeepRunning	= true;
            task->mIsRunning	= true;
            task->mResetTime	= 0;
            
#ifdef DEBUG
            task->mProfilingData.mNumResuscitations++;
            task->mProfilingData.mNumCallsThisReset = 0;
#endif
            
            task->mThread	= SDL_CreateThread(thread_loop, task);

            // Set thread priority a little higher
            BoostThreadPriority(task->mThread);
        }
    }
}

#ifdef DEBUG
// ZZZ addition (to myTM interface): dump profiling data
#define DUMPIT_ZU(structure,field_name) printf("" #field_name ":\t%lu\n", (structure).field_name)
#define DUMPIT_ZS(structure,field_name) printf("" #field_name ":\t%ld\n", (structure).field_name)

void
myTMDumpProfile(myTMTask* inTask) {
    if(inTask != NULL) {
        printf("PROFILE FOR SDL TMTASK %p (function %p)\n", inTask, inTask->mFunction);
        DUMPIT_ZU((*inTask), mPeriod);
        DUMPIT_ZU(inTask->mProfilingData, mStartTime);
        DUMPIT_ZU(inTask->mProfilingData, mFinishTime);
        DUMPIT_ZU(inTask->mProfilingData, mNumCallsThisReset);
        DUMPIT_ZU(inTask->mProfilingData, mNumCallsTotal);
        DUMPIT_ZS(inTask->mProfilingData, mDriftMin);
        DUMPIT_ZS(inTask->mProfilingData, mDriftMax);
        DUMPIT_ZU(inTask->mProfilingData, mNumLateCalls);
        DUMPIT_ZU(inTask->mProfilingData, mNumWarmResets);
        DUMPIT_ZU(inTask->mProfilingData, mNumResuscitations);
    }
}
#endif//DEBUG

// ZZZ addition: clean up outstanding timer task blocks and threads
// This could be slightly more efficient maybe by using a list, condensing calls to erase(), etc...
// but why bother?  It's only used occasionally at non-time-critical moments, and we're only dealing with
// a small handful of (small) elements anyway.
void
myTMCleanup(bool inWaitForFinishers) {
    vector<myTMTaskPtr>::iterator	i;

    for(i = sOutstandingTasks.begin(); i != sOutstandingTasks.end(); ++i) {
        if((*i)->mKeepRunning == false && (inWaitForFinishers || (*i)->mIsRunning == false)) {
            myTMTaskPtr	theDeadTask = *i;
            
            // This does the right thing: i is sent to erase(), but before erase is called, i is decremented so
            // the iterator remains valid.
            sOutstandingTasks.erase(i--);
            
#ifdef DEBUG
            myTMDumpProfile(theDeadTask);
#endif  

            SDL_WaitThread(theDeadTask->mThread, NULL);
            delete theDeadTask;
        }
    }
}

/*
 *  mytm_sdl.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
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

 *  The point of this file is to let us (networking code, in particular) use timing services
 *  with the same source-level API wrapper that Bungie used to access the Mac's Time Manager.
 *
 *  Created by woody on Mon Oct 15 2001.
 *
 *  3 December 2001 (Woody Zenfell): changed dependence on SDL_Threadx's SetRelativeThreadPriority
 *	to simply BoostThreadPriority(), a simpler function with a simpler interface.
 *
 *  14 January 2003 (Woody Zenfell): TMTasks lock each other out while running (better models
 *      Time Manager behavior, so makes code safer).  Also removed missedDeadline stuff.
 */

// The implementation is built on SDL_thread, and approximates the Time Manager behavior.
// Obviously, it's not a perfect emulation.  :)
// In particular, though TMTasks now lock one another out (as they should), TMTasks do not
// (cannot?) effectively lock out the main thread (as they would in Mac OS 9)... but, their
// threads ought to be higher-priority than the main thread, which means that as long as they
// don't block (which they shouldn't anyway), the main thread will not run while they do.

// I probably would have made life easier for myself by using SDL_timer instead, but frankly
// the documentation does not inspire me to trust it.  I'll do things on my own.

#include "cseries.h"
#include "thread_priority_sdl.h"
#include "mytm.h"
#include <atomic>
#include <vector>

#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_error.h>

#include "Logging.h"

#ifndef NO_STD_NAMESPACE
using std::vector;
#endif

#ifdef DEBUG
struct myTMTask_profile {
    uint64_t	mStartTime;
    uint64_t	mFinishTime;
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
    std::atomic_bool mKeepRunning;	// set true by myTMSetup; set false by thread or by myTMRemove.
#ifdef DEBUG
    myTMTask_profile	mProfilingData;
#endif
};

// Only one TMTask should be scheduled at any given time, so they take this mutex.
static SDL_mutex* sTMTaskMutex = NULL;


void
mytm_initialize() {
    // XXX should provide a way to destroy the mutex too - currently we rely on process exit to do that.
    if(sTMTaskMutex == NULL) {
        sTMTaskMutex = SDL_CreateMutex();
        
        //logCheckWarn0(sTMTaskMutex != NULL, "unable to create mytm mutex lock");
        if(sTMTaskMutex == NULL)
            logWarning("unable to create mytm mutex lock");
    }
    else
        logAnomaly("multiple invocations of mytm_initialize()");
}


// The logging system is not (currently) thread-safe, so these logging calls are potentially a Bad Idea
// but if something's going wrong already, maybe it wouldn't hurt to take a small risk to shed some light.
bool
take_mytm_mutex() {
    bool success = (SDL_LockMutex(sTMTaskMutex) != -1);
    if(!success)
        logAnomaly("take_mytm_mutex(): SDL_LockMutex() failed: %s", SDL_GetError());
    return success;
}



bool
release_mytm_mutex() {
    bool success = (SDL_UnlockMutex(sTMTaskMutex) != -1);
    if(!success)
        logAnomaly("release_mytm_mutex(): SDL_UnlockMutex() failed: %s", SDL_GetError());
    return success;
}



// Function that threads execute - does housekeeping and calls user callback
// Tries to be drift-free.
static int
thread_loop(void* inData) {
    myTMTask*	theTMTask	= (myTMTask*) inData;
    
	uint64_t theLastRunTime	= machine_tick_count();
    uint64_t theCurrentRunTime;
    int32	theDrift	= 0;

#ifdef DEBUG
    theTMTask->mProfilingData.mStartTime	= theLastRunTime;
#endif

    while(theTMTask->mKeepRunning) {
        // Delay, unless we're at least a period behind schedule
        // Originally, I didn't compute theDelay explicitly as a signed quantity, which
        // made for some VERY long waits if we were running late...
        int32	theDelay 	= theTMTask->mPeriod - theDrift;
        if(theDelay > 0)
            sleep_for_machine_ticks(theDelay);
        else {
            // We missed a deadline!
#ifdef DEBUG
            theTMTask->mProfilingData.mNumLateCalls++;
#endif                
        }

	theCurrentRunTime	= machine_tick_count();
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

        bool runAgain = true;

        // Lock out other tmtasks while we run ours
        if(take_mytm_mutex()) {
            runAgain = theTMTask->mFunction();
            release_mytm_mutex();
        }
        
        if(!runAgain)
            break;
    }
    
#ifdef DEBUG
    theTMTask->mProfilingData.mFinishTime	= machine_tick_count();
#endif
    
    return 0;
}


static vector<myTMTaskPtr> sOutstandingTasks;

// Set up a periodic callout, with what tries to be a fairly drift-free period.
myTMTaskPtr
myXTMSetup(int32 time, bool (*func)(void)) {
    myTMTaskPtr	theTask = new myTMTask;
    
    theTask->mPeriod		= time;
    theTask->mFunction		= func;
    theTask->mKeepRunning	= true;

#ifdef DEBUG
    obj_clear(theTask->mProfilingData);
#endif
    
    theTask->mThread		= SDL_CreateThread(thread_loop, "myXTMSetup_taskThread", theTask);

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

#ifdef DEBUG
// ZZZ addition (to myTM interface): dump profiling data
#define DUMPIT_ZU(structure,field_name) logDump("" #field_name ":\t%u", (structure).field_name)
#define DUMPIT_ZS(structure,field_name) logDump("" #field_name ":\t%d", (structure).field_name)

void
myTMDumpProfile(myTMTask* inTask) {
    if(inTask != NULL) {
        logDump("PROFILE FOR SDL TMTASK %p (function %p)", inTask, inTask->mFunction);
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
myTMCleanup() {
    auto i = sOutstandingTasks.begin();
    while (i != sOutstandingTasks.end()) {
        if((*i)->mKeepRunning == false) {
            myTMTaskPtr	theDeadTask = *i;
            auto next_i = sOutstandingTasks.erase(i);
            i = next_i;
            
#ifdef DEBUG
            myTMDumpProfile(theDeadTask);
#endif  

            SDL_WaitThread(theDeadTask->mThread, NULL);
            delete theDeadTask;
        }
        else
            ++i; // skip task
    }
}

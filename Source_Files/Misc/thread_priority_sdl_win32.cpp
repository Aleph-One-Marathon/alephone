/*
 *  thread_priority_sdl_win32.cpp
 *  AlephOne-OSX
 *
 *  Created by woody on Mon Dec 03 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#include	"thread_priority_sdl.h"

#include    <windows.h>

#include    <SDL_Thread.h>

static bool
TryToReduceMainThreadPriority() {
    static bool isMainThreadPriorityReduced = false;
    
    if(isMainThreadPriorityReduced)
        return true;
    
    HANDLE	theMainThreadH = GetCurrentThread();
    
    if(SetThreadPriority(theMainThreadH, THREAD_PRIORITY_BELOW_NORMAL) == 0)
        return false;
    
    isMainThreadPriorityReduced = true;
    return true;
}


bool
BoostThreadPriority(SDL_Thread* inThread) {
    HANDLE	theTargetThread = (HANDLE)SDL_GetThreadID(inThread);

    // This seems to work on Win 98, hope it does on others too!
    if(SetThreadPriority(theTargetThread, THREAD_PRIORITY_TIME_CRITICAL) == 0)
        if(SetThreadPriority(theTargetThread, THREAD_PRIORITY_HIGHEST) == 0)
            if(SetThreadPriority(theTargetThread, THREAD_PRIORITY_ABOVE_NORMAL) == 0)
                return TryToReduceMainThreadPriority();
                
    return true;
}

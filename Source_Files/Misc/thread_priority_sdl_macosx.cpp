/*
 *  thread_priority_sdl_macosx.cpp
 *  AlephOne-OSX
 *
 *  Created by woody on Sat Dec 01 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#include	"thread_priority_sdl.h"

#include	<SDL_thread.h>
#include	<pthread.h>
#include        <sched.h>

bool
BoostThreadPriority(SDL_Thread* inThread) {
    pthread_t		theTargetThread = (pthread_t) SDL_GetThreadID(inThread);
    int			theSchedulingPolicy;
    struct sched_param	theSchedulingParameters;
    
    // The following approach seems to work on Mac OS X (10.1.0, anyway)
    if(pthread_getschedparam(theTargetThread, &theSchedulingPolicy, &theSchedulingParameters) != 0)
        return false;
    
    theSchedulingParameters.sched_priority = sched_get_priority_max(theSchedulingPolicy);
    
    if(pthread_setschedparam(theTargetThread, theSchedulingPolicy, &theSchedulingParameters) != 0)
        return false;
    
    return true;
}

/*
 *  thread_priority_sdl_posix.cpp

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
 */

#include	"thread_priority_sdl.h"

#include	<SDL_thread.h>
#include	<pthread.h>
#include        <sched.h>

bool
BoostThreadPriority(SDL_Thread* inThread) {
#if defined(_POSIX_PRIORITY_SCHEDULING)
    pthread_t		theTargetThread = (pthread_t) SDL_GetThreadID(inThread);
    int			theSchedulingPolicy;
    struct sched_param	theSchedulingParameters;
    
    if(pthread_getschedparam(theTargetThread, &theSchedulingPolicy, &theSchedulingParameters) != 0)
      return false;
    
    theSchedulingParameters.sched_priority = 
      sched_get_priority_max(theSchedulingPolicy);
    
    if(pthread_setschedparam(theTargetThread, theSchedulingPolicy, &theSchedulingParameters) != 0)
      return false;
#endif
    return true;
}

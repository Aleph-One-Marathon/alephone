/*
 *  thread_priority_sdl_dummy.cpp
 *  AlephOne-OSX
 *
 *  Created by woody on Mon Dec 03 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#include	"thread_priority_sdl.h"

#include    <stdio.h>

bool
BoostThreadPriority(SDL_Thread* inThread) {
	// LP: simply doing nothing for Mac Classic
	/*
    static bool didPrintOutWarning = false;
    
    if(!didPrintOutWarning) {
        printf("warning: BoostThreadPriority not implemented for this system.  Network performance may suffer.\n");
        didPrintOutWarning = true;
    }
    */

    // We pretend we succeeded as far as the rest of the code is concerned
    return true;
}

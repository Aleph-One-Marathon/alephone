/*
 *  thread_priority_sdl.h
 *  AlephOne-OSX
 *
 *  Created by woody on Sat Dec 01 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef THREAD_PRIORITY_H
#define	THREAD_PRIORITY_H

struct SDL_Thread;

// Should bump up the specified thread's priority quite a bit, or else
// (if that's impossible) reduce the main thread's priority somewhat.
// The main thread's priority should not be further reduced by additional calls.
// This should by called by the main thread (so the priority reduction
// can be accomplished).
extern bool
BoostThreadPriority(SDL_Thread* inThread);

#endif // THREAD_PRIORITY_H

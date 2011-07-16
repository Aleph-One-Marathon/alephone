/*
 *  thread_priority_sdl.h

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

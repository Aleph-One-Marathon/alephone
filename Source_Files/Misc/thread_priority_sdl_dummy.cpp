/*
 *  thread_priority_sdl_dummy.cpp

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

#include    <stdio.h>

bool
BoostThreadPriority(SDL_Thread* inThread) {
    static bool didPrintOutWarning = false;
    
    if(!didPrintOutWarning) {
        printf("warning: BoostThreadPriority not implemented for this system.  Network performance may suffer.\n");
        didPrintOutWarning = true;
    }

    // We pretend we succeeded as far as the rest of the code is concerned
    return true;
}

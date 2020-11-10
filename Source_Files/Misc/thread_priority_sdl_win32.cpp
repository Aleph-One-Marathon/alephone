/*
 *  thread_priority_sdl_win32.cpp

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
#include    <SDL_thread.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

typedef HANDLE (WINAPI *OpenThreadPtrT)(DWORD,BOOL,DWORD);

bool
BoostThreadPriority(SDL_Thread* inThread) {
	bool success = false;
	HMODULE kernel32 = GetModuleHandle("KERNEL32");

	if (kernel32 == NULL)
	{	printf("warning: BoostThreadPriority failed: Could not open KERNEL32.  Network performance may suffer.\n"); }
	else
	{
		OpenThreadPtrT OpenThreadPtr = reinterpret_cast<OpenThreadPtrT>(GetProcAddress(kernel32, "OpenThread"));

		if (OpenThreadPtr == NULL)
		{	printf("warning: BoostThreadPriority failed: No OpenThread (only available on WinME, Win2000, WinXP or better).  Network performance may suffer.\n"); }
		else
		{
            HANDLE theTargetThread = OpenThreadPtr(STANDARD_RIGHTS_REQUIRED | THREAD_SET_INFORMATION, FALSE, SDL_GetThreadID(inThread));

			if (theTargetThread == NULL)
			{	printf("warning: BoostThreadPriority failed: Could not open thread.  Network performance may suffer.\n"); }
			else
			{
                if(SetThreadPriority(theTargetThread, THREAD_PRIORITY_TIME_CRITICAL)
					|| SetThreadPriority(theTargetThread, THREAD_PRIORITY_HIGHEST)
					|| SetThreadPriority(theTargetThread, THREAD_PRIORITY_ABOVE_NORMAL))
							success = true;

				CloseHandle(theTargetThread);
			}
		}

		FreeLibrary(kernel32);
	}

	if (success)
	{	return true; }
	else
	{
		// Supposedly this works under Win98, but it is not documented
		HANDLE theTargetThread = reinterpret_cast<HANDLE>(SDL_GetThreadID(inThread));
		return SetThreadPriority(theTargetThread, THREAD_PRIORITY_TIME_CRITICAL) != 0
			|| SetThreadPriority(theTargetThread, THREAD_PRIORITY_HIGHEST) != 0
			|| SetThreadPriority(theTargetThread, THREAD_PRIORITY_ABOVE_NORMAL) != 0
			|| TryToReduceMainThreadPriority();
	}
}

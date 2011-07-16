/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Carbon uses GetNextEvent instead of GetOSEvent
*/

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
/*
#else
#include <Events.h>
*/
#endif

#include "cstypes.h"
#include "csmisc.h"

extern void initialize_debugger(bool);

uint32 machine_tick_count(void)
{
	return TickCount();
}

bool wait_for_click_or_keypress(
	unsigned long ticks)
{
	unsigned long end;
	EventRecord event;

	end=TickCount()+ticks;
	for (;;) {
		if (GetNextEvent(mDownMask|keyDownMask,&event))
			return true;
		if (event.when>=end)
			return false;
	}
}

void kill_screen_saver(void)
{
}

void initialize_debugger(
	bool ignore)
{
	(void)ignore;
}

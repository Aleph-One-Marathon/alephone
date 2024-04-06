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

*/
#ifndef _CSERIES_MISC_
#define _CSERIES_MISC_

#include "cstypes.h"

#define MACHINE_TICKS_PER_SECOND 1000

extern uint32 machine_tick_count(void);
extern void sleep_for_machine_ticks(uint32 ticks);
extern void sleep_until_machine_tick_count(uint32 ticks);
extern void yield(void);
extern bool wait_for_click_or_keypress(
	uint32 ticks);

extern void kill_screen_saver(void);

#ifdef DEBUG
extern void initialize_debugger(bool on);
#endif

#endif

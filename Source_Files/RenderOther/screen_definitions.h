#ifndef __SCREEN_DEFINITIONS_H
#define __SCREEN_DEFINITIONS_H

/*
	screen_definitions.h

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Tuesday, July 11, 1995 5:32:20 PM- rdm created.

*/

/* -------- this contains the ids for the 8 bit picts */
/* the 16 bit versions are these ids + 10000 */
/* the 32 bit versions are these ids + 20000 */
enum {
	INTRO_SCREEN_BASE= 1000,
	MAIN_MENU_BASE= 1100,
	PROLOGUE_SCREEN_BASE= 1200,
	EPILOGUE_SCREEN_BASE= 1300,
	CREDIT_SCREEN_BASE= 1400,
	CHAPTER_SCREEN_BASE= 1500,
	COMPUTER_INTERFACE_BASE= 1600,
	INTERFACE_PANEL_BASE= 1700,
	FINAL_SCREEN_BASE= 1800
};

#endif


#ifndef _LOCAL_EVENTS_
#define _LOCAL_EVENTS_
/*

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

	Local-events handler
	by Loren Petrich,
	July 29, 2000
	
	This is for handling local events -- events that are purely confined to each local machine,
	such as quitting, pausing, changing map size, screen resolution, and sound volume,
	and so forth.
	
	The way that this works is that local events are put into an event record by
	some input device, and then removed in shell.h and used to compose fake OS events
	(or handled directly, if desired).
*/


// The various kinds of local events
enum
{
	LocalEvent_Quit			 = 0x00000001,
	LocalEvent_Pause		 = 0x00000002,
	LocalEvent_SoundDown	 = 0x00000004,
	LocalEvent_SoundUp		 = 0x00000008,

	LocalEvent_MapOut		 = 0x00000010,
	LocalEvent_MapIn		 = 0x00000020,
	LocalEvent_InvenPrev	 = 0x00000040,
	LocalEvent_InvenNext	 = 0x00000080,

	LocalEvent_SwitchPlayer	 = 0x00000100,
	LocalEvent_BkgdTasks	 = 0x00000200,
	LocalEvent_FramesPerSec	 = 0x00000400,
	LocalEvent_PixelRes		 = 0x00000800,

	LocalEvent_ScreenDown	 = 0x00001000,
	LocalEvent_ScreenUp		 = 0x00002000,
	LocalEvent_BrightDown	 = 0x00004000,
	LocalEvent_BrightUp		 = 0x00008000,
	
	LocalEvent_SwitchSides	 = 0x00010000,
	LocalEvent_ChaseCam		 = 0x00020000,
	LocalEvent_TunnelVision	 = 0x00040000,
	LocalEvent_Crosshairs	 = 0x00080000,
	
	LocalEvent_ShowPosition	 = 0x00100000,
	LocalEvent_Screenshot	 = 0x00200000,
	LocalEvent_ResetTxtrs	 = 0x00400000,
	// May want to add an event flag for allowing type-in cheats
};

// defined in shell.c
extern uint32 LocalEventFlags;

// These functions are designed to be used when the input runs in a different thread
// from the main processing; such is the case with the Marathon engine and its
// polling of the input devices.

// Post an event
inline void PostLocalEvent(uint32 Event)
{
	SET_FLAG(LocalEventFlags,Event,true);
}

// Clear an event
inline void ClearLocalEvent(uint32 Event)
{
	SET_FLAG(LocalEventFlags,Event,false);
}

// Check if an event is present, and if so, clear it
inline bool GetLocalEvent(uint32 Event)
{
	if (TEST_FLAG(LocalEventFlags,Event))
	{
		SET_FLAG(LocalEventFlags,Event,false);
		return true;
	}
	return false;
}

#endif

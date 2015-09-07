#ifndef __INTERFACE_MENUS_H
#define __INTERFACE_MENUS_H

/*
	interface_menus.h

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

	Thursday, September 28, 1995 7:02:23 PM- rdm created.

*/

enum { /* Menus available during the game */
	mGame= 128,
	iPause= 1,
	iSave,
	iRevert,
	iCloseGame,
	iQuitGame
};

enum { /* Menu interface... */
	mInterface= 129,
	iNewGame= 1,
	iLoadGame,
	iGatherGame,
	iJoinGame,
	iPreferences,
	iReplayLastFilm,
	iSaveLastFilm,
	iReplaySavedFilm,
	iCredits,
	iQuit,
	iCenterButton,
	iPlaySingletonLevel,
	iAbout
};

/* This is the menu with nothing in the title, so that it doesn't show up */
/* when the menu bar is drawn atexit.. */
enum {
	mFakeEmptyMenu= 130
};

#endif


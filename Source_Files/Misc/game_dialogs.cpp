/*
GAME_DIALOGS.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Wednesday, December 15, 1993 8:26:14 PM
Saturday, July 9, 1994 5:54:26 PM (alain)
   	ajr--brought back the preferences from the dead.
Thursday, August 11, 1994 3:47:55 PM (alain)
	added dialog for configuring the keys
Monday, September 19, 1994 11:16:09 PM  (alain)
	completely revamping the preferences dialog, now using System 6 popups instead
	of millions of radio buttons. (millions? ok, ok, maybe billions.)
Tuesday, September 20, 1994 7:41:56 PM  (alain)
	key config dialog also has popup for selecting which key setup to use.
Wednesday, June 14, 1995 8:47:39 AM
	gutted.  Keyboard stuff is now in keyboard_dialog.c.  Preferences related stuff is
		now in preferences.c.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
*/

#include "macintosh_cseries.h"
#include <string.h>

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "preferences.h"
#include "screen.h"

#ifdef env68k
	#pragma segment dialogs
#endif

#ifdef USES_NIBS
const CFStringRef Window_Game_Quit_NoSave = CFSTR("Game_Quit_NoSave");
#else
enum {
	dlogQUIT_WITHOUT_SAVING= 129
};
#endif

/* ----------- code */
#ifdef USES_NIBS

static pascal void Idler(EventLoopTimerRef Timer, void *Data)
{
	global_idle_proc();
}

bool quit_without_saving(
	void)
{
	// Get the window
	AutoNibWindow Window(GUI_Nib,Window_Game_Quit_NoSave);
	
	// Add a timer for keeping the global idle task going;
	// it fires once a second
	AutoTimer Timer(1, 1, Idler);
	
	bool HitOK = RunModalDialog(Window(), false);
	
	return HitOK;
}

#else
bool quit_without_saving(
	void)
{
	DialogPtr dialog;
	GrafPtr old_port;
	short item_hit;
	Point origin= {78, 134};
	
	dialog= myGetNewDialog(dlogQUIT_WITHOUT_SAVING, NULL, (WindowPtr) -1, 0);
	assert(dialog);

	GetPort(&old_port);
	SetPort((GrafPtr)GetScreenGrafPort());
	LocalToGlobal(&origin);
	SetPort(old_port);
//#if defined(USE_CARBON_ACCESSORS)
	MoveWindow(GetDialogWindow(dialog), origin.h, origin.v, false);
	ShowWindow(GetDialogWindow(dialog));
/*
#else
	MoveWindow(dialog, origin.h, origin.v, false);
	ShowWindow(dialog);
#endif
*/	
	ModalDialog(get_general_filter_upp(), &item_hit);
	DisposeDialog(dialog);
	
	return item_hit!=iOK ? false : true; /* note default button is the safe, don’t quit, one */
}
#endif

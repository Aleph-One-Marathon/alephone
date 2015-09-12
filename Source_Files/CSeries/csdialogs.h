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


Sept-Nov 2001 (Woody Zenfell): approximate emulations of originally Mac OS-only routines for
    the SDL dialog system, lets us share more code.  Using API that's a bit more specific, so
    we can split what were originally single functions into several related ones.  Mac OS
    implementation of these "split" functions is still handled by the original function.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Added utility routine GetListBoxListHandle for Carbon
*/

#include <string> /* Prefix header doesn't do this? */
#include <vector>

#ifndef _CSERIES_DIALOGS_
#define _CSERIES_DIALOGS_

#define iOK					1

class dialog;

typedef	dialog*	DialogPtr;

#define	CONTROL_INACTIVE	0
#define	CONTROL_ACTIVE		1

extern void copy_cstring_to_static_text(DialogPtr dialog, short item, const char* cstring);

extern void modify_control_enabled(
	DialogPtr dlg,
	short item,
	short hilite);

extern short get_selection_control_value(
        DialogPtr dialog,
        short which_control);
        

#endif//_CSERIES_DIALOGS_

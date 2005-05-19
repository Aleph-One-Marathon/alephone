/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Adjusted when defines were done to not stomp on Carbon's use of them
	Added undefs to mac and BIG_ENDIAN to prevent redefinition warnings
*/

#ifndef _MACINTOSH_CSERIES
#define _MACINTOSH_CSERIES


#ifndef mac
#define mac
#endif

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
#else
/*
#include <Events.h>
#include <AppleEvents.h>
#include <Aliases.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Sound.h>
#include <Gestalt.h>
#include <Devices.h>
#include <Resources.h>
#include <Script.h>
#include <Timer.h>
#include <TextUtils.h>
#include <PictUtils.h>
#include <Lists.h>
*/
#endif

#define HAVE_OPENGL
#undef BIG_ENDIAN
#define BIG_ENDIAN
#undef ALEPHONE_LITTLE_ENDIAN
#define COMPLAIN_BAD_ALLOCS

#include "cstypes.h"

#include "csmacros.h"
#include "cscluts.h"
#include "cskeys.h"
#include "csdialogs.h"
#include "csstrings.h"
#include "csfonts.h"
#include "cspixels.h"
#include "csalerts.h"
#include "csfiles.h"
#include "csmisc.h"

#include "gdspec.h"

// This removes all the ugly #if defined(USE_CARBON_ACCESSORS),
// but still remains here if we ever want to do something with it.
// It isn't a perfect emulation of the old way to do stuff,
// but it should make it easier if someone wishes to undertake
// porting Aleph One back to pre-Carbon Mac OS.
// -Alexei Svitkine
#ifdef CARBON_UNAVAILABLE
#define GetDialogWindow(dialog) ((WindowPtr) dialog)
#define GetWindowPort(window) ((CGrafPtr)window)
#define GetWindowFromPort(port) ((WindowPtr)port)
#define ValidWindowRect(window, rect) ValidRect(rect)
#define InvalWindowRect(window, rect) InvalRect(rect)
#define GetControlHilite(control) ((*(control))->contrlHilite)
#define GetControlPopupMenuHandle(control) ((*((*(control))->contrlData))->mHandle)
#define GetPortBitMapForCopyBits(port) (&(port)->portBits)
#define GetDialogKeyboardFocusItem(dialog) (((DialogRecord *) (dialog))->editField)

#define GetWindowRegion(window, type, region) do { region = ((WindowPeek)window)->updateRgn);} while (0)
#define GetRegionBounds(region, rect) do { (*(rect)) = (*(region))->rgnBBox;} while (0)
#define GetPortBounds(port, rect) do { (*(rect)) = (port)->portRect;} while (0)
#define GetPortClipRegion(port, clip) do { (*(clip)) = (port)->clipRgn;} while (0)
#define SetPortClipRegion(port, region) do { (port)->clipRgn = (region);} while (0)

#define GetQDGlobalsGray(p) do { (*(p)) = qd.gray;} while (0)
#define GetQDGlobalsBlack(p) do { (*(p)) = qd.black;} while (0)
#define GetQDGlobalsArrow(arrow) do { (*(arrow)) = qd.arrow;} while (0)
#define GetQDGlobalsScreenBits(bits) do { (*(bits)) = qd.screenBits;} while (0)
#define SetQDGlobalsRandomSeed(seed) do { qd.randSeed = (seed);} while (0)

#define GetPortTextFont(port) ((port)->txFont)
#define GetPortTextFace(port) ((port)->txFace)
#define GetPortTextSize(port) ((port)->txSize)

#define NewModalFilterUPP(proc) NewModalFilterProc(proc)
#define CountMenuItems(menu) CountMItems(menu)
#define EnableeMenuItem(menu, item) EnableItem(menu, item)
#define DisableMenuItem(menu, item) DisableItem(menu, item)
#define GetNextEvent(mask, event) GetOSEvent(mask, event)

#define CopyCStringToPascal(c, p) do { strncpy((char *)(p), (c), 255); c2pstr((char *)(p));} while (0)
#define CopyPascalStringToC(p, c) do { strncpy((c), (char *)(p), 255); p2cstr((char *)(c));} while (0)
#endif

#endif

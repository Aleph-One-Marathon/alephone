/*

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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Adjusted when defines were done to not stomp on Carbon's use of them
	Added undefs to mac and BIG_ENDIAN to prevent redefinition warnings
*/
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _MACINTOSH_CSERIES
#define _MACINTOSH_CSERIES

#undef mac
#define mac

#if defined(TARGET_API_MAC_CARBON)
    #include <Carbon/Carbon.h>
#else
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
#endif

// JTP: Move after includes to not stomp on Carbon.h's use of DEBUG
#define HAVE_OPENGL
#undef BIG_ENDIAN
#define BIG_ENDIAN
#define DEBUG
#undef ALEPHONE_LITTLE_ENDIAN

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

#endif

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

*/
// Loren Petrich: the author(s) of the "cseries" files is not given, but is probably
// either Bo Lindbergh, Mihai Parparita, or both, given their efforts in getting the
// code working initially.
// AS: It was almost certainly Bo Lindbergh
#ifndef _CSERIES
#define _CSERIES

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "unknown version"
#endif

#include <SDL.h>
#include <SDL_byteorder.h>
#include <time.h>
#include <string>

#define DEBUG

#ifndef __MVCPP__

// mwcc doesn't accept "using namespace std;" otherwise
namespace std {};

#elif __MVCPP__

using namespace std;	// Visual C++ doesn't like that other way of using the namespace.
#if _MSC_VER < 1300
#define for if(false) {} else for // solves a bug in MSVC prior to MSVC7 for the "redeclared" initializers used in for loops.
#endif

//#define		VERSION		"1.0"

#endif

/*
 *  Endianess definitions
 */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define ALEPHONE_LITTLE_ENDIAN 1
#else
#undef ALEPHONE_LITTLE_ENDIAN
#endif


/*
 *  Data types with specific bit width
 */

#include "cstypes.h"

/*
 *  Emulation of MacOS data types and definitions
 */
#if !defined(SDL_RFORK_HACK)// && !defined(TARGET_API_MAC_CARBON)

#if defined(__APPLE__) && defined(__MACH__)
// if we're on the right platform, we can use the real thing (and get headers for functions we might want to use)
#include <CoreFoundation/CoreFoundation.h>
#elif !defined(TARGET_API_MAC_CARBON)
typedef int OSErr;

struct Rect {
	int16 top, left;
	int16 bottom, right;
};

const int noErr = 0;
#endif

struct RGBColor {
	uint16 red, green, blue;
};

const int kFontIDMonaco = 4;
const int kFontIDCourier = 22;

#else
# if !defined(TARGET_API_MAC_CARBON)
#  define DEBUGASSERTMSG
# endif
# ifdef DEBUG
#  define WAS_DEBUG
//AS: this is ugly...

#  undef DEBUG
# endif
# if !defined(TARGET_API_MAC_CARBON)
#  define dialog CHEESEOFDEATH
#  define DialogPtr mDialogPtr
#  include <MacTypes.h>
#  include <Quickdraw.h>
#  undef dialog
#  undef DialogPtr
# endif
# ifdef WAS_DEBUG
#  define DEBUG
# endif
#endif

/*
 *  Include CSeries headers
 */

#include "cstypes.h"
#include "csmacros.h"
#include "cscluts.h"
#include "csstrings.h"
#include "csfonts.h"
#include "cspixels.h"
#include "csalerts.h"
#include "csdialogs.h"
#include "csmisc.h"


#endif

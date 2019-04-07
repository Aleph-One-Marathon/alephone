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
#include <SDL_endian.h>
#include <time.h>
#include <string>

#define DEBUG


/*
 *  Endianess definitions
 */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define ALEPHONE_LITTLE_ENDIAN 1
#else
#undef ALEPHONE_LITTLE_ENDIAN
#endif


constexpr bool PlatformIsLittleEndian() noexcept {
#ifdef ALEPHONE_LITTLE_ENDIAN
	return true;
#else
	return false;
#endif // end ALEPHONE_LITTLE_ENDIAN
}
/*
 *  Data types with specific bit width
 */

#include "cstypes.h"

/*
 *  Emulation of MacOS data types and definitions
 */

#if defined(__APPLE__) && defined(__MACH__)
// if we're on the right platform, we can use the real thing (and get headers for functions we might want to use)
#include <CoreFoundation/CoreFoundation.h>
#else
typedef int OSErr;

struct Rect {
	int16 top, left;
	int16 bottom, right;
};

const int noErr = 0;
#endif

constexpr Rect MakeRect(int16 top, int16 left, int16 bottom, int16 right)
	{ return {top, left, bottom, right}; }

constexpr Rect MakeRect(SDL_Rect r)
	{ return {int16(r.y), int16(r.x), int16(r.y + r.h), int16(r.x + r.w)}; }

struct RGBColor {
	uint16 red, green, blue;
};

const int kFontIDMonaco = 4;
const int kFontIDCourier = 22;

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
#include "cspaths.h"
#include "csmisc.h"


#endif

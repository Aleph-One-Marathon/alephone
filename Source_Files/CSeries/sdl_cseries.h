/*
 *  sdl_cseries.h - Unix-specific defines for CSeries library
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef _SDL_CSERIES
#define _SDL_CSERIES

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_byteorder.h>
#include <string>

#define DEBUG

#define CB 1
#undef LP


/*
 *  General definitions
 */

#if !defined(LITTLE_ENDIAN) && (SDL_BYTEORDER == SDL_LIL_ENDIAN)
#define LITTLE_ENDIAN 1
#endif


/*
 *  Data types with specific bit width
 */

#include "cstypes.h"


/*
 *  Emulation of MacOS data types and definitions
 */

typedef int OSErr;
typedef unsigned char Str255[256];

struct RGBColor {
	uint16 red, green, blue;
};

struct Rect {
	int16 top, left;
	int16 bottom, right;
};

typedef void *DialogPtr;

struct GDSpec {
	int bit_depth;
	int width, height;
};

typedef GDSpec *GDSpecPtr;

#define noErr 0

#define kFontIDMonaco 4
#define normal 0
#define bold 1
#define italic 2
#define underline 4


/*
 *  Include CSeries headers
 */

#include "cstypes.h"
#include "csmacros.h"
#include "cscluts.h"
#include "csdialogs.h"
#include "csstrings.h"
#include "csfonts.h"
#include "cspixels.h"
#include "csalerts.h"
#include "csmisc.h"

#endif

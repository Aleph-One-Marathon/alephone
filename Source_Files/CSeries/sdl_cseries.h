/*
 *  sdl_cseries.h - Unix-specific defines for CSeries library
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef _SDL_CSERIES
#define _SDL_CSERIES

#include "config.h"

#include <SDL/SDL.h>
#include <string>

#define DEBUG


/*
 *  General definitions
 */

#if SDL_BYTE_ORDER == SDL_LIL_ENDIAN
#define LITTLE_ENDIAN 1
#endif


/*
 *  Data types with specific bit width
 */

typedef Uint8 uint8;
typedef Sint8 int8;
typedef Uint16 uint16;
typedef Sint16 int16;
typedef Uint32 uint32;
typedef Sint32 int32;


/*
 *  Emulation of MacOS data types and definitions
 */

typedef bool Boolean;
typedef int OSErr;
typedef uint32 OSType;
typedef unsigned char Str255[256];

struct RGBColor {
	uint16 red, green, blue;
};

struct Rect {
	int16 top, left;
	int16 bottom, right;
};

typedef void *WindowPtr;
typedef void *DialogPtr;
typedef void *MenuHandle;
typedef SDL_Surface *PixMapHandle;
typedef void *GrafPtr;
typedef void *CGrafPtr;
typedef void *GWorldPtr;
typedef void *CTabHandle;
typedef void ModalFilterUPP;

#define EventRecord SDL_Event

struct FileDesc;
#define FSSpec FileDesc

struct GDSpec {
	int bit_depth;
	int width, height;
};

typedef GDSpec *GDSpecPtr;

#define noErr 0
#define pascal

#define kFontIDMonaco 0
#define normal 0
#define bold 1


/*
 *  Include CSeries headers
 */

#include "cstypes.h"
#include "csmacros.h"
#include "cscluts.h"
#include "cskeys.h"
#include "csdialogs.h"
#include "csstrings.h"
#include "csfonts.h"
#include "cspixels.h"
#include "csalerts.h"
#include "csmisc.h"

#endif

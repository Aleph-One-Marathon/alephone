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
#define for if(false) {} else for // solves a bug in MSVC for the "redeclared" initializers used in for loops.

#define		VERSION		"1.0"

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

typedef int OSErr;
typedef unsigned char Str255[256];

struct RGBColor {
	uint16 red, green, blue;
};

struct Rect {
	int16 top, left;
	int16 bottom, right;
};

const int noErr = 0;
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
#include "csmisc.h"

#endif

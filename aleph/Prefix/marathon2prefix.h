/*

Jan 31, 2002 (Loren Petrich):
	Suppressing TARGET_API_MAC_CARBON to enable Br'fin's Carbon support to coexist with Classic
*/

#ifndef __MARATHON_2_PREFIX_H
#define __MARATHON_2_PREFIX_H

// include MacHeaders

#include <MacHeadersCarbon.h>

// define mac

// #ifdef macintosh
#define macintosh
#define mac
// #define TARGET_API_MAC_CARBON
#define HAVE_SDL_NET 1
// #endif

// Debugging always on (mistaken asserts will always print out their info)
#define DEBUG

// WIll use QD3D
#define HAVE_QUESA 1

// App is in a bundle; this is to get away from resource forks
#define APPLICATION_IS_BUNDLED

// Using the new GUI.nib file
#define USES_NIBS 1

// Secondary dialogs are sheets
#define USE_SHEETS 1

// Will use Lua
#define HAVE_LUA 1

// Will use Speex
#define SPEEX 1

// check environs
/*
#if __POWERPC__
#define envppc
#elif __CFM68K__
#error "CFM68K not supported"
#else
#define env68k
#endif
*/

// Both CodeWarrior and OSX gcc have it
#define HAVE_SNPRINTF 1

#endif

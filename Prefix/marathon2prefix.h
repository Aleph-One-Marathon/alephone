/*

Jan 31, 2002 (Loren Petrich):
	Suppressing TARGET_API_MAC_CARBON to enable Br'fin's Carbon support to coexist with Classic
*/

#ifndef __MARATHON_2_PREFIX_H
#define __MARATHON_2_PREFIX_H

// include MacHeaders

#include <MacHeaders.h>

// define mac

#ifdef macintosh
#define mac
#undef TARGET_API_MAC_CARBON
#define HAVE_SDL_NET 1
#endif

// check environs

#if __POWERPC__
#define envppc
#elif __CFM68K__
#error "CFM68K not supported"
#else
#define env68k
#endif

// Both CodeWarrior and OSX gcc have it
#define HAVE_SNPRINTF 1

#endif

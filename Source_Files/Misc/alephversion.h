#ifndef __ALEPHVERSION_H
#define __ALEPHVERSION_H

/* If you change the version number here, you should also change it in:

   Resources/Aleph One Classic SDL.r

*/

#define A1_DISPLAY_VERSION "0.20"
#define A1_DISPLAY_DATE_VERSION "2008-06-21"
#define A1_DATE_VERSION "20080621"

#ifdef WIN32
#define A1_DISPLAY_PLATFORM "Windows"
#define A1_UPDATE_PLATFORM "windows"
#elif defined (__APPLE__) && defined(__MACH__)
#define A1_DISPLAY_PLATFORM "Mac OS X"
#define A1_UPDATE_PLATFORM "macosx"
#elif defined (__MACOS__)
#define A1_DISPLAY_PLATFORM "Mac OS"
#define A1_UPDATE_PLATFORM "macos"
#elif defined (linux)
#define A1_DISPLAY_PLATFORM "Linux"
#define A1_UPDATE_PLATFORM "source"
#elif defined (__BEOS__)
#define A1_DISPLAY_PLATFORM "BeOS"
#define A1_UPDATE_PLATFORM "source"
#elif defined (__NetBSD__)
#define A1_DISPLAY_PLATFORM "NetBSD"
#define A1_UPDATE_PLATFORM "source"
#elif defined (__OpenBSD__)
#define A1_DISPLAY_PLATFORM "OpenBSD"
#define A1_UPDATE_PLATFORM "source"
#else
#define A1_DISPLAY_PLATFORM "Unknown"
#define A1_UPDATE_PLATFORM "source"
#endif

#ifndef A1_VERSION_STRING
#define A1_VERSION_STRING "Aleph One " A1_DISPLAY_PLATFORM " " A1_DISPLAY_DATE_VERSION " " A1_DISPLAY_VERSION
#endif


#endif // ALEPHVERSION_H

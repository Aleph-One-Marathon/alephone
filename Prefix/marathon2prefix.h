#ifndef __MARATHON_2_PREFIX_H
#define __MARATHON_2_PREFIX_H

// include MacHeaders

#include <MacHeaders.h>

// define mac

#ifdef macintosh
#define mac
#endif

// check environs

#if __POWERPC__
#define envppc
#elif __CFM68K__
#error "CFM68K not supported"
#else
#define env68k
#endif

#endif

// Loren Petrich: the author(s) of the "cseries" files is not given, but is probably
// either Bo Lindbergh, Mihai Parparita, or both, given their efforts in getting the
// code working initially.

#ifndef _CSERIES
#define _CSERIES

#if defined(SDL)
#include "sdl_cseries.h"
#elif defined(mac)
#include "macintosh_cseries.h"
#endif

#endif

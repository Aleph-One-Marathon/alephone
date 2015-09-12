/*
 *  snprintf.h - crude, unsafe imitation of the real snprintf() and vsnprintf()

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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


    Jan. 17, 2003 (Woody Zenfell): Created.

*/

// Build this file in only on platforms that actually need it (Mac OS X, e.g., doesn't)

#include "snprintf.h"

#include <stdio.h>

#include "Logging.h"

#ifndef HAVE_SNPRINTF
int
snprintf(char* inBuffer, size_t inBufferSize, const char* inFormat, ...) {
    va_list theArgs;
    va_start(theArgs, inFormat);
    int theResult = vsnprintf(inBuffer, inBufferSize, inFormat, theArgs);
    va_end(theArgs);
    return theResult;
}
#endif

// This could, like, fprintf out to a file and check the file size, or maybe we could
// legally lift a whole vsnprintf() implementation from somewhere (GNU std library?)
// Anyway at least we'll try to give a warning if we overrun.
#ifndef HAVE_VSNPRINTF
int
vsnprintf(char* inBuffer, size_t inBufferSize, const char* inFormat, va_list inArgs) {
    int theResult = vsprintf(inBuffer, inFormat, inArgs);

    // In case logging vsnprintf's a long string, don't warn while warning.
    static bool issuingWarning = false;

    if(theResult + 1 > inBufferSize && !issuingWarning) {
        issuingWarning = true;
        logWarning("vsnprintf emulation wrote too many bytes (%d/%d)", theResult + 1, inBufferSize);
        issuingWarning = false;
    }

    return theResult;
}
#endif // !HAVE_VSNPRINTF

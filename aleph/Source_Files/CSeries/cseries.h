/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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

#if defined(SDL)
#include "sdl_cseries.h"
#elif defined(mac)
#include "macintosh_cseries.h"
#endif

#endif

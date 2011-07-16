/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

*/
#ifndef _BYTE_SWAPPING_
#define _BYTE_SWAPPING_

typedef short _bs_field;

enum {
	_2byte	= -2,
	_4byte	= -4
};

#include <stddef.h>

extern void byte_swap_memory(
	void *memory,
	_bs_field type,
	int fieldcount);

#ifndef ALEPHONE_LITTLE_ENDIAN
#define byte_swap_memory(memory,type,elcount) ((void)0)
#endif

#endif

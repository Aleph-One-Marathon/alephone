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

#include "cseries.h"
#include "byte_swapping.h"


#ifdef ALEPHONE_LITTLE_ENDIAN

// Fieldcount is "int" because it can become negative in the code
void byte_swap_memory(
	void *memory,
	_bs_field type,
	int fieldcount)
{
	uint8 *walk;
	int tmp;

	walk=(uint8 *)memory;
	switch (type) {
	case _2byte:
		while (fieldcount>0) {
			tmp=walk[0];
			walk[0]=walk[1];
			walk[1]=tmp;
			walk+=2;
			fieldcount--;
		}
		break;
	case _4byte:
		while (fieldcount>0) {
			tmp=walk[0];
			walk[0]=walk[3];
			walk[3]=tmp;
			tmp=walk[1];
			walk[1]=walk[2];
			walk[2]=tmp;
			walk+=4;
			fieldcount--;
		}
		break;
	}
}

#endif

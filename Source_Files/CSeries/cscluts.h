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
#ifndef _CSERIES_CLUTS_
#define _CSERIES_CLUTS_

// Need this here
#include "cstypes.h"

class LoadedResource;
struct RGBColor;

typedef struct rgb_color {
	uint16 red;
	uint16 green;
	uint16 blue;
} rgb_color;

typedef struct color_table {
	short color_count;
	rgb_color colors[256];
} color_table;

extern void build_color_table(
	color_table *table,
	LoadedResource &clut);

enum {
	gray15Percent,
	windowHighlight,

	NUM_SYSTEM_COLORS
};

extern RGBColor rgb_black;
extern RGBColor rgb_white;
extern RGBColor system_colors[NUM_SYSTEM_COLORS];

#endif

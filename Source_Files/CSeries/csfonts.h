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

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?

#ifndef _CSERIES_FONTS_
#define _CSERIES_FONTS_

#include "cstypes.h"

const int styleNormal = 0;
const int styleBold = 1;
const int styleItalic = 2;
const int styleUnderline = 4;

typedef struct TextSpec {
	int16 font;
	uint16 style;
	int16 size;
} TextSpec;

extern void GetNewTextSpec(
	TextSpec *spec,
	short resid,
	short item);

extern void GetFont(
	TextSpec *spec);
extern void SetFont(
	TextSpec *spec);

#endif


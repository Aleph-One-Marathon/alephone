/* csfonts.h

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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


    Sept-Nov 2001 (Woody Zenfell): new styleOutline helps put a "halo" (heh) around text.
*/

#ifndef _CSERIES_FONTS_
#define _CSERIES_FONTS_

#include "cstypes.h"

const int styleNormal = 0;
const int styleBold = 1;
const int styleItalic = 2;
const int styleUnderline = 4;
// ZZZ addition - implemented and used in SDL version.  May not play well with other styles.
// Actually this is sort of a poor name.  It will NOT leave a blank area in the middle where
// the text would normally have been drawn.  Also, the width returned when it's drawn won't
// include the extra pixel at each side.
// It's designed mostly to be used like draw_text(blahblah, darkColor, style | styleOutline);
// draw_text(blahblah, normalColor, style); to surround text with a solid color and set it off
// from a complex background image.
// Hope nobody depends on being able to use the upper nybble of this byte ;)
const int styleOutline = 8;

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


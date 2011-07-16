#ifndef _COLOR_PARSER_
#define _COLOR_PARSER_

/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Color parser
	by Loren Petrich,
	April 15, 2000
	
	This parses color elements (name "color").
	and returns the parsed values into a pointed-to array.
*/


#include "cseries.h"
#include "XML_ElementParser.h"

// Returns a parser for the colors;
// several elements may have colors, so this ought to be callable several times.
XML_ElementParser *Color_GetParser();

// This sets the list of colors to be read into.
// Its args are the pointer to that list and the number of colors in it.
// If that number is <= 0, then the color value is assumed to be non-indexed,
// and no "index" attribute will be searched for.
void Color_SetArray(rgb_color *ColorList, int NumColors = 0);

#endif

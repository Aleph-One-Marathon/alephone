#ifndef _COLOR_PARSER_
#define _COLOR_PARSER_

/*
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

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

#include <string.h>
#include "ColorParser.h"


// Color-parser object:
class XML_ColorParser: public XML_ElementParser
{
	rgb_color TempColor;
	short Index;
	bool IsPresent[5];

public:
	rgb_color *ColorList;
	int NumColors;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_ColorParser(): XML_ElementParser("color"), ColorList(NULL) {}
};

bool XML_ColorParser::Start()
{
	for (int k=0; k<4; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_ColorParser::HandleAttribute(const char *Tag, const char *Value)
{
	// Color value to be read in
	float CVal;
	
	if (NumColors > 0)
	{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,NumColors-1))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	}
	if (StringsEqual(Tag,"red"))
	{
		if (ReadFloatValue(Value,CVal))
		{
			IsPresent[0] = true;
			TempColor.red = uint16(PIN(65535*CVal+0.5,0,65535));
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"green"))
	{
		if (ReadFloatValue(Value,CVal))
		{
			IsPresent[1] = true;
			TempColor.green = uint16(PIN(65535*CVal+0.5,0,65535));
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"blue"))
	{
		float CVal;
		if (ReadFloatValue(Value,CVal))
		{
			IsPresent[2] = true;
			TempColor.blue = uint16(PIN(65535*CVal+0.5,0,65535));
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_ColorParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	if (NumColors <= 0)
	{
		IsPresent[3] = true;	// Convenient fakery: no index -- always present
		Index = 0;
	}
	for (int k=0; k<4; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	assert(ColorList);
	ColorList[Index] = TempColor;
	return true;
}

static XML_ColorParser ColorParser;



// Returns a parser for the colors;
// several elements may have colors, so this ought to be callable several times.
XML_ElementParser *Color_GetParser() {return &ColorParser;}

// This sets the array of colors to be read into.
// Its args are the pointer to that array and the number of colors in it.
// If that number is <= 0, then the color value is assumed to be non-indexed,
// and no "index" attribute will be searched for.
void Color_SetArray(rgb_color *ColorList, int NumColors)
{ColorParser.ColorList = ColorList; ColorParser.NumColors = NumColors;}

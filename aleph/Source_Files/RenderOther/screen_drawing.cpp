/*
	SCREEN_DRAWING.C

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

	Monday, August 15, 1994 1:55:21 PM
 
    Wednesday, August 24, 1994 12:50:20 AM (ajr)
	  added _right_justified for _draw_screen_text
	Thursday, June 22, 1995 8:45:41 AM- note that we no longer hold your hand and set the port
		for you.  We have a grafptr and a restore ptr call.\

Apr 30, 2000 (Loren Petrich):
	Added XML-parser support (actually, some days earlier, but had modified it
	so as to have "interface" be defined in "game_window".

Jul 2, 2000 (Loren Petrich):
	The HUD is now always buffered; it is lazily allocated

Oct 19, 2000 (Loren Petrich):
	Added graceful degradation if get_shape_pixmap() returns NULL; CB had already done that
	with the SDL version.
	
Dec 17, 2000 (Loren Petrich):
	Added font-abstraction support (FontHandler.*)
*/

#include "cseries.h"

#include "map.h"
#include "interface.h"
#include "shell.h"
#include "screen_drawing.h"
#include "fades.h"
#include "screen.h"

// LP addition: color and font parsers
#include "ColorParser.h"
#include "FontHandler.h"

#include <string.h>

#define clutSCREEN_COLORS 130
#define finfFONTS 128

extern TextSpec *_get_font_spec(short font_index);

/*
struct interface_font_info 
{
	TextSpec fonts[NUMBER_OF_INTERFACE_FONTS];
	short heights[NUMBER_OF_INTERFACE_FONTS];
	short line_spacing[NUMBER_OF_INTERFACE_FONTS];
};
*/

/* --------- Globals. */
// LP change: hardcoding this quantity since we know how many we need
// Putting in the Moo definitions
static screen_rectangle interface_rectangles[NUMBER_OF_INTERFACE_RECTANGLES] = 
{
	{326, 300, 338, 473},
	{464, 398, 475, 578},
	{464, 181, 475, 361},
	{338, 17, 0, 0},
	{0, 0, 0, 0},
	{352, 204, 454, 384},
	{352, 384, 454, 596},
	{179, 101, 210, 268},
	{221, 25, 253, 238},
	{263, 11, 294, 223},
	{301, 38, 333, 236},
	{304, 421, 331, 563},
	{386, 231, 413, 406},
	{345, 363, 372, 516},
	{344, 83, 374, 271},
	{206, 246, 347, 382},
	// {264, 522, 291, 588}, // inf's bounds
	// {263, 497, 294, 565}, // m2's bounds
	{263, 500, 294, 585}, // adjusted to work with both m2 and inf
      	{0,0,0,0},
	{0, 0, 0, 0},
};

// static screen_rectangle *interface_rectangles;
// static CTabHandle screen_colors;
// LP change: now hardcoded and XML-changeable

// Copied off of original 'finf' resource
// static struct interface_font_info interface_fonts = 
static FontSpecifier InterfaceFonts[NUMBER_OF_INTERFACE_FONTS] =
{
	{"Monaco",   9, styleBold,  "#4"},
	{"Monaco",   9, styleBold,  "#4"},
	{"Monaco",   9, styleBold,  "#4"},
	{"Monaco",   9, styleNormal,"#4"},
	{"Courier", 12, styleNormal,"#22"},
	{"Courier", 14, styleBold,  "#22"},
	{"Monaco",   9, styleNormal,"#4"}
};


// LP change: hardcoding the interface and player colors,
// so as to banish the 'clut' resources
const int NumInterfaceColors = 26;
static rgb_color InterfaceColors[NumInterfaceColors] = 
{
	{0, 65535, 0},
	{0, 5140, 0},
	{0, 0, 0},
	
	{0, 65535, 0},
	{0, 12956, 0},
	{0, 5100, 0},
	
	{9216, 24320, 41728},
	{65535, 0, 0},
	{45056, 0, 24064},
	{65535, 65535, 0},
	{60000, 60000, 60000},
	{62976, 22528, 0},
	{3072, 0, 65535},
	{0, 65535, 0},
	
	{65535, 65535, 65535},
	{0, 5140, 0},
	
	{10000, 0, 0},
	{65535, 0, 0},
	
	{0, 65535, 0},
	{65535, 65535, 65535},
	{65535, 0, 0},
	{0, 40000, 0},
	{0, 45232, 51657},
	{65535, 59367, 0},
	{45000, 0, 0},
	{3084, 0, 65535}
};

/* ------- Private prototypes */
static void load_interface_rectangles(void);
#ifdef mac
static Rect *_get_interface_rect(short index);
#endif
static void	load_screen_interface_colors(void);

/* -------- Code */
void initialize_screen_drawing(
	void)
{
	short loop;

	/* Load the rectangles */
	load_interface_rectangles();
	
	/* Load the colors */
	load_screen_interface_colors();
	
	/* load the font stuff. */
	for(loop=0; loop<NUMBER_OF_INTERFACE_FONTS; ++loop)
	{
		InterfaceFonts[loop].Init();
	}
}

screen_rectangle *get_interface_rectangle(short index)
{
	assert(index>=0 && index<NUMBER_OF_INTERFACE_RECTANGLES);
	return interface_rectangles + index;
}

const rgb_color &get_interface_color(short index)
{
	assert(index>=0 && index<NumInterfaceColors);
	return InterfaceColors[index];
}

FontSpecifier &get_interface_font(short index)
{
	assert(index >= 0 && index < NUMBER_OF_INTERFACE_FONTS);
	return InterfaceFonts[index];
}

// Load platform-specific stuff
#if defined(mac)
#include "screen_drawing_macintosh.h"
#elif defined(SDL)
#include "screen_drawing_sdl.h"
#endif


// Rectangle-parser object:
class XML_RectangleParser: public XML_ElementParser
{
	screen_rectangle TempRect;
	short Index;
	bool IsPresent[5];

public:
	screen_rectangle *RectList;
	int NumRectangles;
	
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_RectangleParser(): XML_ElementParser("rect") {}
};

bool XML_RectangleParser::Start()
{
	for (int k=0; k<5; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_RectangleParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,NumRectangles-1))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"top"))
	{
		if (ReadInt16Value(Value,TempRect.top))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"left"))
	{
		if (ReadInt16Value(Value,TempRect.left))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"bottom"))
	{
		if (ReadInt16Value(Value,TempRect.bottom))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"right"))
	{
		if (ReadInt16Value(Value,TempRect.right))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_RectangleParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<5; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	RectList[Index] = TempRect;
	return true;
}

static XML_RectangleParser RectangleParser;


// LP addition: set up the parser for the interface rectangles and get it
XML_ElementParser *InterfaceRectangles_GetParser()
{
	RectangleParser.RectList = interface_rectangles;
	RectangleParser.NumRectangles = NUMBER_OF_INTERFACE_RECTANGLES;
	return &RectangleParser;
}


void SetColorFontParserToScreenDrawing()
{
	Color_SetArray(InterfaceColors,NumInterfaceColors);
	Font_SetArray(InterfaceFonts,NUMBER_OF_INTERFACE_FONTS);
}

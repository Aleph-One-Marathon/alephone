
/*
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
	int Index;
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
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,0,NumColors-1))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	}
	if (strcmp(Tag,"red") == 0)
	{
		if (ReadNumericalValue(Value,"%f",CVal))
		{
			IsPresent[0] = true;
			TempColor.red = PIN(65535*CVal+0.5,0,65535);
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"green") == 0)
	{
		if (ReadNumericalValue(Value,"%f",CVal))
		{
			IsPresent[1] = true;
			TempColor.green = PIN(65535*CVal+0.5,0,65535);
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"blue") == 0)
	{
		float CVal;
		if (ReadNumericalValue(Value,"%f",CVal))
		{
			IsPresent[2] = true;
			TempColor.blue = PIN(65535*CVal+0.5,0,65535);
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

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

	Shapes parser
	by Loren Petrich,
	May 26, 2000
	
	This parses shapes elements (name "shape").
	and returns the parsed value into a pointed-to values.
*/

#include <string.h>
#include "cseries.h"
#include "ShapesParser.h"


// Shapes-parser object:
class XML_ShapesParser: public XML_ElementParser
{
	// Which are present?
	bool CollPresent, SeqPresent;
	
	// Values to read
	uint16 Coll, CLUT, Seq;
	
public:
	shape_descriptor *DescPtr;
	bool NONE_Is_OK;
		
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_ShapesParser(): XML_ElementParser("shape"), DescPtr(NULL), NONE_Is_OK(true) {}
};

bool XML_ShapesParser::Start()
{
	CollPresent = SeqPresent = false;
	CLUT = 0;	// Reasonable default
	return true;
}

bool XML_ShapesParser::HandleAttribute(const char *Tag, const char *Value)
{
	
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedUInt16Value(Value,Coll,0,MAXIMUM_COLLECTIONS-1))
		{
			CollPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"clut"))
	{
		if (ReadBoundedUInt16Value(Value,CLUT,0,MAXIMUM_CLUTS_PER_COLLECTION-1))
		{
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"seq"))
	{
		if (ReadBoundedUInt16Value(Value,Seq,0,MAXIMUM_SHAPES_PER_COLLECTION-1))
		{
			SeqPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"frame"))
	{
		if (ReadBoundedUInt16Value(Value,Seq,0,MAXIMUM_SHAPES_PER_COLLECTION-1))
		{
			SeqPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_ShapesParser::AttributesDone()
{	
	// Verify and compose the value:
	assert(DescPtr);
	if (CollPresent)
	{
		if (SeqPresent)
		{
			*DescPtr = BUILD_DESCRIPTOR(BUILD_COLLECTION(Coll, CLUT), Seq);
			return true;
		}
		else
		{
			AttribsMissing();
			return false;
		}
	}
	else
	{
		if (SeqPresent)
		{
			AttribsMissing();
			return false;
		}
		else if (NONE_Is_OK)
		{
			*DescPtr = UNONE;
			return true;
		}
		else
		{
			AttribsMissing();
			return false;
		}
	}
	return true;
}

static XML_ShapesParser ShapesParser;


// Returns a parser for the shapes;
// several elements may have shapes, so this ought to be callable several times.
XML_ElementParser *Shape_GetParser() {return &ShapesParser;}

// This sets the pointer to the shape descriptor to be read into.
// Its args are that pointer, and whether "NONE" is an acceptable value for it.
void Shape_SetPointer(shape_descriptor *DescPtr, bool NONE_Is_OK)
{
	ShapesParser.DescPtr = DescPtr;
	ShapesParser.NONE_Is_OK = NONE_Is_OK;
}

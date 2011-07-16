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
	
	Damage parser
	by Loren Petrich,
	May 17, 2000
	
	This parses damage info and places the results into a pointer
*/

#include "cseries.h"

#include "DamageParser.h"

#include <string.h>
#include <limits.h>


// Damage-parser object:
class XML_DamageParser: public XML_ElementParser
{
public:
	damage_definition *DamagePtr;
	
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_DamageParser(): XML_ElementParser("damage") {}
};

bool XML_DamageParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"type"))
	{
		if (ReadBoundedInt16Value(Value,DamagePtr->type,NONE,NUMBER_OF_DAMAGE_TYPES-1))
		{
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"flags"))
	{
		if (ReadBoundedInt16Value(Value,DamagePtr->flags,0,1))
		{
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"base"))
	{
		if (ReadInt16Value(Value,DamagePtr->base))
		{
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"random"))
	{
		if (ReadInt16Value(Value,DamagePtr->random))
		{
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"scale"))
	{
		float Scale;
		if (ReadBoundedNumericalValue(Value,"%f",Scale,float(SHRT_MIN),float(SHRT_MAX+1)))
		{
			if (Scale >= 0)
				DamagePtr->scale = int32(FIXED_ONE*Scale + 0.5);
			else
				DamagePtr->scale = - int32(- FIXED_ONE*Scale + 0.5);
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

static XML_DamageParser DamageParser;


// Returns a parser for the damage;
// several elements may have damage, so this ought to be callable several times.
XML_ElementParser *Damage_GetParser() {return &DamageParser;}

// This sets the a pointer to the damage structure to be read into.
void Damage_SetPointer(damage_definition *DamagePtr) {DamageParser.DamagePtr = DamagePtr;}

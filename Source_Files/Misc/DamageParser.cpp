
/*
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
				DamagePtr->scale = long(FIXED_ONE*Scale + 0.5);
			else
				DamagePtr->scale = - long(- FIXED_ONE*Scale + 0.5);
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


/*
	Damage parser
	by Loren Petrich,
	May 17, 2000
	
	This parses damage info and places the results into a pointer
*/

#include <string.h>
#include "DamageParser.h"


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
	if (strcmp(Tag,"type") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",DamagePtr->type,short(NONE),short(NUMBER_OF_DAMAGE_TYPES-1)))
		{
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"flags") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",DamagePtr->flags,short(0),short(1)))
		{
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"base") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",DamagePtr->base))
		{
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"random") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",DamagePtr->random))
		{
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"scale") == 0)
	{
		float Scale;
		if (ReadBoundedNumericalValue(Value,"%f",Scale,float(SHORT_MIN),float(SHORT_MAX+1)))
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

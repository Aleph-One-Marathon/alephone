#ifndef _DAMAGE_PARSER_
#define _DAMAGE_PARSER_

/*
	Color parser
	by Loren Petrich,
	April 15, 2000
	
	This parses damage elements (name "damage").
	and returns the result into a damage_definition structure.
	
	These are:
	type (NONE, all damage types),
	flags (0 or 1 [is alien: weaker in easier difficulty levels])
	base
	random
	scale (1 = identity scaling)
	These are read in as values of attributes with these names;
	all of them are optional.
*/


#include "map.h"
#include "XML_ElementParser.h"

// Returns a parser for the damage
XML_ElementParser *Damage_GetParser();

// Sets a damage object to be read into.
void Damage_SetPointer(damage_definition *DamagePtr);

#endif

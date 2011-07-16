#ifndef _DAMAGE_PARSER_
#define _DAMAGE_PARSER_

/*
	Color parser

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

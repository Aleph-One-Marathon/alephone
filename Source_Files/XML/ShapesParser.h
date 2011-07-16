#ifndef _SHAPES_PARSER_
#define _SHAPES_PARSER_

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
	
	This parses shape elements (name "shape").
	and returns the parsed value into a a pointer.
*/

#include "shape_descriptors.h"
#include "XML_ElementParser.h"

// Returns a parser for the shapes;
// several elements may have shapes, so this ought to be callable several times.
XML_ElementParser *Shape_GetParser();

// This sets the pointer to the shape descriptor to be read into.
// Its args are that pointer, and whether "NONE" is an acceptable value for it.
void Shape_SetPointer(shape_descriptor *DescPtr, bool NONE_Is_OK = true);

#endif

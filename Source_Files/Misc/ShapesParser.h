#ifndef _SHAPES_PARSER_
#define _SHAPES_PARSER_

/*
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

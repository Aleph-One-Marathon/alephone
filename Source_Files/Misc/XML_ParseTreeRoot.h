#ifndef _XML_PARSE_TREE_ROOT_
#define _XML_PARSE_TREE_ROOT_
/*
	Root of XML-Parser Tree
	by Loren Petrich,
	April 16, 2000

	This is the absolute root element, the one that contains the root elements
	of all the valid XML files. It also contains a method for setting up the parse tree,
	including that root element, of course.
*/

#include "XML_ElementParser.h"

// That absolute root element
extern XML_ElementParser RootParser;

// That parse-tree setup routine; call it before doing anything with the tree
extern void SetupParseTree();

#endif

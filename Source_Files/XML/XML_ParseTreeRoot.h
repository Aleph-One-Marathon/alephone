#ifndef _XML_PARSE_TREE_ROOT_
#define _XML_PARSE_TREE_ROOT_
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

	Root of XML-Parser Tree
	by Loren Petrich,
	April 16, 2000

	This is the absolute root element, the one that contains the root elements
	of all the valid XML files. It also contains a method for setting up the parse tree,
	including that root element, of course.
*/

#include <stddef.h>

extern void ResetAllMMLValues(); // reset everything that's been changed to hard-coded defaults

class FileSpecifier;
extern bool ParseMMLFromFile(const FileSpecifier& filespec);
extern bool ParseMMLFromData(const char *buffer, size_t buflen);

#endif

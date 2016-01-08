#ifndef __SHAPE_DEFINITIONS_H
#define __SHAPE_DEFINITIONS_H
/*
SHAPE_DEFINITIONS.H

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

Tuesday, August 29, 1995 9:56:56 AM  (Jason)

Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.
*/

#include "shape_descriptors.h"

struct collection_definition;

/* ---------- structures */

struct collection_header /* 32 bytes on disk */
{
	int16 status;
	uint16 flags;

	int32 offset, length;
	int32 offset16, length16;

	// LP: handles to pointers
	collection_definition *collection;
	byte *shading_tables;
};
const int SIZEOF_collection_header = 32;

/* ---------- globals */

static struct collection_header collection_headers[MAXIMUM_COLLECTIONS];

#endif

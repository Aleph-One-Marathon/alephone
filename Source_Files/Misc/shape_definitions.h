#ifndef __SHAPE_DEFINITIONS_H
#define __SHAPE_DEFINITIONS_H
/*
SHAPE_DEFINITIONS.H
Tuesday, August 29, 1995 9:56:56 AM  (Jason)

Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.
*/

/* ---------- structures */

struct collection_header /* 32 bytes on disk */
{
	int16 status;
	uint16 flags;

	int32 offset, length;
	int32 offset16, length16;

	// LP: handles to pointers
	collection_definition *collection;
	void *shading_tables;
	
#ifdef mac
	int16 unused[2];
#endif
};

/* ---------- globals */

static struct collection_header collection_headers[MAXIMUM_COLLECTIONS];

#endif

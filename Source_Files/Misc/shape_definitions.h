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
	// Flat chunk now replaced with separate objects
	// collection_definition *collection;
	
	// LP: unpacked shapes data.
	// Note that Sequences and Images are pointers to lists,
	// because the header structs are followed with varying amounts of data
	collection_definition *Definition;
	rgb_color_value *Colors;
	high_level_shape_definition **Sequences;
	low_level_shape_definition *Frames;
	bitmap_definition **Bitmaps;
	
	byte *shading_tables;
	
	// int16 unused[2];
};
const int SIZEOF_collection_header = 32;

/* ---------- globals */

static struct collection_header collection_headers[MAXIMUM_COLLECTIONS];

#endif

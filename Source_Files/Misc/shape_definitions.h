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

struct collection_header /* 32 bytes */
{
	short status;
	word flags;

	long offset, length;
	long offset16, length16;

	// LP: handles to pointers
	collection_definition *collection;
	void *shading_tables;
	
	short unused[2];
/*
#ifdef mac	
	struct collection_definition **collection;
	void **shading_tables;
	
	short unused[2];
#endif

#ifdef win
	short unused[6];
#endif
*/
};

/* ---------- byte-swapping data */

static _bs_field _bs_collection_header[]=
{
	_2byte, _2byte, _4byte, _4byte, _4byte, _4byte,
	_4byte, _4byte, _2byte, _2byte
};

static _bs_field _bs_collection_definition[]=
{
	_2byte, _2byte, _2byte, _2byte, _2byte, _4byte,
	_2byte, _4byte, _2byte, _4byte, _2byte, _4byte,
	_2byte, _4byte, 253*sizeof(short)
};

static _bs_field _bs_high_level_shape_definition[]=
{
	_2byte, _2byte, HIGH_LEVEL_SHAPE_NAME_LENGTH+1, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	14*sizeof(short),
	sizeof(short) // we will byte-swap this later but the structure sizes must match
};

static _bs_field _bs_low_level_shape_definition[]=
{
	_2byte, _4byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	4*sizeof(short)
};

static _bs_field _bs_bitmap_definition[]=
{
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	2*sizeof(short),
	sizeof(void *)
};

static _bs_field _bs_rgb_color_value[]=
{
	2*sizeof(byte),
	_2byte, _2byte, _2byte
};

/* ---------- globals */

static struct collection_header collection_headers[MAXIMUM_COLLECTIONS];

#endif

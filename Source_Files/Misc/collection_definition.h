#ifndef __COLLECTION_DEFINITION_H
#define __COLLECTION_DEFINITION_H

/*
COLLECTION_DEFINITION.H
Friday, June 17, 1994 11:48:27 AM

Friday, June 17, 1994 11:27:13 PM
	added .minimum_light_intensity field to low-level shape.
Tuesday, June 21, 1994 2:59:16 PM
	added collection version number, added unused bytes to all structures.
Wednesday, June 22, 1994 3:53:22 PM
	scaling modifications.
Wednesday, June 22, 1994 10:07:38 PM
	added _scenery_collection type, .size field to collection_definition structure, changed
	‘shape_indexes’ to ‘low_level_shape_indexes’ in high_level_shape_definition structure
Saturday, July 9, 1994 3:36:05 PM
	added NUMBER_OF_PRIVATE_COLORS constant.
*/

/* ---------- collection definition structure */

/* 2 added pixels_to_world to collection_definition structure */
/* 3 added size to collection_definition structure */
#define COLLECTION_VERSION 3

/* at the beginning of the clut, used by the extractor for various opaque reasons */
#define NUMBER_OF_PRIVATE_COLORS 3

enum /* collection types */
{
	_unused_collection= 0, /* raw */
	_wall_collection, /* raw */
	_object_collection, /* rle */
	_interface_collection, /* raw */
	_scenery_collection /* rle */
};

struct collection_definition
{
	short version;
	
	short type; /* used for get_shape_descriptors() */
	word flags; /* [unused.16] */
	
	short color_count, clut_count;
	long color_table_offset; /* an array of clut_count arrays of color_count ColorSpec structures */

	short high_level_shape_count;
	long high_level_shape_offset_table_offset;

	short low_level_shape_count;
	long low_level_shape_offset_table_offset;

	short bitmap_count;
	long bitmap_offset_table_offset;

	short pixels_to_world; /* used to shift pixel values into world coordinates */
	
	long size; /* used to assert offsets */
	
	short unused[253];
};

/* ---------- high level shape definition */

#define HIGH_LEVEL_SHAPE_NAME_LENGTH 32

struct high_level_shape_definition
{
	short type; /* ==0 */
	word flags; /* [unused.16] */
	
	char name[HIGH_LEVEL_SHAPE_NAME_LENGTH+1];
	
	short number_of_views;
	
	short frames_per_view, ticks_per_frame;
	short key_frame;
	
	short transfer_mode;
	short transfer_mode_period; /* in ticks */
	
	short first_frame_sound, key_frame_sound, last_frame_sound;

	short pixels_to_world;

	short loop_frame;

	short unused[14];

	/* number_of_views*frames_per_view indexes of low-level shapes follow */
	short low_level_shape_indexes[1];
};

/* --------- low-level shape definition */

#define _X_MIRRORED_BIT 0x8000
#define _Y_MIRRORED_BIT 0x4000
#define _KEYPOINT_OBSCURED_BIT 0x2000

struct low_level_shape_definition
{
	word flags; /* [x-mirror.1] [y-mirror.1] [keypoint_obscured.1] [unused.13] */

	fixed minimum_light_intensity; /* in [0,FIXED_ONE] */

	short bitmap_index;
	
	/* (x,y) in pixel coordinates of origin */
	short origin_x, origin_y;
	
	/* (x,y) in pixel coordinates of key point */
	short key_x, key_y;

	short world_left, world_right, world_top, world_bottom;
	short world_x0, world_y0;
	
	short unused[4];
};

/* ---------- colors */

enum
{
	SELF_LUMINESCENT_COLOR_FLAG= 0x80
};

struct rgb_color_value
{
	byte flags;
	byte value;
	
	word red, green, blue;
};

#endif

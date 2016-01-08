#ifndef __COLLECTION_DEFINITION_H
#define __COLLECTION_DEFINITION_H

/*
COLLECTION_DEFINITION.H

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

#include "cstypes.h"
#include <vector>

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

struct high_level_shape_definition;
struct low_level_shape_definition;
struct bitmap_definition;
struct rgb_color_value;

struct collection_definition
{
	int16 version;
	
	int16 type; /* used for get_shape_descriptors() */
	uint16 flags; /* [unused.16] */
	
	int16 color_count, clut_count;
	int32 color_table_offset; /* an array of clut_count arrays of color_count ColorSpec structures */

	int16 high_level_shape_count;
	int32 high_level_shape_offset_table_offset;

	int16 low_level_shape_count;
	int32 low_level_shape_offset_table_offset;

	int16 bitmap_count;
	int32 bitmap_offset_table_offset;

	int16 pixels_to_world; /* used to shift pixel values into world coordinates */
	
	int32 size; /* used to assert offsets */
	
	int16 unused[253];

	std::vector<rgb_color_value> color_tables;
	std::vector<std::vector<uint8> > high_level_shapes;
	std::vector<low_level_shape_definition> low_level_shapes;
	std::vector<std::vector<uint8> > bitmaps;
};
const int SIZEOF_collection_definition = 544;

/* ---------- high level shape definition */

#define HIGH_LEVEL_SHAPE_NAME_LENGTH 32

struct high_level_shape_definition // Starting with number_of_views, this is a shape_animation_data structure
{
	int16 type; /* ==0 */
	uint16 flags; /* [unused.16] */
	
	char name[HIGH_LEVEL_SHAPE_NAME_LENGTH+2];
	
	int16 number_of_views;
	
	int16 frames_per_view, ticks_per_frame;
	int16 key_frame;
	
	int16 transfer_mode;
	int16 transfer_mode_period; /* in ticks */
	
	int16 first_frame_sound, key_frame_sound, last_frame_sound;

	int16 pixels_to_world;

	int16 loop_frame;

	int16 unused[14];

	/* see the interface.h/shape_animation_data for a decription of how many
	   low-level indices follow (it's not simply number_of_view * frames_per_view) */
	int16 low_level_shape_indexes[1];
};
const int SIZEOF_high_level_shape_definition = 90;

/* --------- low-level shape definition */

#define _X_MIRRORED_BIT 0x8000
#define _Y_MIRRORED_BIT 0x4000
#define _KEYPOINT_OBSCURED_BIT 0x2000

struct low_level_shape_definition
{
	uint16 flags; /* [x-mirror.1] [y-mirror.1] [keypoint_obscured.1] [unused.13] */

	_fixed minimum_light_intensity; /* in [0,FIXED_ONE] */

	int16 bitmap_index;
	
	/* (x,y) in pixel coordinates of origin */
	int16 origin_x, origin_y;
	
	/* (x,y) in pixel coordinates of key point */
	int16 key_x, key_y;

	int16 world_left, world_right, world_top, world_bottom;
	int16 world_x0, world_y0;
	
	int16 unused[4];
};
const int SIZEOF_low_level_shape_definition = 36;

/* ---------- colors */

enum
{
	SELF_LUMINESCENT_COLOR_FLAG= 0x80
};

struct rgb_color_value
{
	uint8 flags;
	uint8 value;
	
	uint16 red, green, blue;
};
const int SIZEOF_rgb_color_value = 8;

#endif

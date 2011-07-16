/*
	EDITOR.H

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

	Sunday, April 17, 1994 10:50:25 PM

Feb 12, 2000 (Loren Petrich):
	Added MARATHON_INFINITY_DATA_VERSION and set EDITOR_MAP_VERSION to it
*/

#ifndef __EDITOR_H_
#define __EDITOR_H_

#define MARATHON_ONE_DATA_VERSION 0
#define MARATHON_TWO_DATA_VERSION 1
#define MARATHON_INFINITY_DATA_VERSION 2
#define EDITOR_MAP_VERSION (MARATHON_INFINITY_DATA_VERSION)

#define MINIMUM_MAP_X_COORDINATE SHORT_MIN
#define MAXIMUM_MAP_X_COORDINATE SHORT_MAX
#define MINIMUM_MAP_Y_COORDINATE SHORT_MIN
#define MAXIMUM_MAP_Y_COORDINATE SHORT_MAX

#define MINIMUM_FLOOR_HEIGHT (-8*WORLD_ONE)
#define MINIMUM_CEILING_HEIGHT (MINIMUM_FLOOR_HEIGHT+WORLD_ONE)

#define MAXIMUM_FLOOR_HEIGHT (8*WORLD_ONE)
#define MAXIMUM_CEILING_HEIGHT (MAXIMUM_FLOOR_HEIGHT+WORLD_ONE)

#define INVALID_HEIGHT (MINIMUM_FLOOR_HEIGHT-1)

enum {
	_saved_guard_path_is_random= 0x0001
};

struct map_index_data 
{
	char level_name[LEVEL_NAME_LENGTH];
	char unused;
	int32 level_flags;
};

#define MAXIMUM_GUARD_PATH_CONTROL_POINTS 20

struct saved_path
{
	int16 point_count;
	uint16 flags;
	world_point2d points[MAXIMUM_GUARD_PATH_CONTROL_POINTS];
	int16 polygon_indexes[MAXIMUM_GUARD_PATH_CONTROL_POINTS];
};

/* Prevent ridiculous maps.. */
#define MAX_LINES_PER_VERTEX 15
#endif

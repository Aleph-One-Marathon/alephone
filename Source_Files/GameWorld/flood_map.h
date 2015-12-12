#ifndef __FLOOD_MAP_H
#define __FLOOD_MAP_H

/*
FLOOD_MAP.H

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

Saturday, June 18, 1994 8:07:35 PM
*/

#include "world.h"

/* ---------- constants */

enum /* flood modes */
{
	_depth_first, /* unsupported */
	_breadth_first, /* significantly faster than _best_first for large domains */
	_flagged_breadth_first, /* user data is interpreted as an int32 * to 4 bytes of flags */
	_best_first
};

/* ---------- typedefs */

typedef int32 (*cost_proc_ptr)(short source_polygon_index, short line_index, short destination_polygon_index, void *caller_data);

/* ---------- prototypes/PATHFINDING.C */

void allocate_pathfinding_memory(void);
void reset_paths(void);

short new_path(world_point2d *source_point, short source_polygon_index,
	world_point2d *destination_point, short destination_polygon_index,
	world_distance minimum_separation, cost_proc_ptr cost, void *data);
bool move_along_path(short path_index, world_point2d *p);
void delete_path(short path_index);

/* ---------- prototypes/FLOOD_MAP.C */

void allocate_flood_map_memory(void);

/* default cost_proc, NULL, is the area of the destination polygon and is significantly faster
	than supplying a user procedure */
short flood_map(short first_polygon_index, int32 maximum_cost, cost_proc_ptr cost_proc, short flood_mode, void *caller_data);
short reverse_flood_map(void);
short flood_depth(void);

void choose_random_flood_node(world_vector2d *bias);

#endif


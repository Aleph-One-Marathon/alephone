#ifndef __OVERHEAD_MAP_H
#define __OVERHEAD_MAP_H

/*
	OVERHEAD_MAP.H

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

	Saturday, July 9, 1994 11:19:49 PM

May 1, 2000 (Loren Petrich): Added XML parser object for the stuff here.
*/

#include "world.h"

#define OVERHEAD_MAP_MINIMUM_SCALE 1
#define OVERHEAD_MAP_MAXIMUM_SCALE 4
#define DEFAULT_OVERHEAD_MAP_SCALE 3

enum /* modes */
{
	_rendering_saved_game_preview,
	_rendering_checkpoint_map,
	_rendering_game_map
};

struct overhead_map_data
{
	short mode;
	short scale;
	world_point2d origin;
	short origin_polygon_index;
	short half_width, half_height;
	short width, height;
	short top, left;
	
	bool draw_everything;
};

void _render_overhead_map(struct overhead_map_data *data);

class InfoTree;
void parse_mml_overhead_map(const InfoTree& root);
void reset_mml_overhead_map();

#endif

#ifndef EPHEMERA_H
#define EPHEMERA_H

/*
	Copyright (C) 2020 and beyond by Gregory Smith
 
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
*/

#include <cstdint>

#include "map.h"
#include "shape_descriptors.h"
#include "world.h"

// object owner flags are unused, so we can re-use them to specify behavior
enum {
	_ephemera_end_when_animation_loops = 0x0004
};

void allocate_ephemera_storage(int max_ephemera);
void init_ephemera(int16_t polygon_count);
int16_t new_ephemera(const world_point3d& origin, int16_t polygon_index, shape_descriptor shape, angle facing);
void remove_ephemera(int16_t ephemera_index);

object_data* get_ephemera_data(int16_t ephemera_index);
int16_t get_polygon_ephemera(int16_t polygon_index);

void remove_ephemera_from_polygon(int16_t ephemera_index);
void add_ephemera_to_polygon(int16_t ephemera_index, int16_t polygon_index);

void set_ephemera_shape(int16_t ephemera_index, shape_descriptor shape);

void note_ephemera_polygon_rendered(int16_t polygon_index);

void update_ephemera();

#endif

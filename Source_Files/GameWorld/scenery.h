#ifndef __SCENERY_H
#define __SCENERY_H

/*
SCENERY.H

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

Thursday, December 1, 1994 12:19:13 PM  (Jason)

May 18, 2000 (Loren Petrich):
	Added XML-parser support
*/

#include "world.h"

/* ---------- prototypes/SCENERY.C */

void initialize_scenery(void);

short new_scenery(struct object_location *location, short scenery_type);

void animate_scenery(void);

// ghs: allow Lua to add and delete scenery
void deanimate_scenery(short object_index);
void randomize_scenery_shape(short object_index);

void randomize_scenery_shapes(void);

void get_scenery_dimensions(short scenery_type, world_distance *radius, world_distance *height);
void damage_scenery(short object_index);

bool get_scenery_collection(short scenery_type, short &collection);
bool get_damaged_scenery_collection(short scenery_type, short& collection);

class InfoTree;
void parse_mml_scenery(const InfoTree& root);
void reset_mml_scenery();

#endif

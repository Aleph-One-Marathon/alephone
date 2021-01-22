/*
INTERPOLATED_WORLD.CPP

	Copyright (C) 2021 Gregory Smith and the "Aleph One" developers.
 
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

	Storage for interpolated (> 30 fps) world
*/

#include "interpolated_world.h"

#include <cmath>
#include <cstdint>
#include <vector>

#include "map.h"

// ticks positions line up with 30 fps ticks
struct TickObjectData {
	bool used;
	world_point3d location;
};

static std::vector<TickObjectData> previous_tick_objects;
static std::vector<TickObjectData> current_tick_objects;

void init_interpolated_world()
{
	previous_tick_objects.resize(MAXIMUM_OBJECTS_PER_MAP);
	current_tick_objects.resize(MAXIMUM_OBJECTS_PER_MAP);
	
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto object = &objects[i];
		if (SLOT_IS_USED(object))
		{
			tick_object.used = true;
			tick_object.location = object->location;
		}
		else
		{
			tick_object.used = false;
		}
	}
	
	previous_tick_objects.assign(current_tick_objects.begin(),
								 current_tick_objects.end());
}

void enter_interpolated_world()
{
	previous_tick_objects.assign(current_tick_objects.begin(),
								 current_tick_objects.end());

	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto object = &objects[i];
		if (SLOT_IS_USED(object))
		{
			tick_object.used = true;
			tick_object.location = object->location;
		}
		else
		{
			tick_object.used = false;
		}
	}
}

void exit_interpolated_world()
{
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto& object = objects[i];

		if (tick_object.used)
		{
			object.location = tick_object.location;
		}
	}
}

static world_distance world_lerp(world_distance a,
								 world_distance b,
								 float t)
{
	return static_cast<world_distance>(std::round(a + (b - a) * t));
}

void update_interpolated_world(float heartbeat_fraction)
{
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		// Properly speaking, we shouldn't render objects that did not
		// exist "last" tick at all during a fractional frame. Doing
		// so "stretches" new objects' existences by almost (but not
		// quite) one tick. However, this is preferable to the flicker
		// that would otherwise appear when a projectile detonates.
		if (current_tick_objects[i].used && previous_tick_objects[i].used)
		{
			auto& object = objects[i];
			object.location.x =
					world_lerp(previous_tick_objects[i].location.x,
							   current_tick_objects[i].location.x,
							   heartbeat_fraction);
			
			object.location.y =
				world_lerp(previous_tick_objects[i].location.y,
						   current_tick_objects[i].location.y,
						   heartbeat_fraction);
			
			object.location.z =
				world_lerp(previous_tick_objects[i].location.z,
						   current_tick_objects[i].location.z,
						   heartbeat_fraction);

			// TODO: handle objects that change polygons
		}
	}
}

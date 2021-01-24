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
#include "player.h"
#include "render.h"

extern struct view_data* world_view;

// ticks positions line up with 30 fps ticks
struct TickObjectData {
	bool used;
	world_point3d location;
};

static std::vector<TickObjectData> previous_tick_objects;
static std::vector<TickObjectData> current_tick_objects;

struct TickPlayerData {
	int index;
	angle facing;
	angle elevation;
	_fixed weapon_intensity;
};

struct TickWorldView {
	int16_t origin_polygon_index;
	angle yaw, pitch;
	fixed_angle virtual_yaw, virtual_pitch;
	world_point3d origin;
	_fixed maximum_depth_intensity;
};

static TickWorldView previous_tick_world_view;
static TickWorldView current_tick_world_view;
static bool capture_world_view = true;

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

	capture_world_view = true;
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

	capture_world_view = true;
}

static int16_t lerp(int16_t a, int16_t b, float t)
{
	return static_cast<int16_t>(std::round(a + (b - a) * t));
}

static _fixed lerp(_fixed a, _fixed b, float t)
{
	return static_cast<_fixed>(std::round(a + (b - a) * t));
}

static angle lerp_angle(angle a, angle b, float t)
{
	a = NORMALIZE_ANGLE(a);
	b = NORMALIZE_ANGLE(b);
	
	if (a - b > HALF_CIRCLE)
	{
		b += FULL_CIRCLE;
	}
	else if (a - b < -HALF_CIRCLE)
	{
		a += FULL_CIRCLE;
	}
	
	angle ret = std::round(a+(b-a)*t);
	return NORMALIZE_ANGLE(ret);
}

static fixed_angle normalize_fixed_angle(fixed_angle theta)
{
	if (theta >= FULL_CIRCLE * FIXED_ONE)
	{
		theta -= FULL_CIRCLE * FIXED_ONE;
	}

	return theta;
}

static fixed_angle lerp_fixed_angle(fixed_angle a, fixed_angle b, float t)
{
	a = normalize_fixed_angle(a);
	b = normalize_fixed_angle(b);

	if (a - b > HALF_CIRCLE * FIXED_ONE)
	{
		b += FULL_CIRCLE * FIXED_ONE;
	}
	else if (a - b < -HALF_CIRCLE * FIXED_ONE)
	{
		a += FULL_CIRCLE * FIXED_ONE;
	}
	
	auto angle = a + (b - a) * t;
	return static_cast<fixed_angle>(std::round(normalize_fixed_angle(angle)));
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
			object.location.x = lerp(previous_tick_objects[i].location.x,
									  current_tick_objects[i].location.x,
									  heartbeat_fraction);
			
			object.location.y = lerp(previous_tick_objects[i].location.y,
									  current_tick_objects[i].location.y,
									  heartbeat_fraction);
			
			object.location.z = lerp(previous_tick_objects[i].location.z,
									  current_tick_objects[i].location.z,
									  heartbeat_fraction);
			
			// TODO: handle objects that change polygons
			// TODO: impose a maximum speed, in case lua moves stuff?
		}
	}
}

void interpolate_world_view(float heartbeat_fraction)
{
	auto prev = &previous_tick_world_view;
	auto next = &current_tick_world_view;
	auto view = world_view;
	
	if (capture_world_view)
	{
		capture_world_view = false;
		*prev = *next;
		
		next->origin_polygon_index = view->origin_polygon_index;
		next->yaw = view->yaw;
		next->pitch = view->pitch;
		next->virtual_yaw = view->virtual_yaw;
		next->virtual_pitch = view->virtual_pitch;
		next->origin = view->origin;
		next->maximum_depth_intensity = view->maximum_depth_intensity;
	}
	
	view->yaw = lerp_angle(prev->yaw,
						   next->yaw,
						   heartbeat_fraction);
	view->pitch = lerp_angle(prev->pitch,
							 next->pitch,
							 heartbeat_fraction);
	
	view->virtual_yaw = lerp_fixed_angle(prev->virtual_yaw,
										 next->virtual_yaw,
										 heartbeat_fraction);
	
	view->virtual_pitch = lerp_fixed_angle(prev->virtual_pitch,
										   next->virtual_pitch,
										   heartbeat_fraction);

	view->maximum_depth_intensity = lerp(prev->maximum_depth_intensity,
										 next->maximum_depth_intensity,
										 heartbeat_fraction);

	view->origin.x = lerp(prev->origin.x,
						  next->origin.x,
						  heartbeat_fraction);

	view->origin.y = lerp(prev->origin.y,
						  next->origin.y,
						  heartbeat_fraction);

	view->origin.z = lerp(prev->origin.z,
						  next->origin.z,
						  heartbeat_fraction);

	// TODO: handle view crossing polygons
	// TODO: don't always interpolate (could be switching players, or under camera control)
}

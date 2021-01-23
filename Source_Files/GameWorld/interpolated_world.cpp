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

// since we only interpolate the view (rather than the actual current player
// data), we only need to store the one from the previous tick
static TickPlayerData previous_tick_player;

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

	previous_tick_player.index = current_player_index;
	previous_tick_player.facing = current_player->facing;
	previous_tick_player.elevation = current_player->elevation;
	previous_tick_player.weapon_intensity = current_player->weapon_intensity;
}

static int16_t lerp(int16_t a, int16_t b, float t)
{
	return static_cast<int16_t>(std::round(a + (b - a) * t));
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
		}
	}
}

void interpolate_world_view(float heartbeat_fraction)
{
	if (current_player_index == previous_tick_player.index &&
		!(PLAYER_IS_TELEPORTING(current_player) && current_player->teleporting_phase - 1 == PLAYER_TELEPORTING_MIDPOINT))
	{
		auto view = world_view;
		auto prev = &previous_tick_player;
		auto next = current_player;
		
		view->yaw = lerp_angle(prev->facing,
							   next->facing,
							   heartbeat_fraction);
		view->pitch = lerp_angle(prev->elevation,
								 next->elevation,
								 heartbeat_fraction);

		view->virtual_yaw = lerp_fixed_angle(
			prev->facing * FIXED_ONE + prev_virtual_aim_delta().yaw,
			next->facing * FIXED_ONE + virtual_aim_delta().yaw,
			heartbeat_fraction);

		view->virtual_pitch = lerp_fixed_angle(
			prev->elevation * FIXED_ONE + prev_virtual_aim_delta().pitch,
			next->elevation * FIXED_ONE + virtual_aim_delta().pitch,
			heartbeat_fraction);

		view->maximum_depth_intensity = lerp(prev->weapon_intensity,
											 next->weapon_intensity,
											 heartbeat_fraction);
	}
}

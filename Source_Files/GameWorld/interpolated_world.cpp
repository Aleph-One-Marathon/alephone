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

#include "dynamic_limits.h"
#include "ephemera.h"
#include "map.h"
#include "Movie.h"
#include "player.h"
#include "preferences.h"
#include "render.h"
#include "weapons.h"

extern std::vector<int16_t> polygon_ephemera;

// above this speed, don't interpolate
static const world_distance speed_limit = WORLD_ONE_HALF;

bool world_is_interpolated;
static uint32_t start_machine_tick;

extern struct view_data* world_view;

// ticks positions line up with 30 fps ticks
struct TickObjectData {
	world_point3d location;
	int16_t polygon;
	uint16_t flags;
	int16_t next_object;
};

static std::vector<TickObjectData> previous_tick_objects;
static std::vector<TickObjectData> current_tick_objects;

struct TickPolygonData {
	world_distance floor_height;
	world_distance ceiling_height;
	int16_t first_object;
};

static std::vector<TickPolygonData> previous_tick_polygons;
static std::vector<TickPolygonData> current_tick_polygons;

struct TickSideData {
	world_distance y0;
};

static std::vector<TickSideData> previous_tick_sides;
static std::vector<TickSideData> current_tick_sides;

struct TickLineData {
	world_distance highest_adjacent_floor;
	world_distance lowest_adjacent_ceiling;
};

static std::vector<TickLineData> previous_tick_lines;
static std::vector<TickLineData> current_tick_lines;

struct TickPlayerData {
	int index;
	angle facing;
	angle elevation;
	_fixed weapon_intensity;
};

static std::vector<TickObjectData> previous_tick_ephemera;
static std::vector<TickObjectData> current_tick_ephemera;

static std::vector<int16_t> current_tick_polygon_ephemera;

struct TickWorldView {
	int16_t origin_polygon_index;
	angle yaw, pitch;
	fixed_angle virtual_yaw, virtual_pitch;
	world_point3d origin;
	_fixed maximum_depth_intensity;
};

static TickWorldView previous_tick_world_view;
static TickWorldView current_tick_world_view;

static std::vector<weapon_display_information> previous_tick_weapon_display;
static std::vector<weapon_display_information> current_tick_weapon_display;

struct TickWeaponDisplayInfo {
	_fixed vertical_positionin;
	_fixed horizontal_positionin;
};

struct ContrailInfo {
	int16_t projectile_index;
	int16_t polygon;
	world_point3d location;
};

// contrails don't move; store the location of the projectile from the previous
// tick and use that for interpolation
static std::vector<ContrailInfo> contrail_tracking;

void init_interpolated_world()
{
	if (get_fps_target() == 30)
	{
		world_is_interpolated = false;
		return;
	}

	current_tick_objects.resize(MAXIMUM_OBJECTS_PER_MAP);
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto object = &objects[i];

		tick_object.location = object->location;
		tick_object.polygon = object->polygon;
		tick_object.flags = object->flags;
		tick_object.next_object = object->next_object;
	}
	previous_tick_objects.assign(current_tick_objects.begin(),
								 current_tick_objects.end());

	current_tick_polygons.resize(dynamic_world->polygon_count);
	for (auto i = 0; i < dynamic_world->polygon_count; ++i)
	{
		auto& tick_polygon = current_tick_polygons[i];
		auto polygon = &map_polygons[i];

		tick_polygon.floor_height = polygon->floor_height;
		tick_polygon.ceiling_height = polygon->ceiling_height;
		tick_polygon.first_object = polygon->first_object;
	}
	previous_tick_polygons.assign(current_tick_polygons.begin(),
								  current_tick_polygons.end());
	
	current_tick_sides.resize(MAXIMUM_SIDES_PER_MAP);
	for (auto i = 0; i < MAXIMUM_SIDES_PER_MAP; ++i)
	{
		current_tick_sides[i].y0 = map_sides[i].primary_texture.y0;
	}
	previous_tick_sides.assign(current_tick_sides.begin(),
							   current_tick_sides.end());

	current_tick_lines.resize(MAXIMUM_LINES_PER_MAP);
	for (auto i = 0; i < MAXIMUM_LINES_PER_MAP; ++i)
	{
		auto& tick_line = current_tick_lines[i];
		auto line = get_line_data(i);

		tick_line.highest_adjacent_floor = line->highest_adjacent_floor;
		tick_line.lowest_adjacent_ceiling = line->lowest_adjacent_ceiling;
	}
	previous_tick_lines.assign(current_tick_lines.begin(),
							   current_tick_lines.end());

	current_tick_ephemera.resize(get_dynamic_limit(_dynamic_limit_ephemera));
	for (auto i = 0; i < get_dynamic_limit(_dynamic_limit_ephemera); ++i)
	{
		auto& tick_ephemera = current_tick_ephemera[i];
		auto ephemera = get_ephemera_data(i);

		tick_ephemera.location = ephemera->location;
		tick_ephemera.polygon = ephemera->polygon;
		tick_ephemera.flags = ephemera->flags;
		tick_ephemera.next_object = ephemera->next_object;
	}
	previous_tick_ephemera.assign(current_tick_ephemera.begin(),
								  current_tick_ephemera.end());

	current_tick_polygon_ephemera.resize(dynamic_world->polygon_count);
	for (auto i = 0; i < dynamic_world->polygon_count; ++i)
	{
		current_tick_polygon_ephemera[i] = polygon_ephemera[i];
	}

	previous_tick_world_view.origin_polygon_index = NONE;

	weapon_display_information data;
	short count = 0;
	while (get_weapon_display_information(&count, &data))
	{	
		current_tick_weapon_display.push_back(data);
	}
	previous_tick_weapon_display.assign(current_tick_weapon_display.begin(),
										current_tick_weapon_display.end());

	contrail_tracking.resize(MAXIMUM_OBJECTS_PER_MAP);
	for (auto i = 0; i < contrail_tracking.size(); ++i)
	{
		contrail_tracking[i].projectile_index = NONE;
	}

	world_is_interpolated = false;
}

extern void update_world_view_camera();

void enter_interpolated_world()
{
	if (get_fps_target() == 30)
	{
		return;
	}
	
	start_machine_tick = machine_tick_count();
	
	previous_tick_objects.assign(current_tick_objects.begin(),
								 current_tick_objects.end());
	
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto object = &objects[i];
		
		tick_object.location = object->location;
		tick_object.polygon = object->polygon;
		tick_object.flags = object->flags;
		tick_object.next_object = object->next_object;

		if (!SLOT_IS_USED(object))
		{
			contrail_tracking[i].projectile_index = NONE;
		}
	}

	previous_tick_polygons.assign(current_tick_polygons.begin(),
								  current_tick_polygons.end());

	for (auto i = 0; i < dynamic_world->polygon_count; ++i)
	{
		auto& tick_polygon = current_tick_polygons[i];
		auto& polygon = map_polygons[i];

		tick_polygon.floor_height = polygon.floor_height;
		tick_polygon.ceiling_height = polygon.ceiling_height;
		tick_polygon.first_object = polygon.first_object;

		current_tick_polygon_ephemera[i] = polygon_ephemera[i];
	}

	previous_tick_sides.assign(current_tick_sides.begin(),
							   current_tick_sides.end());
	for (auto i = 0; i < MAXIMUM_SIDES_PER_MAP; ++i)
	{
		current_tick_sides[i].y0 = map_sides[i].primary_texture.y0;
	}

	previous_tick_lines.assign(current_tick_lines.begin(),
							   current_tick_lines.end());
	for (auto i = 0; i < MAXIMUM_LINES_PER_MAP; ++i)
	{
		auto& tick_line = current_tick_lines[i];
		auto line = get_line_data(i);

		tick_line.highest_adjacent_floor = line->highest_adjacent_floor;
		tick_line.lowest_adjacent_ceiling = line->lowest_adjacent_ceiling;
	}

	previous_tick_ephemera.assign(current_tick_ephemera.begin(),
								  current_tick_ephemera.end());
	for (auto i = 0; i < get_dynamic_limit(_dynamic_limit_ephemera); ++i)
	{
		auto& tick_ephemera = current_tick_ephemera[i];
		auto ephemera = get_ephemera_data(i);

		tick_ephemera.location = ephemera->location;
		tick_ephemera.polygon = ephemera->polygon;
		tick_ephemera.flags = ephemera->flags;
		tick_ephemera.next_object = ephemera->next_object;
	}

	update_world_view_camera();

	auto prev = &previous_tick_world_view;
	auto next = &current_tick_world_view;
	auto view = world_view;
	
	*prev = *next;
	
	next->origin_polygon_index = view->origin_polygon_index;
	next->yaw = view->yaw;
	next->pitch = view->pitch;
	next->virtual_yaw = view->virtual_yaw;
	next->virtual_pitch = view->virtual_pitch;
	next->origin = view->origin;
	next->maximum_depth_intensity = view->maximum_depth_intensity;

	previous_tick_weapon_display.assign(current_tick_weapon_display.begin(),
										current_tick_weapon_display.end());

	current_tick_weapon_display.clear();
	short count = 0;
	weapon_display_information data;
	while (get_weapon_display_information(&count, &data))
	{
		current_tick_weapon_display.push_back(data);
	}

	for (auto i = 0; i < contrail_tracking.size(); ++i)
	{
		if (contrail_tracking[i].projectile_index != NONE)
		{
			MARK_SLOT_AS_USED(&previous_tick_objects[i]);
			previous_tick_objects[i].polygon = contrail_tracking[i].polygon;
			previous_tick_objects[i].location = contrail_tracking[i].location;
		}
	}

	world_is_interpolated = true;
}

void exit_interpolated_world()
{
	if (!world_is_interpolated)
	{
		return;
	}

	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto& tick_object = current_tick_objects[i];
		auto& object = objects[i];

		object.location = tick_object.location;
		object.polygon = tick_object.polygon;
		object.flags = tick_object.flags;
		object.next_object = tick_object.next_object;
	}

	for (auto i = 0; i < dynamic_world->polygon_count; ++i)
	{
		auto& tick_polygon = current_tick_polygons[i];
		auto& polygon = map_polygons[i];

		polygon.floor_height = tick_polygon.floor_height;
		polygon.ceiling_height = tick_polygon.ceiling_height;
		polygon.first_object = tick_polygon.first_object;

		polygon_ephemera[i] = current_tick_polygon_ephemera[i];
	}

	for (auto i = 0; i < MAXIMUM_SIDES_PER_MAP; ++i)
	{
		map_sides[i].primary_texture.y0 = current_tick_sides[i].y0;
	}

	for (auto i = 0; i < MAXIMUM_LINES_PER_MAP; ++i)
	{
		auto& tick_line = current_tick_lines[i];
		auto line = get_line_data(i);

		line->highest_adjacent_floor = tick_line.highest_adjacent_floor;
		line->lowest_adjacent_ceiling = tick_line.lowest_adjacent_ceiling;
	}

	for (auto i = 0; i < get_dynamic_limit(_dynamic_limit_ephemera); ++i)
	{
		auto& tick_ephemera = current_tick_ephemera[i];
		auto ephemera = get_ephemera_data(i);

		ephemera->location = tick_ephemera.location;
		ephemera->polygon = tick_ephemera.polygon;
		ephemera->flags = tick_ephemera.flags;
		ephemera->next_object = tick_ephemera.next_object;
	}

	world_is_interpolated = false;
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


static bool should_interpolate(world_point3d& prev, world_point3d& next)
{
	return world_is_interpolated &&
		guess_distance2d(reinterpret_cast<world_point2d*>(&prev),
						 reinterpret_cast<world_point2d*>(&next))
		<= speed_limit;
}

extern void add_object_to_polygon_object_list(short, short);

void update_interpolated_world(float heartbeat_fraction)
{
	if (!world_is_interpolated || heartbeat_fraction > 1.f)
	{
		return;
	}

	for (auto i = 0; i < dynamic_world->polygon_count; ++i)
	{
		if (!TEST_RENDER_FLAG(i, _polygon_is_visible))
		{
			continue;
		}

		auto& prev = previous_tick_polygons[i];
		auto& next = current_tick_polygons[i];
		auto& polygon = map_polygons[i];

		if (prev.floor_height != next.floor_height)
		{
			polygon.floor_height = lerp(prev.floor_height,
										next.floor_height,
										heartbeat_fraction);
		}

		if (prev.ceiling_height != next.ceiling_height)
		{
			polygon.ceiling_height = lerp(prev.ceiling_height,
										  next.ceiling_height,
										  heartbeat_fraction);
		}

		for (auto j = 0; j < polygon.vertex_count; ++j)
		{
			auto side_index = polygon.side_indexes[j];
			if (side_index != NONE &&
				current_tick_sides[side_index].y0 !=
				previous_tick_sides[side_index].y0)
			{
				map_sides[side_index].primary_texture.y0 = lerp(
					previous_tick_sides[side_index].y0,
					current_tick_sides[side_index].y0,
					heartbeat_fraction);
			}
		}
	}

	for (auto i = 0; i < MAXIMUM_LINES_PER_MAP; ++i)
	{
		auto line = get_line_data(i);
		if ((line->clockwise_polygon_owner == NONE ||
			!TEST_RENDER_FLAG(line->clockwise_polygon_owner,
							  _polygon_is_visible))
			&&
			(line->counterclockwise_polygon_owner == NONE ||
			!TEST_RENDER_FLAG(line->counterclockwise_polygon_owner,
							  _polygon_is_visible)))
		{
			continue;
		}
		
		auto& prev = previous_tick_lines[i];
		auto& next = current_tick_lines[i];

		if (prev.highest_adjacent_floor != next.highest_adjacent_floor)
		{
			line->highest_adjacent_floor = lerp(prev.highest_adjacent_floor,
												next.highest_adjacent_floor,
												heartbeat_fraction);
		}

		if (prev.lowest_adjacent_ceiling != next.lowest_adjacent_ceiling)
		{
			line->lowest_adjacent_ceiling = lerp(prev.lowest_adjacent_ceiling,
												 next.lowest_adjacent_ceiling,
												 heartbeat_fraction);
		}
	}
	
	for (auto i = 0; i < MAXIMUM_OBJECTS_PER_MAP; ++i)
	{
		auto prev = &previous_tick_objects[i];
		auto next = &current_tick_objects[i];

		if (!SLOT_IS_USED(next))
		{
			continue;
		}

		// Properly speaking, we shouldn't render objects that did not
		// exist "last" tick at all during a fractional frame. Doing
		// so "stretches" new objects' existences by almost (but not
		// quite) one tick. However, this is preferable to the flicker
		// that would otherwise appear when a projectile detonates.
		if (!SLOT_IS_USED(prev))
		{
			continue;
		}

		if (!TEST_RENDER_FLAG(prev->polygon, _polygon_is_visible) &&
			!TEST_RENDER_FLAG(next->polygon, _polygon_is_visible))
		{
			continue;
		}


		if (!should_interpolate(prev->location, next->location))
		{
			continue;
		}


		auto object = &objects[i];
		object->location.x = lerp(prev->location.x,
								  next->location.x,
								  heartbeat_fraction);
		
		object->location.y = lerp(prev->location.y,
								  next->location.y,
								  heartbeat_fraction);
		
		object->location.z = lerp(prev->location.z,
								  next->location.z,
								  heartbeat_fraction);
		
		if (object->polygon != next->polygon)
		{
			auto polygon_index = find_new_object_polygon(
				reinterpret_cast<world_point2d*>(&object->location),
				reinterpret_cast<world_point2d*>(&next->location),
				object->polygon);
			
			if (polygon_index == NONE)
			{
				object->location = next->location;
			}
			else
			{
				remove_object_from_polygon_object_list(i);
				add_object_to_polygon_object_list(i, polygon_index);
			}
		}
	}

	// TODO: this is not very DRY, see above
	for (auto i = 0; i < get_dynamic_limit(_dynamic_limit_ephemera); ++i)
	{
		auto prev = &previous_tick_ephemera[i];
		auto next = &current_tick_ephemera[i];

		// Properly speaking, we shouldn't render objects that did not
		// exist "last" tick at all during a fractional frame. Doing
		// so "stretches" new objects' existences by almost (but not
		// quite) one tick. However, this is preferable to the flicker
		// that would otherwise appear when a projectile detonates.
		if (!SLOT_IS_USED(next) || !SLOT_IS_USED(prev))
		{
			continue;
		}

		if (!TEST_RENDER_FLAG(prev->polygon, _polygon_is_visible) &&
			!TEST_RENDER_FLAG(next->polygon, _polygon_is_visible))
		{
			continue;
		}

		if (!should_interpolate(prev->location, next->location))
		{
			continue;
		}

		auto ephemera = get_ephemera_data(i);
		ephemera->location.x = lerp(prev->location.x,
									next->location.x,
									heartbeat_fraction);

		ephemera->location.y = lerp(prev->location.y,
									next->location.y,
									heartbeat_fraction);

		ephemera->location.z = lerp(prev->location.z,
									next->location.z,
									heartbeat_fraction);

		if (ephemera->polygon != next->polygon)
		{
			auto polygon_index = find_new_object_polygon(
				reinterpret_cast<world_point2d*>(&ephemera->location),
				reinterpret_cast<world_point2d*>(&next->location),
				ephemera->polygon);
			
			if (polygon_index == NONE)
			{
				ephemera->location = next->location;
			}
			else
			{
				remove_ephemera_from_polygon(i);
				add_ephemera_to_polygon(i, polygon_index);
			}
		}
	}
}

void interpolate_world_view(float heartbeat_fraction)
{
	auto prev = &previous_tick_world_view;
	auto next = &current_tick_world_view;
	auto view = world_view;
	
	if (!world_is_interpolated ||
		heartbeat_fraction > 1.f ||
		prev->origin_polygon_index == NONE ||
		!should_interpolate(prev->origin, next->origin))
	{
		return;
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
		
	if (prev->origin_polygon_index != next->origin_polygon_index)
	{
		auto polygon_index = find_new_object_polygon(
			reinterpret_cast<world_point2d*>(&prev->origin),
			reinterpret_cast<world_point2d*>(&view->origin),
			prev->origin_polygon_index);
		
		if (polygon_index == NONE)
		{
			view->origin = next->origin;
		}
		else
		{
			view->origin_polygon_index = polygon_index;
		}
	}
}

extern bool game_is_being_replayed();
extern int get_replay_speed();

int movie_export_phase;

float get_heartbeat_fraction()
{
	if (Movie::instance()->IsRecording())
	{
		if (get_fps_target())
		{
			auto skip = get_fps_target() / 30;
			if (movie_export_phase % skip)
			{
				return static_cast<float>(movie_export_phase % skip) / skip;
			}
			else
			{
				return 1.f;
			}
		}
		else
		{
			return 1.f;
		}
	}

	if (get_fps_target() == 30)
	{
		return 1.f;
	}
	else
	{
		auto fraction = static_cast<float>((machine_tick_count() - start_machine_tick) * TICKS_PER_SECOND + 1) / MACHINE_TICKS_PER_SECOND;
		
		auto speed = 1.f;
		if (game_is_being_replayed() && get_replay_speed() < 0)
		{
			speed = -get_replay_speed() + 1;
		}

		if (get_fps_target() > 0)
		{
			float q = get_fps_target() / TICKS_PER_SECOND;
			return std::min(std::ceil(fraction * q) / (q * speed), 1.f);
		}
		else
		{
			return fraction / speed;
		}
	}
}

void track_contrail_interpolation(int16_t projectile_index, int16_t effect_index)
{
	if (contrail_tracking.size() == 0)
	{
		return;
	}

	auto projectile = &previous_tick_objects[projectile_index];
	if (SLOT_IS_USED(projectile))
	{
		auto& contrail = contrail_tracking[effect_index];

		contrail.projectile_index = projectile_index;
		contrail.polygon = projectile->polygon;
		contrail.location = projectile->location;
	}
}

static void interpolate_weapon_display_information(
	short index,
	weapon_display_information* data)
{
	auto heartbeat_fraction = get_heartbeat_fraction();

	if (heartbeat_fraction >= 1.f)
	{
		return;
	}
	
	static constexpr int _shell_casing_type = 1; // from weapons.cpp

	weapon_display_information* next;
	weapon_display_information* prev;

	if (data->interpolation_data & 0x3 == _shell_casing_type)
	{
		next = &current_tick_weapon_display[index];
		prev = &previous_tick_weapon_display[index];

		// these shift around because it's a circular buffer, so find the match
		if (prev->interpolation_data != next->interpolation_data)
		{
			for (auto i = 0; i < previous_tick_weapon_display.size(); ++i)
			{
				if (previous_tick_weapon_display[i].interpolation_data == next->interpolation_data)
				{
					prev = &previous_tick_weapon_display[i];
					break;
				}
			}
		}
	}
	else
	{
		if (index >= previous_tick_weapon_display.size())
		{
			return;
		}

		next = &current_tick_weapon_display[index];
		prev = &previous_tick_weapon_display[index];
	}

	if (prev->interpolation_data != next->interpolation_data ||
		prev->horizontal_positioning_mode != next->horizontal_positioning_mode ||
		prev->vertical_positioning_mode != next->vertical_positioning_mode)
	{
		return;
	}
	
	auto dx = next->horizontal_position - prev->horizontal_position;
	auto dy = next->vertical_position - prev->vertical_position;

	data->vertical_position = lerp(prev->vertical_position,
								   next->vertical_position,
								   heartbeat_fraction);
	
	data->horizontal_position = lerp(prev->horizontal_position,
									 next->horizontal_position,
									 heartbeat_fraction);
}

bool get_interpolated_weapon_display_information(short* count,
												 weapon_display_information* data)
{
	auto heartbeat_fraction = get_heartbeat_fraction();
	if (*count < current_tick_weapon_display.size())
	{
		*data = current_tick_weapon_display[*count];
		interpolate_weapon_display_information(*count, data);
		++(*count);
		return true;
	}
	else
	{
		return false;
	}
}

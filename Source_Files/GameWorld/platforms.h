#ifndef __PLATFORMS_H
#define __PLATFORMS_H

/*
PLATFORMS.H

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

Friday, July 15, 1994 3:42:39 PM

Saturday, October 29, 1994 2:42:22 AM (Jason)
	razed.

May 18, 2000 (Loren Petrich):
	Added XML-parser support
*/

#include "map.h"

/* ---------- constants */

// #define MAXIMUM_PLATFORMS_PER_MAP 64

enum /* platform types */
{
	_platform_is_spht_door,
	_platform_is_spht_split_door,
	_platform_is_locked_spht_door,
	_platform_is_spht_platform,
	_platform_is_noisy_spht_platform,
	_platform_is_heavy_spht_door,
	_platform_is_pfhor_door,
	_platform_is_heavy_spht_platform,
	_platform_is_pfhor_platform,
	
	NUMBER_OF_PLATFORM_TYPES
};

enum /* platform speeds */
{
	_very_slow_platform= WORLD_ONE/(4*TICKS_PER_SECOND),
	_slow_platform= WORLD_ONE/(2*TICKS_PER_SECOND),
	_fast_platform= 2*_slow_platform,
	_very_fast_platform= 3*_slow_platform,
	_blindingly_fast_platform= 4*_slow_platform
};

enum /* platform delays */
{
	_no_delay_platform= 0, /* use carefully; difficult to reincarnate on */
	_short_delay_platform= TICKS_PER_SECOND,
	_long_delay_platform= 2*TICKS_PER_SECOND,
	_very_long_delay_platform= 4*TICKS_PER_SECOND,
	_extremely_long_delay_platform= 8*TICKS_PER_SECOND
};

enum /* static platform flags */
{
	_platform_is_initially_active, /* otherwise inactive */
	_platform_is_initially_extended, /* high for floor platforms, low for ceiling platforms, closed for two-way platforms */
	_platform_deactivates_at_each_level, /* this platform will deactivate each time it reaches a discrete level */
	_platform_deactivates_at_initial_level, /* this platform will deactivate upon returning to its original position */
	_platform_activates_adjacent_platforms_when_deactivating, /* when deactivating, this platform activates adjacent platforms */
	_platform_extends_floor_to_ceiling, /* i.e., there is no empty space when the platform is fully extended */
	_platform_comes_from_floor, /* platform rises from floor */
	_platform_comes_from_ceiling, /* platform lowers from ceiling */
	_platform_causes_damage, /* when obstructed by monsters, this platform causes damage */
	_platform_does_not_activate_parent, /* does not reactive it’s parent (i.e., that platform which activated it) */
	_platform_activates_only_once, /* cannot be activated a second time */
	_platform_activates_light, /* activates floor and ceiling lightsources while activating */
	_platform_deactivates_light, /* deactivates floor and ceiling lightsources while deactivating */
	_platform_is_player_controllable, /* i.e., door: players can use action key to change the state and/or direction of this platform */
	_platform_is_monster_controllable, /* i.e., door: monsters can expect to be able to move this platform even if inactive */
	_platform_reverses_direction_when_obstructed,
	_platform_cannot_be_externally_deactivated, /* when active, can only be deactivated by itself */
	_platform_uses_native_polygon_heights, /* complicated interpretation; uses native polygon heights during automatic min,max calculation */
	_platform_delays_before_activation, /* whether or not the platform begins with the maximum delay before moving */
	_platform_activates_adjacent_platforms_when_activating,
	_platform_deactivates_adjacent_platforms_when_activating,
	_platform_deactivates_adjacent_platforms_when_deactivating,
	_platform_contracts_slower,
	_platform_activates_adjacent_platforms_at_each_level,
	_platform_is_locked,
	_platform_is_secret,
	_platform_is_door,
	_platform_floods_m1,
	NUMBER_OF_STATIC_PLATFORM_FLAGS /* <=32 */
};

#define PLATFORM_IS_INITIALLY_ACTIVE(p) TEST_FLAG32((p)->static_flags, _platform_is_initially_active)
#define PLATFORM_IS_INITIALLY_EXTENDED(p) TEST_FLAG32((p)->static_flags, _platform_is_initially_extended)
#define PLATFORM_IS_INITIALLY_CONTRACTED(p) (!PLATFORM_IS_INITIALLY_EXTENDED(p))
#define PLATFORM_EXTENDS_FLOOR_TO_CEILING(p) TEST_FLAG32((p)->static_flags, _platform_extends_floor_to_ceiling)
#define PLATFORM_COMES_FROM_FLOOR(p) TEST_FLAG32((p)->static_flags, _platform_comes_from_floor)
#define PLATFORM_COMES_FROM_CEILING(p) TEST_FLAG32((p)->static_flags, _platform_comes_from_ceiling)
#define PLATFORM_GOES_BOTH_WAYS(p) (PLATFORM_COMES_FROM_FLOOR(p)&&PLATFORM_COMES_FROM_CEILING(p))
#define PLATFORM_CAUSES_DAMAGE(p) TEST_FLAG32((p)->static_flags, _platform_causes_damage)
#define PLATFORM_REVERSES_DIRECTION_WHEN_OBSTRUCTED(p) TEST_FLAG32((p)->static_flags, _platform_reverses_direction_when_obstructed)
#define PLATFORM_ACTIVATES_ONLY_ONCE(p) TEST_FLAG32((p)->static_flags, _platform_activates_only_once)
#define PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(p) TEST_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_when_deactivating)
#define PLATFORM_ACTIVATES_LIGHT(p) TEST_FLAG32((p)->static_flags, _platform_activates_light)
#define PLATFORM_DEACTIVATES_LIGHT(p) TEST_FLAG32((p)->static_flags, _platform_deactivates_light)
#define PLATFORM_DEACTIVATES_AT_EACH_LEVEL(p) TEST_FLAG32((p)->static_flags, _platform_deactivates_at_each_level)
#define PLATFORM_DEACTIVATES_AT_INITIAL_LEVEL(p) TEST_FLAG32((p)->static_flags, _platform_deactivates_at_initial_level)
#define PLATFORM_DOES_NOT_ACTIVATE_PARENT(p) TEST_FLAG32((p)->static_flags, _platform_does_not_activate_parent)
#define PLATFORM_IS_PLAYER_CONTROLLABLE(p) TEST_FLAG32((p)->static_flags, _platform_is_player_controllable)
#define PLATFORM_IS_MONSTER_CONTROLLABLE(p) TEST_FLAG32((p)->static_flags, _platform_is_monster_controllable)
#define PLATFORM_CANNOT_BE_EXTERNALLY_DEACTIVATED(p) TEST_FLAG32((p)->static_flags, _platform_cannot_be_externally_deactivated)
#define PLATFORM_USES_NATIVE_POLYGON_HEIGHTS(p) TEST_FLAG32((p)->static_flags, _platform_uses_native_polygon_heights)
#define PLATFORM_DELAYS_BEFORE_ACTIVATION(p) TEST_FLAG32((p)->static_flags, _platform_delays_before_activation)
#define PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(p) TEST_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_when_activating)
#define PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(p) TEST_FLAG32((p)->static_flags, _platform_deactivates_adjacent_platforms_when_activating)
#define PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(p) TEST_FLAG32((p)->static_flags, _platform_deactivates_adjacent_platforms_when_deactivating)
#define PLATFORM_CONTRACTS_SLOWER(p) TEST_FLAG32((p)->static_flags, _platform_contracts_slower)
#define PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_AT_EACH_LEVEL(p) TEST_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_at_each_level)
#define PLATFORM_IS_LOCKED(p) TEST_FLAG32((p)->static_flags, _platform_is_locked)
#define PLATFORM_IS_SECRET(p) TEST_FLAG32((p)->static_flags, _platform_is_secret)
#define PLATFORM_IS_DOOR(p) TEST_FLAG32((p)->static_flags, _platform_is_door)
#define PLATFORM_FLOODS_M1(p) TEST_FLAG32((p)->static_flags, _platform_floods_m1)

#define SET_PLATFORM_IS_INITIALLY_ACTIVE(p, v) SET_FLAG32((p)->static_flags, _platform_is_initially_active, (v))
#define SET_PLATFORM_IS_INITIALLY_EXTENDED(p, v) SET_FLAG32((p)->static_flags, _platform_is_initially_extended, (v))
#define SET_PLATFORM_EXTENDS_FLOOR_TO_CEILING(p, v) SET_FLAG32((p)->static_flags, _platform_extends_floor_to_ceiling, (v))
#define SET_PLATFORM_COMES_FROM_FLOOR(p, v) SET_FLAG32((p)->static_flags, _platform_comes_from_floor, (v))
#define SET_PLATFORM_COMES_FROM_CEILING(p, v) SET_FLAG32((p)->static_flags, _platform_comes_from_ceiling, (v))
#define SET_PLATFORM_CAUSES_DAMAGE(p, v) SET_FLAG32((p)->static_flags, _platform_causes_damage, (v))
#define SET_PLATFORM_REVERSES_DIRECTION_WHEN_OBSTRUCTED(p, v) SET_FLAG32((p)->static_flags, _platform_reverses_direction_when_obstructed, (v))
#define SET_PLATFORM_ACTIVATES_ONLY_ONCE(p, v) SET_FLAG32((p)->static_flags, _platform_activates_only_once, (v))
#define SET_PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(p, v) SET_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_when_deactivating, (v))
#define SET_PLATFORM_ACTIVATES_LIGHT(p, v) SET_FLAG32((p)->static_flags, _platform_activates_light, (v))
#define SET_PLATFORM_DEACTIVATES_LIGHT(p, v) SET_FLAG32((p)->static_flags, _platform_deactivates_light, (v))
#define SET_PLATFORM_DEACTIVATES_AT_EACH_LEVEL(p, v) SET_FLAG32((p)->static_flags, _platform_deactivates_at_each_level, (v))
#define SET_PLATFORM_DEACTIVATES_AT_INITIAL_LEVEL(p, v) SET_FLAG32((p)->static_flags, _platform_deactivates_at_initial_level, (v))
#define SET_PLATFORM_DOES_NOT_ACTIVATE_PARENT(p, v) SET_FLAG32((p)->static_flags, _platform_does_not_activate_parent, (v))
#define SET_PLATFORM_IS_PLAYER_CONTROLLABLE(p, v) SET_FLAG32((p)->static_flags, _platform_is_player_controllable, (v))
#define SET_PLATFORM_IS_MONSTER_CONTROLLABLE(p, v) SET_FLAG32((p)->static_flags, _platform_is_monster_controllable, (v))
#define SET_PLATFORM_CANNOT_BE_EXTERNALLY_DEACTIVATED(p, v) SET_FLAG32((p)->static_flags, _platform_cannot_be_externally_deactivated, (v))
#define SET_PLATFORM_USES_NATIVE_POLYGON_HEIGHTS(p, v) SET_FLAG32((p)->static_flags, _platform_uses_native_polygon_heights, (v))
#define SET_PLATFORM_DELAYS_BEFORE_ACTIVATION(p, v) SET_FLAG32((p)->static_flags, _platform_delays_before_activation, (v))
#define SET_PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(p, v) SET_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_when_activating, (v))
#define SET_PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(p, v) SET_FLAG32((p)->static_flags, _platform_deactivates_adjacent_platforms_when_activating, (v))
#define SET_PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(p, v) SET_FLAG32((p)->static_flags, _platform_deactivates_adjacent_platforms_when_deactivating, (v))
#define SET_PLATFORM_CONTRACTS_SLOWER(p, v) SET_FLAG32((p)->static_flags, _platform_contracts_slower, (v))
#define SET_PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_AT_EACH_LEVEL(p, v) SET_FLAG32((p)->static_flags, _platform_activates_adjacent_platforms_at_each_level, (v))
#define SET_PLATFORM_IS_LOCKED(p, v) SET_FLAG32((p)->static_flags, _platform_is_locked, (v))
#define SET_PLATFORM_IS_SECRET(p, v) SET_FLAG32((p)->static_flags, _platform_is_secret, (v))
#define SET_PLATFORM_IS_DOOR(p, v) SET_FLAG32((p)->static_flags, _platform_is_door, (v))
#define SET_PLATFORM_FLOODS_M1(p, v) SET_FLAG32((p)->static_flags, _platform_floods_m1, (v))

enum /* dynamic platform flags */
{
	_platform_is_active, /* otherwise inactive */
	_platform_is_extending, /* otherwise contracting; could be waiting between levels */
	_platform_is_moving, /* otherwise at rest (waiting between levels) */
	_platform_has_been_activated, /* in case we can only be activated once */
	_platform_was_moving, /* the platform moved unobstructed last tick */
	_platform_is_fully_extended,
	_platform_is_fully_contracted,
	_platform_was_just_activated_or_deactivated,
	_platform_floor_below_media,
	_platform_ceiling_below_media,
	NUMBER_OF_DYNAMIC_PLATFORM_FLAGS /* <=16 */
};

#define PLATFORM_IS_ACTIVE(p) TEST_FLAG16((p)->dynamic_flags, _platform_is_active)
#define PLATFORM_IS_EXTENDING(p) TEST_FLAG16((p)->dynamic_flags, _platform_is_extending)
#define PLATFORM_IS_CONTRACTING(p) (!PLATFORM_IS_EXTENDING(p))
#define PLATFORM_IS_MOVING(p) TEST_FLAG16((p)->dynamic_flags, _platform_is_moving)
#define PLATFORM_HAS_BEEN_ACTIVATED(p) TEST_FLAG16((p)->dynamic_flags, _platform_has_been_activated)
#define PLATFORM_WAS_MOVING(p) TEST_FLAG16((p)->dynamic_flags, _platform_was_moving)
#define PLATFORM_IS_FULLY_EXTENDED(p) TEST_FLAG16((p)->dynamic_flags, _platform_is_fully_extended)
#define PLATFORM_IS_FULLY_CONTRACTED(p) TEST_FLAG16((p)->dynamic_flags, _platform_is_fully_contracted)
#define PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(p) TEST_FLAG16((p)->dynamic_flags, _platform_was_just_activated_or_deactivated)
#define PLATFORM_FLOOR_BELOW_MEDIA(p) TEST_FLAG16((p)->dynamic_flags, _platform_floor_below_media)
#define PLATFORM_CEILING_BELOW_MEDIA(p) TEST_FLAG16((p)->dynamic_flags, _platform_ceiling_below_media)

#define SET_PLATFORM_IS_ACTIVE(p, v) SET_FLAG16((p)->dynamic_flags, _platform_is_active, (v))
#define SET_PLATFORM_IS_EXTENDING(p) SET_FLAG16((p)->dynamic_flags, _platform_is_extending, true)
#define SET_PLATFORM_IS_CONTRACTING(p) SET_FLAG16((p)->dynamic_flags, _platform_is_extending, false)
#define SET_PLATFORM_IS_MOVING(p, v) SET_FLAG16((p)->dynamic_flags, _platform_is_moving, (v))
#define SET_PLATFORM_HAS_BEEN_ACTIVATED(p) SET_FLAG16((p)->dynamic_flags, _platform_has_been_activated, true)
#define SET_PLATFORM_WAS_MOVING(p) SET_FLAG16((p)->dynamic_flags, _platform_was_moving, true)
#define SET_PLATFORM_WAS_BLOCKED(p) SET_FLAG16((p)->dynamic_flags, _platform_was_moving, false)
#define SET_PLATFORM_IS_FULLY_EXTENDED(p) SET_FLAG16((p)->dynamic_flags, _platform_is_fully_extended, true)
#define SET_PLATFORM_IS_FULLY_CONTRACTED(p) SET_FLAG16((p)->dynamic_flags, _platform_is_fully_contracted, true)
#define CLEAR_PLATFORM_POSITIONING_FLAGS(p) SET_FLAG16((p)->dynamic_flags, _platform_is_fully_contracted, false), SET_FLAG16((p)->dynamic_flags, _platform_is_fully_extended, false)
#define SET_PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(p) SET_FLAG16((p)->dynamic_flags, _platform_was_just_activated_or_deactivated, true)
#define CLEAR_PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(p) SET_FLAG16((p)->dynamic_flags, _platform_was_just_activated_or_deactivated, false)
#define SET_PLATFORM_FLOOR_BELOW_MEDIA(p, v) SET_FLAG16((p)->dynamic_flags, _platform_floor_below_media, v)
#define SET_PLATFORM_CEILING_BELOW_MEDIA(p, v) SET_FLAG16((p)->dynamic_flags, _platform_ceiling_below_media, v)

// using "fully contracted" is close enough to act like Marathon... I hope
#define PLATFORM_IS_FLOODED(p) (PLATFORM_FLOODS_M1(p) && PLATFORM_IS_FULLY_CONTRACTED(p))

struct endpoint_owner_data
{
	int16 first_polygon_index, polygon_index_count;
	int16 first_line_index, line_index_count;
};

struct static_platform_data /* size platform-dependant */
{
	int16 type;
	int16 speed, delay;
	world_distance maximum_height, minimum_height; /* if NONE then calculated in some reasonable way */

	uint32 static_flags;
	
	int16 polygon_index;
	
	int16 tag;
	
	int16 unused[7];
};
const int SIZEOF_static_platform_data = 32;

struct platform_data /* 140 bytes */
{
	int16 type;
	uint32 static_flags;
	int16 speed, delay;
	world_distance minimum_floor_height, maximum_floor_height;
	world_distance minimum_ceiling_height, maximum_ceiling_height;

	int16 polygon_index;

	uint16 dynamic_flags;
	world_distance floor_height, ceiling_height;
	int16 ticks_until_restart; /* if we’re not moving but are active, this is our delay until we move again */

	struct endpoint_owner_data endpoint_owners[MAXIMUM_VERTICES_PER_POLYGON];

	int16 parent_platform_index; /* the platform_index which activated us, if any */
	
	int16 tag;
	
	int16 unused[22];
};
const int SIZEOF_platform_data = 140;

/* --------- globals */

// Turned the list of platforms into a variable array;
// took over their maximum number as how many of them

extern vector<platform_data> PlatformList;
#define platforms (PlatformList.data())
#define MAXIMUM_PLATFORMS_PER_MAP (PlatformList.size())

// extern struct platform_data *platforms;

/* --------- prototypes/PLATFORMS.C */

short new_platform(struct static_platform_data *data, short polygon_index, short version);
struct static_platform_data *get_defaults_for_platform_type(short type);

void update_platforms(void);

void platform_was_entered(short platform_index, bool player);

bool try_and_change_platform_state(short platform_index, bool state);
bool try_and_change_tagged_platform_states(short tag, bool state);

enum /* return values from monster_can_enter_platform() and monster_can_leave_platform() */
{
	_platform_will_never_be_accessable,
	_platform_will_be_accessable,
	_platform_might_be_accessable,
	_platform_is_accessable,
	
	_exit_will_never_be_accessable,
	_exit_will_be_accessable,
	_exit_might_be_accessable,
	_exit_is_accessable
};
short monster_can_enter_platform(short platform_index, short source_polygon_index, world_distance height, world_distance minimum_ledge_delta, world_distance maximum_ledge_delta);
short monster_can_leave_platform(short platform_index, short destination_polygon_index, world_distance height, world_distance minimum_ledge_delta, world_distance maximum_ledge_delta);

bool platform_is_on(short platform_index);

void adjust_platform_for_media(short platform_index, bool initialize);
void adjust_platform_endpoint_and_line_heights(short platform_index);

void player_touch_platform_state(short player_index, short platform_index);
bool platform_is_legal_player_target(short platform_index);

bool platform_is_at_initial_state(short platform_index);

short get_platform_moving_sound(short platform_index);

platform_data *get_platform_data(
	short platform_index);

// LP: to pack and unpack this data;
// these do not make the definitions visible to the outside world

uint8 *unpack_static_platform_data(uint8 *Stream, static_platform_data *Objects, size_t Count);
uint8 *pack_static_platform_data(uint8 *Stream, static_platform_data *Objects, size_t Count);
uint8 *unpack_platform_data(uint8 *Stream, platform_data *Objects, size_t Count);
uint8 *pack_platform_data(uint8 *Stream, platform_data *Objects, size_t Count);

class InfoTree;
void parse_mml_platforms(const InfoTree& root);
void reset_mml_platforms();

#endif

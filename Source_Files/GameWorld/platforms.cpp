/*
PLATFORMS.C

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

Saturday, April 30, 1994 1:18:29 AM

Friday, September 16, 1994 7:50:32 PM   (alain)
	fixed update_polygon_endpoint_data_for_height_change() so that it actually
	calculates highest_adjacent_floor and lowest_adjacent_ceiling correctly.
Saturday, September 17, 1994 6:04:11 PM   (alain)
	added _one_stop_platform which moves one level, then won't move until you get off and back on.
Saturday, October 29, 1994 2:42:22 AM (Jason)
	razed.
Saturday, November 5, 1994 2:53:39 PM (Jason)
	added _platform_cannot_be_externally_deactivated.
Sunday, November 6, 1994 8:31:29 PM  (Jason)
	added _platform_uses_native_polygon_heights.
Tuesday, November 15, 1994 11:36:37 PM  (Jason)
	fixed recursive activates/deactivates; added flooding.
Wednesday, May 3, 1995 4:37:18 PM  (Jason)
	updates endpoint transparency correctly.
Friday, June 9, 1995 11:43:11 AM  (Jason')
	keys.
Tuesday, July 11, 1995 11:32:46 AM  (Jason)
	media sounds.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 25, 2000 (Loren Petrich):
	Suppressed consistency check for platform extrema in calculate_platform_extrema()
	as possibly unnecessary

May 17, 2000 (Loren Petrich):
	Added XML support, including a damage parser

Dec 19, 2000 (Loren Petrich):
	Suppressed assertion that a platform polygon must have at least one moving surface;
	this is for compatibility with some Pfhorte maps like "Descent". Also, added softer
	failure mode for get_platform_definition().
	Also suppressed an assertion that platform[polygon[platform]] = platform;
	currently handling failure in that by skipping over the platform.
	
Jun 30, 2002 (tiennou):
	Added support for Pfhortran Procedure: platform_activated
*/

#include <string.h>
#include "cseries.h"

#include "world.h"
#include "map.h"
#include "platforms.h"
#include "lightsource.h"
#include "SoundManager.h"
#include "player.h"
#include "media.h"
#include "InfoTree.h"

// LP addition: XML parser for damage
#include "items.h"
#include "Packing.h"

//MH: Lua scripting
#include "lua_script.h"

#include <string.h>

#include "editor.h" // MARATHON_ONE_DATA_VERSION

/*
//opening sounds made by closed platforms are sometimes obscured
*/

/* ---------- constants */

/* ---------- structures */

/* ---------- globals */

#include "platform_definitions.h"

/* ---------- private prototypes */

static short polygon_index_to_platform_index(short polygon_index);

bool set_platform_state(short platform_index, bool state, short parent_platform_index);
static void set_adjacent_platform_states(short platform_index, bool state);

static void take_out_the_garbage(short platform_index);
static void adjust_platform_sides(short platform_index, world_distance old_ceiling_height, world_distance new_ceiling_height);
static void calculate_platform_extrema(short platform_index, world_distance lowest_level,
	world_distance highest_level);

static void play_platform_sound(short platform_index, short sound_code);

static platform_definition *get_platform_definition(const short type);

/* ---------- code */

platform_data *get_platform_data(
	short platform_index)
{
	struct platform_data *platform = GetMemberWithBounds(platforms,platform_index,dynamic_world->platform_count);
	
	vassert(platform, csprintf(temporary, "platform index #%d is out of range", platform_index));
	
	return platform;
}

platform_definition *get_platform_definition(const short type)
{
	return GetMemberWithBounds(platform_definitions,type,NUMBER_OF_PLATFORM_TYPES);
}


short new_platform(
	struct static_platform_data *data,
	short polygon_index,
	short version)
{
	short platform_index= NONE;
	struct platform_data *platform;

	assert(NUMBER_OF_DYNAMIC_PLATFORM_FLAGS<=16);
	assert(NUMBER_OF_STATIC_PLATFORM_FLAGS<=32);
	// LP: OK for a platform to be a do-nothing platform
	// assert(data->static_flags&(FLAG(_platform_comes_from_floor)|FLAG(_platform_comes_from_ceiling)));

	if (dynamic_world->platform_count<int(MAXIMUM_PLATFORMS_PER_MAP))
	{
		struct polygon_data *polygon= get_polygon_data(polygon_index);
		short i;
		
		platform_index= dynamic_world->platform_count++;
		platform= platforms+platform_index;

		/* remember the platform_index in the polygonÕs .permutation field */
		polygon->permutation= platform_index;
		polygon->type= _polygon_is_platform;
		
		/* initialize the platform */
		platform->type= data->type;
		platform->static_flags= data->static_flags;
		platform->tag= data->tag;
		platform->speed= data->speed;
		platform->delay= data->delay;
		platform->polygon_index= polygon_index;
		platform->parent_platform_index= NONE;
		calculate_platform_extrema(platform_index, data->minimum_height, data->maximum_height);
		
		if (version == MARATHON_ONE_DATA_VERSION)
		{
			switch (platform->type)
			{
			case 0: // marathon door
			case 3: // pfhor door
				SET_PLATFORM_IS_DOOR(platform, true);
				break;
			}
			
			if (PLATFORM_IS_LOCKED(platform))
			{
				SET_PLATFORM_IS_LOCKED(platform, false);
				SET_PLATFORM_FLOODS_M1(platform, true);
			}
		}

#if 0
		switch (platform->type)
		{
			case _platform_is_spht_door:
			case _platform_is_spht_split_door:
			case _platform_is_locked_spht_door:
			case _platform_is_pfhor_door:
				SET_PLATFORM_IS_DOOR(platform, true);
				break;
		}
#endif
		
		/* stuff in the correct defaults; if the platform is initially active it begins moving
			immediately */
		platform->dynamic_flags= 0;
		platform->floor_height= polygon->floor_height;
		platform->ceiling_height= polygon->ceiling_height;
		if (PLATFORM_IS_INITIALLY_ACTIVE(platform))
		{
			SET_PLATFORM_IS_ACTIVE(platform, true);
			SET_PLATFORM_HAS_BEEN_ACTIVATED(platform);
			SET_PLATFORM_IS_MOVING(platform, true);
		}
		if (PLATFORM_IS_INITIALLY_EXTENDED(platform))
		{
			if (PLATFORM_COMES_FROM_FLOOR(platform)) platform->floor_height= platform->maximum_floor_height;
			if (PLATFORM_COMES_FROM_CEILING(platform)) platform->ceiling_height= platform->minimum_ceiling_height;
			SET_PLATFORM_IS_CONTRACTING(platform);
			SET_PLATFORM_IS_FULLY_EXTENDED(platform);
		}
		else
		{
			if (PLATFORM_COMES_FROM_FLOOR(platform)) platform->floor_height= platform->minimum_floor_height;
			if (PLATFORM_COMES_FROM_CEILING(platform)) platform->ceiling_height= platform->maximum_ceiling_height;
			SET_PLATFORM_IS_EXTENDING(platform);
			SET_PLATFORM_IS_FULLY_CONTRACTED(platform);
		}
		
		/* remember what polygons and lines are adjacent to the endpoints of the platform
			polygon so we can quickly recalculate heights later */
		for (i= 0; i<polygon->vertex_count; ++i)
		{
			calculate_endpoint_polygon_owners(polygon->endpoint_indexes[i], &platform->endpoint_owners[i].first_polygon_index,
				&platform->endpoint_owners[i].polygon_index_count);
			calculate_endpoint_line_owners(polygon->endpoint_indexes[i], &platform->endpoint_owners[i].first_line_index,
				&platform->endpoint_owners[i].line_index_count);
		}
		
		polygon->floor_height= platform->floor_height;
		polygon->ceiling_height= platform->ceiling_height;
		adjust_platform_endpoint_and_line_heights(platform_index);
		adjust_platform_for_media(platform_index, true);
	}
	
	return platform_index;
}

struct static_platform_data *get_defaults_for_platform_type(
	short type)
{
	struct platform_definition *definition= get_platform_definition(type);
	// Fallback for out-of-range type
	if (!definition) definition = get_platform_definition(0);
	
	return &definition->defaults;
}

void update_platforms(
	void)
{
	short platform_index;
	struct platform_data *platform;
	
	for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count; ++platform_index, ++platform)
	{
		CLEAR_PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(platform);
		
		if (PLATFORM_IS_ACTIVE(platform))
		{
			struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
			short sound_code= NONE;
			bool was_flooded = PLATFORM_IS_FLOODED(platform);
			
			// Should there be some warning message about platform-polygon inconsistences?
			// assert(polygon->permutation==platform_index);
			if (!(polygon->permutation==platform_index)) continue;
			
			if (!PLATFORM_IS_MOVING(platform))
			{
				/* waiting to move */
				if ((platform->ticks_until_restart-= 1)<=0)
				{
					SET_PLATFORM_IS_MOVING(platform, true);
					sound_code= _starting_sound;
				}
			}

			if (PLATFORM_IS_MOVING(platform))
			{
				struct platform_definition *definition= get_platform_definition(platform->type);
				if (!definition) continue;
				world_distance new_floor_height= platform->floor_height, new_ceiling_height= platform->ceiling_height;
				world_distance delta_height= PLATFORM_IS_EXTENDING(platform) ? platform->speed :
					(PLATFORM_CONTRACTS_SLOWER(platform) ? (-(platform->speed>>2)) : -platform->speed);

				/* adjust and pin heights: if we think weÕre fully contracted or expanded, make
					sure our heights reflect that (we donÕt want a split platform to have blank
					space between it because it didnÕt quite close all the way) */
				CLEAR_PLATFORM_POSITIONING_FLAGS(platform);
				if (PLATFORM_COMES_FROM_FLOOR(platform))
				{
					new_floor_height+= delta_height;
					if (new_floor_height>=platform->maximum_floor_height)
						SET_PLATFORM_IS_FULLY_EXTENDED(platform);
					if (new_floor_height<=platform->minimum_floor_height)
						SET_PLATFORM_IS_FULLY_CONTRACTED(platform);
				}
				if (PLATFORM_COMES_FROM_CEILING(platform))
				{
					new_ceiling_height-= delta_height;
					if (new_ceiling_height>=platform->maximum_ceiling_height)
						SET_PLATFORM_IS_FULLY_CONTRACTED(platform);
					if (new_ceiling_height<=platform->minimum_ceiling_height)
						SET_PLATFORM_IS_FULLY_EXTENDED(platform);
				}
				if (PLATFORM_IS_FULLY_EXTENDED(platform))
				{
					if (PLATFORM_COMES_FROM_FLOOR(platform)) new_floor_height= platform->maximum_floor_height;
					if (PLATFORM_COMES_FROM_CEILING(platform)) new_ceiling_height= platform->minimum_ceiling_height;
				}
				if (PLATFORM_IS_FULLY_CONTRACTED(platform))
				{
					if (PLATFORM_COMES_FROM_FLOOR(platform)) new_floor_height= platform->minimum_floor_height;
					if (PLATFORM_COMES_FROM_CEILING(platform)) new_ceiling_height= platform->maximum_ceiling_height;
				}
				
				/* calculate new ceiling and floor heights for the platform polygon and see if
					the change is obstructed */
				if (change_polygon_height(platform->polygon_index, new_floor_height, new_ceiling_height,
					PLATFORM_CAUSES_DAMAGE(platform) ? &definition->damage : (struct damage_definition *) NULL))
				{
					/* if we werenÕt blocked, remember that we moved last time, change our current
						level, adjust the textures if weÕre coming down from the ceiling,
						and finally adjust the heights of all endpoints and lines which make
						up our polygon to reflect the height change */
					if (PLATFORM_COMES_FROM_CEILING(platform))
						adjust_platform_sides(platform_index, platform->ceiling_height, new_ceiling_height);
					platform->ceiling_height= new_ceiling_height, platform->floor_height= new_floor_height;
					SET_PLATFORM_WAS_MOVING(platform);
					adjust_platform_endpoint_and_line_heights(platform_index);
					adjust_platform_for_media(platform_index, false);
				}
				else
				{
					/* if we were blocked, play a sound if we werenÕt blocked last time and reverse
						directions if weÕre supposed to */
					if (PLATFORM_WAS_MOVING(platform)) sound_code= _obstructed_sound;
					if (PLATFORM_REVERSES_DIRECTION_WHEN_OBSTRUCTED(platform))
					{
						PLATFORM_IS_EXTENDING(platform) ?
							SET_PLATFORM_IS_CONTRACTING(platform) :
							SET_PLATFORM_IS_EXTENDING(platform);
					}
					else
					{
						SET_PLATFORM_WAS_BLOCKED(platform);
					}
				}

				if (PLATFORM_IS_FULLY_EXTENDED(platform) || PLATFORM_IS_FULLY_CONTRACTED(platform))
				{
					bool deactivate= false;
					
					SET_PLATFORM_IS_MOVING(platform, false);
					platform->ticks_until_restart= platform->delay;
					sound_code= _stopping_sound;
					
					/* handle changing directions at extremes and deactivating if necessary */
					if (PLATFORM_IS_FULLY_CONTRACTED(platform))
					{
						if (PLATFORM_IS_INITIALLY_CONTRACTED(platform) && PLATFORM_DEACTIVATES_AT_INITIAL_LEVEL(platform))
							deactivate= true;
						SET_PLATFORM_IS_EXTENDING(platform);
					}
					else
					{
						if (PLATFORM_IS_FULLY_EXTENDED(platform))
						{
							if (platform->floor_height==platform->ceiling_height)
								take_out_the_garbage(platform_index);
							if (PLATFORM_IS_INITIALLY_EXTENDED(platform)
									&& PLATFORM_DEACTIVATES_AT_INITIAL_LEVEL(platform))
								deactivate= true;
							SET_PLATFORM_IS_CONTRACTING(platform);
						}
						else
						{
							assert(false);
						}
					}
					if (PLATFORM_DEACTIVATES_AT_EACH_LEVEL(platform)) deactivate= true;
					
					if (PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_AT_EACH_LEVEL(platform)) set_adjacent_platform_states(platform_index, true);
					if (deactivate) set_platform_state(platform_index, false, NONE);
				}
			}

			if (sound_code!=NONE) play_platform_sound(platform_index, sound_code);
			
			if (was_flooded != PLATFORM_IS_FLOODED(platform))
			{
				// flood status changed - update side lights
				// FIXME: this assumes Marathon 1 map lighting
				for (int i = 0; i < polygon->vertex_count; i++)
				{
					short side_index = polygon->side_indexes[i];
					if (side_index == NONE) continue;
					guess_side_lightsource_indexes(side_index);
				}
			}
		}
	}
}

bool platform_is_on(
	short platform_index)
{
	struct platform_data *platform;

	platform= get_platform_data(platform_index);	
	
	return PLATFORM_IS_ACTIVE(platform) ? true : false;
}

short monster_can_enter_platform(
	short platform_index,
	short source_polygon_index,
	world_distance height,
	world_distance minimum_ledge_delta,
	world_distance maximum_ledge_delta)
{
	struct polygon_data *source_polygon= get_polygon_data(source_polygon_index);
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *destination_polygon= get_polygon_data(platform->polygon_index);
	world_distance destination_floor_height= destination_polygon->floor_height;
	world_distance destination_ceiling_height= destination_polygon->ceiling_height;
	world_distance delta_height;
	short result_code= _platform_is_accessable;
	
	if (PLATFORM_IS_DOOR(platform))
	{
		if (PLATFORM_IS_MONSTER_CONTROLLABLE(platform) && platform->delay>=_short_delay_platform)
		{
			destination_floor_height= platform->minimum_floor_height;
			destination_ceiling_height= platform->maximum_ceiling_height;

			result_code= PLATFORM_IS_FULLY_CONTRACTED(platform) ? _platform_is_accessable : _platform_will_be_accessable;
		}
	}
	else
	{
		if (PLATFORM_IS_ACTIVE(platform) && PLATFORM_COMES_FROM_FLOOR(platform) && !PLATFORM_COMES_FROM_CEILING(platform))
		{
			/* if this platform doesnÕt go floor to ceiling and it stops at the source polygon, it might be ok */
			if (platform->maximum_floor_height!=platform->minimum_ceiling_height &&
				(platform->minimum_floor_height==source_polygon->floor_height ||
				platform->maximum_floor_height==source_polygon->floor_height))
			{
				if (platform->minimum_floor_height==source_polygon->floor_height)
				{
					destination_floor_height= platform->minimum_floor_height;
					destination_ceiling_height= platform->maximum_ceiling_height;
				}
				else
				{
					destination_floor_height= platform->maximum_floor_height;
					destination_ceiling_height= platform->minimum_ceiling_height;
				}
				result_code= (platform->floor_height==source_polygon->floor_height) ? _platform_is_accessable : _platform_will_be_accessable;
			}
		}
	}

	delta_height= destination_floor_height-source_polygon->floor_height;
	if (delta_height<minimum_ledge_delta || delta_height>maximum_ledge_delta ||
		MIN(destination_ceiling_height, source_polygon->ceiling_height) - MAX(destination_floor_height, source_polygon->floor_height)<height)
	{
		result_code= _platform_will_never_be_accessable;
	}
	
	return result_code;
}

short monster_can_leave_platform(
	short platform_index,
	short destination_polygon_index,
	world_distance height,
	world_distance minimum_ledge_delta,
	world_distance maximum_ledge_delta) /* negative */
{
	struct polygon_data *destination_polygon= get_polygon_data(destination_polygon_index);
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *source_polygon= get_polygon_data(platform->polygon_index);
	world_distance source_floor_height= source_polygon->floor_height;
	world_distance source_ceiling_height= source_polygon->ceiling_height;
	world_distance delta_height;
	short result_code= _exit_is_accessable;

	if (PLATFORM_IS_DOOR(platform))
	{
		source_floor_height= platform->minimum_floor_height;
		source_ceiling_height= platform->maximum_ceiling_height;
	}
	else
	{
		if (PLATFORM_IS_ACTIVE(platform) && PLATFORM_COMES_FROM_FLOOR(platform) && !PLATFORM_COMES_FROM_CEILING(platform))
		{
			if (platform->minimum_floor_height==destination_polygon->floor_height ||
				platform->maximum_floor_height==destination_polygon->floor_height)
			{
				source_floor_height= destination_polygon->floor_height;
				result_code= (platform->floor_height==destination_polygon->floor_height) ? _exit_is_accessable : _exit_will_be_accessable;
			}
		}
	}
	
	delta_height= destination_polygon->floor_height-source_floor_height;
	if (delta_height<minimum_ledge_delta || delta_height>maximum_ledge_delta ||
		MIN(destination_polygon->ceiling_height, source_ceiling_height) - MAX(destination_polygon->floor_height, source_floor_height)<height)
	{
		result_code= _exit_will_never_be_accessable;
	}

	return result_code;
}

void player_touch_platform_state(
	short player_index,
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct platform_definition *definition= get_platform_definition(platform->type);
	if (!definition) return;
	short sound_code= NONE;
	
	/* if we canÕt control this platform, play the uncontrollable sound, if itÕs inactive activate
		it and if itÕs active and moving reverse itÕs direction if thatÕs what it does when itÕs
		obstructed, if itÕs active but not moving then zero the delay */
	if (PLATFORM_IS_PLAYER_CONTROLLABLE(platform))
	{
		if (PLATFORM_IS_ACTIVE(platform))
		{
			if (PLATFORM_CANNOT_BE_EXTERNALLY_DEACTIVATED(platform))
			{
				sound_code= _uncontrollable_sound;
			}
			else
			{
				if (PLATFORM_IS_MOVING(platform))
				{
					if (PLATFORM_REVERSES_DIRECTION_WHEN_OBSTRUCTED(platform))
					{
						PLATFORM_IS_EXTENDING(platform) ?
							SET_PLATFORM_IS_CONTRACTING(platform) :
							SET_PLATFORM_IS_EXTENDING(platform);
						sound_code= _starting_sound;
					}
					else
					{
						sound_code= _uncontrollable_sound;
					}
				}
				else
				{
					platform->ticks_until_restart= 0;
				}
			}
		}
		else
		{
			if (definition->key_item_index==NONE || try_and_subtract_player_item(player_index, definition->key_item_index))
			{
				set_platform_state(platform_index, true, NONE);
			}
			else
			{
				// no key
				sound_code= _uncontrollable_sound;
			}
		}
	}
	else
	{
		sound_code= _uncontrollable_sound;
	}
	
	if (sound_code!=NONE) play_platform_sound(platform_index, sound_code);
}


void platform_was_entered(
	short platform_index,
	bool player)
{
	struct platform_data *platform= get_platform_data(platform_index);

	if (!PLATFORM_IS_DOOR(platform))
	{
		if ((player && PLATFORM_IS_PLAYER_CONTROLLABLE(platform)) ||
			(!player && PLATFORM_IS_MONSTER_CONTROLLABLE(platform)))
		{
			try_and_change_platform_state(platform_index, true);
		}
	}
}

bool platform_is_legal_player_target(
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct platform_definition *definition= get_platform_definition(platform->type);
	if (!definition) return false;
	bool legal_player_target= false;

	if (PLATFORM_IS_DOOR(platform))
	{
		if ((PLATFORM_IS_PLAYER_CONTROLLABLE(platform) || definition->uncontrollable_sound!=NONE) &&
			(!PLATFORM_ACTIVATES_ONLY_ONCE(platform) || !PLATFORM_HAS_BEEN_ACTIVATED(platform)))
		{
			legal_player_target= true;
		}
	}
	
	return legal_player_target;
}

bool platform_is_at_initial_state(
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	
	return (PLATFORM_HAS_BEEN_ACTIVATED(platform) && (!PLATFORM_IS_ACTIVE(platform) || PLATFORM_CANNOT_BE_EXTERNALLY_DEACTIVATED(platform))) ? false : true;
}

bool try_and_change_platform_state(
	short platform_index,
	bool state)
{
	struct platform_data *platform= get_platform_data(platform_index);
	bool changed= false;
	
	if (state || !PLATFORM_IS_ACTIVE(platform) || !PLATFORM_CANNOT_BE_EXTERNALLY_DEACTIVATED(platform))
	{
		bool new_state= set_platform_state(platform_index, state, NONE);
		
		changed= (new_state && state) || (!new_state && !state);
	}
	
	return changed;
}

bool try_and_change_tagged_platform_states(
	short tag,
	bool state)
{
	struct platform_data *platform;
	bool changed= false;
	short platform_index;
	
	if (tag)
	{
		for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count; ++platform_index, ++platform)
		{
			if (platform->tag==tag)
			{
				if (try_and_change_platform_state(platform_index, state))
				{
					changed= true;
				}
			}
		}
	}
	
	return changed;
}

short get_platform_moving_sound(
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct platform_definition *definition= get_platform_definition(platform->type);
	if (!definition) return NONE;
	
	return definition->moving_sound;
}

/* ---------- private code */


static short polygon_index_to_platform_index(
	short polygon_index)
{
	short platform_index;
	struct platform_data *platform;
	
	for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count; ++platform_index, ++platform)
	{
		if (platform->polygon_index==polygon_index) break;
	}
	if (platform_index==dynamic_world->platform_count) platform_index= NONE;
	
	return platform_index;
}

bool set_platform_state(
	short platform_index,
	bool state,
	short parent_platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	bool new_state= PLATFORM_IS_ACTIVE(platform) ? true : false;
	short sound_code= NONE;
	
	if (!PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(platform))
	{
		if (!state || !PLATFORM_ACTIVATES_ONLY_ONCE(platform) || !PLATFORM_HAS_BEEN_ACTIVATED(platform))
		{
			if ((state && !PLATFORM_IS_ACTIVE(platform)) || (!state && PLATFORM_IS_ACTIVE(platform)))
			{
				struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
				
				/* the state of this platform cannot be changed again this tick */
				SET_PLATFORM_WAS_JUST_ACTIVATED_OR_DEACTIVATED(platform);
				
				if (state)
				{
					SET_PLATFORM_HAS_BEEN_ACTIVATED(platform);
					SET_PLATFORM_IS_MOVING(platform, false);
					platform->ticks_until_restart= PLATFORM_DELAYS_BEFORE_ACTIVATION(platform) ?
						platform->delay : 0;

					if (PLATFORM_ACTIVATES_LIGHT(platform))
					{
						set_light_status(polygon->floor_lightsource_index, true);
						set_light_status(polygon->ceiling_lightsource_index, true);
					}
	
					platform->parent_platform_index= parent_platform_index;
	
					if (PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(platform)) set_adjacent_platform_states(platform_index, true);
					if (PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_ACTIVATING(platform)) set_adjacent_platform_states(platform_index, false);
				}
				else
				{
					if (PLATFORM_DEACTIVATES_LIGHT(platform))
					{
						set_light_status(polygon->floor_lightsource_index, false);
						set_light_status(polygon->ceiling_lightsource_index, false);
					}
					
					if (PLATFORM_ACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(platform)) set_adjacent_platform_states(platform_index, true);
					if (PLATFORM_DEACTIVATES_ADJACENT_PLATFORMS_WHEN_DEACTIVATING(platform)) set_adjacent_platform_states(platform_index, false);
					
					if (PLATFORM_IS_MOVING(platform)) sound_code= _obstructed_sound;
				}
				
				/* assume the correct state, and correctly update all switches referencing this platform */
				SET_PLATFORM_IS_ACTIVE(platform, state);
                                //MH: Lua script hook
                                L_Call_Platform_Activated(platform->polygon_index);
				assume_correct_switch_position(_panel_is_platform_switch, platform->polygon_index, state);
				
				new_state= state;
			}
		}
	
		if (sound_code!=NONE) play_platform_sound(platform_index, sound_code);
	}
	
	return new_state;
}

static void set_adjacent_platform_states(
	short platform_index,
	bool state)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	short i;
	
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		short adjacent_polygon_index= polygon->adjacent_polygon_indexes[i];
		short adjacent_platform_index= polygon_index_to_platform_index(adjacent_polygon_index);

		if (adjacent_platform_index!=NONE)
		{
			struct polygon_data *adjacent_polygon= get_polygon_data(adjacent_polygon_index);
			
			if (!PLATFORM_DOES_NOT_ACTIVATE_PARENT(platform) || platform->parent_platform_index!=adjacent_platform_index)
			{
				set_platform_state(adjacent_polygon->permutation, state, platform_index);
			}
		}
	}
}

/* remove all garbage objects in the platform */
static void take_out_the_garbage(
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	short object_index= polygon->first_object;
	
	while (object_index!=NONE)
	{
		struct object_data *object= get_object_data(object_index);
		
		if (GET_OBJECT_OWNER(object)==_object_is_garbage) remove_map_object(object_index);
		object_index= object->next_object; /* relies on remove_map_object() not changing this */
	}
}

void adjust_platform_for_media(
	short platform_index,
	bool initialize)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	
	if (polygon->media_index!=NONE)
	{
		// LP change: idiot-proofing
		struct media_data *media= get_media_data(polygon->media_index);
		if (media)
		{
		bool floor_below_media= platform->floor_height<media->height;
		bool ceiling_below_media= platform->ceiling_height<media->height;
		
		if (!initialize)
		{
			short sound_code= NONE;
			
			if ((PLATFORM_FLOOR_BELOW_MEDIA(platform) && !floor_below_media) ||
				(PLATFORM_CEILING_BELOW_MEDIA(platform) && !ceiling_below_media))
			{
				sound_code= _media_snd_platform_leaving;
			}
			if ((!PLATFORM_FLOOR_BELOW_MEDIA(platform) && floor_below_media) ||
				(!PLATFORM_CEILING_BELOW_MEDIA(platform) && ceiling_below_media))
			{
				sound_code= _media_snd_platform_entering;
			}
			
			if (sound_code!=NONE)
			{
				play_polygon_sound(platform->polygon_index, get_media_sound(polygon->media_index, sound_code));
			}
		}
		
		SET_PLATFORM_FLOOR_BELOW_MEDIA(platform, floor_below_media);
		SET_PLATFORM_CEILING_BELOW_MEDIA(platform, ceiling_below_media);
		}
	}
}

void adjust_platform_endpoint_and_line_heights(
	short platform_index)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	short i;
	
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		struct endpoint_data *endpoint= get_endpoint_data(polygon->endpoint_indexes[i]);
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		short polygon_count= platform->endpoint_owners[i].polygon_index_count;
		short *polygon_indexes= get_map_indexes(platform->endpoint_owners[i].first_polygon_index, polygon_count);
		short line_count= platform->endpoint_owners[i].line_index_count;
		short *line_indexes= get_map_indexes(platform->endpoint_owners[i].first_line_index, line_count);
		short lowest_adjacent_ceiling= 0, highest_adjacent_floor= 0, supporting_polygon_index = NONE;
		struct polygon_data *adjacent_polygon;
		short j;
		
		/* adjust line heights and set proper line transparency and solidity */
		// Skip this step if line indexes were not found
		if (polygon->adjacent_polygon_indexes[i]!=NONE && line_indexes)
		{
			adjacent_polygon= get_polygon_data(polygon->adjacent_polygon_indexes[i]);
			line->highest_adjacent_floor= MAX(polygon->floor_height, adjacent_polygon->floor_height);
			line->lowest_adjacent_ceiling= MIN(polygon->ceiling_height, adjacent_polygon->ceiling_height);

			/* only worry about transparency and solidity if thereÕs a polygon on the other side */
			if (LINE_IS_VARIABLE_ELEVATION(line))
			{
				SET_LINE_TRANSPARENCY(line, line->highest_adjacent_floor<line->lowest_adjacent_ceiling);
				SET_LINE_SOLIDITY(line, line->highest_adjacent_floor>=line->lowest_adjacent_ceiling);
			}
			
			/* and only if there is another polygon does this endpoint have a chance of being transparent */
			for (j= 0; j<line_count; ++j) if (LINE_IS_SOLID(get_line_data(line_indexes[j]))) break;
			SET_ENDPOINT_SOLIDITY(endpoint, (j!=line_count));

			/* and only if there is another polygon does this endpoint have a chance of being transparent */
			for (j= 0; j<line_count; ++j) if (!LINE_IS_TRANSPARENT(get_line_data(line_indexes[j]))) break;
			SET_ENDPOINT_TRANSPARENCY(endpoint, (j==line_count));
		}
		else
		{
			line->highest_adjacent_floor= polygon->floor_height;
			line->lowest_adjacent_ceiling= polygon->ceiling_height;
		}

		/* adjust endpoint heights */
		// Skip this step if no polygon indexes were found
		if (polygon_indexes)
		{
			for (j= 0; j<polygon_count; ++j)
			{
				adjacent_polygon= get_polygon_data(polygon_indexes[j]);
				if (!j || highest_adjacent_floor<adjacent_polygon->floor_height) highest_adjacent_floor= adjacent_polygon->floor_height, supporting_polygon_index= polygon_indexes[j];
				if (!j || lowest_adjacent_ceiling>adjacent_polygon->ceiling_height) lowest_adjacent_ceiling= adjacent_polygon->ceiling_height;
			}
		}
		endpoint->highest_adjacent_floor_height= highest_adjacent_floor;
		endpoint->lowest_adjacent_ceiling_height= lowest_adjacent_ceiling;
		endpoint->supporting_polygon_index= supporting_polygon_index;
	}
}

static void play_platform_sound(
	short platform_index,
	short type)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct platform_definition *definition= get_platform_definition(platform->type);
	if (!definition) return;
	short sound_code;
	
	switch (type)
	{
		case _obstructed_sound:
			sound_code= definition->obstructed_sound;
			break;
		
		case _uncontrollable_sound:
			sound_code= definition->uncontrollable_sound;
			break;
		
		case _starting_sound:
			sound_code= PLATFORM_IS_EXTENDING(platform) ? definition->starting_extension : definition->starting_contraction;
			break;
		case _stopping_sound:
			sound_code= PLATFORM_IS_FULLY_CONTRACTED(platform) ? definition->stopping_contraction : definition->stopping_extension;
			break;
		
		default:
			assert(false);
			break;
	}
	
	play_polygon_sound(platform->polygon_index, sound_code);
	SoundManager::instance()->CauseAmbientSoundSourceUpdate();
}

/* rules for using native polygon heights: a) if this is a floor platform, then take the polygonÕs
	native floor height to be the maximum height if it is greater than the minimum height, otherwise
	use it as the minimum height; b) if this is a ceiling platform, then take the polygonÕs native
	ceiling height to be the minimum height if it is less than the maximum height, otherwise use it
	as the maximum height; c) native polygon height is not used for floor/ceiling platforms */
static void calculate_platform_extrema(
	short platform_index,
	world_distance lowest_level,
	world_distance highest_level)
{
	short i;
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	world_distance lowest_adjacent_floor, highest_adjacent_ceiling;
	world_distance highest_adjacent_floor, lowest_adjacent_ceiling;
	
	// LP change: no need for this test
	// assert(lowest_level==NONE||highest_level==NONE||lowest_level<highest_level);
	
	/* calculate lowest and highest adjacent floors and ceilings */
	lowest_adjacent_floor= highest_adjacent_floor= polygon->floor_height;
	lowest_adjacent_ceiling= highest_adjacent_ceiling= polygon->ceiling_height;
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		if (polygon->adjacent_polygon_indexes[i]!=NONE)
		{
			struct polygon_data *adjacent_polygon= get_polygon_data(polygon->adjacent_polygon_indexes[i]);
			
			if (adjacent_polygon->floor_height<lowest_adjacent_floor) lowest_adjacent_floor= adjacent_polygon->floor_height;
			if (adjacent_polygon->floor_height>highest_adjacent_floor) highest_adjacent_floor= adjacent_polygon->floor_height;
			if (adjacent_polygon->ceiling_height<lowest_adjacent_ceiling) lowest_adjacent_ceiling= adjacent_polygon->ceiling_height;
			if (adjacent_polygon->ceiling_height>highest_adjacent_ceiling) highest_adjacent_ceiling= adjacent_polygon->ceiling_height;
		}
	}

	/* take into account the EXTENDS_FLOOR_TO_CEILING flag */
	if (PLATFORM_EXTENDS_FLOOR_TO_CEILING(platform))
	{
		if (polygon->ceiling_height>highest_adjacent_floor) highest_adjacent_floor= polygon->ceiling_height;
		if (polygon->floor_height<lowest_adjacent_ceiling) lowest_adjacent_ceiling= polygon->floor_height;
	}
	
	/* calculate floor and ceiling min, max values as appropriate for the platform direction */
	if (PLATFORM_GOES_BOTH_WAYS(platform))
	{
		/* split platforms always meet in the center */
		platform->minimum_floor_height= lowest_level==NONE ? lowest_adjacent_floor : lowest_level;
		platform->maximum_ceiling_height= highest_level==NONE ? highest_adjacent_ceiling : highest_level;
		platform->maximum_floor_height= platform->minimum_ceiling_height=
			(platform->minimum_floor_height+platform->maximum_ceiling_height)/2;
	}
	else
	{
		if (PLATFORM_COMES_FROM_FLOOR(platform))
		{
			if (PLATFORM_USES_NATIVE_POLYGON_HEIGHTS(platform))
			{
				if (polygon->floor_height<lowest_adjacent_floor || PLATFORM_EXTENDS_FLOOR_TO_CEILING(platform))
				{
					lowest_adjacent_floor= polygon->floor_height;
				}
				else
				{
					highest_adjacent_floor= polygon->floor_height;
				}
			}
			
			platform->minimum_floor_height= lowest_level==NONE ? lowest_adjacent_floor : lowest_level;
			platform->maximum_floor_height= highest_level==NONE ? highest_adjacent_floor : highest_level;
			platform->minimum_ceiling_height= platform->maximum_ceiling_height= polygon->ceiling_height;
		}
		else if (PLATFORM_COMES_FROM_CEILING(platform))
		{

			if (PLATFORM_USES_NATIVE_POLYGON_HEIGHTS(platform))
			{
				if (polygon->ceiling_height>highest_adjacent_ceiling || PLATFORM_EXTENDS_FLOOR_TO_CEILING(platform))
				{
					highest_adjacent_ceiling= polygon->ceiling_height;
				}
				else
				{
					lowest_adjacent_ceiling= polygon->ceiling_height;
				}
			}
			
			platform->minimum_ceiling_height= lowest_level==NONE ? lowest_adjacent_ceiling : lowest_level;
			platform->maximum_ceiling_height= highest_level==NONE ? highest_adjacent_ceiling : highest_level;
			platform->minimum_floor_height= platform->maximum_floor_height= polygon->floor_height;
		}
	}
}

static void adjust_platform_sides(
	short platform_index, 
	world_distance old_ceiling_height,
	world_distance new_ceiling_height)
{
	struct platform_data *platform= get_platform_data(platform_index);
	struct polygon_data *polygon= get_polygon_data(platform->polygon_index);
	world_distance delta_height= new_ceiling_height-old_ceiling_height;
	short i;
	
	for (i= 0; i<polygon->vertex_count; ++i)
	{
		short side_index;
		struct side_data *side;
		struct line_data *line= get_line_data(polygon->line_indexes[i]);
		short adjacent_polygon_index= polygon->adjacent_polygon_indexes[i];
		
		/* adjust the platform side (i.e., the texture on the side of the platform) */
		if (adjacent_polygon_index!=NONE)
		{
			side_index= adjacent_polygon_index==line->clockwise_polygon_owner ? line->clockwise_polygon_side_index : line->counterclockwise_polygon_side_index;
			if (side_index!=NONE)
			{
				side= get_side_data(side_index);
				switch (side->type)
				{
					case _full_side:
					case _high_side:
					case _split_side:
						side->primary_texture.y0+= delta_height;
						break;
				}
			}
		}

		/* adjust the shaft side (i.e., the texture the platform slides against) */		
		side_index= polygon->side_indexes[i];
		if (side_index!=NONE)
		{
			world_distance top_of_side_height;
			
			side= get_side_data(side_index);
			switch (side->type)
			{
				case _split_side: /* secondary */
					top_of_side_height= MIN(line->highest_adjacent_floor,  polygon->ceiling_height);
					side->primary_texture.y0-= (old_ceiling_height<top_of_side_height && new_ceiling_height<top_of_side_height) ?
						delta_height : new_ceiling_height-top_of_side_height;
					break;
				case _high_side: /* primary */
//					top_of_side_height= polygon->ceiling_height;
					side->primary_texture.y0-= delta_height; //(old_ceiling_height<top_of_side_height && new_ceiling_height<top_of_side_height) ?
//						delta_height : new_ceiling_height-top_of_side_height;
					break;
				case _full_side: /* primary */
					side->primary_texture.y0-= delta_height;
					break;
				case _low_side: /* primary */
					// ghs: the following doesn't appear to be necessary at all!
/*
					top_of_side_height= MIN(line->highest_adjacent_floor, polygon->ceiling_height);
					side->primary_texture.y0-= (old_ceiling_height<top_of_side_height && new_ceiling_height<top_of_side_height) ?
						delta_height : new_ceiling_height-top_of_side_height;
*/
					break;
			
				default:
					vhalt(csprintf(temporary, "wasn't expecting side #%d to have type #%d", side_index, side->type));
					break;
			}
		}
	}
}

uint8 *unpack_static_platform_data(uint8 *Stream, static_platform_data* Objects, size_t Count)
{
	uint8* S = Stream;
	static_platform_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->speed);
		StreamToValue(S,ObjPtr->delay);
		StreamToValue(S,ObjPtr->maximum_height);
		StreamToValue(S,ObjPtr->minimum_height);
		
		StreamToValue(S,ObjPtr->static_flags);
		
		StreamToValue(S,ObjPtr->polygon_index);
		
		StreamToValue(S,ObjPtr->tag);
		
		S += 7*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_static_platform_data));
	return S;
}

uint8 * pack_static_platform_data(uint8 *Stream, static_platform_data* Objects, size_t Count)
{
	uint8* S = Stream;
	static_platform_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->speed);
		ValueToStream(S,ObjPtr->delay);
		ValueToStream(S,ObjPtr->maximum_height);
		ValueToStream(S,ObjPtr->minimum_height);
		
		ValueToStream(S,ObjPtr->static_flags);
		
		ValueToStream(S,ObjPtr->polygon_index);
		
		ValueToStream(S,ObjPtr->tag);
		
		S += 7*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_static_platform_data));
	return S;
}


inline void StreamToEndpointOwner(uint8* &S, endpoint_owner_data& Object)
{
	StreamToValue(S,Object.first_polygon_index);
	StreamToValue(S,Object.polygon_index_count);
	StreamToValue(S,Object.first_line_index);
	StreamToValue(S,Object.line_index_count);
}

inline void EndpointOwnerToStream(uint8* &S, endpoint_owner_data& Object)
{
	ValueToStream(S,Object.first_polygon_index);
	ValueToStream(S,Object.polygon_index_count);
	ValueToStream(S,Object.first_line_index);
	ValueToStream(S,Object.line_index_count);
}


uint8 *unpack_platform_data(uint8 *Stream, platform_data* Objects, size_t Count)
{
	uint8* S = Stream;
	platform_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->static_flags);
		StreamToValue(S,ObjPtr->speed);
		StreamToValue(S,ObjPtr->delay);
		StreamToValue(S,ObjPtr->minimum_floor_height);
		StreamToValue(S,ObjPtr->maximum_floor_height);
		StreamToValue(S,ObjPtr->minimum_ceiling_height);
		StreamToValue(S,ObjPtr->maximum_ceiling_height);
		
		StreamToValue(S,ObjPtr->polygon_index);
		StreamToValue(S,ObjPtr->dynamic_flags);
		StreamToValue(S,ObjPtr->floor_height);
		StreamToValue(S,ObjPtr->ceiling_height);
		StreamToValue(S,ObjPtr->ticks_until_restart);
		
		for (int k=0; k<MAXIMUM_VERTICES_PER_POLYGON; k++)
			StreamToEndpointOwner(S,ObjPtr->endpoint_owners[k]);
		
		StreamToValue(S,ObjPtr->parent_platform_index);
		
		StreamToValue(S,ObjPtr->tag);
		
		S += 22*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_platform_data));
	return S;
}

uint8 *pack_platform_data(uint8 *Stream, platform_data* Objects, size_t Count)
{
	uint8* S = Stream;
	platform_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->static_flags);
		ValueToStream(S,ObjPtr->speed);
		ValueToStream(S,ObjPtr->delay);
		ValueToStream(S,ObjPtr->minimum_floor_height);
		ValueToStream(S,ObjPtr->maximum_floor_height);
		ValueToStream(S,ObjPtr->minimum_ceiling_height);
		ValueToStream(S,ObjPtr->maximum_ceiling_height);
		
		ValueToStream(S,ObjPtr->polygon_index);
		ValueToStream(S,ObjPtr->dynamic_flags);
		ValueToStream(S,ObjPtr->floor_height);
		ValueToStream(S,ObjPtr->ceiling_height);
		ValueToStream(S,ObjPtr->ticks_until_restart);
		
		for (int k=0; k<MAXIMUM_VERTICES_PER_POLYGON; k++)
			EndpointOwnerToStream(S,ObjPtr->endpoint_owners[k]);
		
		ValueToStream(S,ObjPtr->parent_platform_index);
		
		ValueToStream(S,ObjPtr->tag);
		
		S += 22*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_platform_data));
	return S;
}

struct platform_definition *original_platform_definitions = NULL;

void reset_mml_platforms()
{
	if (original_platform_definitions) {
		for (int i = 0; i < NUMBER_OF_PLATFORM_TYPES; i++)
			platform_definitions[i] = original_platform_definitions[i];
		free(original_platform_definitions);
		original_platform_definitions = NULL;
	}
}

void parse_mml_platforms(const InfoTree& root)
{
	// back up old values first
	if (!original_platform_definitions) {
		original_platform_definitions = (struct platform_definition *) malloc(sizeof(struct platform_definition) * NUMBER_OF_PLATFORM_TYPES);
		assert(original_platform_definitions);
		for (int i = 0; i < NUMBER_OF_PLATFORM_TYPES; i++)
			original_platform_definitions[i] = platform_definitions[i];
	}
	
	BOOST_FOREACH(InfoTree ptree, root.children_named("platform"))
	{
		int16 index;
		if (!ptree.read_indexed("index", index, NUMBER_OF_PLATFORM_TYPES))
			continue;
		platform_definition& def = platform_definitions[index];
		
		ptree.read_indexed("start_extend", def.starting_extension, SHRT_MAX+1, true);
		ptree.read_indexed("start_contract", def.starting_contraction, SHRT_MAX+1, true);
		ptree.read_indexed("stop_extend", def.stopping_extension, SHRT_MAX+1, true);
		ptree.read_indexed("stop_contract", def.stopping_contraction, SHRT_MAX+1, true);
		ptree.read_indexed("obstructed", def.obstructed_sound, SHRT_MAX+1, true);
		ptree.read_indexed("uncontrollable", def.uncontrollable_sound, SHRT_MAX+1, true);
		ptree.read_indexed("moving", def.moving_sound, SHRT_MAX+1, true);
		ptree.read_indexed("item", def.key_item_index, NUMBER_OF_DEFINED_ITEMS, true);
		
		BOOST_FOREACH(InfoTree dmg, ptree.children_named("damage"))
		{
			dmg.read_damage(def.damage);
		}
	}
}

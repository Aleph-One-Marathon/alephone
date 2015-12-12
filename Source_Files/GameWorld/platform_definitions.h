#ifndef __PLATFORM_DEFINITIONS_H
#define __PLATFORM_DEFINITIONS_H

/*
PLATFORM_DEFINITIONS.H

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

Monday, June 26, 1995 9:43:22 AM  (Jason)
*/

#include "platforms.h"
#include "SoundManagerEnums.h"

/* ---------- constants */

enum /* sound codes for play_platform_sound() */
{
	_stopping_sound,
	_starting_sound,
	_obstructed_sound,
	_uncontrollable_sound
};

/* ---------- structures */

struct platform_definition
{
	/* sounds; specific sounds are played if they can be (i.e., ...at_bottom) otherwise the
		general sound is played */
	int16 starting_extension, starting_contraction;
	int16 stopping_extension, stopping_contraction;
	int16 obstructed_sound, uncontrollable_sound;
	int16 moving_sound;
	
	int16 key_item_index;
	
	struct static_platform_data defaults;
	
	struct damage_definition damage;
};

/* ---------- definitions */

struct platform_definition platform_definitions[NUMBER_OF_PLATFORM_TYPES]=
{
	{ // _platform_is_spht_door
		_snd_spht_door_opening, _snd_spht_door_closing,
		NONE, NONE,
		_snd_spht_door_obstructed, _snd_spht_door_obstructed,
		_ambient_snd_spht_door,

		NONE, /* key item index */
		
		{ /* defaults */
			_platform_is_spht_door, _fast_platform, _very_long_delay_platform, NONE, NONE,
				FLAG(_platform_deactivates_at_initial_level) | FLAG(_platform_extends_floor_to_ceiling) |
				FLAG(_platform_is_player_controllable) | FLAG(_platform_is_monster_controllable) |
				FLAG(_platform_reverses_direction_when_obstructed) | FLAG(_platform_is_initially_extended) |
				FLAG(_platform_comes_from_ceiling) | FLAG(_platform_is_door)
		},
		
		{_damage_crushing, 0, 30, 10, FIXED_ONE} /* damage, if necessary */
	},
	
	{ // _platform_is_split_spht_door
		_snd_spht_door_opening, _snd_spht_door_closing,
		NONE, NONE,
		_snd_spht_door_obstructed, _snd_spht_door_obstructed,
		_ambient_snd_spht_door,
		
		NONE, /* key item index */

		{ /* defaults */
			_platform_is_spht_split_door, _slow_platform, _very_long_delay_platform, NONE, NONE,
				FLAG(_platform_deactivates_at_initial_level) | FLAG(_platform_extends_floor_to_ceiling) |
				FLAG(_platform_is_player_controllable) | FLAG(_platform_is_monster_controllable) |
				FLAG(_platform_reverses_direction_when_obstructed) | FLAG(_platform_comes_from_floor) |
				FLAG(_platform_comes_from_ceiling) | FLAG(_platform_is_initially_extended) |
				FLAG(_platform_is_door)
		},
		
		{_damage_crushing, 0, 5, 4, FIXED_ONE}
	},
	
	{ // _platform_is_locked_spht_door
		_snd_spht_door_opening, _snd_spht_door_closing,
		NONE, NONE,
		_snd_spht_door_obstructed, _snd_spht_door_obstructed,
		_ambient_snd_spht_door,
		
		NONE, /* key item index */

		{ /* defaults */
			_platform_is_locked_spht_door, _slow_platform, _very_long_delay_platform, NONE, NONE,
				FLAG(_platform_deactivates_at_initial_level) | FLAG(_platform_extends_floor_to_ceiling) |
				FLAG(_platform_is_player_controllable) | FLAG(_platform_is_monster_controllable) |
				FLAG(_platform_reverses_direction_when_obstructed) | FLAG(_platform_comes_from_floor) |
				FLAG(_platform_comes_from_ceiling) | FLAG(_platform_is_initially_extended) |
				FLAG(_platform_is_door)
		},
		
		{_damage_crushing, 0, 5, 4, FIXED_ONE}
	},
	
	{ // _platform_is_spht_platform
		NONE, NONE,
		NONE, NONE,
		NONE, NONE,
		NONE,
		
		NONE, /* key item index */
		
		{ /* defaults */
			_platform_is_spht_platform, _slow_platform, _long_delay_platform, NONE, NONE,
				FLAG(_platform_is_initially_active) | FLAG(_platform_is_initially_extended) | FLAG(_platform_comes_from_floor) |
				FLAG(_platform_reverses_direction_when_obstructed)
		},
		
		{_damage_crushing, 0, 30, 10, FIXED_ONE} /* damage, if necessary */
	},
	
	{ // _platform_is_noisy_spht_platform
		_snd_spht_platform_starting, _snd_spht_platform_starting,
		_snd_spht_platform_stopping, _snd_spht_platform_stopping,
		_snd_spht_platform_stopping, NONE,
		_ambient_snd_spht_platform,
		
		NONE, /* key item index */
		
		{ /* defaults */
			_platform_is_noisy_spht_platform, _slow_platform, _long_delay_platform, NONE, NONE,
				FLAG(_platform_is_initially_active) | FLAG(_platform_is_initially_extended) | FLAG(_platform_comes_from_floor) |
				FLAG(_platform_reverses_direction_when_obstructed)
		},
		
		{_damage_crushing, 0, 30, 10, FIXED_ONE} /* damage, if necessary */
	},
	
	{ // _platform_is_heavy_spht_door
		_snd_heavy_spht_door_closing, _snd_heavy_spht_door_opening,
		_snd_heavy_spht_door_closed, _snd_heavy_spht_door_open,
		_snd_heavy_spht_door_obstructed, _snd_heavy_spht_door_obstructed,
		_ambient_snd_heavy_spht_door,
		
		NONE, /* key item index */

		{ /* defaults */
			_platform_is_heavy_spht_door, _slow_platform, _very_long_delay_platform, NONE, NONE,
				FLAG(_platform_deactivates_at_initial_level) | FLAG(_platform_extends_floor_to_ceiling) |
				FLAG(_platform_is_player_controllable) | FLAG(_platform_is_monster_controllable) |
				FLAG(_platform_reverses_direction_when_obstructed) | FLAG(_platform_comes_from_ceiling) |
				FLAG(_platform_is_initially_extended) | FLAG(_platform_is_door)
		},
		
		{_damage_crushing, 0, 5, 4, FIXED_ONE}
	},
	
	{ // pfhor door
		_snd_pfhor_door_opening, _snd_pfhor_door_closing,
		NONE, NONE,
		_snd_pfhor_door_obstructed, _snd_pfhor_door_obstructed,
		_ambient_snd_pfhor_door,

		NONE, /* key item index */
		
		{ /* defaults */
			_platform_is_pfhor_door, _fast_platform, _very_long_delay_platform, NONE, NONE,
				FLAG(_platform_deactivates_at_initial_level) | FLAG(_platform_extends_floor_to_ceiling) |
				FLAG(_platform_is_player_controllable) | FLAG(_platform_is_monster_controllable) |
				FLAG(_platform_reverses_direction_when_obstructed) | FLAG(_platform_is_initially_extended) |
				FLAG(_platform_comes_from_ceiling) | FLAG(_platform_is_door)
		},
		
		{_damage_crushing, 0, 30, 10, FIXED_ONE} /* damage, if necessary */
	},
	
	{ // _platform_is_heavy_spht_platform
		_snd_heavy_spht_platform_starting, _snd_heavy_spht_platform_starting,
		_snd_heavy_spht_platform_stopping, _snd_heavy_spht_platform_stopping,
		_snd_heavy_spht_platform_stopping, NONE,
		_ambient_snd_heavy_spht_platform,
		
		NONE, /* key item index */

		{ /* defaults */
			_platform_is_heavy_spht_platform, _slow_platform, _long_delay_platform, NONE, NONE,
				FLAG(_platform_is_initially_active) | FLAG(_platform_is_initially_extended) |
				FLAG(_platform_comes_from_floor) | FLAG(_platform_reverses_direction_when_obstructed),
		},
		
		{_damage_crushing, 0, 5, 4, FIXED_ONE}
	},

	{ // pfhor platform
		_snd_pfhor_platform_starting, _snd_pfhor_platform_starting,
		_snd_pfhor_platform_stopping, _snd_pfhor_platform_stopping,
		_snd_pfhor_platform_stopping, NONE,
		_ambient_snd_pfhor_platform,
		
		NONE, /* key item index */
		
		{ /* defaults */
			_platform_is_pfhor_platform, _slow_platform, _long_delay_platform, NONE, NONE,
				FLAG(_platform_is_initially_active) | FLAG(_platform_is_initially_extended) | FLAG(_platform_comes_from_floor) |
				FLAG(_platform_reverses_direction_when_obstructed)
		},
		
		{_damage_crushing, 0, 30, 10, FIXED_ONE} /* damage, if necessary */
	},
};

#endif


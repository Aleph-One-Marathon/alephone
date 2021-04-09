#ifndef __ITEM_DEFINITIONS_H
#define __ITEM_DEFINITIONS_H

/*
ITEM_DEFINITIONS.H

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

Saturday, October 22, 1994 3:02:32 PM

Feb 4, 2000 (Loren Petrich):
	Added SMG and its ammo

*/

#include "items.h"
#include "map.h"

/* ---------- structures */

struct item_definition
{
	int16 item_kind;
	int16 singular_name_id;
	int16 plural_name_id;
	shape_descriptor base_shape;
	int16 maximum_count_per_player;
	int16 invalid_environments;

	// extension to support per-difficulty maximums
	bool has_extended_maximum_count[NUMBER_OF_GAME_DIFFICULTY_LEVELS];
	int16 extended_maximum_count[NUMBER_OF_GAME_DIFFICULTY_LEVELS];

	int16 get_maximum_count_per_player(bool is_m1, int difficulty_level) const;
};


// So as not to repeat in script_instructions.cpp (Pfhortran)
#ifndef DONT_REPEAT_DEFINITIONS

/* ---------- globals */

static struct item_definition item_definitions[]=
{
	/* Knife */
	{_weapon, 0, 0, UNONE, 1, 0},

	// pistol and ammo
	{_weapon, 1, 2, BUILD_DESCRIPTOR(_collection_items, 0), 2, 0},
	{_ammunition, 3, 4, BUILD_DESCRIPTOR(_collection_items, 3), 50, 0},

	// fusion pistol and ammo
	{_weapon, 5, 5, BUILD_DESCRIPTOR(_collection_items, 1), 1, 0},
	{_ammunition, 6, 7, BUILD_DESCRIPTOR(_collection_items, 4), 25, 0},

	// assault rifle, bullets and grenades
	{_weapon, 8, 8, BUILD_DESCRIPTOR(_collection_items, 2), 1, _environment_vacuum},
	{_ammunition, 9, 10, BUILD_DESCRIPTOR(_collection_items, 5), 15, _environment_vacuum},
	{_ammunition, 11, 12, BUILD_DESCRIPTOR(_collection_items, 6), 8, _environment_vacuum},

	// rocket launcher and ammo
	{_weapon, 13, 13, BUILD_DESCRIPTOR(_collection_items, 12), 1, _environment_vacuum},
	{_ammunition, 14, 15, BUILD_DESCRIPTOR(_collection_items, 7), 4, _environment_vacuum},
	
	// invisibility, invincibility, invfravision
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 8), 1, 0},
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 9), 1, 0}, 
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 14), 1, 0}, 

	// alien weapon and ammunition
	{_weapon, 16, 16, BUILD_DESCRIPTOR(_collection_items, 13), 1, 0}, 
	{_ammunition, 17, 18, UNONE, 999, 0}, 
	
	// flamethrower and ammo
	{_weapon, 19, 19, BUILD_DESCRIPTOR(_collection_items, 10), 1, _environment_vacuum}, 
	{_ammunition, 20, 21, BUILD_DESCRIPTOR(_collection_items, 11), 3, _environment_vacuum},
	
	/* extravision powerup */
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 15), 1, 0},
	
	// energy and oxygen recharges
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 23), 1, 0}, /* oxygen recharge */
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 20), 1, 0}, /* x1 recharge */
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 21), 1, 0}, /* x2 recharge */
	{_powerup, NONE, NONE, BUILD_DESCRIPTOR(_collection_items, 22), 1, 0}, /* x3 recharge */
	
	// shotgun and ammo
	{_weapon, 27, 28, BUILD_DESCRIPTOR(_collection_items, 18), 2, 0}, 
	{_ammunition, 17, 18, BUILD_DESCRIPTOR(_collection_items, 19), 80, 0},
	
	// _i_spht_door_key, _i_uplink_chip
	{_item, 29, 30, BUILD_DESCRIPTOR(_collection_items, 17), 8, 0},
	{_item, 31, 32, BUILD_DESCRIPTOR(_collection_items, 16), 1, 0},

	// Net game balls.
	{_ball, 33, 33, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 0), 29), 1, _environment_single_player},
	{_ball, 34, 34, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 1), 29), 1, _environment_single_player},
	{_ball, 35, 35, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 2), 29), 1, _environment_single_player},
	{_ball, 36, 36, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 3), 29), 1, _environment_single_player},
	{_ball, 37, 37, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 4), 29), 1, _environment_single_player},
	{_ball, 38, 38, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 5), 29), 1, _environment_single_player},
	{_ball, 39, 39, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 6), 29), 1, _environment_single_player},
	{_ball, 40, 40, BUILD_DESCRIPTOR(BUILD_COLLECTION(_collection_player, 7), 29), 1, _environment_single_player},

	// LP addition: smg and ammo
	{_weapon, 41, 41, BUILD_DESCRIPTOR(_collection_items, 25), 1, 0},
	{_ammunition, 42, 43, BUILD_DESCRIPTOR(_collection_items, 24), 8, 0},
};

#endif

#endif


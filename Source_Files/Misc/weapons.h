#ifndef __WEAPONS_H
#define __WEAPONS_H

/*

	weapons2.h
	Saturday, May 13, 1995 4:41:42 PM- rdm created.

Feb 4, 2000 (Loren Petrich):
	Added SMG stuff

Feb 6, 2000 (Loren Petrich):
	Added access to size of weapon-definition structure and to the number of weapon types

May 26, 2000 (Loren Petrich):
	Added XML support for configuring various weapon features

Aug 31, 2000 (Loren Petrich):
	Added stuff for unpacking and packing
*/

/* enums for player.c */
enum { /* Weapons */
	_weapon_fist,
	_weapon_pistol,
	_weapon_plasma_pistol,
	_weapon_assault_rifle,
	_weapon_missile_launcher,
	_weapon_flamethrower,
	_weapon_alien_shotgun,
	_weapon_shotgun,
	_weapon_ball, // or something
	// LP addition:
	_weapon_smg,
	MAXIMUM_NUMBER_OF_WEAPONS,
	
	_weapon_doublefisted_pistols= MAXIMUM_NUMBER_OF_WEAPONS, /* This is a pseudo-weapon */
	_weapon_doublefisted_shotguns,
	PLAYER_TORSO_SHAPE_COUNT
};

enum {
	_shape_weapon_idle,
	_shape_weapon_charging,
	_shape_weapon_firing
};

enum {
	_primary_weapon,
	_secondary_weapon,
	NUMBER_OF_TRIGGERS
};

enum /* weapon display positioning modes */
{
	_position_low, /* position==0 is invisible, position==FIXED_ONE is sticking out from left/bottom */
	_position_center, /* position==0 is off left/bottom, position==FIXED_ONE is off top/right */
	_position_high /* position==0 is invisible, position==FIXED_ONE is sticking out from right/top
		(mirrored, whether you like it or not) */
};

/* ----------------- structures */

struct weapon_display_information
{
	short collection, low_level_shape_index;
	
	fixed vertical_position, horizontal_position;
	short vertical_positioning_mode, horizontal_positioning_mode;
	short transfer_mode;
	fixed transfer_phase;
	
	boolean flip_horizontal, flip_vertical;
};

// For external access:
const int SIZEOF_weapon_definition = 134;

const int SIZEOF_player_weapon_data = 472;

/* ----------------- prototypes */
/* called once at startup */
void initialize_weapon_manager(void);

/* Initialize the weapons for a completely new game. */
void initialize_player_weapons_for_new_game(short player_index);

/* initialize the given players weapons-> called after creating a player */
void initialize_player_weapons(short player_index);

// Old external-access stuff: superseded by the packing and unpacking routines below
void *get_weapon_array(void);
long calculate_weapon_array_length(void);

/* while this returns true, keep calling.. */
boolean get_weapon_display_information(short *count, 
	struct weapon_display_information *data);

/* When the player runs over an item, check for reloads, etc. */
void process_new_item_for_reloading(short player_index, short item_type);

/* Update the given player's weapons */
void update_player_weapons(short player_index, long action_flags);

/* Mark the weapon collections for loading or unloading.. */
void mark_weapon_collections(boolean loading);

/* Called when a player dies to discharge the weapons that they have charged up. */
void discharge_charged_weapons(short player_index);

/* Called on entry to a level, and will change weapons if this one doesn't work */
/*  in the given environment. */
void check_player_weapons_for_environment_change(void);

/* Tell me when one of my projectiles hits, and return the weapon_identifier I passed */
/*  to new_projectile... */
void player_hit_target(short player_index, short weapon_identifier);

/* for drawing the player */
void get_player_weapon_mode_and_type(short player_index, short *shape_weapon_type,
	short *shape_mode);

/* For the game window to update properly */
short get_player_desired_weapon(short player_index);

/* This is pinned to the maximum I think I can hold.. */
short get_player_weapon_ammo_count(short player_index, short which_weapon, short which_trigger);

#ifdef DEBUG
void debug_print_weapon_status(void);
#endif

// LP: to pack and unpack this data;
// these do not make the definitions visible to the outside world

uint8 *unpack_player_weapon_data(uint8 *Stream, int Count);
uint8 *pack_player_weapon_data(uint8 *Stream, int Count);
uint8 *unpack_weapon_definition(uint8 *Stream, int Count);
uint8 *pack_weapon_definition(uint8 *Stream, int Count);

// LP additions: get number of weapon types;
int get_number_of_weapon_types();

// LP addition: XML-parser support
XML_ElementParser *Weapons_GetParser();

#endif


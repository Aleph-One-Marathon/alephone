#ifndef __PLAYER_H
#define __PLAYER_H

/*
PLAYER.H

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

Sunday, July 10, 1994 10:07:21 PM

Feb 6, 2000 (Loren Petrich):
	Added access to size of physics-definition structure
	and to the number of physics models (restricted sense: player physics)

Feb 18, 2000 (Loren Petrich):
	Added support for a chase cam

Feb 25, 2000 (Loren Petrich):
	Made it possible to switch viewing sides with the chase cam

Feb 26, 2000 (Loren Petrich):	
	Added chase-cam reset feature, for the purpose of doing chase-cam inertia.
	The reset is necessary to take into account teleporting or entering a level.

Mar 2, 2000 (Loren Petrich):
	Moved the chase-cam stuff into ChaseCam.c/h

May 14, 2000 (Loren Petrich):
	Added XML support for configuring various player features

May 23, 2000 (Loren Petrich):
	Added XML configuration of self-luminosity

July 1, 2000 (Loren Petrich):
	Made player-data accessor inline; added map.h to define some stuff for it

Aug 31, 2000 (Loren Petrich):
	Added stuff for unpacking and packing
        
Oct 21, 2001 (Woody Zenfell):
        Moved some info from player.cpp to here, so can be shared (in particular, with SDL net dialog widgets)

Feb 20, 2002 (Woody Zenfell):
    Support for multiple sets of action queues (including gRealActionQueues)

May 20, 2002 (Woody Zenfell):
    Can now find out how long since local player used a terminal
*/

// LP additions: stuff that this file needs
#include "cseries.h"
#include "world.h"
#include "map.h"
// ZZZ addition: same deal
#include "weapons.h"

/* ---------- constants */

#ifdef DEMO
#define MAXIMUM_NUMBER_OF_PLAYERS 2
#else
#define MAXIMUM_NUMBER_OF_PLAYERS 8
#endif

enum
{
	NUMBER_OF_ITEMS= 64
};

// All the player values settable by MML put into one struct for easier management.
struct player_settings_definition {
	// LP additions: variables for initial energy, initial oxygen, and stripped energy:
	short InitialEnergy;
	short InitialOxygen;
	short StrippedEnergy;
	// For picked-up powerups
	short SingleEnergy;
	short DoubleEnergy;
	short TripleEnergy;
	// LP addition: self-luminosity
	_fixed PlayerSelfLuminosity;
	// LP: can one swim?
	bool CanSwim;
	// Used in weapons.cpp: player can have guided missiles
	bool PlayerShotsGuided;
	short PlayerHalfVisualArc;
	short PlayerHalfVertVisualArc;
	float PlayerVisualRange;
	float PlayerDarkVisualRange;
	// LP additions: oxygen depletion and replenishment rates
	// (number of units per tick);
	// oxygen change is set equal to depletion or replenishment,
	// whichever one is appropriate for the environment (vacuum/liquid vs. normal air)
	short OxygenDepletion;
	short OxygenReplenishment;
	short OxygenChange;
	// LP addition: invincibility-powerup vulnerability
	short Vulnerability;
};

extern struct player_settings_definition player_settings;
#define NATURAL_LIGHT_INTENSITY player_settings.PlayerSelfLuminosity

enum /* physics models */
{
	_editor_model,
	_earth_gravity_model,
	_low_gravity_model /* cyberspace? */
};

enum /* player actions; irrelevant if the player is dying or something */
{
	_player_stationary,
	_player_walking,
	_player_running,
	_player_sliding,
	_player_airborne,
	NUMBER_OF_PLAYER_ACTIONS
};

enum /* team colors */
{
	_violet_team,
	_red_team,
	_tan_team,
	_light_blue_team,
	_yellow_team,
	_brown_team,
	_blue_team,
	_green_team,
	NUMBER_OF_TEAM_COLORS
};

enum /* stringset that holds the names of the above colors */
{
	kTeamColorsStringSetID	= 152 // matches STR# for colors in original Marathon (m2 and inf moved it to a menu)
};

// Is here for script_instructions.cpp
// ZZZ: increasing this queue size so machines (esp. in netgames) are even more tolerant
// of abnormalities (can probably reduce it back once the current biggest source of stalled
// update_world() calls - OpenGL texture setup in PreloadTextures() etc. - comes before NetSync().)
//#define ACTION_QUEUE_BUFFER_DIAMETER 0x100
#define ACTION_QUEUE_BUFFER_DIAMETER 0x400

/* ---------- action flags */

#define ABSOLUTE_YAW_BITS 7
#define MAXIMUM_ABSOLUTE_YAW (1<<ABSOLUTE_YAW_BITS)
#define GET_ABSOLUTE_YAW(i) (((i)>>(_absolute_yaw_mode_bit+1))&(MAXIMUM_ABSOLUTE_YAW-1))
#define SET_ABSOLUTE_YAW(i,y) (((i)&~((MAXIMUM_ABSOLUTE_YAW-1)<<(_absolute_yaw_mode_bit+1))) | ((y)<<(_absolute_yaw_mode_bit+1)))

#define ABSOLUTE_PITCH_BITS 5
#define MAXIMUM_ABSOLUTE_PITCH (1<<ABSOLUTE_PITCH_BITS)
#define GET_ABSOLUTE_PITCH(i) (((i)>>(_absolute_pitch_mode_bit+1))&(MAXIMUM_ABSOLUTE_PITCH-1))
#define SET_ABSOLUTE_PITCH(i,y) (((i)&~((MAXIMUM_ABSOLUTE_PITCH-1)<<(_absolute_pitch_mode_bit+1))) | ((y)<<(_absolute_pitch_mode_bit+1)))

#define ABSOLUTE_POSITION_BITS 7
#define MAXIMUM_ABSOLUTE_POSITION (1<<ABSOLUTE_POSITION_BITS)
#define GET_ABSOLUTE_POSITION(i) (((i)>>(_absolute_position_mode_bit+1))&(MAXIMUM_ABSOLUTE_POSITION-1))
#define SET_ABSOLUTE_POSITION(i,y) (((i)&~((MAXIMUM_ABSOLUTE_POSITION-1)<<(_absolute_position_mode_bit+1)))|((y)<<(_absolute_position_mode_bit+1)))

enum /* action flag bit offsets */
{
	_absolute_yaw_mode_bit,
	_turning_left_bit,
	_turning_right_bit,
	_sidestep_dont_turn_bit,
	_looking_left_bit,
	_looking_right_bit,
	_absolute_yaw_bit0,
	_absolute_yaw_bit1,
 
	_absolute_pitch_mode_bit,
	_looking_up_bit,
	_looking_down_bit,
	_looking_center_bit,
	_absolute_pitch_bit0,
	_absolute_pitch_bit1,

	_absolute_position_mode_bit,
	_moving_forward_bit,
	_moving_backward_bit,
	_run_dont_walk_bit,
	_look_dont_turn_bit,
	_absolute_position_bit0,
	_absolute_position_bit1,
	_absolute_position_bit2,

	_sidestepping_left_bit,
	_sidestepping_right_bit,
	_left_trigger_state_bit,
	_right_trigger_state_bit,
	_action_trigger_state_bit,
	_cycle_weapons_forward_bit,
	_cycle_weapons_backward_bit,
	_toggle_map_bit,
	_microphone_button_bit,
	_swim_bit,
	
	NUMBER_OF_ACTION_FLAG_BITS /* should be <=32 */
};

#define _override_absolute_yaw (_turning_left|_turning_right|_looking_left|_looking_right)
#define _override_absolute_pitch (_looking_up|_looking_down|_looking_center)
#define _override_absolute_position (_moving_forward|_moving_backward)

enum /* action_flags */
{
	_absolute_yaw_mode= 1<<_absolute_yaw_mode_bit,
	_turning_left= 1<<_turning_left_bit,
	_turning_right= 1<<_turning_right_bit,
	_sidestep_dont_turn= 1<<_sidestep_dont_turn_bit,
	_looking_left= 1<<_looking_left_bit,
	_looking_right= 1<<_looking_right_bit,

	_absolute_pitch_mode= 1<<_absolute_pitch_mode_bit,
	_looking_up= 1<<_looking_up_bit,
	_looking_down= 1<<_looking_down_bit,
	_looking_center= 1<<_looking_center_bit,
	_look_dont_turn= 1<<_look_dont_turn_bit,

	_absolute_position_mode= 1<<_absolute_position_mode_bit,
	_moving_forward= 1<<_moving_forward_bit,
	_moving_backward= 1<<_moving_backward_bit,
	_run_dont_walk= 1<<_run_dont_walk_bit,

	_sidestepping_left= 1<<_sidestepping_left_bit,
	_sidestepping_right= 1<<_sidestepping_right_bit,
	_left_trigger_state= 1<<_left_trigger_state_bit,
	_right_trigger_state= 1<<_right_trigger_state_bit,
	_action_trigger_state= 1<<_action_trigger_state_bit,
	_cycle_weapons_forward= 1<<_cycle_weapons_forward_bit,
	_cycle_weapons_backward= 1<<_cycle_weapons_backward_bit,
	_toggle_map= 1<<_toggle_map_bit,
	_microphone_button= 1<<_microphone_button_bit,
	_swim= 1<<_swim_bit,

	_turning= _turning_left|_turning_right,
	_looking= _looking_left|_looking_right,
	_moving= _moving_forward|_moving_backward,
	_sidestepping= _sidestepping_left|_sidestepping_right,
	_looking_vertically= _looking_up|_looking_down|_looking_center
};

/* ---------- structures */

enum /* player flag bits */
{
	_RECENTERING_BIT= 0x8000,
	_ABOVE_GROUND_BIT= 0x4000,
	_BELOW_GROUND_BIT= 0x2000,
	_FEET_BELOW_MEDIA_BIT= 0x1000,
	_HEAD_BELOW_MEDIA_BIT= 0x0800,
	_STEP_PERIOD_BIT= 0x0400
};

struct physics_variables
{
	_fixed head_direction;
	_fixed last_direction, direction, elevation, angular_velocity, vertical_angular_velocity;
	_fixed velocity, perpendicular_velocity; /* in and perpendicular to direction, respectively */
	fixed_point3d last_position, position;
	_fixed actual_height;

	/* used by mask_in_absolute_positioning_information (because it is not really absolute) to
		keep track of where weÕre going */
	_fixed adjusted_pitch, adjusted_yaw;
	
	fixed_vector3d external_velocity; /* from impacts; slowly absorbed */
	_fixed external_angular_velocity; /* from impacts; slowly absorbed */
	
	_fixed step_phase; /* step_phase is in [0,1) and is some function of the distance travelled
		(for bobbing the gun and the viewpoint) */
	_fixed step_amplitude; /* step amplitude is in [0,1) and is some function of velocity */
	
	_fixed floor_height; /* the height of the floor on the polygon where we ended up last time */
	_fixed ceiling_height; /* same as above, but ceiling height */
	_fixed media_height; /* media height */

	int16 action; /* what the playerÕs legs are doing, basically */
	uint16 old_flags, flags; /* stuff like _RECENTERING */
};

enum { /* Player flags */
	_player_doesnt_auto_switch_weapons_flag= 0x0080,
	_player_is_interlevel_teleporting_flag= 0x0100,
	_player_has_cheated_flag= 0x0200,
	_player_is_teleporting_flag= 0x0400,	
	_player_has_map_open_flag= 0x0800,	
	_player_is_totally_dead_flag= 0x1000,
	_player_is_zombie_flag= 0x2000, // IS THIS USED??
	_player_is_dead_flag= 0x4000,
	_player_is_pfhortran_controlled_flag= 0x8000
};

#define PLAYER_PERSISTANT_FLAGS (_player_has_cheated_flag | _player_doesnt_auto_switch_weapons_flag)

#define PLAYER_IS_DEAD(p) ((p)->flags&_player_is_dead_flag)
#define SET_PLAYER_DEAD_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_is_dead_flag):((p)->flags&=(uint16)~_player_is_dead_flag)))

#define PLAYER_IS_ZOMBIE(p) ((p)->flags&_player_is_zombie_flag)
#define SET_PLAYER_ZOMBIE_STATUS(p,v) ((v)?((p)->flags|=(uint16)_player_is_zombie_flag):((p)->flags&=(uint16)~_player_is_zombie_flag))

#define PLAYER_IS_PFHORTRAN_CONTROLLED(p) ( ( p )->flags & _player_is_pfhortran_controlled_flag )
#define SET_PLAYER_IS_PFHORTRAN_CONTROLLED_STATUS(p,v) ((v)?((p)->flags|=(uint16)_player_is_pfhortran_controlled_flag):((p)->flags&=(uint16)~_player_is_pfhortran_controlled_flag))

/* i.e., our animation has stopped */
#define PLAYER_IS_TOTALLY_DEAD(p) ((p)->flags&_player_is_totally_dead_flag)
#define SET_PLAYER_TOTALLY_DEAD_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_is_totally_dead_flag):((p)->flags&=(uint16)~_player_is_totally_dead_flag)))

#define PLAYER_HAS_MAP_OPEN(p) ( (p)->flags & _player_has_map_open_flag )
#define SET_PLAYER_MAP_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_has_map_open_flag):((p)->flags&=(uint16)~_player_has_map_open_flag)))

#define PLAYER_IS_TELEPORTING(p) ((p)->flags&_player_is_teleporting_flag)
#define SET_PLAYER_TELEPORTING_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_is_teleporting_flag):((p)->flags&=(uint16)~_player_is_teleporting_flag)))

#define PLAYER_IS_INTERLEVEL_TELEPORTING(p) ((p)->flags&_player_is_interlevel_teleporting_flag)
#define SET_PLAYER_INTERLEVEL_TELEPORTING_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_is_interlevel_teleporting_flag):((p)->flags&=(uint16)~_player_is_interlevel_teleporting_flag)))

#define PLAYER_HAS_CHEATED(p) ((p)->flags&_player_has_cheated_flag)
#define SET_PLAYER_HAS_CHEATED(p) ((p)->flags|=(uint16)_player_has_cheated_flag)

#define PLAYER_DOESNT_AUTO_SWITCH_WEAPONS(p) ((p)->flags&_player_doesnt_auto_switch_weapons_flag)
#define SET_PLAYER_DOESNT_AUTO_SWITCH_WEAPONS_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)_player_doesnt_auto_switch_weapons_flag):((p)->flags&=(uint16)~_player_doesnt_auto_switch_weapons_flag)))

#define PLAYER_MAXIMUM_SUIT_ENERGY (150)
#define PLAYER_MAXIMUM_SUIT_OXYGEN (6*TICKS_PER_MINUTE)

#define MAXIMUM_PLAYER_NAME_LENGTH 32

#define PLAYER_TELEPORTING_DURATION TELEPORTING_DURATION
#define PLAYER_TELEPORTING_MIDPOINT TELEPORTING_MIDPOINT

struct damage_record
{
	int32 damage;
	int16 kills;
};

struct player_data
{
	int16 identifier;
	int16 flags; // Player flags

	int16 color;
	int16 team;
	char name[MAXIMUM_PLAYER_NAME_LENGTH+1];
	
	/* shadowed from physics_variables structure below and the playerÕs object (read-only) */
	world_point3d location;
	world_point3d camera_location; // beginning of fake world_location3d structure
	int16 camera_polygon_index;
	angle facing, elevation;
	int16 supporting_polygon_index; /* what polygon is actually supporting our weight */
	int16 last_supporting_polygon_index;

	/* suit energy shadows vitality in the playerÕs monster slot */
	int16 suit_energy, suit_oxygen;
	
	int16 monster_index; /* this playerÕs entry in the monster list */
	int16 object_index; /* monster->object_index */
	
	/* Reset by initialize_player_weapons */
	int16 weapon_intensity_decay; /* zero is idle intensity */
	_fixed weapon_intensity;

	/* powerups */
	int16 invisibility_duration;
	int16 invincibility_duration;
	int16 infravision_duration;
	int16 extravision_duration;

	/* teleporting */
	int16 delay_before_teleport; /* This is only valid for interlevel teleports (teleporting_destination is a negative number) */
	int16 teleporting_phase; /* NONE means no teleporting, otherwise [0,TELEPORTING_PHASE) */
	int16 teleporting_destination; /* level number or NONE if intralevel transporter */
	int16 interlevel_teleport_phase; /* This is for the other players when someone else initiates the teleport */

	/* there is no state information associated with items; each item slot is only a count */
	int16 items[NUMBER_OF_ITEMS];

	/* Used by the game window code to keep track of the interface state. */
	int16 interface_flags;
	int16 interface_decay;

	struct physics_variables variables;

	struct damage_record total_damage_given;
	struct damage_record damage_taken[MAXIMUM_NUMBER_OF_PLAYERS];
	struct damage_record monster_damage_taken, monster_damage_given;

	int16 reincarnation_delay;

	int16 control_panel_side_index; // NONE, or the side index of a control panel the user is using that requires passage of time
	
	int32 ticks_at_last_successful_save;

	int32 netgame_parameters[2];

	bool	netdead;	// ZZZ: added this; it should not be serialized/deserialized

	world_distance step_height; // not serialized, used to correct chase cam bob
	uint8_t hotkey_sequence;    // not serialized, used to decode hotkey
	int16_t hotkey; 			// not serialized, used to store hotkey

	// ZZZ: since we don't put this structure directly into files or network communications,
	// there ought? to be no reason for the padding
//	int16 unused[256];
};

const int SIZEOF_player_data = 930;

const int SIZEOF_physics_constants = 104;

// ZZZ: moved here so we can get/use in files other than player.cpp
struct player_shape_definitions
{
	short collection;

	short dying_hard, dying_soft;
	short dead_hard, dead_soft;
	short legs[NUMBER_OF_PLAYER_ACTIONS]; /* stationary, walking, running, sliding, airborne */
	short torsos[PLAYER_TORSO_SHAPE_COUNT]; /* NONE, ..., double pistols */
	short charging_torsos[PLAYER_TORSO_SHAPE_COUNT]; /* NONE, ..., double pistols */
	short firing_torsos[PLAYER_TORSO_SHAPE_COUNT]; /* NONE, ..., double pistols */
};

// ghs: added these for Lua

#define MAXIMUM_ACTIVATION_RANGE (3*WORLD_ONE)

enum
{
	_target_is_platform,
	_target_is_control_panel,
	_target_is_unrecognized
};

short find_action_key_target(short player_index, world_distance range, short *target_type, bool perform_panel_actions);

/* ---------- globals */

extern struct player_data *players;
extern struct damage_record team_damage_given[NUMBER_OF_TEAM_COLORS];
extern struct damage_record team_damage_taken[NUMBER_OF_TEAM_COLORS];
extern struct damage_record team_monster_damage_taken[NUMBER_OF_TEAM_COLORS];
extern struct damage_record team_monster_damage_given[NUMBER_OF_TEAM_COLORS];
extern struct damage_record team_friendly_fire[NUMBER_OF_TEAM_COLORS];

/* use set_local_player_index() and set_current_player_index() to change these! */
extern short local_player_index, current_player_index;
extern struct player_data *local_player, *current_player;

// ZZZ: The set of "real" action queues; existing calls like queue_action_flags
// will be operations on the returned value.  Returned from a function to avoid
// accidental assignment to the pointer.
class ActionQueues;
class ModifiableActionQueues;
extern ActionQueues*    GetRealActionQueues();

/* ---------- prototypes/PLAYER.C */

void initialize_players(void);
void reset_action_queues(void);
void allocate_player_memory(void);

void set_local_player_index(short player_index);
void set_current_player_index(short player_index);

// Flags for new_player()
using new_player_flags = uint32;
constexpr new_player_flags
	new_player_make_local = 1u << 0,
	new_player_make_current = 1u << 1,
	new_player_make_local_and_current = new_player_make_local | new_player_make_current;

short new_player(short team, short color, short player_identifier, new_player_flags flags);
void delete_player(short player_number);

void recreate_players_for_new_level(void);

void team_damage_from_player_data(void);

// ZZZ: this now takes a set of ActionQueues as a parameter so the caller can redirect
// the update routine's input.  Also, now callers can request a 'predictive update',
// which changes less state, in an effort to make partial state saving/restoration successful.
void update_players(ActionQueues* inActionQueuesToUse, bool inPredictive); /* assumes ¶t==1 tick */
void decode_hotkeys(ModifiableActionQueues& action_queues);

// handle pausing Marathon 1 terminals
bool m1_solo_player_in_terminal();
void update_m1_solo_player_in_terminal(ActionQueues* inActionQueuesToUse);

void walk_player_list(void);

void damage_player(short monster_index, short aggressor_index, short aggressor_type,
	struct damage_definition *damage, short projectile_index);

void mark_player_collections(bool loading);

// ZZZ: new function to get current player_shape_definitions
player_shape_definitions* get_player_shape_definitions();

short player_identifier_to_player_index(short player_identifier);

player_data *get_player_data(
	const size_t player_index);

short monster_index_to_player_index(short monster_index);

short get_polygon_index_supporting_player(short player_index);

bool legal_player_powerup(short player_index, short item_index);
void process_player_powerup(short player_index, short item_index);

world_distance dead_player_minimum_polygon_height(short polygon_index);

bool try_and_subtract_player_item(short player_index, short item_type);

/* ---------- prototypes/PHYSICS.C */

void initialize_player_physics_variables(short player_index);
void update_player_physics_variables(short player_index, uint32 action_flags, bool predictive);

void adjust_player_for_polygon_height_change(short monster_index, short polygon_index, world_distance new_floor_height,
	world_distance new_ceiling_height);
void accelerate_player(short monster_index, world_distance vertical_velocity, angle direction, world_distance velocity);

void kill_player_physics_variables(short player_index);

void get_absolute_pitch_range(_fixed *minimum, _fixed *maximum);

_fixed get_player_forward_velocity_scale(short player_index);

// Delta from the low-precision physical aim to the virtual "true" aim implied by high-precision aiming input;
// |<yaw or pitch delta>| <= FIXED_ONE/2
fixed_yaw_pitch virtual_aim_delta();

fixed_yaw_pitch prev_virtual_aim_delta(); // for interpolation

// Resync the virtual aim to the current physical aim
void resync_virtual_aim();

// Update the virtual aim and return action flags updated with yaw/pitch deltas (if appropriate)
uint32 process_aim_input(uint32 action_flags, fixed_yaw_pitch delta);


// LP: to pack and unpack this data;
// these do not make the definitions visible to the outside world

uint8 *unpack_player_data(uint8 *Stream, player_data *Objects, size_t Count);
uint8 *pack_player_data(uint8 *Stream, player_data *Objects, size_t Count);
uint8 *unpack_physics_constants(uint8 *Stream, size_t Count);
uint8 *pack_physics_constants(uint8 *Stream, size_t Count);

// LP addition: get number of physics models (restricted sense)
size_t get_number_of_physics_models();

// ZZZ addition: get number of ticks (NOT measured carefully, beware)
// since local player was in terminal mode.
int get_ticks_since_local_player_in_terminal();

class InfoTree;
void parse_mml_player(const InfoTree& root);
void reset_mml_player();

#endif


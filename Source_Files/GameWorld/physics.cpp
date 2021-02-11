/*
PHYSICS.C

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

Wednesday, May 11, 1994 9:32:16 AM

Saturday, May 21, 1994 11:36:31 PM
	missing effects due to map (i.e., gravity and collision detection).  last day in san
	jose after WWDC.
Sunday, May 22, 1994 11:14:55 AM
	there are two viable methods of running a synchronized network game.  the first is doom's,
	where each player shares with each other player only his control information for that tick
	(this imposes a maximum frame rate, as the state-of-the-world will be advanced at the same
	time on all machines).  the second is the continuous lag-tolerant model where each player
	shares absolute information with each other player as often as possible and local machines
	do their best at guessing what everyone else in the game is doing until they get better
	information.  whichever choice is made will change the physics drastically.  we're going to
	take the latter approach, and cache the KeyMap at interrupt time to be batch-processed
	later at frame time.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 6, 2000 (Loren Petrich):
	Added access to size of physics-definition structure

Feb 20, 2000 (Loren Petrich):
	Fixed chase-cam behavior: DROP_DEAD_HEIGHT is effectively zero for it.
	Also, set up-and-down bob to zero when it is active.

Aug 31, 2000 (Loren Petrich):
	Added stuff for unpacking and packing

May 16, 2002 (Woody Zenfell):
    Letting user decide whether to auto-recenter when running

 June 14, 2003 (Woody Zenfell):
	update_player_physics_variables() can now operate in a reduced-impact mode
		that changes less of the game state.  Useful for partial-game-state
		save-and-restore code (as used by prediction mechanism).
*/

/*
running backwards shouldn’t mean doom in a fistfight

//who decides on the physics model, anyway?  static_world-> or player->
//falling through gridlines and crapping on elevators has to do with variables->flags being wrong after the player dies
//absolute (or nearly-absolute) positioning information for yaw, pitch and velocity
//the physics model is too soft (more noticable at high frame rates)
//we can continually boot ourselves out of nearly-orthogonal walls by tiny amounts, resulting in a slide
//it’s fairly obvious that players can still end up in walls
//the recenter key should work faster
*/

#ifdef DEBUG
//#define DIVERGENCE_CHECK
#endif

#include "cseries.h"
#include "render.h"
#include "map.h"
#include "player.h"
#include "interface.h"
#include "monsters.h"
#include "preferences.h"

#define DONT_REPEAT_DEFINITIONS
#include "monster_definitions.h"

#include "media.h"

// LP addition:
#include "ChaseCam.h"
#include "Packing.h"

#include <string.h>
#include <cstdlib>
#include <algorithm>

/* ---------- constants */

#define COEFFICIENT_OF_ABSORBTION 2
#define SMALL_ENOUGH_VELOCITY (constants->climbing_acceleration)
#define CLOSE_ENOUGH_TO_FLOOR WORLD_TO_FIXED(WORLD_ONE/16)

#define AIRBORNE_HEIGHT WORLD_TO_FIXED(WORLD_ONE/16)

#define DROP_DEAD_HEIGHT WORLD_TO_FIXED(WORLD_ONE_HALF)

#define FLAGS_WHICH_PREVENT_RECENTERING (_turning|_looking|_sidestepping|_looking_vertically|_look_dont_turn|_sidestep_dont_turn)

/* ---------- private prototypes */

/* needed for set_player_position -SB */
/*static*/ struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
/*static*/ void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);
static void physics_update(struct physics_constants *constants, struct physics_variables *variables, struct player_data *player, uint32 action_flags);

/* ---------- globals */

/* import constants, structures and globals for physics models */
#include "physics_models.h"

/* ---------- code */

#ifdef DIVERGENCE_CHECK
#define SAVED_POINT_COUNT 8192
static world_point3d *saved_points;
static angle *saved_thetas;
static short saved_point_count, saved_point_iterations= 0;
static bool saved_divergence_warning;
#endif

static struct physics_constants physics_models[NUMBER_OF_PHYSICS_MODELS];
static fixed_yaw_pitch vir_aim_delta = {0, 0};

/* every other field in the player structure should be valid when this call is made */
void initialize_player_physics_variables(
	short player_index)
{
	if (player_index == local_player_index)
		resync_virtual_aim();
	
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *object= get_object_data(monster->object_index);
	struct physics_variables *variables= &player->variables;
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, 0);

//#ifdef DEBUG
	obj_set(*variables, 0x80);
//#endif

	variables->head_direction= 0;
	variables->adjusted_yaw= variables->direction= INTEGER_TO_FIXED(object->facing);
	variables->adjusted_pitch= variables->elevation= 0;
	variables->angular_velocity= variables->vertical_angular_velocity= 0;
	variables->velocity= 0, variables->perpendicular_velocity= 0;
	variables->position.x= WORLD_TO_FIXED(object->location.x);
	variables->position.y= WORLD_TO_FIXED(object->location.y);
	variables->position.z= WORLD_TO_FIXED(object->location.z);
	variables->last_position= variables->position;
	variables->last_direction= variables->direction;
	/* .floor_height, .ceiling_height and .media_height will be calculated by instantiate, below */

	variables->external_angular_velocity= 0;
	variables->external_velocity.i= variables->external_velocity.j= variables->external_velocity.k= 0;
	variables->actual_height= constants->height;
	
	variables->step_phase= 0;
	variables->step_amplitude= 0;
	
	variables->action= _player_stationary;
	variables->old_flags= variables->flags= 0; /* not recentering, not above ground, not below ground (i.e., on floor) */

	/* setup shadow variables in player_data structure */
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0),
		&player->variables, player_index, true, true);

#ifdef DIVERGENCE_CHECK
	if (!saved_point_iterations)
	{
		saved_points= new world_point3d[SAVED_POINT_COUNT];
		saved_thetas= new angle[SAVED_POINT_COUNT];
	}
	saved_point_count= 0;
	saved_point_iterations+= 1;
	saved_divergence_warning= false;
#endif
}

void update_player_physics_variables(
	short player_index,
	uint32 action_flags,
	bool predictive)
{
	struct player_data *player= get_player_data(player_index);
	struct physics_variables *variables= &player->variables;
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, action_flags);

	physics_update(constants, variables, player, action_flags);
	instantiate_physics_variables(constants, variables, player_index, false, !predictive);

#ifdef DIVERGENCE_CHECK
	if (saved_point_count<SAVED_POINT_COUNT)
	{
		struct object_data *object= get_object_data(get_monster_data(player->monster_index)->object_index);
		world_point3d p= object->location;
		world_point3d *q= saved_points+saved_point_count;
		angle *facing= saved_thetas+saved_point_count;
		
		if (saved_point_iterations==1)
		{
			saved_points[saved_point_count]= p;
			*facing= object->facing;
		}
		else
		{
			if (p.x!=q->x||p.y!=q->y||p.z!=q->z||*facing!=object->facing&&!saved_divergence_warning)
			{
				dprintf("divergence @ tick %d: (%d,%d,%d,%d)!=(%d,%d,%d,%d)", saved_point_count,
					q->x, q->y, q->z, *facing, p.x, p.y, p.z, object->facing);
				saved_divergence_warning= true;
			}
		}
		
		saved_point_count+= 1;
	}
#endif
}

void adjust_player_for_polygon_height_change(
	short monster_index,
	short polygon_index,
	world_distance new_floor_height,
	world_distance new_ceiling_height)
{
	short player_index= monster_index_to_player_index(monster_index);
	struct player_data *player= get_player_data(player_index);
	struct physics_variables *variables= &player->variables;
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	world_distance old_floor_height= polygon->floor_height;

	(void) (new_ceiling_height);

	if (player->supporting_polygon_index==polygon_index)
	{
		if (FIXED_TO_WORLD(variables->position.z)<=old_floor_height) /* must be <= */
		{
			variables->floor_height= variables->position.z= WORLD_TO_FIXED(new_floor_height);
			if (film_profile.fix_sliding_on_platforms && variables->external_velocity.k < 0) 
			{
				variables->external_velocity.k = 0;
			}

			if (PLAYER_IS_DEAD(player)) variables->external_velocity.k= 0;
		}
	}
}

void accelerate_player(
	short monster_index,
	world_distance vertical_velocity,
	angle direction,
	world_distance velocity)
{
	short player_index= monster_index_to_player_index(monster_index);
	struct player_data *player= get_player_data(player_index);
	struct physics_variables *variables= &player->variables;
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, 0);

	variables->external_velocity.k+= WORLD_TO_FIXED(vertical_velocity);
	variables->external_velocity.k= PIN(variables->external_velocity.k, -constants->terminal_velocity, constants->terminal_velocity);
	
	if (get_monster_definition_external(_monster_marine)->flags & _monster_can_grenade_climb)
	{
		variables->external_velocity.i= (cosine_table[direction]*velocity)>>(TRIG_SHIFT+WORLD_FRACTIONAL_BITS-FIXED_FRACTIONAL_BITS);
		variables->external_velocity.j= (sine_table[direction]*velocity)>>(TRIG_SHIFT+WORLD_FRACTIONAL_BITS-FIXED_FRACTIONAL_BITS);
	} else {
		variables->external_velocity.i+= (cosine_table[direction]*velocity)>>(TRIG_SHIFT+WORLD_FRACTIONAL_BITS-FIXED_FRACTIONAL_BITS);
		variables->external_velocity.j+= (sine_table[direction]*velocity)>>(TRIG_SHIFT+WORLD_FRACTIONAL_BITS-FIXED_FRACTIONAL_BITS);
	}	
}

void get_absolute_pitch_range(
	_fixed *minimum,
	_fixed *maximum)
{
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, 0);
	
	*minimum= -constants->maximum_elevation;
	*maximum= constants->maximum_elevation;
}

void kill_player_physics_variables(
	short player_index)
{
}

/* return a number in [-FIXED_ONE,FIXED_ONE] (arguably) */
_fixed get_player_forward_velocity_scale(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct physics_variables *variables= &player->variables;
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, _run_dont_walk);
	_fixed dx= variables->position.x - variables->last_position.x;
	_fixed dy= variables->position.y - variables->last_position.y;

	return INTEGER_TO_FIXED(((dx*cosine_table[FIXED_INTEGERAL_PART(variables->direction)] +
		dy*sine_table[FIXED_INTEGERAL_PART(variables->direction)])>>TRIG_SHIFT))/constants->maximum_forward_velocity;
}

fixed_yaw_pitch virtual_aim_delta()
{
	return vir_aim_delta;
}

void resync_virtual_aim()
{
	vir_aim_delta = {0, 0};
}

uint32 process_aim_input(uint32 action_flags, fixed_yaw_pitch delta)
{
	// Classic behavior modes
	const bool classic_precision = !input_preferences->extra_mouse_precision;
	const bool classic_limits = input_preferences->classic_aim_speed_limits;
	
	// Classic precision behavior:
	// - round magnitudes within (0, FIXED_ONE) to FIXED_ONE
	// - round toward zero instead of nearest
	// - lock virtual aim to physical aim
	
	auto clamp = [classic_limits](fixed_angle theta, int encoding_bits) -> fixed_angle
	{
		const angle encoding_bias = (1<<encoding_bits)/2;
		const angle encoding_limit = encoding_bias - 1; // encoding supports [-bias, bias-1] but we want +/- symmetry
		const angle limit = classic_limits ? encoding_bias/2 : encoding_limit;
		return A1_PIN(theta, -limit*FIXED_ONE, limit*FIXED_ONE);
	};
	
	auto round = [classic_precision](fixed_angle theta) -> angle
	{
		return classic_precision ?
			SGN(theta) * std::max<angle>(1, std::abs(theta/FIXED_ONE)) :
			(theta + SGN(theta)*FIXED_ONE/2) / FIXED_ONE;
	};
	
	auto encode = [](angle theta, int encoding_bits) -> uint32
	{
		return (theta + (1<<encoding_bits)/2) & ((1<<encoding_bits) - 1);
	};
	
	const physics_variables& local_phys = local_player->variables;
	
	// The delta from the current physical aim to the requested virtual aim
	const fixed_yaw_pitch full_delta = {delta.yaw + vir_aim_delta.yaw, delta.pitch + vir_aim_delta.pitch};
	
	// Process yaw input
	if (!(action_flags & _override_absolute_yaw))
	{
		const fixed_angle target = clamp(full_delta.yaw, ABSOLUTE_YAW_BITS); // the high-precision target delta
		const angle payload = round(target); // the low-precision delta to be encoded
		
		if (payload || local_phys.angular_velocity)
			action_flags = SET_ABSOLUTE_YAW(action_flags, encode(payload, ABSOLUTE_YAW_BITS)) | _absolute_yaw_mode;
		
		// Update virtual yaw
		vir_aim_delta.yaw = classic_precision ? 0 : target - payload*FIXED_ONE;
		assert(std::abs(vir_aim_delta.yaw) <= FIXED_ONE/2);
	}
	
	// Explicit and automatic recentering do not occur under absolute pitch mode; therefore we
	// 1) try to always use absolute pitch mode if the user doesn't want auto-recentering; and
	// 2) always avoid absolute pitch mode while an explicit recentering operation is in progress
	// (pitch control is necessarily locked out until the recentering completes; no way to cancel)
	
	const bool explicitly_recentering = local_phys.flags & _RECENTERING_BIT;
	
	// Process pitch input
	if (!(action_flags & _override_absolute_pitch) && !explicitly_recentering)
	{
		const fixed_angle target = clamp(full_delta.pitch, ABSOLUTE_PITCH_BITS); // the high-precision target delta
		const angle payload = round(target); // the low-precision delta to be encoded
		
		if (payload || local_phys.vertical_angular_velocity || dont_auto_recenter())
			action_flags = SET_ABSOLUTE_PITCH(action_flags, encode(payload, ABSOLUTE_PITCH_BITS)) | _absolute_pitch_mode;
		
		// Update virtual pitch
		vir_aim_delta.pitch = classic_precision ? 0 : target - payload*FIXED_ONE;
		assert(std::abs(vir_aim_delta.pitch) <= FIXED_ONE/2);
	}
	
	return action_flags;
}


/* ---------- private code */

/*static*/ struct physics_constants *get_physics_constants_for_model(
	short physics_model,
	uint32 action_flags)
{
	struct physics_constants *constants;
	
	switch (physics_model)
	{
		case _editor_model:
		case _earth_gravity_model: constants= physics_models + ((action_flags&_run_dont_walk) ? _model_game_running : _model_game_walking); break;
		case _low_gravity_model:
			assert(false);
			break;
		default:
			assert(false);
			break;
	}
	
	return constants;
}

/*static*/ void instantiate_physics_variables(
	struct physics_constants *constants,
	struct physics_variables *variables,
	short player_index,
	bool first_time,
	bool take_action)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *legs= get_object_data(monster->object_index);
	struct object_data *torso= get_object_data(legs->parasitic_object);
	short old_polygon_index= legs->polygon;
	world_point3d new_location;
	world_distance adjusted_floor_height, adjusted_ceiling_height, object_floor;
	bool clipped;
	_fixed step_height;
	angle facing, elevation;
	_fixed fixed_facing;

	/* convert to world coordinates before doing collision detection */
	new_location.x= FIXED_TO_WORLD(variables->position.x);
	new_location.y= FIXED_TO_WORLD(variables->position.y);
	new_location.z= FIXED_TO_WORLD(variables->position.z);

	/* check for 2d collisions with walls and knock the player back out of the wall (because of
		the way the physics updates work, we don’t worry about collisions with the floor or
		ceiling).  ONLY MODIFY THE PLAYER’S FIXED_POINT3D POSITION IF WE HAD A COLLISION */
	if (PLAYER_IS_DEAD(player)) new_location.z+= FIXED_TO_WORLD(DROP_DEAD_HEIGHT);
	if (take_action && !first_time && player->last_supporting_polygon_index!=player->supporting_polygon_index) changed_polygon(player->last_supporting_polygon_index, player->supporting_polygon_index, player_index);
	player->last_supporting_polygon_index= first_time ? NONE : player->supporting_polygon_index;
	clipped= keep_line_segment_out_of_walls(legs->polygon, &legs->location, &new_location,
		WORLD_ONE/3, FIXED_TO_WORLD(variables->actual_height), &adjusted_floor_height, &adjusted_ceiling_height,
		&player->supporting_polygon_index);
	if (PLAYER_IS_DEAD(player)) new_location.z-= FIXED_TO_WORLD(DROP_DEAD_HEIGHT);

	/* check for 2d collisions with solid objects and knock the player back out of the object.
		ONLY MODIFY THE PLAYER’S FIXED_POINT3D POSITION IF WE HAD A COLLISION. */
	object_floor= INT16_MIN;
	{
		short obstruction_index= legal_player_move(player->monster_index, &new_location, &object_floor);
		
		if (obstruction_index!=NONE)
		{
			struct object_data *object= get_object_data(obstruction_index);
			
			switch (GET_OBJECT_OWNER(object))
			{
				case _object_is_monster:
					if(take_action)
						bump_monster(player->monster_index, object->permutation);
				case _object_is_scenery:
					new_location.x= legs->location.x, new_location.y= legs->location.y;
					clipped= true;
					break;
				
				default:
					assert(false);
					break;
			}
		}
	}

	/* translate_map_object will handle crossing polygon boundaries */
	if (translate_map_object(monster->object_index, &new_location, NONE))
	{
		if (old_polygon_index==legs->polygon) clipped= true; /* oops; trans_map_obj destructively changed our position */
		if(take_action)
			monster_moved(player->monster_index, old_polygon_index);
	}

	/* if our move got clipped, copy the new coordinate back into the physics variables */
	if (clipped)
	{
		variables->position.x= WORLD_TO_FIXED(new_location.x);
		variables->position.y= WORLD_TO_FIXED(new_location.y);
		variables->position.z= WORLD_TO_FIXED(new_location.z);
	}
	
	/* shadow position in player structure, build camera location */
	step_height= (constants->step_amplitude*sine_table[variables->step_phase>>(FIXED_FRACTIONAL_BITS-ANGULAR_BITS+1)])>>TRIG_SHIFT;
	step_height= (step_height*variables->step_amplitude)>>FIXED_FRACTIONAL_BITS;

	player->camera_location= new_location;
	if (PLAYER_IS_DEAD(player) && new_location.z<adjusted_floor_height) new_location.z= adjusted_floor_height;
	player->location= new_location;
	player->camera_location.z+= FIXED_TO_WORLD(step_height+variables->actual_height-constants->camera_height);
	player->step_height = FIXED_TO_WORLD(step_height);
	player->camera_polygon_index= legs->polygon;

	/* shadow facing in player structure and object structure */
	fixed_facing= variables->direction+variables->head_direction;
	facing= FIXED_INTEGERAL_PART(fixed_facing), facing= NORMALIZE_ANGLE(facing);
	elevation= FIXED_INTEGERAL_PART(variables->elevation), elevation= NORMALIZE_ANGLE(elevation);
	legs->location.z= player->location.z;
	legs->facing= NORMALIZE_ANGLE(FIXED_INTEGERAL_PART(variables->direction)), torso->facing= player->facing= facing;
	player->elevation= elevation;

	/* initialize floor_height and ceiling_height for next call to physics_update() */
	variables->floor_height= WORLD_TO_FIXED(MAX(adjusted_floor_height, object_floor));
	variables->ceiling_height= WORLD_TO_FIXED(adjusted_ceiling_height);
	{
		short media_index= get_polygon_data(legs->polygon)->media_index;
		// LP change: idiot-proofing
		media_data *media = get_media_data(media_index);
		world_distance media_height= (media_index==NONE || !media) ? INT16_MIN : media->height;

		if (player->location.z<media_height) variables->flags|= _FEET_BELOW_MEDIA_BIT; else variables->flags&= (uint16)~_FEET_BELOW_MEDIA_BIT;
		if (player->camera_location.z<media_height) variables->flags|= _HEAD_BELOW_MEDIA_BIT; else variables->flags&= (uint16)~_HEAD_BELOW_MEDIA_BIT;
	}

	// so our sounds come from the right place
	monster->sound_location= player->camera_location;
	monster->sound_polygon_index= player->camera_polygon_index;
}

/* separate physics_constant structures are passed in for running/walking modes */
// ZZZ note: 'player' is only used in this routine for PLAYER_IS_DEAD(player) - as such,
// perhaps this should take an "is_dead" flag as a parameter instead of the player structure.
static void physics_update(
	struct physics_constants *constants,
	struct physics_variables *variables,
	struct player_data *player,
	uint32 action_flags)
{
	fixed_point3d new_position;
	short sine, cosine;
	_fixed delta_z;
	_fixed delta; /* used as a scratch ‘change’ variable */
	
	const bool player_is_local = (player == local_player);

	if (PLAYER_IS_DEAD(player)) /* dead players immediately loose all bodily control */
	{
		int32 dot_product;
		
		cosine= cosine_table[FIXED_INTEGERAL_PART(variables->direction)], sine= sine_table[FIXED_INTEGERAL_PART(variables->direction)];
		dot_product= ((((variables->velocity*cosine)>>TRIG_SHIFT) + variables->external_velocity.i)*cosine +
			(((variables->velocity*sine)>>TRIG_SHIFT) + variables->external_velocity.j)*sine)>>TRIG_SHIFT;

		if (dot_product>0 && dot_product<(constants->maximum_forward_velocity>>4)) dot_product= 0;
		switch (SGN(dot_product))
		{
			case -1: action_flags= _looking_up; break;
			case 1: action_flags= _looking_down; break;
			case 0: action_flags= 0; break;
			default:
				assert(false);
				break;
		}
		
		variables->floor_height-= DROP_DEAD_HEIGHT;
		
		// Prohibit twitching while dead (!)
		if (player_is_local)
			resync_virtual_aim();
	}
	delta_z= variables->position.z-variables->floor_height;

	/* process modifier keys (sidestepping and looking) into normal actions */
	if ((action_flags&_turning) && (action_flags&_sidestep_dont_turn) && !(action_flags&_absolute_yaw_mode))
	{
		if (action_flags&_turning_left) action_flags|= _sidestepping_left;
		if (action_flags&_turning_right) action_flags|= _sidestepping_right;
		action_flags&= ~_turning;
	}
	if ((action_flags&_moving) && (action_flags&_look_dont_turn) && !(action_flags&_absolute_position_mode))
	{
		if (action_flags&_moving_forward) action_flags|= _looking_up;
		if (action_flags&_moving_backward) action_flags|= _looking_down;
		action_flags&= ~_moving;
		action_flags&= ~_absolute_pitch_mode;
	}

	/* handle turning left or right; if we’ve exceeded our maximum velocity lock out user actions
		until we return to a legal range */
	if (action_flags&_absolute_yaw_mode)
	{
		variables->angular_velocity= (GET_ABSOLUTE_YAW(action_flags)-MAXIMUM_ABSOLUTE_YAW/2)<<(FIXED_FRACTIONAL_BITS); // !!!!!!!!!!
	}
	else
	{
		if (variables->angular_velocity<-constants->maximum_angular_velocity||variables->angular_velocity>constants->maximum_angular_velocity) action_flags&= ~_turning;
		switch (action_flags&_turning)
		{
			case _turning_left:
				delta= variables->angular_velocity>0 ? constants->angular_acceleration+constants->angular_deceleration : constants->angular_acceleration;
				variables->angular_velocity= FLOOR(variables->angular_velocity-delta, -constants->maximum_angular_velocity);
				break;
			case _turning_right:
				delta= variables->angular_velocity<0 ? constants->angular_acceleration+constants->angular_deceleration : constants->angular_acceleration;
				variables->angular_velocity= CEILING(variables->angular_velocity+delta, constants->maximum_angular_velocity);
				break;
			
			default: /* slow down */
				variables->angular_velocity= (variables->angular_velocity>=0) ?
					FLOOR(variables->angular_velocity-constants->angular_deceleration, 0) :
					CEILING(variables->angular_velocity+constants->angular_deceleration, 0);
				break;
		}
		
		/* handling looking left/right */
		switch (action_flags&_looking)
		{
			case _looking_left:
				variables->head_direction= FLOOR(variables->head_direction-constants->fast_angular_velocity, -constants->fast_angular_maximum);
				break;
			case _looking_right:
				variables->head_direction= CEILING(variables->head_direction+constants->fast_angular_velocity, constants->fast_angular_maximum);
				break;
			case _looking: /* do nothing if both keys are down */
				break;
			
			default: /* recenter head */
				variables->head_direction= (variables->head_direction>=0) ?
					FLOOR(variables->head_direction-constants->fast_angular_velocity, 0) :
					CEILING(variables->head_direction+constants->fast_angular_velocity, 0);
		}
		
		// Let yaw controls resync virtual yaw
		if (player_is_local && (action_flags & (_turning|_looking)) != 0)
			vir_aim_delta.yaw = 0;
	}

	if (action_flags&_absolute_pitch_mode)
	{
		variables->vertical_angular_velocity= (GET_ABSOLUTE_PITCH(action_flags)-MAXIMUM_ABSOLUTE_PITCH/2)<<(FIXED_FRACTIONAL_BITS);
	}
	else
	{
		/* if the user touched the recenter key, set the recenter flag and override all up/down
			keypresses with our own */
		if (action_flags&_looking_center) variables->flags|= _RECENTERING_BIT;
		if (variables->flags&_RECENTERING_BIT)
		{
			action_flags&= ~_looking_vertically;
			action_flags|= variables->elevation<0 ? _looking_up : _looking_down;
		}
	
		/* handle looking up and down; if we’re moving at our terminal velocity forward or backward,
			without any side-to-side motion, recenter our head vertically */

		if (!(action_flags&FLAGS_WHICH_PREVENT_RECENTERING)) /* can’t recenter if any of these are true */
		{
			if (((action_flags&_moving_forward) && (variables->velocity==constants->maximum_forward_velocity)) ||
				((action_flags&_moving_backward) && (variables->velocity==-constants->maximum_backward_velocity)))
			{
				if (variables->elevation<0)
				{
					variables->elevation= CEILING(variables->elevation+constants->angular_recentering_velocity, 0);
				}
				else
				{
					variables->elevation= FLOOR(variables->elevation-constants->angular_recentering_velocity, 0);
				}
				
				// Let auto-recentering resync virtual pitch
				if (player_is_local)
					vir_aim_delta.pitch = 0;
			}
		}

		switch (action_flags&_looking_vertically)
		{
			case _looking_down:
				delta= variables->vertical_angular_velocity>0 ? constants->angular_acceleration+constants->angular_deceleration : constants->angular_acceleration;
				variables->vertical_angular_velocity= FLOOR(variables->vertical_angular_velocity-delta, PLAYER_IS_DEAD(player) ? -(constants->maximum_angular_velocity>>3) : -constants->maximum_angular_velocity);
				break;
			case _looking_up:
				delta= variables->vertical_angular_velocity<0 ? constants->angular_acceleration+constants->angular_deceleration : constants->angular_acceleration;
				variables->vertical_angular_velocity= CEILING(variables->vertical_angular_velocity+delta, PLAYER_IS_DEAD(player) ? (constants->maximum_angular_velocity>>3) : constants->maximum_angular_velocity);
				break;
			
			default: /* if no key is being held down, decelerate; if the player is moving try and return to phi==0 */
				variables->vertical_angular_velocity= (variables->vertical_angular_velocity>=0) ?
					FLOOR(variables->vertical_angular_velocity-constants->angular_deceleration, 0) :
					CEILING(variables->vertical_angular_velocity+constants->angular_deceleration, 0);
				break;
		}
		
		// Let pitch controls resync virtual pitch
		if (player_is_local && (action_flags & _looking_vertically) != 0)
			vir_aim_delta.pitch = 0;
	}

	/* if we’re on the ground (or rising up from it), allow movement; if we’re flying through
		the air, don’t let the player adjust his velocity in any way */
	if (delta_z<=0 || (variables->flags&_HEAD_BELOW_MEDIA_BIT))
	{
		if (action_flags&_absolute_position_mode)
		{
			short encoded_delta= GET_ABSOLUTE_POSITION(action_flags)-MAXIMUM_ABSOLUTE_POSITION/2;
			
			if (encoded_delta<0)
			{
				variables->velocity= (encoded_delta*constants->maximum_backward_velocity)>>(ABSOLUTE_POSITION_BITS-1);
			}
			else
			{
				variables->velocity= (encoded_delta*constants->maximum_forward_velocity)>>(ABSOLUTE_POSITION_BITS-1);
			}
		}
		else
		{
			/* handle moving forward or backward; if we’ve exceeded our maximum velocity lock out user actions
				until we return to a legal range */
			if (variables->velocity<-constants->maximum_backward_velocity||variables->velocity>constants->maximum_forward_velocity) action_flags&= ~_moving;
			switch (action_flags&_moving)
			{
				case _moving_forward:
					delta= variables->velocity<0 ? constants->deceleration+constants->acceleration : constants->acceleration;
					variables->velocity= CEILING(variables->velocity+delta, constants->maximum_forward_velocity);
					break;
				case _moving_backward:
					delta= variables->velocity>0 ? constants->deceleration+constants->acceleration : constants->acceleration;
					variables->velocity= FLOOR(variables->velocity-delta, -constants->maximum_backward_velocity);
					break;
				
				default: /* slow down */
					variables->velocity= (variables->velocity>=0) ?
						FLOOR(variables->velocity-constants->deceleration, 0) :
						CEILING(variables->velocity+constants->deceleration, 0);
					break;
			}
		}
		
		/* handle sidestepping left or right; if we’ve exceeded our maximum velocity lock out user actions
			until we return to a legal range */
		if (variables->perpendicular_velocity<-constants->maximum_perpendicular_velocity||variables->perpendicular_velocity>constants->maximum_perpendicular_velocity) action_flags&= ~_sidestepping;
		switch (action_flags&_sidestepping)
		{
			case _sidestepping_left:
				delta= variables->perpendicular_velocity>0 ? constants->acceleration+constants->deceleration : constants->acceleration;
				variables->perpendicular_velocity= FLOOR(variables->perpendicular_velocity-delta, -constants->maximum_perpendicular_velocity);
				break;
			case _sidestepping_right:
				delta= variables->perpendicular_velocity<0 ? constants->acceleration+constants->deceleration : constants->acceleration;
				variables->perpendicular_velocity= CEILING(variables->perpendicular_velocity+delta, constants->maximum_perpendicular_velocity);
				break;
			
			default: /* slow down */
				variables->perpendicular_velocity= (variables->perpendicular_velocity>=0) ?
					FLOOR(variables->perpendicular_velocity-constants->deceleration, 0) :
					CEILING(variables->perpendicular_velocity+constants->deceleration, 0);
				break;
		}
	}
	
	/* change vertical_velocity based on difference between player height and surface height
		(if we are standing on an object, like a body, take that into account, too: this
		means a player could actually use bodies as ramps to reach ledges he couldn't
		otherwise jump to).  we should think about absorbing forward (or perpendicular)
		velocity to compensate for an increase in vertical velocity, which would slow down
		a player climbing stairs, etc. */
	if (delta_z<0)
	{
		variables->external_velocity.k= CEILING(variables->external_velocity.k+constants->climbing_acceleration, constants->terminal_velocity);
	}
	if (delta_z>0)
	{
		_fixed gravity= constants->gravitational_acceleration;
		_fixed terminal_velocity= constants->terminal_velocity;
		
		if (static_world->environment_flags&_environment_low_gravity) gravity>>= 1;
		if (variables->flags&_FEET_BELOW_MEDIA_BIT) gravity>>= 1, terminal_velocity>>= 1;
		
		variables->external_velocity.k= FLOOR(variables->external_velocity.k-gravity, -terminal_velocity);
	}

	if ((action_flags&_swim) && (variables->flags&_HEAD_BELOW_MEDIA_BIT) && variables->external_velocity.k<10*constants->climbing_acceleration)
	{
		variables->external_velocity.k+= constants->climbing_acceleration;
	}

	// Apply vertical angular velocity
	variables->elevation+= variables->vertical_angular_velocity;
	
	// Clamp virtual pitch to the effective limits of the low-precision physical pitch
	// (this won't enlarge the virtual pitch delta)
	if (player_is_local)
	{
		const fixed_angle min_pitch = FIXED_INTEGERAL_PART(-constants->maximum_elevation) * FIXED_ONE;
		const fixed_angle max_pitch = FIXED_INTEGERAL_PART(constants->maximum_elevation) * FIXED_ONE;
		const fixed_angle unclamped_physical_pitch = FIXED_INTEGERAL_PART(variables->elevation) * FIXED_ONE;
		const fixed_angle unclamped_virtual_pitch = unclamped_physical_pitch + vir_aim_delta.pitch;
		const fixed_angle clamped_physical_pitch = A1_PIN(unclamped_physical_pitch, min_pitch, max_pitch);
		const fixed_angle clamped_virtual_pitch = A1_PIN(unclamped_virtual_pitch, min_pitch, max_pitch);
		const fixed_angle new_delta = clamped_virtual_pitch - clamped_physical_pitch;
		assert(std::abs(new_delta) <= std::abs(vir_aim_delta.pitch));
		vir_aim_delta.pitch = new_delta;
	}
	
	// Clamp high-precision physical pitch to physics model limits
	// (note that the low-precision pitch can slightly violate a non-integral lower bound due to rounding toward -inf)
	variables->elevation= PIN(variables->elevation, -constants->maximum_elevation, constants->maximum_elevation);
	
	// If we're explicitly recentering and have reached or passed 0 pitch, stop at 0
	if ((variables->flags&_RECENTERING_BIT) && !(action_flags&_absolute_pitch_mode))
	{
		if ((variables->elevation<=0&&(action_flags&_looking_down))||(variables->elevation>=0&&(action_flags&_looking_up)))
		{
			variables->elevation= variables->vertical_angular_velocity= 0;
			variables->flags&= (uint16)~_RECENTERING_BIT;
		}
	}

	/* change the player’s heading based on his angular velocities */
	variables->last_direction= variables->direction;
	variables->direction+= variables->angular_velocity;
	if (variables->direction<0) variables->direction+= INTEGER_TO_FIXED(FULL_CIRCLE);
	if (variables->direction>=INTEGER_TO_FIXED(FULL_CIRCLE)) variables->direction-= INTEGER_TO_FIXED(FULL_CIRCLE);
	
	/* change the player’s x,y position based on his direction and velocities (parallel and perpendicular)  */
	new_position= variables->position;
	cosine= cosine_table[FIXED_INTEGERAL_PART(variables->direction)], sine= sine_table[FIXED_INTEGERAL_PART(variables->direction)];
	new_position.x+= (variables->velocity*cosine-variables->perpendicular_velocity*sine)>>TRIG_SHIFT;
	new_position.y+= (variables->velocity*sine+variables->perpendicular_velocity*cosine)>>TRIG_SHIFT;
	
	/* set above/below floor flags, remember old flags */
	variables->old_flags= variables->flags;
	if (new_position.z<variables->floor_height) variables->flags|= _BELOW_GROUND_BIT; else variables->flags&= (uint16)~_BELOW_GROUND_BIT;
	if (new_position.z>variables->floor_height) variables->flags|= _ABOVE_GROUND_BIT; else variables->flags&= (uint16)~_ABOVE_GROUND_BIT;

	/* if we just landed on the ground, or we just came up through the ground, absorb some of
		the player’s external_velocity.k (and in the case of hitting the ground, reflect it) */
	if (variables->external_velocity.k>0 && (variables->old_flags&_BELOW_GROUND_BIT) && !(variables->flags&_BELOW_GROUND_BIT))
	{
		variables->external_velocity.k/= 2*COEFFICIENT_OF_ABSORBTION; /* slow down */
	}
	if (variables->external_velocity.k>0 && new_position.z+variables->actual_height>=variables->ceiling_height)
	{
		variables->external_velocity.k/= -COEFFICIENT_OF_ABSORBTION, new_position.z= variables->ceiling_height-variables->actual_height; // &&variables->position.z+variables->actual_height<variables->ceiling_height
	}
	if (variables->external_velocity.k<0&&!(variables->old_flags&_BELOW_GROUND_BIT)&&!(variables->flags&_ABOVE_GROUND_BIT))
	{
		variables->external_velocity.k/= -COEFFICIENT_OF_ABSORBTION;
	}

	_fixed small_enough_velocity;
	if (get_monster_definition_external(_monster_marine)->flags & _monster_can_grenade_climb) {
		_fixed gravity= constants->gravitational_acceleration;		
		if (static_world->environment_flags&_environment_low_gravity) gravity>>= 1;
		if (variables->flags&_FEET_BELOW_MEDIA_BIT) gravity>>= 1;

		small_enough_velocity = gravity;
	} 
	else 
	{
		small_enough_velocity = SMALL_ENOUGH_VELOCITY;
	}
	if (ABS(variables->external_velocity.k)<small_enough_velocity &&
		ABS(variables->floor_height-new_position.z)<CLOSE_ENOUGH_TO_FLOOR)
	{
		variables->external_velocity.k= 0, new_position.z= variables->floor_height;
		variables->flags&= ~(_BELOW_GROUND_BIT|_ABOVE_GROUND_BIT);
	}

	/* change the player’s z position based on his vertical velocity (if we hit the ground coming down
		then bounce and absorb most of the blow */
	new_position.x+= variables->external_velocity.i;
	new_position.y+= variables->external_velocity.j;
	new_position.z+= variables->external_velocity.k;
	
	{
		short dx= variables->external_velocity.i, dy= variables->external_velocity.j;
		_fixed delta= (delta_z<=0) ? constants->external_deceleration : (constants->external_deceleration>>2);
		int32 magnitude= isqrt(dx*dx + dy*dy);

		if (magnitude && magnitude>ABS(delta))
		{
			variables->external_velocity.i-= (dx*delta)/magnitude;
			variables->external_velocity.j-= (dy*delta)/magnitude;
		}
		else
		{
			variables->external_velocity.i= variables->external_velocity.j= 0;
		}
	}

	/* lower the player’s externally-induced angular velocity */
	variables->external_angular_velocity= (variables->external_angular_velocity>=0) ?
		FLOOR(variables->external_angular_velocity-constants->external_angular_deceleration, 0) :
		CEILING(variables->external_angular_velocity+constants->external_angular_deceleration, 0);

	/* instantiate new position, save old position */
	variables->last_position= variables->position;
	variables->position= new_position;

	/* if the player is moving, adjust step_phase by step_delta (if the player isn’t moving
		continue to adjust step_phase until it is zero)  if the player is in the air, don’t
		update phase until he lands. */
	variables->flags&= (uint16)~_STEP_PERIOD_BIT;
	if (constants->maximum_forward_velocity)
		variables->step_amplitude= (MAX(ABS(variables->velocity), ABS(variables->perpendicular_velocity))*FIXED_ONE)/constants->maximum_forward_velocity;
	else	// CB: "Missed Island" physics would produce a division by 0
		variables->step_amplitude= MAX(ABS(variables->velocity), ABS(variables->perpendicular_velocity))*FIXED_ONE;
	if (delta_z>=0)
	{
		if (variables->velocity||variables->perpendicular_velocity)
		{
//			fixed old_step_phase= variables->step_phase;
			
			if ((variables->step_phase+= constants->step_delta)>=FIXED_ONE)
			{
				variables->step_phase-= FIXED_ONE;
				variables->flags|= _STEP_PERIOD_BIT;
			}
//			else
//			{
//				if (variables->step_phase>=FIXED_ONE_HALF && old_step_phase<FIXED_ONE_HALF)
//				{
//					variables->flags|= _STEP_PERIOD_BIT;
//				}
//			}
		}
		else
		{
			if (variables->step_phase)
			{
				if (variables->step_phase>FIXED_ONE_HALF)
				{
					if ((variables->step_phase+= constants->step_delta)>=FIXED_ONE) variables->step_phase= 0;
				}
				else
				{
					if ((variables->step_phase-= constants->step_delta)<0) variables->step_phase= 0;
				}
			}
		}
	}

	if (delta_z >= (PLAYER_IS_DEAD(player) ? (AIRBORNE_HEIGHT+DROP_DEAD_HEIGHT) : AIRBORNE_HEIGHT))
	{
		variables->action= _player_airborne;
	}
	else
	{
		if (variables->angular_velocity||variables->velocity||variables->perpendicular_velocity)
		{
			variables->action= (action_flags&_run_dont_walk) ? _player_running : _player_walking;
		}
		else
		{
			variables->action= (variables->external_velocity.i||variables->external_velocity.j||variables->external_velocity.k) ? _player_sliding : _player_stationary;
		}
	}
}


uint8 *unpack_physics_constants(uint8 *Stream, size_t Count)
{
	return unpack_physics_constants(Stream,physics_models,Count);
}

uint8 *unpack_physics_constants(uint8 *Stream, physics_constants *Objects, size_t Count)
{
	uint8* S = Stream;
	physics_constants* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->maximum_forward_velocity);
		StreamToValue(S,ObjPtr->maximum_backward_velocity);
		StreamToValue(S,ObjPtr->maximum_perpendicular_velocity);
		StreamToValue(S,ObjPtr->acceleration);
		StreamToValue(S,ObjPtr->deceleration);
		StreamToValue(S,ObjPtr->airborne_deceleration);
		StreamToValue(S,ObjPtr->gravitational_acceleration);
		StreamToValue(S,ObjPtr->climbing_acceleration);
		StreamToValue(S,ObjPtr->terminal_velocity);
		StreamToValue(S,ObjPtr->external_deceleration);

		StreamToValue(S,ObjPtr->angular_acceleration);
		StreamToValue(S,ObjPtr->angular_deceleration);
		StreamToValue(S,ObjPtr->maximum_angular_velocity);
		StreamToValue(S,ObjPtr->angular_recentering_velocity);
		StreamToValue(S,ObjPtr->fast_angular_velocity);
		StreamToValue(S,ObjPtr->fast_angular_maximum);
		StreamToValue(S,ObjPtr->maximum_elevation);
		StreamToValue(S,ObjPtr->external_angular_deceleration);
		
		StreamToValue(S,ObjPtr->step_delta);
		StreamToValue(S,ObjPtr->step_amplitude);
		StreamToValue(S,ObjPtr->radius);
		StreamToValue(S,ObjPtr->height);
		StreamToValue(S,ObjPtr->dead_height);
		StreamToValue(S,ObjPtr->camera_height);
		StreamToValue(S,ObjPtr->splash_height);
		
		StreamToValue(S,ObjPtr->half_camera_separation);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_physics_constants));
	return S;
}

uint8* unpack_m1_physics_constants(uint8* Stream, size_t Count)
{
	static const int SIZEOF_old_physics_entry = 100;
	uint8* S = Stream + SIZEOF_old_physics_entry; // first is "editor" record
	physics_constants* ObjPtr = physics_models;
	
	for (size_t k = 0; k < Count - 1; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->maximum_forward_velocity);
		StreamToValue(S,ObjPtr->maximum_backward_velocity);
		StreamToValue(S,ObjPtr->maximum_perpendicular_velocity);
		StreamToValue(S,ObjPtr->acceleration);
		StreamToValue(S,ObjPtr->deceleration);
		StreamToValue(S,ObjPtr->airborne_deceleration);
		StreamToValue(S,ObjPtr->gravitational_acceleration);
		StreamToValue(S,ObjPtr->climbing_acceleration);
		StreamToValue(S,ObjPtr->terminal_velocity);
		StreamToValue(S,ObjPtr->external_deceleration);

		StreamToValue(S,ObjPtr->angular_acceleration);
		StreamToValue(S,ObjPtr->angular_deceleration);
		StreamToValue(S,ObjPtr->maximum_angular_velocity);
		StreamToValue(S,ObjPtr->angular_recentering_velocity);
		StreamToValue(S,ObjPtr->fast_angular_velocity);
		StreamToValue(S,ObjPtr->fast_angular_maximum);
		StreamToValue(S,ObjPtr->maximum_elevation);
		StreamToValue(S,ObjPtr->external_angular_deceleration);
		
		StreamToValue(S,ObjPtr->step_delta);
		StreamToValue(S,ObjPtr->step_amplitude);
		StreamToValue(S,ObjPtr->radius);
		StreamToValue(S,ObjPtr->height);
		StreamToValue(S,ObjPtr->dead_height);
		StreamToValue(S,ObjPtr->camera_height);
		ObjPtr->splash_height = 0;
		
		StreamToValue(S,ObjPtr->half_camera_separation);
	}
	
	return S;
}

uint8 *pack_physics_constants(uint8 *Stream, size_t Count)
{
	return pack_physics_constants(Stream,physics_models,Count);
}

uint8 *pack_physics_constants(uint8 *Stream, physics_constants *Objects, size_t Count)
{
	uint8* S = Stream;
	physics_constants* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->maximum_forward_velocity);
		ValueToStream(S,ObjPtr->maximum_backward_velocity);
		ValueToStream(S,ObjPtr->maximum_perpendicular_velocity);
		ValueToStream(S,ObjPtr->acceleration);
		ValueToStream(S,ObjPtr->deceleration);
		ValueToStream(S,ObjPtr->airborne_deceleration);
		ValueToStream(S,ObjPtr->gravitational_acceleration);
		ValueToStream(S,ObjPtr->climbing_acceleration);
		ValueToStream(S,ObjPtr->terminal_velocity);
		ValueToStream(S,ObjPtr->external_deceleration);

		ValueToStream(S,ObjPtr->angular_acceleration);
		ValueToStream(S,ObjPtr->angular_deceleration);
		ValueToStream(S,ObjPtr->maximum_angular_velocity);
		ValueToStream(S,ObjPtr->angular_recentering_velocity);
		ValueToStream(S,ObjPtr->fast_angular_velocity);
		ValueToStream(S,ObjPtr->fast_angular_maximum);
		ValueToStream(S,ObjPtr->maximum_elevation);
		ValueToStream(S,ObjPtr->external_angular_deceleration);
		
		ValueToStream(S,ObjPtr->step_delta);
		ValueToStream(S,ObjPtr->step_amplitude);
		ValueToStream(S,ObjPtr->radius);
		ValueToStream(S,ObjPtr->height);
		ValueToStream(S,ObjPtr->dead_height);
		ValueToStream(S,ObjPtr->camera_height);
		ValueToStream(S,ObjPtr->splash_height);
		
		ValueToStream(S,ObjPtr->half_camera_separation);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_physics_constants));
	return S;
}

void init_physics_constants()
{
	memcpy(physics_models, original_physics_models, sizeof(physics_models));
}

// LP addition: get number of physics models (restricted sense)
size_t get_number_of_physics_models() {return NUMBER_OF_PHYSICS_MODELS;}

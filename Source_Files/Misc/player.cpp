/*
PLAYER.C
Saturday, December 11, 1993 10:25:55 AM

Friday, September 30, 1994 5:48:25 PM (Jason)
	moved nearly all sounds out of the damage_definition structure and into shapes.
Wednesday, October 26, 1994 3:18:59 PM (Jason)
	invincible players are now damaged by fusion projectiles.
Wednesday, November 30, 1994 6:56:20 PM  (Jason)
	oxygen is used up faster by running and by firing.
Thursday, January 12, 1995 11:18:18 AM  (Jason')
	dead players don’t continue to use up oxygen.
Thursday, July 6, 1995 4:53:52 PM
	supports multi-player cooperative games. (Ryan)

Feb 4, 2000 (Loren Petrich):
	Added SMG wielding stuff

	Changed halt() to assert(false) for better debugging

Feb 18, 2000 (Loren Petrich):
	Added support for a chase cam.
	Note that mark_player_collections() always loads the player sprites
	in expectation of a chase cam; this could be made to conditional on
	whether a chase cam will ever be active.

Feb 21, 2000 (Loren Petrich):
	Changed NO_TELEPORTATION_DESTINATION to SHORT_MAX, an idiot-proof value,
	since there are unlikely to be that many polygons in a map.
	
	Added upward and rightward shifts of the chase-cam position

Feb 25, 2000 (Loren Petrich):
	Moved chase-cam data into preferences data; using accessor in "interface.h"
	Made it possible to swim under a liquid if one has the ball

Feb 26, 2000 (Loren Petrich):
	Fixed level-0 teleportation bug; the hack is to move the destination
	down by 1.
	
	Added chase-cam reset feature, for the purpose of doing chase-cam inertia.
	The reset is necessary to take into account teleporting or entering a level.

Mar 2, 2000 (Loren Petrich):
	Moved the chase-cam stuff into ChaseCam.c/h
	
Mar 22, 2000 (Loren Petrich):
	Added a function to revive_player() to reset the field of view properly
	when reviving

May 14, 2000 (Loren Petrich):
	Added XML-configuration support for various player features

May 22, 2000 (Loren Petrich):
	Added XML configurability for the powerup durations

May 27, 2000 (Loren Petrich):
	Added oxygen depletion and replenishment rates

Jun 11, 2000 (Loren Petrich):
	Pegging health and oxygen to maximum values when damaged;
	takes into account negative damage from healing projectiles.
	Also turned "agressor" into "aggressor".

Jun 15, 2000 (Loren Petrich):
	Added support for Chris Pruett's Pfhortran

Jun 28, 2000 (Loren Petrich):
	Generalized the invincibility-powerup vulnerability and added XML support for that

Jul 1, 2000 (Loren Petrich):
	Added Benad's changes

Jul 10, 2000 (Loren Petrich):
	Changed calculate_player_team() slightly; no more first vassert()
*/

#include "cseries.h"
#include "map.h"
#include "player.h"
#include "monsters.h"
#include "interface.h"
#include "mysound.h"
#include "fades.h"
#include "media.h"
#include "items.h"
#include "weapons.h"
#include "game_window.h"
#include "computer_interface.h"
#include "projectiles.h"
#include "network_games.h"
#include "screen.h"

/*
//anybody on the receiving pad of a teleport should explode (what happens to invincible guys?)
// Really should create a function that initializes the player state.
??new players should teleport in
*/

// LP addition:
#include "ChaseCam.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef env68k
#pragma segment player
#endif

/* ---------- constants */

#define ACTION_QUEUE_BUFFER_DIAMETER 0x100
#define ACTION_QUEUE_BUFFER_INDEX_MASK 0xff

// These are now variables, because they can be set with an XML parser
static short kINVISIBILITY_DURATION = (70*TICKS_PER_SECOND);
static short kINVINCIBILITY_DURATION = (50*TICKS_PER_SECOND);
static short kEXTRAVISION_DURATION = (3*TICKS_PER_MINUTE);
static short kINFRAVISION_DURATION = (3*TICKS_PER_MINUTE);
/*
#define kINVISIBILITY_DURATION (70*TICKS_PER_SECOND)
#define kINVINCIBILITY_DURATION (50*TICKS_PER_SECOND)
#define kEXTRAVISION_DURATION (3*TICKS_PER_MINUTE)
#define kINFRAVISION_DURATION (3*TICKS_PER_MINUTE)
*/

#define MINIMUM_REINCARNATION_DELAY (TICKS_PER_SECOND)
#define NORMAL_REINCARNATION_DELAY (10*TICKS_PER_SECOND)
#define SUICIDE_REINCARNATION_DELAY (15*TICKS_PER_SECOND)

#define DEAD_PLAYER_HEIGHT WORLD_ONE_FOURTH

#define OXYGEN_WARNING_LEVEL TICKS_PER_MINUTE
#define OXYGEN_WARNING_FREQUENCY (TICKS_PER_MINUTE/4)
#define OXYGEN_WARNING_OFFSET (10*TICKS_PER_SECOND)

#define LAST_LEVEL 100

// LP addition: self-luminosity
fixed PlayerSelfLuminosity = FIXED_ONE_HALF;

// LP additions: oxygen depletion and replenishment rates
// (number of units per tick);
// oxygen change is set equal to depletion or replenishment,
// whichever one is appropriate for the environment (vacuum/liquid vs. normal air)
static short OxygenDepletion = 1, OxygenReplenishment = 0, OxygenChange = 0;

// LP addition: invincibility-powerup vulnerability
static short Vulnerability = _damage_fusion_bolt;
				
/* ---------- structures */

struct action_queue /* 8 bytes */
{
	short read_index, write_index;
	
	long *buffer;
};

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

struct damage_response_definition
{
	short type;
	short damage_threshhold; /* NONE is none, otherwise bumps fade by one if over threshhold */
	
	short fade;
	short sound, death_sound, death_action;
};

/* ---------- globals */

struct player_data *players;

struct player_data *local_player, *current_player;
short local_player_index, current_player_index;

static struct action_queue *action_queues;

static struct player_shape_definitions player_shapes=
{
	6, /* collection */
	
	9, 8, /* dying hard, dying soft */
	11, 10, /* dead hard, dead soft */
	{7, 0, 0, 24, 23}, /* legs: stationary, walking, running, sliding, airborne */
	// LP additions: SMG-wielding/firing shapes (just before last two)
	{1, 3, 20, 26, 14, 12, 31, 16, 28, 33, 5, 18}, /* idle torsos: fists, magnum, fusion, assault, rocket, flamethrower, alien, shotgun, double pistol, double shotgun, da ball */
	{1, 3, 21, 26, 14, 12, 31, 16, 28, 33, 5, 18}, /* charging torsos: fists, magnum, fusion, assault, rocket, flamethrower, alien, shotgun, double pistol, double shotgun, ball */
	{2, 4, 22, 27, 15, 13, 32, 17, 28, 34, 6, 19}, /* firing torsos: fists, magnum, fusion, assault, rocket, flamethrower, alien, shotgun, double pistol, double shotgun, ball */
};

#define NUMBER_OF_PLAYER_INITIAL_ITEMS (sizeof(player_initial_items)/sizeof(short))
static short player_initial_items[]= 
{ 
	_i_magnum,  // First weapon is the weapon he will use...
	_i_knife,
	_i_knife,
	_i_magnum_magazine, 
	_i_magnum_magazine,
	_i_magnum_magazine,
	
	// LP additions, in case one wants to start very loaded
	_i_knife,
	_i_knife,
	_i_knife,
	_i_knife,
	_i_knife,
	
	_i_knife,
	_i_knife,
	_i_knife,
	_i_knife,
	_i_knife
};
	
#define NUMBER_OF_DAMAGE_RESPONSE_DEFINITIONS (sizeof(damage_response_definitions)/sizeof(struct damage_response_definition))

static struct damage_response_definition damage_response_definitions[]=
{
	{_damage_explosion, 100, _fade_yellow, NONE, _snd_human_scream, _monster_is_dying_hard},
	{_damage_crushing, NONE, _fade_red, NONE, _snd_human_wail, _monster_is_dying_hard},
	{_damage_projectile, NONE, _fade_red, NONE, _snd_human_scream, NONE},
	{_damage_shotgun_projectile, NONE, _fade_red, NONE, _snd_human_scream, NONE},
	{_damage_electrical_staff, NONE, _fade_cyan, NONE, _snd_human_scream, NONE},
	{_damage_hulk_slap, NONE, _fade_cyan, NONE, _snd_human_scream, NONE},
	{_damage_absorbed, 100, _fade_white, _snd_absorbed, NONE, NONE},
	{_damage_teleporter, 100, _fade_white, _snd_absorbed, NONE, NONE},
	{_damage_flame, NONE, _fade_orange, NONE, _snd_human_wail, _monster_is_dying_flaming},
	{_damage_hound_claws, NONE, _fade_red, NONE, _snd_human_scream, NONE},
	{_damage_compiler_bolt, NONE, _fade_static, NONE, _snd_human_scream, NONE},
	{_damage_alien_projectile, NONE, _fade_dodge_purple, NONE, _snd_human_wail, _monster_is_dying_flaming},
	{_damage_hunter_bolt, NONE, _fade_burn_green, NONE, _snd_human_scream, NONE},
	{_damage_fusion_bolt, 60, _fade_negative, NONE, _snd_human_scream, NONE},
	{_damage_fist, 40, _fade_red, NONE, _snd_human_scream, NONE},
	{_damage_yeti_claws, NONE, _fade_burn_cyan, NONE, _snd_human_scream, NONE},
	{_damage_yeti_projectile, NONE, _fade_dodge_yellow, NONE, _snd_human_scream, NONE},
	{_damage_defender, NONE, _fade_purple, NONE, _snd_human_scream, NONE},
	{_damage_lava, NONE, _fade_long_orange, NONE, _snd_human_wail, _monster_is_dying_flaming},
	{_damage_goo, NONE, _fade_long_green, NONE, _snd_human_wail, _monster_is_dying_flaming},
	{_damage_suffocation, NONE, NONE, NONE, _snd_suffocation, _monster_is_dying_soft},
	{_damage_energy_drain, NONE, NONE, NONE, NONE, NONE},
	{_damage_oxygen_drain, NONE, NONE, NONE, NONE, NONE},
	{_damage_hummer_bolt, NONE, _fade_flicker_negative, NONE, _snd_human_scream, NONE},
};

/* For teleportation */
#define EPILOGUE_LEVEL_NUMBER 256
// LP change: made this much bigger than the number of M2/Moo polygons
// #define NO_TELEPORTATION_DESTINATION 512
#define NO_TELEPORTATION_DESTINATION SHORT_MAX

// LP additions: variables for initial energy, initial oxygen, and stripped energy:
static short InitialEnergy = PLAYER_MAXIMUM_SUIT_ENERGY;
static short InitialOxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;
static short StrippedEnergy = PLAYER_MAXIMUM_SUIT_ENERGY/4;

/* ---------- private prototypes */

static void set_player_shapes(short player_index, boolean animate);
static void revive_player(short player_index);
static void recreate_player(short player_index);
static void kill_player(short player_index, short aggressor_player_index, short action);
static void give_player_initial_items(short player_index);
static void get_player_transfer_mode(short player_index, short *transfer_mode, short *transfer_period);
static void set_player_dead_shape(short player_index, boolean dying);
static void remove_dead_player_items(short player_index);
static void update_player_teleport(short player_index);
static void handle_player_in_vacuum(short player_index, long action_flags);
static void update_player_media(short player_index);
static short calculate_player_team(short base_team);

static void try_and_strip_player_items(short player_index);

// LP addition:
static void ReplenishPlayerOxygen(short player_index, long action_flags);

/* ---------- code */

void allocate_player_memory(
	void)
{
	long *action_queue_buffer;
	short i;

	/* allocate space for all our players */
	players= (struct player_data *) malloc(sizeof(struct player_data)*MAXIMUM_NUMBER_OF_PLAYERS);
	assert(players);

#ifdef BETA
	dprintf("#%d players at %p (%x bytes each) ---------------------------------------;g;", MAXIMUM_NUMBER_OF_PLAYERS, players, sizeof(struct player_data));
#endif

	/* allocate space for our action queue headers and the queues themselves */
	action_queues= (struct action_queue *) malloc(sizeof(struct action_queue)*MAXIMUM_NUMBER_OF_PLAYERS);
	action_queue_buffer= (long *) malloc(sizeof(long)*MAXIMUM_NUMBER_OF_PLAYERS*ACTION_QUEUE_BUFFER_DIAMETER);
	assert(action_queues&&action_queue_buffer);
	
	/* tell the queues where their buffers are */
	for (i=0;i<MAXIMUM_NUMBER_OF_PLAYERS;++i)
	{
		action_queues[i].buffer= action_queue_buffer + i*ACTION_QUEUE_BUFFER_DIAMETER;
	}

	return;
}

/* returns player index */
short new_player(
	short team,
	short color,
	short identifier)
{
	short player_index, loop;
	struct player_data *player;

	/* find a free slot */
	player_index= dynamic_world->player_count;
	assert(player_index<MAXIMUM_NUMBER_OF_PLAYERS);
	dynamic_world->player_count += 1;
	player= get_player_data(player_index);

	/* and initialize it */
	memset(player, 0, sizeof(struct player_data));
	player->teleporting_destination= NO_TELEPORTATION_DESTINATION;
	player->interface_flags= 0; // Doesn't matter-> give_player_initial_items will take care of it.
	// LP change: using variables for these
	player->suit_energy= InitialEnergy;
	player->suit_oxygen= InitialOxygen;
	// player->suit_energy= PLAYER_MAXIMUM_SUIT_ENERGY;
	// player->suit_oxygen= PLAYER_MAXIMUM_SUIT_OXYGEN;
	player->color= color;
	player->team= team;
	player->flags= 0;
	
	player->invincibility_duration= 0;
	player->invisibility_duration= 0;
	player->infravision_duration= 0;
	player->extravision_duration= 0;
	player->identifier= identifier;

	/* initialize inventory */	
	for (loop=0;loop<NUMBER_OF_ITEMS;++loop) player->items[loop]= NONE;

	/* create the player.. */
	recreate_player(player_index);

	/* Mark the player's inventory as dirty */
	mark_player_inventory_as_dirty(player_index, NONE);
	initialize_player_weapons(player_index);
	
	/* give the player his initial items */
	give_player_initial_items(player_index);
	try_and_strip_player_items(player_index);

	return player_index;
}

void walk_player_list(
	void)
{
	struct player_data *player;
	short player_index= current_player_index;
	
	/* find the next player in the list we can look at and switch to them */
	do
	{
		if ((player_index+= 1)>=dynamic_world->player_count) player_index= 0;
		player= get_player_data(player_index);
	}
	while (!(GET_GAME_OPTIONS()&_overhead_map_is_omniscient) && local_player->team!=player->team);
	
	if (current_player_index!=player_index)
	{
		set_current_player_index(player_index);
		update_interface(NONE);
		dirty_terminal_view(player_index); /* In case they are in terminal mode.. */
	}
	
	return;
}

void initialize_players(
	void)
{
	short i;
	
	/* no players */
	dynamic_world->player_count= 0;
	
	/* reset the action flag queues and zero the player slots */
	for (i=0;i<MAXIMUM_NUMBER_OF_PLAYERS;++i)
	{
		memset(players+i, 0, sizeof(struct player_data));
		action_queues[i].read_index= action_queues[i].write_index= 0;
	}
	
	return;
}

/* This will be called by entering map for two reasons:
 * 1) get rid of crap typed between levels, by accident.
 * 2) loading a game doesn't currently reset the player queues, so garbage will cause lags.
 */
/* The above comment is stale.  Now loading map calls this and so does new_game. Calling this */
/*  from entering map would bone us. */
void reset_player_queues(
	void)
{
	short i;

	for (i=0;i<MAXIMUM_NUMBER_OF_PLAYERS;++i)
	{
		action_queues[i].read_index= action_queues[i].write_index= 0;
	}

	reset_recording_and_playback_queues();
	sync_heartbeat_count(); //•• MY ADDITION...
}

/* queue an action flag on the given player’s queue (no zombies allowed) */
void queue_action_flags(
	short player_index,
	long *action_flags,
	short count)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= action_queues+player_index;

	//assert(!PLAYER_IS_ZOMBIE(player)); // CP: Changed for scripting
	if (PLAYER_IS_ZOMBIE(player))
		return;
	while ((count-= 1)>=0)
	{
		queue->buffer[queue->write_index]= *action_flags++;
		queue->write_index= (queue->write_index+1)&ACTION_QUEUE_BUFFER_INDEX_MASK;
		if (queue->write_index==queue->read_index) dprintf("blew player %d’s queue at %p;g;", player_index, queue);
	}

	return;
}

/* dequeue’s a single action flag from the given queue (zombies always return zero) */
long dequeue_action_flags(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= action_queues+player_index;
	long action_flags;

	if (PLAYER_IS_ZOMBIE(player))
	{
		//dprintf("Player is zombie!", player_index);	// CP: Disabled for scripting
		action_flags= 0;
	}
	else
	{
		assert(queue->read_index!=queue->write_index);
		action_flags= queue->buffer[queue->read_index];
		queue->read_index= (queue->read_index+1)&ACTION_QUEUE_BUFFER_INDEX_MASK;
	}

	return action_flags;
}

/* returns the number of elements sitting in the given queue (zombies always return queue diameter) */
short get_action_queue_size(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct action_queue *queue= action_queues+player_index;
	short size;

	if (PLAYER_IS_ZOMBIE(player))
	{
		//dprintf("PLayer %d is a zombie", player_index);  // CP: Disabled for scripting
		size= ACTION_QUEUE_BUFFER_DIAMETER;
	} 
	else
	{
		if ((size= queue->write_index-queue->read_index)<0) size+= ACTION_QUEUE_BUFFER_DIAMETER;
	}
	
	return size;
}

/* assumes ∂t==1 tick */
void update_players(
	void)
{
	struct player_data *player;
	short player_index;
	
	for (player_index= 0, player= players; player_index<dynamic_world->player_count; ++player_index, ++player)
	{
		struct polygon_data *polygon= get_polygon_data(player->supporting_polygon_index);
		long action_flags= dequeue_action_flags(player_index);
		
		if (action_flags==NONE)
		{
			// net dead
			if (!PLAYER_IS_DEAD(player))
			{
				// kills invincible players, too
				detonate_projectile(&player->location, player->camera_polygon_index, _projectile_minor_fusion_dispersal,
					NONE, NONE, 10*FIXED_ONE);
			}
			
			action_flags= 0;
		}

		if (PLAYER_IS_TELEPORTING(player)) action_flags= 0;
		
		/* Deal with the terminal mode crap. */
		if (player_in_terminal_mode(player_index))
		{
			update_player_keys_for_terminal(player_index, action_flags);
			action_flags= 0;
			update_player_for_terminal_mode(player_index);
		}

		// if we’ve got the ball we can’t run (that sucks)
		// Benad: also works with _game_of_rugby and _game_of_capture_the_flag
		// LP change: made it possible to swim under a liquid if one has the ball
		if (((GET_GAME_TYPE()==_game_of_kill_man_with_ball) || (GET_GAME_TYPE()==_game_of_rugby) || (GET_GAME_TYPE()==_game_of_capture_the_flag)) 
		 && dynamic_world->game_player_index==player_index && !(player->variables.flags&_HEAD_BELOW_MEDIA_BIT)) action_flags&= ~_run_dont_walk;
		// if (GET_GAME_TYPE()==_game_of_kill_man_with_ball && dynamic_world->game_player_index==player_index) action_flags&= ~_run_dont_walk;
		
		// if our head is under media, we can’t run (that sucks, too)
		if ((player->variables.flags&_HEAD_BELOW_MEDIA_BIT) && (action_flags&_run_dont_walk)) action_flags&= ~_run_dont_walk, action_flags|= _swim;
		
		update_player_physics_variables(player_index, action_flags);

		player->invisibility_duration= FLOOR(player->invisibility_duration-1, 0);
		player->invincibility_duration= FLOOR(player->invincibility_duration-1, 0);
		player->infravision_duration= FLOOR(player->infravision_duration-1, 0);
		player->reincarnation_delay= FLOOR(player->reincarnation_delay-1, 0);
		if (player->extravision_duration)
		{
			if (!(player->extravision_duration-= 1))
			{
				if (player_index==current_player_index) start_extravision_effect(FALSE);
			}
		}
		// LP change: made this code more general;
		// find the oxygen-change rate appropriate to each environment,
		// then handle the rate appropriately.
		if ((static_world->environment_flags&_environment_vacuum) || (player->variables.flags&_HEAD_BELOW_MEDIA_BIT))
			OxygenChange = - OxygenDepletion;
		else
			OxygenChange = OxygenReplenishment;
		
		if (OxygenChange < 0)
			handle_player_in_vacuum(player_index, action_flags);
		else if (OxygenChange > 0)
			ReplenishPlayerOxygen(player_index, action_flags);
		
		// if ((static_world->environment_flags&_environment_vacuum) || (player->variables.flags&_HEAD_BELOW_MEDIA_BIT)) handle_player_in_vacuum(player_index, action_flags);

		/* handle arbitration of the communications channel (i.e., dynamic_world->speaking_player_index) */
		if ((action_flags&_microphone_button) && !PLAYER_IS_DEAD(player))
		{
			if (dynamic_world->speaking_player_index==NONE)
			{
				dynamic_world->speaking_player_index= player_index;
				if (player_index==local_player_index) set_interface_microphone_recording_state(TRUE);
			}
		}
		else
		{
			if (dynamic_world->speaking_player_index==player_index)
			{
				dynamic_world->speaking_player_index= NONE;
				if (player_index==local_player_index) set_interface_microphone_recording_state(FALSE);
			}
		}

		if (PLAYER_IS_DEAD(player))
		{
			/* do things dead players do (sit around and check for self-reincarnation) */
			if (PLAYER_HAS_MAP_OPEN(player))
				SET_PLAYER_MAP_STATUS(player, FALSE);
			if (PLAYER_IS_TOTALLY_DEAD(player) && (action_flags&_action_trigger_state) && !player->reincarnation_delay && (player->variables.action==_player_stationary||dynamic_world->player_count==1))
			{
				if (dynamic_world->player_count==1) set_game_state(_revert_game);
				else revive_player(player_index);
			}
			update_player_weapons(player_index, 0);
			update_action_key(player_index, FALSE);
		}
		else
		{
			/* do things live players do (get items, update weapons, check action key, breathe) */
			swipe_nearby_items(player_index);
			update_player_weapons(player_index, action_flags);
			update_action_key(player_index, (action_flags&_action_trigger_state) ? TRUE : FALSE);
			if (action_flags&_toggle_map)
				SET_PLAYER_MAP_STATUS(player, !PLAYER_HAS_MAP_OPEN(player));
		}

		update_player_teleport(player_index);
		update_player_media(player_index);
		set_player_shapes(player_index, TRUE);
	}
	
	return;
}

void damage_player(
	short monster_index,
	short aggressor_index,
	short aggressor_type,
	struct damage_definition *damage)
{
	short player_index= monster_index_to_player_index(monster_index);
	short aggressor_player_index= NONE; /* will be valid if the aggressor is a player */
	struct player_data *player= get_player_data(player_index);
	short damage_amount= calculate_damage(damage);
	short damage_type= damage->type;
	struct damage_response_definition *definition;

	(void) (aggressor_type);
	
	// LP change: made this more general
	if (player->invincibility_duration && damage->type!=Vulnerability)
	// if (player->invincibility_duration && damage->type!=_damage_fusion_bolt)
	{
		damage_type= _damage_absorbed;
	}

	{
		short i;
		
		for (i=0,definition=damage_response_definitions;
				definition->type!=damage_type && i<NUMBER_OF_DAMAGE_RESPONSE_DEFINITIONS;
				++i,++definition)
			;
		vwarn(i!=NUMBER_OF_DAMAGE_RESPONSE_DEFINITIONS, csprintf(temporary, "can't react to damage type #%d", damage_type));
		// vassert(i!=NUMBER_OF_DAMAGE_RESPONSE_DEFINITIONS, csprintf(temporary, "can't react to damage type #%d", damage_type));
	}
	
	if (damage_type!=_damage_absorbed)
	{
		/* record damage taken */
		if (aggressor_index!=NONE)
		{
			struct monster_data *aggressor= get_monster_data(aggressor_index);
			
			if (!PLAYER_IS_DEAD(player))
			{
				if (MONSTER_IS_PLAYER(aggressor))
				{
					struct player_data *aggressor_player;
					
					aggressor_player_index= monster_index_to_player_index(aggressor_index);
					aggressor_player= get_player_data(aggressor_player_index);
					player->damage_taken[aggressor_player_index].damage+= damage_amount;
					aggressor_player->total_damage_given.damage+= damage_amount;
				}
				else
				{
					player->monster_damage_taken.damage+= damage_amount;
				}
			}
		}

		switch (damage->type)
		{
			case _damage_oxygen_drain:
				// LP change: pegging to maximum value
				if ((player->suit_oxygen= MIN(long(player->suit_oxygen)-long(damage_amount),long(SHORT_MAX)))<0) player->suit_oxygen= 0;
				// if ((player->suit_oxygen-= damage_amount)<0) player->suit_oxygen= 0;
				if (player_index==current_player_index) mark_oxygen_display_as_dirty();
				break;
			
			default:
				/* damage the player, recording the kill if the aggressor was another player and we died */
				// LP change: pegging to maximum value
				if ((player->suit_energy= MIN(long(player->suit_energy)-long(damage_amount),long(SHORT_MAX)))<0)
				// if ((player->suit_energy-= damage_amount)<0)
				{
					if (damage->type!=_damage_energy_drain)
					{
						if (!PLAYER_IS_DEAD(player))
						{
							short action= definition->death_action;
							
							play_object_sound(player->object_index, definition->death_sound);

							if (action==NONE)
							{
								action= (damage_amount>PLAYER_MAXIMUM_SUIT_ENERGY/2) ? _monster_is_dying_hard : _monster_is_dying_soft;
							}
							
							kill_player(player_index, aggressor_player_index, action);
							if (aggressor_player_index!=NONE)
							{
								struct player_data *agressor_player= get_player_data(aggressor_player_index);
								
								if (player_killed_player(player_index, aggressor_player_index))
								{
									player->damage_taken[aggressor_player_index].kills+= 1;
									if (aggressor_player_index != player_index)
									{
										agressor_player->total_damage_given.kills+= 1;
									}
								}
							}
							else
							{
								player->monster_damage_taken.kills+= 1;
							}
						}
						
						player->suit_oxygen= 0;
						if (player_index==current_player_index) mark_oxygen_display_as_dirty();
					}
					
					player->suit_energy= 0;
				}
				break;
		}
	}
	
	{
		if (!PLAYER_IS_DEAD(player)) play_object_sound(player->object_index, definition->sound);
		if (player_index==current_player_index)
		{
			if (definition->fade!=NONE) start_fade((definition->damage_threshhold!=NONE&&damage_amount>definition->damage_threshhold) ? (definition->fade+1) : definition->fade);
			if (damage_amount) mark_shield_display_as_dirty();
		}
	}

	if(player_in_terminal_mode(player_index))
	{
		abort_terminal_mode(player_index);
	}

	return;
}

short player_identifier_to_player_index(
	short player_identifier)
{
	struct player_data *player;
	short player_index;
	
	for (player_index=0;player_index<dynamic_world->player_count;++player_index)
	{
		player= get_player_data(player_index);
		
		if (player->identifier==player_identifier) break;
	}
	assert(player_index!=dynamic_world->player_count);
	
	return player_index;
}

void mark_player_collections(
	boolean loading)
{
	mark_collection(player_shapes.collection, loading);
	// LP change: unload player shapes for single-player game only if
	// a chase cam cannot exist;
	if (!ChaseCam_CanExist())
		if (dynamic_world->player_count==1&&loading) strip_collection(player_shapes.collection);

	mark_weapon_collections(loading);
	mark_item_collections(loading);
	mark_interface_collections(loading);
	
	return;
}

/*
#ifdef DEBUG
struct player_data *get_player_data(
	short player_index)
{
	vassert(player_index>=0&&player_index<dynamic_world->player_count,
		csprintf(temporary, "asked for player #%d/#%d", player_index, dynamic_world->player_count));
	
	return players + player_index;
}
#endif
*/

void set_local_player_index(
	short player_index)
{
	local_player_index= player_index;
	local_player= get_player_data(player_index);
	
	return;
}

void set_current_player_index(
	short player_index)
{
	current_player_index= player_index;
	current_player= get_player_data(player_index);
	
	return;
}

/* We just teleported in as it were-> recreate all the players..  */
void recreate_players_for_new_level(
	void)
{
	short player_index;
	
	for (player_index= 0; player_index<dynamic_world->player_count; ++player_index)
	{
		/* Recreate all of the players for the new level.. */	
		recreate_player(player_index);
	}
	
	return;
}

short monster_index_to_player_index(
	short monster_index)
{
	struct player_data *player;
	short player_index;
	
	for (player_index=0;player_index<dynamic_world->player_count;++player_index)
	{
		player= get_player_data(player_index);
		if (player->monster_index==monster_index) break;
	}
	assert(player_index!=dynamic_world->player_count);
	
	return player_index;
}

short get_polygon_index_supporting_player(
	short monster_index)
{
	short player_index= monster_index_to_player_index(monster_index);
	struct player_data *player= get_player_data(player_index);
	
	return player->supporting_polygon_index;
}

boolean legal_player_powerup(
	short player_index,
	short item_index)
{
	struct player_data *player= get_player_data(player_index);
	boolean legal= TRUE;
	
	switch (item_index)
	{
		case _i_invisibility_powerup:
			if (player->invisibility_duration>kINVISIBILITY_DURATION) legal= FALSE;
			break;

		case _i_invincibility_powerup:
			if (player->invincibility_duration) legal= FALSE;
			break;
		
		case _i_infravision_powerup:
			if (player->infravision_duration) legal= FALSE;
			break;
		
		case _i_extravision_powerup:
			if (player->extravision_duration) legal= FALSE;
			break;
		
		case _i_oxygen_powerup:
			if (player->suit_oxygen>5*PLAYER_MAXIMUM_SUIT_OXYGEN/6) legal= FALSE;
			break;
		
		case _i_energy_powerup:
			if (player->suit_energy>=1*PLAYER_MAXIMUM_SUIT_ENERGY) legal= FALSE;
			break;
		case _i_double_energy_powerup:
			if (player->suit_energy>=2*PLAYER_MAXIMUM_SUIT_ENERGY) legal= FALSE;
			break;
		case _i_triple_energy_powerup:
			if (player->suit_energy>=3*PLAYER_MAXIMUM_SUIT_ENERGY) legal= FALSE;
			break;
	}

	return legal;
}

void process_player_powerup(
	short player_index,
	short item_index)
{
	struct player_data *player= get_player_data(player_index);
	
	switch (item_index)
	{
		case _i_invisibility_powerup:
			player->invisibility_duration+= kINVISIBILITY_DURATION;
			break;

		case _i_invincibility_powerup:
			player->invincibility_duration+= kINVINCIBILITY_DURATION;
			break;
		
		case _i_infravision_powerup:
			player->infravision_duration+= kINFRAVISION_DURATION;
			break;
		
		case _i_extravision_powerup:
			if (player_index==current_player_index) start_extravision_effect(TRUE);
			player->extravision_duration+= kEXTRAVISION_DURATION;
			break;
		
		case _i_oxygen_powerup:
			player->suit_oxygen= CEILING(player->suit_oxygen+PLAYER_MAXIMUM_SUIT_OXYGEN/2, PLAYER_MAXIMUM_SUIT_OXYGEN);
			if (player_index==current_player_index) mark_oxygen_display_as_dirty();
			break;
		
		case _i_energy_powerup:
			if (player->suit_energy<1*PLAYER_MAXIMUM_SUIT_ENERGY)
			{
				player->suit_energy= 1*PLAYER_MAXIMUM_SUIT_ENERGY;
				if (player_index==current_player_index) mark_shield_display_as_dirty();
			}
			break;
		case _i_double_energy_powerup:
			if (player->suit_energy<2*PLAYER_MAXIMUM_SUIT_ENERGY)
			{
				player->suit_energy= 2*PLAYER_MAXIMUM_SUIT_ENERGY;
				if (player_index==current_player_index) mark_shield_display_as_dirty();
			}
			break;
		case _i_triple_energy_powerup:
			if (player->suit_energy<3*PLAYER_MAXIMUM_SUIT_ENERGY)
			{
				player->suit_energy= 3*PLAYER_MAXIMUM_SUIT_ENERGY;
				if (player_index==current_player_index) mark_shield_display_as_dirty();
			}
			break;
	}

	return;
}

world_distance dead_player_minimum_polygon_height(
	short polygon_index)
{
	short player_index;
	struct player_data *player;
	world_distance minimum_height= 0;
	
	for (player_index= 0, player= players; player_index<dynamic_world->player_count; ++player_index, ++player)
	{
		if (polygon_index==player->camera_polygon_index)
		{
			if (PLAYER_IS_DEAD(player)) minimum_height= DEAD_PLAYER_HEIGHT;
			break;
		}
	}
	
	return minimum_height;
}

boolean try_and_subtract_player_item(
	short player_index,
	short item_type)
{
	struct player_data *player= get_player_data(player_index);
	boolean found_one= FALSE;

	assert(item_type>=0 && item_type<NUMBER_OF_ITEMS);
	if (player->items[item_type]>=0)
	{
		if (!(player->items[item_type]-= 1)) player->items[item_type]= NONE;
		mark_player_inventory_as_dirty(player_index, item_type);
		found_one= TRUE;
	}
	
	return found_one;
}

/* ---------- private prototypes */

// LP change: assumes nonpositive change rate
static void handle_player_in_vacuum(
	short player_index,
	long action_flags)
{
	struct player_data *player= get_player_data(player_index);

	if (player->suit_oxygen>0)	
	{
		short breathing_frequency;
		
		switch (player->suit_oxygen/TICKS_PER_MINUTE)
		{
			case 0: breathing_frequency= TICKS_PER_MINUTE/6;
			case 1: breathing_frequency= TICKS_PER_MINUTE/5;
			case 2: breathing_frequency= TICKS_PER_MINUTE/4;
			case 3: breathing_frequency= TICKS_PER_MINUTE/3;
			default: breathing_frequency= TICKS_PER_MINUTE/2;
		}

		if (!(player->suit_oxygen%breathing_frequency)) play_local_sound(_snd_breathing);
		if ((player->suit_oxygen+OXYGEN_WARNING_OFFSET)<OXYGEN_WARNING_LEVEL && !((player->suit_oxygen+OXYGEN_WARNING_OFFSET)%OXYGEN_WARNING_FREQUENCY)) play_local_sound(_snd_oxygen_warning);
		
		// LP change: modified to use global variable for change rate
		assert(OxygenChange <= 0);
		player->suit_oxygen+= OxygenChange;
		switch (dynamic_world->game_information.difficulty_level)
		{
			case _total_carnage_level:
				if (action_flags&_run_dont_walk) player->suit_oxygen+= OxygenChange;
			case _major_damage_level:
				if (action_flags&(_left_trigger_state|_right_trigger_state)) player->suit_oxygen+= OxygenChange;
				break;
		}
		/*
		player->suit_oxygen-= 1;
		switch (dynamic_world->game_information.difficulty_level)
		{
			case _total_carnage_level:
				if (action_flags&_run_dont_walk) player->suit_oxygen-= 1;
			case _major_damage_level:
				if (action_flags&(_left_trigger_state|_right_trigger_state)) player->suit_oxygen-= 1;
				break;
		}
		*/
		
		if (player->suit_oxygen<=0)
		{
			struct damage_definition damage;
			
			damage.flags= 0;
			damage.type= _damage_suffocation;
			damage.base= player->suit_energy+1;
			damage.random= 0;
			damage.scale= FIXED_ONE;
			
			damage_player(player->monster_index, NONE, NONE, &damage);
		}
	}
	
	return;
}

// LP: assumes nonnegative change rate
static void ReplenishPlayerOxygen(short player_index, long action_flags)
{
	(void)(action_flags);
	
	struct player_data *player= get_player_data(player_index);
	
	// Be careful to avoid short-integer wraparound
	assert(OxygenChange >= 0);
	if (player->suit_oxygen < PLAYER_MAXIMUM_SUIT_OXYGEN)
	{
		if (player->suit_oxygen < PLAYER_MAXIMUM_SUIT_OXYGEN - OxygenChange)
			player->suit_oxygen += OxygenChange;
		else
			player->suit_oxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;
	}
}

static void update_player_teleport(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *object= get_object_data(monster->object_index);
	struct polygon_data *polygon= get_polygon_data(object->polygon);
	boolean player_was_interlevel_teleporting= FALSE;
	
	/* This is the players that are carried across the teleport unwillingly.. */
	if(PLAYER_IS_INTERLEVEL_TELEPORTING(player))
	{
		player_was_interlevel_teleporting= TRUE;
		
		player->interlevel_teleport_phase+= 1;
		switch(player->interlevel_teleport_phase)
		{
			case PLAYER_TELEPORTING_DURATION:
				monster->action= _monster_is_moving;
				SET_PLAYER_TELEPORTING_STATUS(player, FALSE);
				SET_PLAYER_INTERLEVEL_TELEPORTING_STATUS(player, FALSE);
				break;

			/* +1 because they are teleporting to a new level, and we want the squeeze in to happen */
			/*  after the level transition */
			case PLAYER_TELEPORTING_MIDPOINT+1:
				/* Either the player is teleporting, or everyone is. (level change) */
				if(player_index==current_player_index)
				{
					start_teleporting_effect(FALSE);
					play_object_sound(player->object_index, _snd_teleport_in); 
				}
				player->teleporting_destination= NO_TELEPORTATION_DESTINATION;
				break;
				
			default:
				break;
		}
	}

	if (PLAYER_IS_TELEPORTING(player))
	{
		switch (player->teleporting_phase+= 1)
		{
			case PLAYER_TELEPORTING_MIDPOINT:
				if(player->teleporting_destination>=0) /* Intralevel. */
				{
					short destination_polygon_index= player->teleporting_destination;
					struct polygon_data *destination_polygon= get_polygon_data(destination_polygon_index);
					struct damage_definition damage;
					world_point3d destination;
					
					/* Determine where we are going. */
					*((world_point2d *)&destination)= destination_polygon->center;
					destination.z= destination_polygon->floor_height;

					damage.type= _damage_teleporter;
					damage.base= damage.random= damage.flags= damage.scale= 0;
					damage_monsters_in_radius(NONE, NONE, NONE, &destination, destination_polygon_index,
						WORLD_ONE, &damage);

					translate_map_object(player->object_index, &destination, destination_polygon_index);
					initialize_player_physics_variables(player_index);
	
					// LP addition: handles the current player's chase cam;
					// in screen.c, we find that it's the current player whose view gets rendered
					if (player_index == current_player_index) ChaseCam_Reset();
				} else { /* -level number is the interlevel */
					// LP change: moved down by 1 so that level 0 will be valid
 					short level_number= -player->teleporting_destination - 1;
					
					if(level_number==EPILOGUE_LEVEL_NUMBER)
					{
						/* Should this do something sly in cooperative play? */
						set_game_state(_display_epilogue);
						return;
					} else {
						set_game_state(_change_level);
						set_change_level_destination(level_number);
					}
				}
				break;

			case PLAYER_TELEPORTING_MIDPOINT+1:
				 /* Interlevel or my intralevel.. */
				if(player_index==current_player_index)
				{
					start_teleporting_effect(FALSE);
					play_object_sound(player->object_index, _snd_teleport_in); 
				} 
				player->teleporting_destination= NO_TELEPORTATION_DESTINATION;
				break;
			
			case PLAYER_TELEPORTING_DURATION:
				monster->action= _monster_is_moving;
				SET_PLAYER_TELEPORTING_STATUS(player, FALSE);
				break;
		}
	}
	else if(!player_was_interlevel_teleporting)
	{
		/* Note that control panels can set the teleporting destination. */
		if (player->teleporting_destination!=NO_TELEPORTATION_DESTINATION ||
			((polygon->type==_polygon_is_automatic_exit && calculate_level_completion_state()!=_level_unfinished) || polygon->type==_polygon_is_teleporter) &&
				player->variables.position.x==player->variables.last_position.x &&
				player->variables.position.y==player->variables.last_position.y &&
				player->variables.position.z==player->variables.last_position.z &&
				player->variables.last_direction==player->variables.direction &&
				object->location.z==polygon->floor_height)
		{
			if(--player->delay_before_teleport<0)
			{
				SET_PLAYER_TELEPORTING_STATUS(player, TRUE);
				monster->action= _monster_is_teleporting;
				player->teleporting_phase= 0;
				player->delay_before_teleport= 0; /* The only function that changes this are */
													/* computer terminals. */

				/* They are in an automatic exit. */
				if (player->teleporting_destination==NO_TELEPORTATION_DESTINATION)
				{
					if (polygon->type==_polygon_is_automatic_exit && calculate_level_completion_state()!=_level_unfinished)
					{
						/* This is an auto exit, and they are successful */
						// LP change: moved down by 1 so that level 0 will be valid
						player->teleporting_destination= -polygon->permutation - 1;
					}
					else
					{
						/* This is a simple teleporter */
						player->teleporting_destination= polygon->permutation;
					}
				}
	
				if (player->teleporting_destination>=0) /* simple teleport */
				{
					if (player_index==current_player_index) 
					{
						start_teleporting_effect(TRUE);
					}
					play_object_sound(player->object_index, _snd_teleport_out);
				}
				else /* Level change */
				{
					short other_player_index;
				
					/* Everyone plays the teleporting effect out. */
					start_teleporting_effect(TRUE);
					play_object_sound(current_player->object_index, _snd_teleport_out);
					
					/* Every players object plays the sound, and everyones monster responds. */
					for (other_player_index= 0; other_player_index<dynamic_world->player_count; ++other_player_index)
					{
						player= get_player_data(other_player_index);

						/* Set them to be teleporting if the already aren’t, or if they are but it */
						/*  is a simple teleport (intralevel) */
						if (player_index!=other_player_index)
						{
							monster= get_monster_data(player->monster_index);
					
							/* Tell everyone else to use the teleporting junk... */
							SET_PLAYER_INTERLEVEL_TELEPORTING_STATUS(player, TRUE);
							player->interlevel_teleport_phase= 0;
						
							monster->action= _monster_is_teleporting;
						}
					}
				}
			}
		}
	}
	
	return;
}

static void update_player_media(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *object= get_object_data(monster->object_index);
	struct polygon_data *polygon= get_polygon_data(object->polygon);

	{
		short sound_type= NONE;
			
		if (player_index==current_player_index) set_fade_effect((player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ? get_media_submerged_fade_effect(polygon->media_index) : NONE);
	
		if (player->variables.flags&_FEET_BELOW_MEDIA_BIT)
		{
			// LP change: idiot-proofing
			struct media_data *media= get_media_data(polygon->media_index); // should be valid
			{
			world_distance current_magnitude= (player->variables.old_flags&_HEAD_BELOW_MEDIA_BIT) ? media->current_magnitude : (media->current_magnitude>>1);
			world_distance external_magnitude= FIXED_TO_WORLD(GUESS_HYPOTENUSE(ABS(player->variables.external_velocity.i), ABS(player->variables.external_velocity.j)));
			struct damage_definition *damage= get_media_damage(polygon->media_index, (player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ? FIXED_ONE : FIXED_ONE/4);
			
			// apply current if possible
			if (!PLAYER_IS_DEAD(player) && external_magnitude<current_magnitude) accelerate_player(player->monster_index, 0, NORMALIZE_ANGLE(media->current_direction-HALF_CIRCLE), media->current_magnitude>>2);
			
			// cause damage if possible
			if (damage) damage_player(player->monster_index, NONE, NONE, damage);
			
			// feet entering media sound
			if (!(player->variables.old_flags&_FEET_BELOW_MEDIA_BIT)) sound_type= _media_snd_feet_entering;
			// head entering media sound
			if (!(player->variables.old_flags&_HEAD_BELOW_MEDIA_BIT) && (player->variables.flags&_HEAD_BELOW_MEDIA_BIT)) sound_type= _media_snd_head_entering;
			// head leaving media sound
			if (!(player->variables.flags&_HEAD_BELOW_MEDIA_BIT) && (player->variables.old_flags&_HEAD_BELOW_MEDIA_BIT)) sound_type= _media_snd_head_leaving;
			}
		}
		else
		{
			// feet leaving media sound
			if (polygon->media_index!=NONE && (player->variables.old_flags&_FEET_BELOW_MEDIA_BIT)) sound_type= _media_snd_feet_leaving;
		}
		
		if (sound_type!=NONE)
		{
			play_object_sound(monster->object_index, get_media_sound(polygon->media_index, sound_type));
		}
	}

	if (player->variables.flags&_STEP_PERIOD_BIT)
	{
		short sound_index= NONE;
		
		if ((player->variables.flags&_FEET_BELOW_MEDIA_BIT) && !(player->variables.flags&_HEAD_BELOW_MEDIA_BIT))
		{
			sound_index= get_media_sound(polygon->media_index, _media_snd_splashing);
		}
		else
		{
			/* make ordinary walking sound */
		}
		
		if (sound_index!=NONE)
		{
			play_object_sound(monster->object_index, sound_index);
		}
	}
	
	return;
}

static void set_player_shapes(
	short player_index,
	boolean animate)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct physics_variables *variables= &player->variables;
	struct object_data *legs= get_object_data(monster->object_index);
	struct object_data *torso= get_object_data(legs->parasitic_object);
	shape_descriptor new_torso_shape, new_legs_shape;
	short transfer_mode, transfer_period;
	
	get_player_transfer_mode(player_index, &transfer_mode, &transfer_period);
	
	/* if we’re not dead, handle changing shapes (if we are dead, the correct dying shape has
		already been set and we just have to wait for the animation to finish) */
	if (!PLAYER_IS_DEAD(player))
	{
		short torso_shape;
		short mode, pseudo_weapon_type;
		
		get_player_weapon_mode_and_type(player_index, &pseudo_weapon_type, &mode);
		vassert(pseudo_weapon_type>=0 && pseudo_weapon_type<PLAYER_TORSO_SHAPE_COUNT, 
			csprintf(temporary, "Pseudo Weapon Type out of range: %d", pseudo_weapon_type));
		switch(mode)
		{
			case _shape_weapon_firing: torso_shape= player_shapes.firing_torsos[pseudo_weapon_type]; break;
			case _shape_weapon_idle: torso_shape= player_shapes.torsos[pseudo_weapon_type]; break;
			case _shape_weapon_charging: torso_shape= player_shapes.charging_torsos[pseudo_weapon_type]; break;
			default:
				// LP change:
				assert(false);
				// halt();
		}
		assert(player->variables.action>=0 && player->variables.action<NUMBER_OF_PLAYER_ACTIONS);
		
		new_legs_shape= BUILD_DESCRIPTOR(BUILD_COLLECTION(player_shapes.collection, player->team), player_shapes.legs[player->variables.action]);
		new_torso_shape= BUILD_DESCRIPTOR(BUILD_COLLECTION(player_shapes.collection, player->color), torso_shape);

		/* stuff in the transfer modes */
		if (legs->transfer_mode!=transfer_mode) legs->transfer_mode= transfer_mode, legs->transfer_period= transfer_period, legs->transfer_phase= 0;
		if (torso->transfer_mode!=transfer_mode) torso->transfer_mode= transfer_mode, torso->transfer_period= transfer_period, torso->transfer_phase= 0;
		
		/* stuff in new shapes only if they have changed (and reset phases if they have) */
		if (new_legs_shape!= legs->shape) legs->shape= new_legs_shape, legs->sequence= 0;
		if (new_torso_shape!=torso->shape) torso->shape= new_torso_shape, torso->sequence= 0;
	}
	
	if (animate)
	{
		/* animate the player only if we’re not airborne and not totally dead */
		if ((variables->action!=_player_airborne || (PLAYER_IS_TELEPORTING(player) || PLAYER_IS_INTERLEVEL_TELEPORTING(player)))&&!PLAYER_IS_TOTALLY_DEAD(player)) animate_object(monster->object_index);
		if (PLAYER_IS_DEAD(player) && !PLAYER_IS_TELEPORTING(player) && (GET_OBJECT_ANIMATION_FLAGS(legs)&_obj_last_frame_animated) && !PLAYER_IS_TOTALLY_DEAD(player))
		{
			/* we’ve finished the animation; let the player reincarnate if he wants to */
			SET_PLAYER_TOTALLY_DEAD_STATUS(player, TRUE);
			set_player_dead_shape(player_index, FALSE);

			/* If you had something cool, you don't anymore.. */
			remove_dead_player_items(player_index);
		}
	}

	return;
}

/* We can rebuild him!! */
static void revive_player(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_location location;
	struct object_data *object;
	short team;

	/* Figure out where the player starts */
	team= calculate_player_team(player->team);
	get_random_player_starting_location_and_facing(dynamic_world->player_count, team, &location);

	monster->action= _monster_is_moving; /* was probably _dying or something */

	/* remove only the player’s torso, which should be invisible anyway, and turn his legs
		into garbage */
	remove_parasitic_object(monster->object_index);
	turn_object_to_shit(monster->object_index);

	/* create a new pair of legs, and (completely behind MONSTERS.C’s back) reattach it to
		it’s monster (shape will be set by set_player_shapes, below) */
	player->object_index= monster->object_index= new_map_object(&location, 0);
	object= get_object_data(monster->object_index);
	SET_OBJECT_SOLIDITY(object, TRUE);
	SET_OBJECT_OWNER(object, _object_is_monster);
	object->permutation= player->monster_index;
	
	/* create a new torso (shape will be set by set_player_shapes, below) */
	attach_parasitic_object(monster->object_index, 0, location.yaw);

	initialize_player_physics_variables(player_index);

	player->weapon_intensity_decay= 0;
	player->suit_energy= PLAYER_MAXIMUM_SUIT_ENERGY;
	player->suit_oxygen= PLAYER_MAXIMUM_SUIT_OXYGEN;
	SET_PLAYER_DEAD_STATUS(player, FALSE);
	SET_PLAYER_TOTALLY_DEAD_STATUS(player, FALSE);
	SET_PLAYER_TELEPORTING_STATUS(player, FALSE);
	SET_PLAYER_INTERLEVEL_TELEPORTING_STATUS(player, FALSE);
	player->invincibility_duration= 0;
	player->invisibility_duration= 0;
	player->infravision_duration= 0;
	player->extravision_duration= 0;
	player->control_panel_side_index= NONE; // not using a control panel.

	give_player_initial_items(player_index);

	/* set the correct shapes and transfer mode */
	set_player_shapes(player_index, FALSE);

	try_and_strip_player_items(player_index);

	/* Update the interface to reflect your player's changed status */
	if (player_index==current_player_index) update_interface(NONE); 
	
	// LP addition: handles the current player's chase cam;
	// in screen.c, we find that it's the current player whose view gets rendered
	if (player_index == current_player_index) ChaseCam_Reset();
	
	// LP addition: set field-of-view approrpriately
	if (player_index == current_player_index) ResetFieldOfView();
	
	return;
}

/* The player just changed map levels, recreate him, and all of the objects */
/*  associated with him. */
static void recreate_player(
	short player_index)
{
	short monster_index;
	struct monster_data *monster;
	struct player_data *player= get_player_data(player_index);
	short placement_team;
	struct object_location location;
	boolean  player_teleported_dead= FALSE;
	
	/* Determine the location */
	placement_team= calculate_player_team(player->team);
	get_random_player_starting_location_and_facing(player_index, placement_team, &location);

	/* create an object and a monster for this player */
	monster_index= new_monster(&location, _monster_marine);
	monster= get_monster_data(monster_index);

	/* add our parasitic torso */
	attach_parasitic_object(monster->object_index, 0, location.yaw);
	
	/* and initialize it */
	if(PLAYER_IS_TOTALLY_DEAD(player) || PLAYER_IS_DEAD(player))
	{
		player_teleported_dead= TRUE;
	}

//	player->flags= 0;
	player->flags &= (_player_has_cheated_flag | _player_is_teleporting_flag | _player_is_interlevel_teleporting_flag); /* Player has cheated persists. */
	player->monster_index= monster_index;
	player->object_index= monster->object_index;

	/* initialize_player_physics_variables sets all of these */
	player->facing= player->elevation= 0;
	player->location.x= player->location.y= player->location.z= 0;
	player->camera_location.x= player->camera_location.y= player->camera_location.z= 0;

	/* We don't change... */
	/* physics_model, suit_energy, suit_oxygen, current_weapon, desired_weapon */
	/* None of the weapons array data... */
	/* None of the items array data.. */
	/* The inventory offset/dirty flags.. */
	mark_player_inventory_screen_as_dirty(player_index, _weapon);

	/* Nuke the physics */
	memset(&player->variables, 0, sizeof(struct physics_variables));

	/* Reset the player weapon data and the physics variable.. (after updating player_count) */
	initialize_player_physics_variables(player_index);
	set_player_shapes(player_index, FALSE);

	player->control_panel_side_index = NONE; // not using a control panel.
	initialize_player_terminal_info(player_index);

#ifdef OBSOLETE
	/* If the player transported dead.. */
	if(player_needs_weapons)
	{
		initialize_player_weapons(player_index);
	
		/* give the player his initial items */
		give_player_initial_items(player_index);
	}
#endif

	try_and_strip_player_items(player_index);

	if(player_teleported_dead)
	{
		kill_player(player_index, NONE, _monster_is_dying_soft);
	}
	
	// LP addition: handles the current player's chase cam;
	// in screen.c, we find that it's the current player whose view gets rendered
	if (player_index == current_player_index) ChaseCam_Reset();
	
	return;
}

static void kill_player(
	short player_index,
	short aggressor_player_index,
	short action)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *legs= get_object_data(monster->object_index);
	struct object_data *torso= get_object_data(legs->parasitic_object);

	/* discharge any of our weapons which were holding charges */
	discharge_charged_weapons(player_index);
	initialize_player_weapons(player_index);

	/* make our legs ownerless scenery, mark our monster as dying, stuff in the right dying shape */
	SET_OBJECT_OWNER(legs, _object_is_normal);
	monster->action= action;
	monster_died(player->monster_index);
	set_player_dead_shape(player_index, TRUE);
	
	/* make our torso invisible */
	SET_OBJECT_INVISIBILITY(torso, TRUE);

	/* make our player dead */
	SET_PLAYER_DEAD_STATUS(player, TRUE);

	player->reincarnation_delay= MINIMUM_REINCARNATION_DELAY;
	if (GET_GAME_OPTIONS()&_dying_is_penalized) player->reincarnation_delay+= NORMAL_REINCARNATION_DELAY;
	if (aggressor_player_index==player_index && (GET_GAME_OPTIONS()&_suicide_is_penalized)) player->reincarnation_delay+= SUICIDE_REINCARNATION_DELAY;

	kill_player_physics_variables(player_index);

	return;
}

static void give_player_initial_items(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	short loop;

	for(loop= 0; loop<NUMBER_OF_PLAYER_INITIAL_ITEMS; ++loop)
	{
		/* Get the item.. */
		assert(player_initial_items[loop]>=0 && player_initial_items[loop]<NUMBER_OF_ITEMS);

		if(player->items[player_initial_items[loop]]==NONE)
		{
			player->items[player_initial_items[loop]]= 1;
		} else {
			player->items[player_initial_items[loop]]+= 1;
		}
		
		process_new_item_for_reloading(player_index, player_initial_items[loop]);
	}
	
	return;
}

static void remove_dead_player_items(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	short item_type;
	short i;

	// subtract all initial items	
	for (i= 0; i<NUMBER_OF_PLAYER_INITIAL_ITEMS; ++i)
	{
		if (player->items[player_initial_items[i]]>0)
		{
			player->items[player_initial_items[i]]-= 1;
		}
	}

	// drop any balls
	{
		short ball_color= find_player_ball_color(player_index);
		
		if (ball_color!=NONE)
		{
			short item_type= BALL_ITEM_BASE + ball_color;
			
			// Benad: using location and supporting_polygon_index instead of
			// camera_location and camera_polygon_index... D'uh...
			drop_the_ball(&player->location, player->supporting_polygon_index, 
				player->monster_index, _monster_marine, item_type);
			player->items[item_type]= NONE;
		}
	}

	for (item_type= 0; item_type<NUMBER_OF_ITEMS; ++item_type)
	{
		short item_count= player->items[item_type];
		// START Benad
		if ((item_type >= BALL_ITEM_BASE) && (item_type < BALL_ITEM_BASE+MAXIMUM_NUMBER_OF_PLAYERS))
			continue;
		// END Benad
		while ((item_count-= 1)>=0)
		{
			short item_kind= get_item_kind(item_type);
			boolean dropped= FALSE;
			
			// if we’re not set to burn items or this is an important item (i.e., repair chip) drop it
			if (!(GET_GAME_OPTIONS()&_burn_items_on_death) ||
				(item_kind==_item && dynamic_world->player_count>1))
			{
				if (item_kind!=_ammunition || !(global_random()&1))
				{
					world_point3d random_point;
					short random_polygon_index;
					short retries= 5;
					
					do
					{
						random_point_on_circle(&player->location, player->supporting_polygon_index, WORLD_ONE_FOURTH, &random_point, &random_polygon_index);
					}
					while (random_polygon_index==NONE && --retries);
					
					if (random_polygon_index!=NONE)
					{
						struct object_location location;

						location.polygon_index= random_polygon_index;
						location.p.x= random_point.x, location.p.y= random_point.y, location.p.z= 0;
						location.yaw= 0;
						location.flags= 0;
						new_item(&location, item_type);
						
						dropped= TRUE;
					}
				}
			}
			
			if (!dropped) object_was_just_destroyed(_object_is_item, item_type);
		}

		player->items[item_type]= NONE;
	}

	mark_player_inventory_as_dirty(player_index, NONE);

	return;
}

static void get_player_transfer_mode(
	short player_index,
	short *transfer_mode,
	short *transfer_period)
{
	struct player_data *player= get_player_data(player_index);
	short duration= 0;

	*transfer_period= 1;
	*transfer_mode= NONE;
	if (PLAYER_IS_TELEPORTING(player))
	{
		*transfer_mode= player->teleporting_phase<PLAYER_TELEPORTING_MIDPOINT ? _xfer_fold_out : _xfer_fold_in;
		*transfer_period= PLAYER_TELEPORTING_MIDPOINT+1;
	} 
	else if (PLAYER_IS_INTERLEVEL_TELEPORTING(player))
	{
		*transfer_mode= player->interlevel_teleport_phase<PLAYER_TELEPORTING_MIDPOINT ? _xfer_fold_out : _xfer_fold_in;
		*transfer_period= PLAYER_TELEPORTING_MIDPOINT+1;
	}
	else
	{
		if (player->invincibility_duration) 
		{
			*transfer_mode= _xfer_static;
			duration= player->invincibility_duration;
		}
		else
		{
			if (player->invisibility_duration) 
			{
				*transfer_mode= player->invisibility_duration>kINVISIBILITY_DURATION ? _xfer_subtle_invisibility : _xfer_invisibility;
				duration= player->invisibility_duration;
			}
		}
		
		if (duration && duration<10*TICKS_PER_SECOND)
		{
			switch (duration/(TICKS_PER_SECOND/6))
			{
				case 46: case 37: case 29: case 22: case 16: case 11: case 7: case 4: case 2:
					*transfer_mode= (player->invincibility_duration && player->invisibility_duration) ?
						(player->invisibility_duration>kINVISIBILITY_DURATION ? _xfer_subtle_invisibility : _xfer_invisibility) : NONE;
					break;
			}
		}
	}
	
	return;
}

static void set_player_dead_shape(
	short player_index,
	boolean dying)
{
	struct player_data *player= get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	short shape;
	
	if (monster->action==_monster_is_dying_flaming)
	{
		shape= dying ? FLAMING_DYING_SHAPE : FLAMING_DEAD_SHAPE;
	}
	else
	{
		if (dying)
		{
			shape= (monster->action==_monster_is_dying_hard) ? player_shapes.dying_hard : player_shapes.dying_soft;
		}
		else
		{
			shape= (monster->action==_monster_is_dying_hard) ? player_shapes.dead_hard : player_shapes.dead_soft;
		}
	
		shape= BUILD_DESCRIPTOR(BUILD_COLLECTION(player_shapes.collection, player->team), shape);
	}

	if (dying)
	{
		set_object_shape_and_transfer_mode(monster->object_index, shape, NONE);
	}
	else
	{
		randomize_object_sequence(monster->object_index, shape);
	}
	
	return;
}

static short calculate_player_team(
	short base_team)
{
	short team;

	/* Starting locations are based on the team type. */
	switch(GET_GAME_TYPE())
	{
		case _game_of_kill_monsters:
		case _game_of_cooperative_play:
		case _game_of_tag:
		case _game_of_king_of_the_hill:
		case _game_of_kill_man_with_ball:
			team= NONE;
			break;

		case _game_of_defense:
		case _game_of_rugby:						
		case _game_of_capture_the_flag:
			// START Benad
			if ((dynamic_world->game_information.kill_limit == 819) ||
				((GET_GAME_TYPE() == _game_of_defense) &&
				(dynamic_world->game_information.kill_limit == 1080))) // 1080 seconds, or 18:00...
				team= NONE;
			else
				team= base_team;
				// vassert(false, csprintf(temporary, "Kill limit: %d", dynamic_world->game_information.kill_limit));
			// END Benad
			break;
	}
	
	return team;
}	

// #define STRIPPED_ENERGY (PLAYER_MAXIMUM_SUIT_ENERGY/4)
static void try_and_strip_player_items(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	short item_type;
	
	if (static_world->environment_flags&_environment_rebellion)
	{
		for (item_type= 0; item_type<NUMBER_OF_ITEMS; ++item_type)
		{
			switch (item_type)
			{
				case _i_knife:
					break;
				
				default:
					player->items[item_type]= NONE;
					break;
			}
		}
	
		mark_player_inventory_as_dirty(player_index, NONE);
		initialize_player_weapons(player_index);
		
		// LP change: using variable for this
		if (player->suit_energy>StrippedEnergy) player->suit_energy= StrippedEnergy;
		// if (player->suit_energy>STRIPPED_ENERGY) player->suit_energy= STRIPPED_ENERGY;
	}
	
	return;
}


class XML_StartItemParser: public XML_ElementParser
{
	int Index;
	short Type;
	
	// What is present?
	bool IndexPresent;
	enum {NumberOfValues = 1};
	bool IsPresent[NumberOfValues];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_StartItemParser(): XML_ElementParser("item") {}
};

bool XML_StartItemParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<NumberOfValues; k++)
		IsPresent[k] = false;
	
	return true;
}

bool XML_StartItemParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,int(0),int(NUMBER_OF_PLAYER_INITIAL_ITEMS-1)))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"type") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Type,short(0),short(NUMBER_OF_DEFINED_ITEMS-1)))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StartItemParser::AttributesDone()
{
	// Verify...
	if (!IndexPresent || !IsPresent[0])
	{
		AttribsMissing();
		return false;
	}
	player_initial_items[Index] = Type;
			
	return true;
}

static XML_StartItemParser StartItemParser;


class XML_PlayerDamageParser: public XML_ElementParser
{
	int Index;
	damage_response_definition Data;
	
	// What is present?
	bool IndexPresent;
	enum {NumberOfValues = 5};
	bool IsPresent[NumberOfValues];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_PlayerDamageParser(): XML_ElementParser("damage") {}
};

bool XML_PlayerDamageParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<NumberOfValues; k++)
		IsPresent[k] = false;
	
	return true;
}

bool XML_PlayerDamageParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%d",Index,int(0),int(NUMBER_OF_DAMAGE_RESPONSE_DEFINITIONS-1)))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"threshold") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.damage_threshhold))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"fade") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.fade))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"sound") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.sound))
		{
			IsPresent[2] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"death_sound") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.death_sound))
		{
			IsPresent[3] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"death_action") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",Data.death_action))
		{
			IsPresent[4] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_PlayerDamageParser::AttributesDone()
{
	// Verify...
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}
	damage_response_definition& OrigData = damage_response_definitions[Index];
	
	if (IsPresent[0]) OrigData.damage_threshhold = Data.damage_threshhold;
	if (IsPresent[1]) OrigData.fade = Data.fade;
	if (IsPresent[2]) OrigData.sound = Data.sound;
	if (IsPresent[3]) OrigData.death_sound = Data.death_sound;
	if (IsPresent[4]) OrigData.death_action = Data.death_action;
		
	return true;
}

static XML_PlayerDamageParser PlayerDamageParser;



class XML_PowerupParser: public XML_ElementParser
{
	
public:
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_PowerupParser(): XML_ElementParser("powerup") {}
};

bool XML_PowerupParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"invisibility") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",kINVISIBILITY_DURATION,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"invincibility") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",kINVINCIBILITY_DURATION,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"extravision") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",kEXTRAVISION_DURATION,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"infravision") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",kINFRAVISION_DURATION,short(0),short(SHORT_MAX)));
	}
	UnrecognizedTag();
	return false;
}

static XML_PowerupParser PowerupParser;



class XML_PlayerParser: public XML_ElementParser
{
	
public:
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_PlayerParser(): XML_ElementParser("player") {}
};

bool XML_PlayerParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"energy") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",InitialEnergy,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"oxygen") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",InitialOxygen,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"stripped") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",StrippedEnergy,short(0),short(SHORT_MAX)));
	}
	else if (strcmp(Tag,"light") == 0)
	{
		float Luminosity;
		if (ReadBoundedNumericalValue(Value,"%f",Luminosity,float(0),float(SHORT_MAX)))
		{
			PlayerSelfLuminosity = long(FIXED_ONE*Luminosity + 0.5);
			return true;
		}
		else
			return false;
	}
	else if (strcmp(Tag,"oxygen_deplete") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",OxygenDepletion));
	}
	else if (strcmp(Tag,"oxygen_replenish") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",OxygenReplenishment));
	}
	else if (strcmp(Tag,"vulnerability") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",Vulnerability,short(NONE),short(NUMBER_OF_DAMAGE_TYPES-1)));
	}
	UnrecognizedTag();
	return false;
}

static XML_PlayerParser PlayerParser;


// XML-parser support
XML_ElementParser *Player_GetParser()
{
	PlayerParser.AddChild(&StartItemParser);
	PlayerParser.AddChild(&PlayerDamageParser);
	PlayerParser.AddChild(&PowerupParser);

	return &PlayerParser;
}


/*
MARATHON.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Friday, December 3, 1993 10:00:32 AM

Monday, September 5, 1994 2:42:28 PM (ajr)
	fixed kill_limit.
Saturday, September 17, 1994 6:04:59 PM   (alain)
	fixed autotriggering of platforms
Thursday, December 8, 1994 3:58:12 PM  (Jason)
	only players trigger platforms.

Feb 6, 2000 (Loren Petrich):
	Added typecode initialization

Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits initialization

Feb 15, 2000 (Loren Petrich):
	Added item-initialization and item-animation stuff

Mar 12, 2000 (Loren Petrich):
	Added OpenGL initializer

May 11, 2000 (Loren Petrich):
	Rewrote to get rid of dynamic-limit and animated-texture initializers;
	also used new animated-texture update function.

June 15, 2000 (Loren Petrich):
	Added support for Chris Pruett's Pfhortran

Aug 10, 2000 (Loren Petrich):
	Added Chris Pruett's Pfhortran changes
*/

#include "cseries.h"
#include "map.h"
#include "render.h"
#include "interface.h"
#include "flood_map.h"
#include "effects.h"
#include "monsters.h"
#include "projectiles.h"
#include "player.h"
#include "network.h"
#include "scenery.h"
#include "platforms.h"
#include "lightsource.h"
#include "media.h"
#include "music.h"
#include "fades.h"
#include "items.h"
#include "weapons.h"
#include "game_window.h"
#include "mysound.h"
#include "network_games.h"
// LP additions:
#include "tags.h"
#include "AnimatedTextures.h"
#include "ChaseCam.h"
#include "OGL_Setup.h"
// #include "items.h" -- already implicitly included

// CP additions:
#include "scripting.h"
#include "script_parser.h"

#include <limits.h>

#ifdef env68k
#pragma segment marathon
#endif

/* ---------- constants */

/* ---------- private prototypes */

static void game_timed_out(void);

static void load_all_game_sounds(short environment_code);

/* ---------- code */

void initialize_marathon(
	void)
{
#ifndef DEMO /* no external physics models for demo */
//	import_definition_structures();
#endif
	
	build_trig_tables();
	allocate_map_memory();
	// Rendering and flood-map done when starting a level,
	// since they require map-geometry sizes
	// allocate_render_memory();
	allocate_pathfinding_memory();
	// allocate_flood_map_memory();
	allocate_texture_tables();
	initialize_weapon_manager();
	initialize_game_window();
	initialize_scenery();
	// LP additions:
	initialize_items();
#ifdef HAVE_OPENGL
	OGL_Initialize();
#endif
	// CP addition: init pfhortran
	init_pfhortran();

	return;
}

short update_world(
	void)
{
	short lowest_time, highest_time;
	short i, time_elapsed;
	short player_index;
	bool game_over= false;

	/* find who has the most and the least queued action flags (we can only advance the world
		as far as we have action flags for every player).  the difference between the most and
		least queued action flags should be bounded in some way by the maximum number of action
		flags we can generate locally at interrupt time. */
	highest_time= SHRT_MIN, lowest_time= SHRT_MAX;
	for (player_index= 0;player_index<dynamic_world->player_count; ++player_index)
	{
		short queue_size;

		if (game_is_networked)
		{
			queue_size= MIN(get_action_queue_size(player_index), NetGetNetTime() - dynamic_world->tick_count);
		}
		else
		{
			queue_size= MIN(get_action_queue_size(player_index), get_heartbeat_count() - dynamic_world->tick_count);
		}
		if (queue_size<0) queue_size= 0; // thumb in dike to prevent update_interface from getting -1
		
		if (queue_size>highest_time) highest_time= queue_size;
		if (queue_size<lowest_time) lowest_time= queue_size;
	}

	time_elapsed= lowest_time;
	for (i=0;i<time_elapsed;++i)
	{
		//CP Addition: Scripting handling stuff
		bool success = true;
		if (script_in_use() && success /*&& instruction_finished()*/)
		{
			success = do_next_instruction();
		}
		
		update_lights();
		update_medias();
		update_platforms();
		
		update_control_panels(); // don't put after update_players
		update_players();
		move_projectiles();
		move_monsters();
		update_effects();
		recreate_objects();
		
		handle_random_sound_image();
		animate_scenery();
		// LP additions:
		animate_items();
		AnimTxtr_Update();
		ChaseCam_Update();

		update_net_game();

		if(check_level_change()) 
		{
			time_elapsed= 0; /* So that we don't update things & try to render the world. */
			break; /* Break if we change the level */
		}
		game_over= game_is_over();
		if(game_over)
			break;
		
		dynamic_world->tick_count+= 1;
		dynamic_world->game_information.game_time_remaining-= 1;
	}

	/* Game is over. */
	if(game_over) 
	{
		game_timed_out();
		time_elapsed= 0;
	} 
	else if (time_elapsed)
	{
		update_interface(time_elapsed);
		update_fades();
	}
	
	check_recording_replaying();

	return time_elapsed;
}

/* call this function before leaving the old level, but DO NOT call it when saving the player.
	it should be called when you're leaving the game (i.e., quitting or reverting, etc.) */
void leaving_map(
	void)
{
	
	remove_all_projectiles();
	remove_all_nonpersistent_effects();
	
	/* mark our shape collections for unloading */
	mark_environment_collections(static_world->environment_code, false);
	mark_all_monster_collections(false);
	mark_player_collections(false);

	/* all we do is mark them for unloading, we don't explicitly dispose of them; whenever the
		next level is loaded someone (probably entering_map, below) will call load_collections()
		and the stuff we marked as needed to be ditched will be */
	
	/* stop counting world ticks */
//	set_keyboard_controller_status(false);

#ifdef WIN32
    // Hackish. Should probably be in stop_all_sounds(), but that just
    // doesn't work out. 
    StopLevelMusic(); 
#endif
	stop_all_sounds();

	return;
}

/* call this function after the new level has been completely read into memory, after
	player->location and player->facing have been updated, and as close to the end of
	the loading process in general as possible. */
// LP: added whether a savegame is being restored (skip Pfhortran init if that's the case)
bool entering_map(bool restoring_saved)
{
	bool success= true;

	set_fade_effect(NONE);

	/* if any active monsters think they have paths, we'll make them reconsider */
	initialize_monsters_for_new_level();

	/* and since no monsters have paths, we should make sure no paths think they have monsters */
	reset_paths();
	
	/* mark our shape collections for loading and load them */
	mark_environment_collections(static_world->environment_code, true);
	mark_all_monster_collections(true);
	mark_player_collections(true);
	load_collections();

	load_all_monster_sounds();
	load_all_game_sounds(static_world->environment_code);

	/* tell the keyboard controller to start recording keyboard flags */
	if (game_is_networked) success= NetSync(); /* make sure everybody is ready */
	
	/* make sure nobodyÕs holding a weapon illegal in the new environment */
	check_player_weapons_for_environment_change();

	if (dynamic_world->player_count>1) initialize_net_game();	
	randomize_scenery_shapes();

	reset_player_queues(); //¦¦
	//CP Addition: Run startup script (if available)
	script_init(restoring_saved);
//	sync_heartbeat_count();
//	set_keyboard_controller_status(true);

	if (!success) leaving_map();

	return success;
}

/* This is called when an object of some mass enters a polygon from another */
/* polygon.  It handles triggering lightsources, platforms, and whatever */
/* else it is that we can think of. */
void changed_polygon(
	short original_polygon_index,
	short new_polygon_index,
	short player_index)
{
	struct polygon_data *new_polygon= get_polygon_data(new_polygon_index);
	struct player_data *player= player_index!=NONE ? get_player_data(player_index) : (struct player_data *) NULL;
	
	(void) (original_polygon_index);
	
	/* Entering this polygon.. */
	switch (new_polygon->type)
	{
		case _polygon_is_visible_monster_trigger:
			if (player)
			{
				activate_nearby_monsters(player->monster_index, player->monster_index,
					_pass_solid_lines|_activate_deaf_monsters|_use_activation_biases|_activation_cannot_be_avoided);
			}
			break;
		case _polygon_is_invisible_monster_trigger:
		case _polygon_is_dual_monster_trigger:
			if (player)
			{
				activate_nearby_monsters(player->monster_index, player->monster_index,
					_pass_solid_lines|_activate_deaf_monsters|_activate_invisible_monsters|_use_activation_biases|_activation_cannot_be_avoided);
			}
			break;
		
		case _polygon_is_item_trigger:
			if (player)
			{
				trigger_nearby_items(new_polygon_index);
			}
			break;

		case _polygon_is_light_on_trigger:
		case _polygon_is_light_off_trigger:
			set_light_status(new_polygon->permutation,
				new_polygon->type==_polygon_is_light_off_trigger ? false : true);
			break;
			
		case _polygon_is_platform:
			platform_was_entered(new_polygon->permutation, player ? true : false);
			break;
		case _polygon_is_platform_on_trigger:
		case _polygon_is_platform_off_trigger:
			if (player)
			{
				try_and_change_platform_state(get_polygon_data(new_polygon->permutation)->permutation,
					new_polygon->type==_polygon_is_platform_off_trigger ? false : true);
			}
			break;
			
		case _polygon_must_be_explored:
			/* When a player enters a must be explored, it now becomes a normal polygon, to allow */
			/*  for must be explored flags to work across cooperative net games */
			if(player)
			{
				new_polygon->type= _polygon_is_normal;
			}
			break;
			
		default:
			break;
	}

	return;
}

/* _level_failed is the same as _level_finished but indicates a non-fatal failure condition (e.g.,
	too many civilians died during _mission_rescue) */
short calculate_level_completion_state(
	void)
{
	short completion_state= _level_finished;
	
	/* if there are any monsters left on an extermination map, we havenÕt finished yet */
	if (static_world->mission_flags&_mission_extermination)
	{
		if (live_aliens_on_map()) completion_state= _level_unfinished;
	}
	
	/* if there are any polygons which must be explored and have not been entered, weÕre not done */
	if (static_world->mission_flags&_mission_exploration)
	{
		short polygon_index;
		struct polygon_data *polygon;
		
		for (polygon_index= 0, polygon= map_polygons; polygon_index<dynamic_world->polygon_count; ++polygon_index, ++polygon)
		{
			if (polygon->type==_polygon_must_be_explored)
			{
				completion_state= _level_unfinished;
				break;
			}
		}
	}
	
	/* if there are any items left on this map, weÕre not done */
	if (static_world->mission_flags&_mission_retrieval)
	{
		if (unretrieved_items_on_map()) completion_state= _level_unfinished;
	}
	
	/* if there are any untoggled repair switches on this level then weÕre not there */
	if (static_world->mission_flags&_mission_repair)
	{
		if (untoggled_repair_switches_on_level()) completion_state= _level_unfinished;
	}

	/* if weÕve finished the level, check failure conditions */
	if (completion_state==_level_finished)
	{
		/* if this is a rescue mission and more than half of the civilians died, the mission failed */
		if (static_world->mission_flags&_mission_rescue &&
			dynamic_world->current_civilian_causalties>dynamic_world->current_civilian_count/2)
		{
			completion_state= _level_failed;
		}
	}
	
	return completion_state;
}

short calculate_damage(
	struct damage_definition *damage)
{
	short total_damage= damage->base + (damage->random ? global_random()%damage->random : 0);
	
	total_damage= FIXED_INTEGERAL_PART(total_damage*damage->scale);
	
	/* if this damage was caused by an alien modify it for the current difficulty level */
	if (damage->flags&_alien_damage)
	{
		switch (dynamic_world->game_information.difficulty_level)
		{
			case _wuss_level: total_damage-= total_damage>>1; break;
			case _easy_level: total_damage-= total_damage>>2; break;
			/* harder levels do not cause more damage */
		}
	}
	
	return total_damage;
}

#define MINOR_OUCH_FREQUENCY 0xf
#define MAJOR_OUCH_FREQUENCY 0x7
#define MINOR_OUCH_DAMAGE 15
#define MAJOR_OUCH_DAMAGE 7

// LP temp fix: assignments are intended to approximate Marathon 1 (minor = lava, major = PfhorSlime)
#define _damage_polygon _damage_lava
#define _damage_major_polygon _damage_goo

void cause_polygon_damage(
	short polygon_index,
	short monster_index)
{
	struct polygon_data *polygon= get_polygon_data(polygon_index);
	struct monster_data *monster= get_monster_data(monster_index);
	struct object_data *object= get_object_data(monster->object_index);

// #if 0
	if ((polygon->type==_polygon_is_minor_ouch && !(dynamic_world->tick_count&MINOR_OUCH_FREQUENCY) && object->location.z==polygon->floor_height) ||
		(polygon->type==_polygon_is_major_ouch && !(dynamic_world->tick_count&MAJOR_OUCH_FREQUENCY)))
	{
		struct damage_definition damage;
		
		damage.flags= _alien_damage;
		damage.type= polygon->type==_polygon_is_minor_ouch ? _damage_polygon : _damage_major_polygon;
		damage.base= polygon->type==_polygon_is_minor_ouch ? MINOR_OUCH_DAMAGE : MAJOR_OUCH_DAMAGE;
		damage.random= 0;
		damage.scale= FIXED_ONE;
		
		damage_monster(monster_index, NONE, NONE, (world_point3d *) NULL, &damage);
	}
// #endif
	
	return;
}

/* ---------- private code */
	
/* They ran out of time.  This means different things depending on the */
/* type of game.. */
static void game_timed_out(
	void)
{
	if(player_controlling_game())
	{
		set_game_state(_close_game);
	} else {
		set_game_state(_switch_demo);
	}
}


// LP: suppressed this as superfluous; won't try to reassign these sounds for M1 compatibility
static void load_all_game_sounds(
	short environment_code)
{
}

/*
#define NUMBER_OF_PRELOAD_SOUNDS (sizeof(preload_sounds)/sizeof(short))
static short preload_sounds[]=
{
	_snd_teleport_in,
	_snd_teleport_out,
	_snd_bullet_ricochet,
	_snd_magnum_firing,
	_snd_assault_rifle_firing,
	_snd_body_falling,
	_snd_body_exploding,
	_snd_bullet_hitting_flesh
};

#define NUMBER_OF_PRELOAD_SOUNDS0 (sizeof(preload_sounds0)/sizeof(short))
static short preload_sounds0[]= {_snd_water, _snd_wind};

#define NUMBER_OF_PRELOAD_SOUNDS1 (sizeof(preload_sounds1)/sizeof(short))
static short preload_sounds1[]= {_snd_lava, _snd_wind};

static void load_all_game_sounds(
	short environment_code)
{
	load_sounds(preload_sounds, NUMBER_OF_PRELOAD_SOUNDS);

	switch (environment_code)
	{
		case 0: load_sounds(preload_sounds0, NUMBER_OF_PRELOAD_SOUNDS0); break;
		case 1: load_sounds(preload_sounds1, NUMBER_OF_PRELOAD_SOUNDS1); break;
		case 2: break;
		case 3: break;
		case 4: break;
	}
	
	return;
}
*/

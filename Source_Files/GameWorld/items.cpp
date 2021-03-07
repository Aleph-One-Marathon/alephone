/*
ITEMS.C

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

Monday, January 3, 1994 10:06:08 PM

Monday, September 5, 1994 2:17:43 PM
	razed.
Friday, October 21, 1994 3:44:11 PM
	changed inventory updating mechanism, added maximum counts of items.
Wednesday, November 2, 1994 3:49:57 PM (Jason)
	object_was_just_destroyed is now called immediately on powerups.
Tuesday, January 31, 1995 1:24:10 PM  (Jason')
	can only hold unlimited ammo on total carnage (not everything)
Wednesday, October 11, 1995 3:10:34 PM  (Jason)
	network-only items

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 15, 2000 (Loren Petrich):
	Added item-animation handling
	Non-animated items ought to be randomized, but one problem is that
	randomize_object_sequence() only works when the shapes are loaded,
	and the shapes are usually not loaded when the map items are created.

May 16, 2000 (Loren Petrich):
	Added XML support for configuring various item features

May 26, 2000 (Loren Petrich):
	Added XML shapes support

Jul 1, 2000 (Loren Petrich):
	Did some inlining of the item-definition accessor
	
	Added Benad's netgame-type changes

Aug 10, 2000 (Loren Petrich):
	Added Chris Pruett's Pfhortran changes

Feb 11, 2001 (Loren Petrich):
	Reversed the "polarity" of the "facing" member of "object",
	which is used as a flag in the case of randomized unanimated objects.
	It will become NONE when these objects are inited.
*/

#include "cseries.h"

#include "map.h"
#include "interface.h"
#include "monsters.h"
#include "player.h"
#include "SoundManager.h"
#include "platforms.h"
#include "fades.h"
#include "FilmProfile.h"
#include "items.h"
#include "flood_map.h"
#include "effects.h"
#include "game_window.h"
#include "weapons.h" /* needed for process_new_item_for_reloading */
#include "network_games.h"
#include "InfoTree.h"

// LP addition: for the XML stuff
#include <string.h>
#include <limits.h>

//MH: Lua scripting
#include "lua_script.h"

/* ---------- structures */

#define strITEM_NAME_LIST 150
#define strHEADER_NAME_LIST 151

#define MAXIMUM_ARM_REACH (3*WORLD_ONE_FOURTH)

/* ---------- private prototypes */

/* ---------- globals */

#include "item_definitions.h"

/* ---------- private prototypes */

// Item-definition accessor
static item_definition *get_item_definition(
	const short type);

static bool get_item(short player_index, short object_index);

static bool test_item_retrieval(short polygon_index1, world_point3d *location1, world_point3d *location2);

static int32 item_trigger_cost_function(short source_polygon_index, short line_index,
	short destination_polygon_index, void *unused);

/* ---------- code */

// Item-definition accessor
item_definition *get_item_definition(
	const short type)
{
	return GetMemberWithBounds(item_definitions,type,NUMBER_OF_DEFINED_ITEMS);
}

//a non-inlined version for external use
item_definition *get_item_definition_external(
	const short type)
{
	return get_item_definition(type);
}

int16 item_definition::get_maximum_count_per_player(bool is_m1, int difficulty_level) const
{
	if (has_extended_maximum_count[difficulty_level])
	{
		return extended_maximum_count[difficulty_level];
	}
	else
	{
		if (difficulty_level == _total_carnage_level &&
			(is_m1 || item_kind == _ammunition))
		{
			return INT16_MAX;
		}
		else
		{
			return maximum_count_per_player;
		}
	}
}

short new_item(
	struct object_location *location,
	short type)
{
	short object_index;
	struct item_definition *definition= get_item_definition(type);
	// LP change: added idiot-proofing
	if (!definition) return false;
	
	bool add_item= true;

	assert(sizeof(item_definitions)/sizeof(struct item_definition)==NUMBER_OF_DEFINED_ITEMS);

	/* Do NOT add items that are network-only in a single player game, and vice-versa */
	if (dynamic_world->player_count>1)
	{
		if (definition->invalid_environments & _environment_network) add_item= false;
		if (get_item_kind(type)==_ball && !current_game_has_balls()) add_item= false;
	} 
	else
	{
		if (definition->invalid_environments & _environment_single_player) add_item= false;
	}

	if (add_item)
	{
		/* add the object to the map */
		object_index= new_map_object(location, definition->base_shape);
		if (object_index!=NONE)
		{
			struct object_data *object= get_object_data(object_index);
			
			// LP addition: using the facing direction as a flag in the "unanimated" case:
			// will be initially zero, but will become nonzero when initialized,
			// so that the shape randomization will be done only once.
			
			SET_OBJECT_OWNER(object, _object_is_item);
			object->permutation= type;
			
			if ((location->flags&_map_object_is_network_only) && dynamic_world->player_count<=1)
			{
//				dprintf("killed #%d;g;", type);
				SET_OBJECT_INVISIBILITY(object, true);
				object->permutation= NONE;
			}
			else if ((get_item_kind(type) == _ball) && !static_world->ball_in_play)
			{
				static_world->ball_in_play = true;
				SoundManager::instance()->PlayLocalSound(_snd_got_ball);
			}
			
			/* let PLACEMENT.C keep track of how many there are */
			object_was_just_added(_object_is_item, type);
			// and let Lua know too
			L_Call_Item_Created(object_index);
 		}
	}
	else
	{
		object_index= NONE;
	}
	
	return object_index;
}

void trigger_nearby_items(
	short polygon_index)
{
	polygon_index= flood_map(polygon_index, INT32_MAX, item_trigger_cost_function, _breadth_first, (void *) NULL);
	while (polygon_index!=NONE)
	{
		struct object_data *object;
		short object_index;

		for (object_index= get_polygon_data(polygon_index)->first_object; object_index!=NONE; object_index= object->next_object)
		{
			object= get_object_data(object_index);
			switch (GET_OBJECT_OWNER(object))
			{
				case _object_is_item:
					if (OBJECT_IS_INVISIBLE(object) && object->permutation!=NONE)
					{
						teleport_object_in(object_index);
					}
					break;
			}
		}
		
		polygon_index= flood_map(NONE, INT32_MAX, item_trigger_cost_function, _breadth_first, (void *) NULL);
	}
}

/* returns the color of the ball or NONE if they don't have one */
short find_player_ball_color(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	short ball_color= NONE;
	short index;

	for(index= BALL_ITEM_BASE; ball_color==NONE && index<BALL_ITEM_BASE+MAXIMUM_NUMBER_OF_PLAYERS; ++index)
	{
		if(player->items[index]>0) 
		{
			ball_color= index-BALL_ITEM_BASE;
		}
	}

	return ball_color;	
}

void get_item_name(
	char *buffer,
	short item_id,
	bool plural)
{
	struct item_definition *definition= get_item_definition(item_id);
	// LP change: added idiot-proofing
	if (!definition)
	{
		if (plural)
			sprintf(buffer,"Unlisted items with ID %d",item_id);
		else
			sprintf(buffer,"Unlisted item with ID %d",item_id);
		
		return;
	}
	
	getcstr(buffer, strITEM_NAME_LIST, plural ? definition->plural_name_id :
		definition->singular_name_id);
}

void get_header_name(
	char *buffer,
	short type)
{
	getcstr(buffer, strHEADER_NAME_LIST, type);
}

void calculate_player_item_array(
	short player_index,
	short type,
	short *items,
	short *counts,
	short *array_count)
{
	struct player_data *player= get_player_data(player_index);
	short loop;
	short count= 0;
	
	for(loop=0; loop<NUMBER_OF_DEFINED_ITEMS; ++loop)
	{
		if (loop==_i_knife) continue;
	 	if(player->items[loop] != NONE)
		{
			if(get_item_kind(loop)==type)
			{
				items[count]= loop;
				counts[count]= player->items[loop];
				count++;
			}
		}
	}
	
	*array_count= count;
}

short count_inventory_lines(
	short player_index)
{
	struct player_data *player= get_player_data(player_index);
	bool types[NUMBER_OF_ITEM_TYPES];
	short count= 0;
	short loop;
	
	/* Clean out the header array, so we can count properly */
	for(loop=0; loop<NUMBER_OF_ITEM_TYPES; ++loop)
	{
		types[loop]= false;
	}
	
	for(loop=0; loop<NUMBER_OF_DEFINED_ITEMS; ++loop)
	{
		if (loop==_i_knife) continue;
		if (player->items[loop] != NONE)
		{
			count++;
			types[get_item_kind(loop)]= true;
		}
	}
	
	/* Now add in the header lines.. */
	for(loop= 0; loop<NUMBER_OF_ITEM_TYPES; ++loop)
	{
		if(types[loop]) count++;
	}
	
	return count;
}

static void a1_swipe_nearby_items(
	short player_index)
{
	struct object_data *object;
	struct object_data *player_object;
	struct player_data *player= get_player_data(player_index);
	short next_object;
	struct polygon_data *polygon;
	short *neighbor_indexes;
	short i;

	player_object= get_object_data(get_monster_data(player->monster_index)->object_index);

	polygon= get_polygon_data(player_object->polygon);
	neighbor_indexes= get_map_indexes(polygon->first_neighbor_index, polygon->neighbor_count);
	
	// Skip this step if neighbor indexes were not found
	if (!neighbor_indexes) return;

	for (i=0;i<polygon->neighbor_count;++i)
	{	
		
		struct polygon_data *neighboring_polygon= get_polygon_data(*neighbor_indexes++);
		
		/*
			LP change: since precalculate_map_indexes() and its associated routine
			intersecting_flood_proc() appear to have some bugs in them, I will
			instead search the neighbors of each indexed polygon.
			
			Starting the search from -1 is a kludge designed to include a search
			for the current polygon.
		*/
		struct polygon_data *source_polygon = neighboring_polygon;
		for (int ngbr_indx = -1; ngbr_indx<source_polygon->vertex_count; ngbr_indx++)
		{
		if (ngbr_indx >= 0)
		{
			// Be sure to check on whether there is a valid polygon on the other side
			short adjacent_index = source_polygon->adjacent_polygon_indexes[ngbr_indx];
			if (adjacent_index == NONE) continue;
			neighboring_polygon = get_polygon_data(adjacent_index);
		}
		else
			neighboring_polygon = source_polygon;
		
		if (!POLYGON_IS_DETACHED(neighboring_polygon))
		{
			next_object= neighboring_polygon->first_object;

			while(next_object != NONE)
			{
				object= get_object_data(next_object);
				if (GET_OBJECT_OWNER(object)==_object_is_item && !OBJECT_IS_INVISIBLE(object)) 
				{
					if (guess_distance2d((world_point2d *) &player->location, (world_point2d *) &object->location)<=MAXIMUM_ARM_REACH)
					{
						world_distance radius, height;
						
						get_monster_dimensions(player->monster_index, &radius, &height);
		
						if (object->location.z >= player->location.z - MAXIMUM_ARM_REACH && object->location.z <= player->location.z + height &&
							test_item_retrieval(player_object->polygon, &player_object->location, &object->location))
						{
							if(get_item(player_index, next_object))
							{
								/* Start the search again.. */
								next_object= neighboring_polygon->first_object;
								continue;
							}
						}
					}
				}
				
				next_object= object->next_object;
			}
		}
		// LP addition: end of that kludgy search loop
		}
	}
}

static void m2_swipe_nearby_items(
	short player_index)
{
	struct object_data *object;
	struct object_data *player_object;
	struct player_data *player= get_player_data(player_index);
	short next_object;
	struct polygon_data *polygon;
	short *neighbor_indexes;
	short i;

	player_object= get_object_data(get_monster_data(player->monster_index)->object_index);

	polygon= get_polygon_data(player_object->polygon);
	neighbor_indexes= get_map_indexes(polygon->first_neighbor_index, polygon->neighbor_count);
	
	// Skip this step if neighbor indexes were not found
	if (!neighbor_indexes) return;

	for (i=0;i<polygon->neighbor_count;++i)
	{	
		
		struct polygon_data *neighboring_polygon= get_polygon_data(*neighbor_indexes++);
	
		if (!POLYGON_IS_DETACHED(neighboring_polygon))
		{
			next_object= neighboring_polygon->first_object;

			while(next_object != NONE)
			{
				object= get_object_data(next_object);
				if (GET_OBJECT_OWNER(object)==_object_is_item && !OBJECT_IS_INVISIBLE(object)) 
				{
					if (guess_distance2d((world_point2d *) &player->location, (world_point2d *) &object->location)<=MAXIMUM_ARM_REACH)
					{
						world_distance radius, height;
						
						get_monster_dimensions(player->monster_index, &radius, &height);
		
						if (object->location.z >= player->location.z - MAXIMUM_ARM_REACH && object->location.z <= player->location.z + height &&
							test_item_retrieval(player_object->polygon, &player_object->location, &object->location))
						{
							if(get_item(player_index, next_object))
							{
								/* Start the search again.. */
								next_object= neighboring_polygon->first_object;
								continue;
							}
						}
					}
				}
				
				next_object= object->next_object;
			}
		}
	}
}

void swipe_nearby_items(short player_index)
{
	if (film_profile.swipe_nearby_items_fix)
	{
		a1_swipe_nearby_items(player_index);
	}
	else
	{
		m2_swipe_nearby_items(player_index);
	}
}


void mark_item_collections(
	bool loading)
{
	mark_collection(_collection_items, loading);
}

bool unretrieved_items_on_map(
	void)
{
	bool found_item= false;
	struct object_data *object;
	short object_index;
	
	for (object_index= 0, object= objects; object_index<MAXIMUM_OBJECTS_PER_MAP; ++object_index, ++object)
	{
		if (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object)==_object_is_item)
		{
			if (get_item_kind(object->permutation)==_item)
			{
				found_item= true;
				break;
			}
		}
	}
	
	return found_item;
}

bool item_valid_in_current_environment(
	short item_type)
{
	bool valid= true;
	struct item_definition *definition= get_item_definition(item_type);
	// LP change: added idiot-proofing
	if (!definition) return false;
	
	if (definition->invalid_environments & static_world->environment_flags)
	{
		valid= false;
	}
	
	return valid;
}

short get_item_kind(
	short item_id)
{
	struct item_definition *definition= get_item_definition(item_id);
	// LP change: added idiot-proofing
	if (!definition) return NONE;
	
	return definition->item_kind;
}

short get_item_shape(
	short item_id)
{
	struct item_definition *definition= get_item_definition(item_id);
	// LP change: added idiot-proofing
	if (!definition) return NONE;

	return definition->base_shape;
}

bool try_and_add_player_item(
	short player_index,
	short type) 
{
	struct item_definition *definition= get_item_definition(type);
	// LP change: added idiot-proofing
	if (!definition) return false;
	
	struct player_data *player= get_player_data(player_index);
	short grabbed_sound_index= NONE;
	bool success= false;

	switch (definition->item_kind)
	{
		case _powerup: /* powerups donÕt get added to your inventory */
			if (legal_player_powerup(player_index, type))
			{
				process_player_powerup(player_index, type);
				object_was_just_destroyed(_object_is_item, type);
				grabbed_sound_index= Sound_GotPowerup();
				success= true;
			}
			break;
		
		case _ball:
			// START Benad
			/* Note that you can only carry ONE ball (ever) */
			if(find_player_ball_color(player_index)==NONE)
			{
				struct player_data *player= get_player_data(player_index);
				
				// When taking ball of your own team, it returns to its original
				// position on the map, unless it's already in our base (or hill).
				if ( (GET_GAME_TYPE() == _game_of_capture_the_flag) &&
					 (type - BALL_ITEM_BASE == player->team)  )
				{
					// START Benad modified oct. 1st
					struct polygon_data *polygon= get_polygon_data(player->supporting_polygon_index);
					if (polygon->type!=_polygon_is_base)
					{
						object_was_just_destroyed(_object_is_item, type);
						grabbed_sound_index= Sound_GotItem();
						success= true;
						goto DONE;
					}
					else // _polygon_is_base and base == player->team
						 // base != player->team taken care of in update_net_game
						 // (your ball should NEVER get there)
					{
						success= false;
						goto DONE;
					}
					// END Benad modified oct. 1st
				}
				else if (GET_GAME_TYPE() == _game_of_rugby)
				{
					// ghs: work around for SF 2894880

					// if you're in an enemy base
					// and pick up the ball, you
					// score
					struct polygon_data* polygon = get_polygon_data(player->supporting_polygon_index);
					if (polygon->type == _polygon_is_base && polygon->permutation != player->team)
					{
						/* Goal! */

						// defined in network_games.cpp
						const int _points_scored = 0;
						player->netgame_parameters[_points_scored]++;
						team_netgame_parameters[player->team][_points_scored]++;
						object_was_just_destroyed(_object_is_item, type);
						grabbed_sound_index = Sound_GotItem();
						success = true;
						goto DONE;
					}
				}
				
				player->items[type]= 1;

				// OK, since only for loading weapon. Ignores item_type, cares
				// only about item_kind (here, _ball).
				/* Load the ball weapon.. */
				process_new_item_for_reloading(player_index, _i_red_ball);
				
				/* Tell the interface to redraw next time it has to */
				mark_player_inventory_as_dirty(player_index, type);
				
				success= true;
			}
			grabbed_sound_index= NONE;
			break;
			// END Benad
					
		case _weapon:
		case _ammunition:
		case _item:
			/* Increment the count */	
			assert(type>=0 && type<NUMBER_OF_ITEMS);
			if(player->items[type]==NONE)
			{
				/* just got the first one.. */
				player->items[type]= 1;
				success= true;
			} 
			else if(player->items[type]+1<=definition->get_maximum_count_per_player(static_world->environment_flags & _environment_m1_weapons, dynamic_world->game_information.difficulty_level))
			{
				/* Increment your count.. */
				player->items[type]++;
				success= true;
			} else {
				/* You have exceeded the count of these items */
			}

			grabbed_sound_index= Sound_GotItem();

			if(success)
			{
				/* Reload or whatever.. */
				process_new_item_for_reloading(player_index, type);
					
				/* Tell the interface to redraw next time it has to */
				mark_player_inventory_as_dirty(player_index, type);
			}
			break;
		
		default:
			assert(false);
			break;
	}
	// Benad. Burk.
	DONE:
	
	//CP Addition: call any script traps available
	// jkvw: but only if we actually got the item
	if (success)
	{
		//MH: Call Lua script hook
		L_Call_Got_Item(type, player_index);
	}

	/* Play the pickup sound */
	if (success && player_index==current_player_index)
	{
		SoundManager::instance()->PlayLocalSound(grabbed_sound_index);
	
		/* Flash screen */
		start_fade(_fade_bonus);
	}

	return success;
}

/* ---------- private code */


static int32 item_trigger_cost_function(
	short source_polygon_index,
	short line_index,
	short destination_polygon_index,
	void *unused)
{
	struct polygon_data *destination_polygon= get_polygon_data(destination_polygon_index);
//	struct polygon_data *source_polygon= get_polygon_data(source_polygon_index);
//	struct line_data *line= get_line_data(line_index);
	int32 cost= 1;
	
	(void) (unused);
	(void) (source_polygon_index);
	(void) (line_index);

	if (destination_polygon->type==_polygon_is_zone_border) cost= -1;
	
	return cost;
}

static bool get_item(
	short player_index,
	short object_index) 
{
	struct object_data *object= get_object_data(object_index);	
	bool success;

	assert(GET_OBJECT_OWNER(object)==_object_is_item);
	
	success= try_and_add_player_item(player_index, object->permutation);
	if (success)
	{
		/* remove it */
		remove_map_object(object_index);
	}
	
	return success;
}

static bool test_item_retrieval(
	short polygon_index1,
	world_point3d *location1,
	world_point3d *location2)
{
	bool valid_retrieval= true;
	short polygon_index= polygon_index1;

	do
	{
		short line_index= find_line_crossed_leaving_polygon(polygon_index, (world_point2d *) location1,
			(world_point2d *) location2);
		
		if (line_index!=NONE)
		{
			polygon_index= find_adjacent_polygon(polygon_index, line_index);
			if (LINE_IS_SOLID(get_line_data(line_index))) valid_retrieval= false;
			if (polygon_index!=NONE)
			{
				struct polygon_data *polygon= get_polygon_data(polygon_index);
				
				if (polygon->type==_polygon_is_platform)
				{
					struct platform_data *platform= get_platform_data(polygon->permutation);
					
					if (PLATFORM_IS_MOVING(platform)) valid_retrieval= false;
				}
			}
		}
		else
		{
			polygon_index= NONE;
		}
	}
	while (polygon_index!=NONE && valid_retrieval);
	
	return valid_retrieval;
}


// LP addition: initializer
void initialize_items(void) {
}

// LP addition: animator
void animate_items(void) {

	short object_index;
	object_data *object;
	for (object_index= 0, object= objects; object_index<MAXIMUM_OBJECTS_PER_MAP; ++object_index, ++object)
	{
		if (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object)==_object_is_item && !OBJECT_IS_INVISIBLE(object))
		{
			short type = object->permutation;
			if (get_item_kind(type) != NONE)
			{
				struct item_definition *ItemDef = get_item_definition(type);
				// LP change: added idiot-proofing
				if (!ItemDef) continue;
				
				shape_descriptor shape = ItemDef->base_shape;
				struct shape_animation_data *animation= get_shape_animation_data(shape);
				if (!animation) continue;
				
				// Randomize if non-animated; do only once
				if (object->facing >= 0) {
					if (randomize_object_sequence(object_index,shape))
					{
						object->facing = NONE;
					}
				}
				// Now the animation
				if (object->facing >= 0)
					animate_object(object_index);
			}
		}
	}
}

struct item_definition *original_item_definitions = NULL;

void reset_mml_items()
{
	if (original_item_definitions) {
		for (unsigned i = 0; i < NUMBER_OF_DEFINED_ITEMS; i++)
			item_definitions[i] = original_item_definitions[i];
		free(original_item_definitions);
		original_item_definitions = NULL;
	}
}

void parse_mml_items(const InfoTree& root)
{
	// back up old values first
	if (!original_item_definitions) {
		original_item_definitions = (struct item_definition *) malloc(sizeof(struct item_definition) * NUMBER_OF_DEFINED_ITEMS);
		assert(original_item_definitions);
		for (unsigned i = 0; i < NUMBER_OF_DEFINED_ITEMS; i++)
			original_item_definitions[i] = item_definitions[i];
	}
	
	BOOST_FOREACH(InfoTree itree, root.children_named("item"))
	{
		int16 index;
		if (!itree.read_indexed("index", index, NUMBER_OF_DEFINED_ITEMS))
			continue;
		
		item_definition& def = item_definitions[index];
		itree.read_attr("singular", def.singular_name_id);
		itree.read_attr("plural", def.plural_name_id);
		itree.read_indexed("maximum", def.maximum_count_per_player, SHRT_MAX+1);
		itree.read_attr("invalid", def.invalid_environments);
		itree.read_indexed("type", def.item_kind, NUMBER_OF_ITEM_TYPES);

		for (auto max : itree.children_named("difficulty"))
		{
			int16 difficulty;
			if (!max.read_indexed("index", difficulty, NUMBER_OF_GAME_DIFFICULTY_LEVELS))
				continue;

			if (max.read_indexed("maximum", def.extended_maximum_count[difficulty], SHRT_MAX+1))
			{
				def.has_extended_maximum_count[difficulty] = true;
			}
		}

		BOOST_FOREACH(InfoTree shape, itree.children_named("shape"))
			shape.read_shape(def.base_shape);
	}
}

/*

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

	Created for non-duplication of code between mac and SDL ports.
	(Loren Petrich and others)

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Disabled network_speaker_idle_proc under Carbon

Mar 08, 2002 (Woody Zenfell):
    Enabled network_microphone_idle_proc (in global_idle_proc()) under SDL
*/

bool CheatsActive = false;
bool chat_input_mode = false;

#include "cseries.h"

#include "XML_ParseTreeRoot.h"
#include "interface.h"
#include "world.h"
#include "screen.h"
#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "vbl.h"
#include "player.h"
#include "Music.h"
#include "items.h"
#include "network_sound.h"
#include "TextStrings.h"
#include "InfoTree.h"

#include <ctype.h>


extern void process_new_item_for_reloading(short player_index, short item_type);
extern bool try_and_add_player_item(short player_index,	short type);
extern void mark_shield_display_as_dirty();
extern void mark_oxygen_display_as_dirty();
extern void accelerate_monster(short monster_index,	world_distance vertical_velocity, 
							   angle direction, world_distance velocity);
extern void network_speaker_idle_proc(void);
extern void update_interface(short time_elapsed);


// Cheat-keyword definition: tag and associated text string

#define MAXIMUM_KEYWORD_LENGTH 20

struct keyword_data
{
	short tag;
	char keyword[MAXIMUM_KEYWORD_LENGTH+1]; /* in uppercase */
};


enum // cheat tags
{
	_tag_health,
	_tag_oxygen,
	_tag_map,
	_tag_fusion,
	_tag_invincible,
	_tag_invisible,
	_tag_extravision,
	_tag_infravision,
	_tag_pistol,
	_tag_rifle,
	_tag_missile,	// LP change: corrected spelling
	_tag_toaster,  // flame-thrower
	_tag_pzbxay,
	_tag_shotgun,	// LP addition
	_tag_smg,		// LP addition
	_tag_ammo,
	_tag_pathways,
	_tag_view,
	_tag_jump,
	_tag_aslag,
	_tag_save
};

void AddItemsToPlayer(short ItemType, short MaxNumber);

// Here, only one is added, unless the number of items is at least as great as MaxNumber
void AddOneItemToPlayer(short ItemType, short MaxNumber);

// LP addition: XML support for controlling whether cheats are active
keyword_data *original_keywords = NULL;

static keyword_data keywords[]=
{
	{_tag_health, "NRG"},
	{_tag_oxygen, "OTWO"},
	{_tag_map, "MAP"},
	{_tag_invisible, "BYE"},
	{_tag_invincible, "NUKE"},
	{_tag_infravision, "SEE"},
	{_tag_extravision, "WOW"},
	{_tag_pistol, "MAG"},
	{_tag_rifle, "RIF"},
	{_tag_missile, "POW"},
	{_tag_toaster, "TOAST"},
	{_tag_fusion, "MELT"},
	{_tag_shotgun, "PUFF"},
	{_tag_smg, "ZIP"},
	{_tag_pzbxay, "PZBXAY"}, // the alien shotgon, in the phfor's language
	{_tag_ammo, "AMMO"},
	{_tag_jump, "QWE"},
	{_tag_aslag, "SHIT"},
	{_tag_save, "YOURMOM"}
};



void AddItemsToPlayer(short ItemType, short MaxNumber)
{
	for (int i=0; i<MaxNumber; i++)
		try_and_add_player_item(local_player_index,ItemType);
}

void AddOneItemToPlayer(short ItemType, short MaxNumber)
{
	local_player->items[ItemType] = MAX(local_player->items[ItemType],0);
	if (local_player->items[ItemType] < MaxNumber)
		try_and_add_player_item(local_player_index,ItemType);
}


void handle_keyword(int tag)
{

	switch (tag)
	{
		case _tag_health:
			if (local_player->suit_energy<PLAYER_MAXIMUM_SUIT_ENERGY)
			{
				local_player->suit_energy= PLAYER_MAXIMUM_SUIT_ENERGY;
			}
			else
			{
				if (local_player->suit_energy<2*PLAYER_MAXIMUM_SUIT_ENERGY)
				{
					local_player->suit_energy= 2*PLAYER_MAXIMUM_SUIT_ENERGY;
				}
				else
				{
					local_player->suit_energy= MAX(local_player->suit_energy, 3*PLAYER_MAXIMUM_SUIT_ENERGY);
				}
			}
			mark_shield_display_as_dirty();
			break;
		case _tag_oxygen:
			local_player->suit_oxygen= MAX(local_player->suit_oxygen,PLAYER_MAXIMUM_SUIT_OXYGEN);
			mark_oxygen_display_as_dirty();
			break;
		case _tag_map:
			dynamic_world->game_information.game_options^= (_overhead_map_shows_items|_overhead_map_shows_monsters|_overhead_map_shows_projectiles);
			break;
		case _tag_invincible:
			process_player_powerup(local_player_index, _i_invincibility_powerup);
			break;
		case _tag_invisible:
			process_player_powerup(local_player_index, _i_invisibility_powerup);
			break;
		case _tag_infravision:
			process_player_powerup(local_player_index, _i_infravision_powerup);
			break;
		case _tag_extravision:
			process_player_powerup(local_player_index, _i_extravision_powerup);
			break;
		case _tag_jump:
			accelerate_monster(local_player->monster_index, WORLD_ONE/10, 0, 0);
			break;
		// LP: changed these cheats and added new ones:
		case _tag_pistol:
			AddOneItemToPlayer(_i_magnum,2);
			AddItemsToPlayer(_i_magnum_magazine,10);
			break;
		case _tag_rifle:
			AddItemsToPlayer(_i_assault_rifle,10);
			AddItemsToPlayer(_i_assault_rifle_magazine,10);
			AddItemsToPlayer(_i_assault_grenade_magazine,10);
			break;
		case _tag_missile:
			AddItemsToPlayer(_i_missile_launcher,1);
			AddItemsToPlayer(_i_missile_launcher_magazine,10);
			break;
		case _tag_toaster:
			AddItemsToPlayer(_i_flamethrower,1);
			AddItemsToPlayer(_i_flamethrower_canister,10);
			break;
		case _tag_fusion:
			AddItemsToPlayer(_i_plasma_pistol,1);
			AddItemsToPlayer(_i_plasma_magazine,10);
			break;
		case _tag_pzbxay:
			AddItemsToPlayer(_i_alien_shotgun,1);
			break;
		case _tag_shotgun:
			AddOneItemToPlayer(_i_shotgun,2);
			AddItemsToPlayer(_i_shotgun_magazine,10);
			break;
		case _tag_smg:
			AddOneItemToPlayer(_i_smg,2);
			AddItemsToPlayer(_i_smg_ammo,10);
			break;
		case _tag_save:
			save_game();
			break;
		// LP guess as to what might be good: ammo-only version of "aslag"
		case _tag_ammo:
			{
				short items[]= { _i_assault_rifle, _i_magnum, _i_missile_launcher, _i_flamethrower,
					_i_plasma_pistol, _i_alien_shotgun, _i_shotgun,
					_i_assault_rifle_magazine, _i_assault_grenade_magazine, 
					_i_magnum_magazine, _i_missile_launcher_magazine, _i_flamethrower_canister,
					_i_plasma_magazine, _i_shotgun_magazine, _i_shotgun, _i_smg, _i_smg_ammo};
				
				for(unsigned index= 0; index<sizeof(items)/sizeof(short); ++index)
				{
					switch(get_item_kind(items[index]))
					{	
						case _ammunition:
							AddItemsToPlayer(items[index],10);
							break;
					} 
				}
			}
			break;
		case _tag_aslag:
			{
				// LP change: added the SMG and its ammo
				short items[]= { _i_assault_rifle, _i_magnum, _i_missile_launcher, _i_flamethrower,
					_i_plasma_pistol, _i_alien_shotgun, _i_shotgun,
					_i_assault_rifle_magazine, _i_assault_grenade_magazine, 
					_i_magnum_magazine, _i_missile_launcher_magazine, _i_flamethrower_canister,
					_i_plasma_magazine, _i_shotgun_magazine, _i_shotgun, _i_smg, _i_smg_ammo};
				
				for(unsigned index= 0; index<sizeof(items)/sizeof(short); ++index)
				{
					switch(get_item_kind(items[index]))
					{
						case _weapon:
							if(items[index]==_i_shotgun || items[index]==_i_magnum)
							{
								AddOneItemToPlayer(items[index],2);
							} else {	
								AddItemsToPlayer(items[index],1);
							}
							break;
							
						case _ammunition:
							AddItemsToPlayer(items[index],10);
							break;
							
						case _powerup:
						case _weapon_powerup:
							break;
							
						default:
							break;
					} 
					process_new_item_for_reloading(local_player_index, items[index]);
				}
			}
			local_player->suit_energy = MAX(local_player->suit_energy, 3*PLAYER_MAXIMUM_SUIT_ENERGY);
			update_interface(NONE);
			break;
			
		default:
			break;
	}

	return;

}

/*
 *  Called regularly during event loops
 */

void global_idle_proc(void)
{
	Music::instance()->Idle();
	network_speaker_idle_proc();
	network_microphone_idle_proc();
	SoundManager::instance()->Idle();
}

/*
 *  Somebody wants to do something important; free as much temporary memory as possible
 */

void free_and_unlock_memory(void)
{
	SoundManager::instance()->StopAllSounds();
}

/*
 *  Special version of malloc() used for level transitions, frees some
 *  memory if possible
 */

void *level_transition_malloc(
	size_t size)
{
	void *ptr= malloc(size);
	if (!ptr)
	{
		SoundManager::instance()->UnloadAllSounds();
		
		ptr= malloc(size);
		if (!ptr)
		{
			unload_all_collections();
			
			ptr= malloc(size);
		}
	}
	
	return ptr;
}

// LP change: implementing Benad's "cheats always on"
// Originally, cheats were not on in the "FINAL" version
//
#define NUMBER_OF_KEYWORDS (sizeof(keywords)/sizeof(keyword_data))

static char keyword_buffer[MAXIMUM_KEYWORD_LENGTH+1];

int process_keyword_key(
	char key)
{
	short tag = NONE;

	// copy the buffer down and insert the new character 
	for (unsigned i=0;i<MAXIMUM_KEYWORD_LENGTH-1;++i)
	{
		keyword_buffer[i]= keyword_buffer[i+1];
	}
	keyword_buffer[MAXIMUM_KEYWORD_LENGTH-1]= toupper(key);
	keyword_buffer[MAXIMUM_KEYWORD_LENGTH]= 0;
	
	// any matches? 
	for (unsigned i=0; i<NUMBER_OF_KEYWORDS; ++i)
	{
		if (!strcmp(keywords[i].keyword, keyword_buffer+MAXIMUM_KEYWORD_LENGTH-strlen(keywords[i].keyword)))
		{
			// wipe the buffer if we have a match 
			memset(keyword_buffer, 0, MAXIMUM_KEYWORD_LENGTH);
			tag= keywords[i].tag;
			break;
		}
	}
	
	return tag;
}

void reset_mml_cheats()
{
	CheatsActive = false;
	if (original_keywords)
	{
		for (int i = 0; i < NUMBER_OF_KEYWORDS; i++)
			keywords[i] = original_keywords[i];
	}
}

void parse_mml_cheats(const InfoTree& root)
{
	// back up old values first
	if (!original_keywords)
	{
		original_keywords = (keyword_data *) malloc(sizeof(keywords));
		assert(original_keywords);
		for (int i = 0; i < NUMBER_OF_KEYWORDS; i++)
			original_keywords[i] = keywords[i];
	}
	
	root.read_attr("on", CheatsActive);
	
	BOOST_FOREACH(InfoTree ktree, root.children_named("keyword"))
	{
		int16 index;
		if (!ktree.read_indexed("index", index, NUMBER_OF_KEYWORDS))
			continue;
		
		std::string word = ktree.get_value(std::string(""));
		char *dest = keywords[index].keyword;
		size_t decoded_len = DeUTF8_C(word.c_str(), word.size(), dest, MAXIMUM_KEYWORD_LENGTH);
		for (size_t c = 0; c < decoded_len; c++, dest++)
			*dest = toupper(*dest);
	}
}

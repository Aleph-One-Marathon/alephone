/*
 *  shell.cpp - Main game loop and input handling
 */

#include "cseries.h"

#include "map.h"
#include "monsters.h"
#include "player.h"
#include "render.h"
#include "shell.h"
#include "interface.h"
#include "mysound.h"
#include "fades.h"
#include "screen.h"
#include "music.h"
#include "images.h"
#include "vbl.h"
#include "preferences.h"
#include "tags.h" /* for scenario file type.. */
#include "network_sound.h"
#include "mouse.h"
#include "screen_drawing.h"
#include "computer_interface.h"
#include "game_wad.h" /* yuck... */
#include "game_window.h" /* for draw_interface() */
#include "extensions.h"
#include "items.h"
#include "interface_menus.h"
#include "weapons.h"

#include "Crosshairs.h"
#include "OGL_Render.h"
#include "XML_ParseTreeRoot.h"
#include "FileHandler.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// LP addition: whether or not the cheats are active
bool CheatsActive = false;

#ifndef HAVE_OPENGL
// No OpenGL, so define these here
float FogDepth = 1;
float FogColor[4] = {0, 0, 0, 0};
#endif

// Prototypes
static void main_event_loop(void);
static int process_keyword_key(char key);
static void handle_keyword(int type_of_cheat);

// Include platform-specific file
#if defined(mac)
#include "shell_macintosh.cpp"
#elif defined(SDL)
#include "shell_sdl.cpp"
#endif


/*
 *  Somebody want to do something important; free as much temporary memory as possible
 */

void free_and_unlock_memory(void)
{
	stop_all_sounds();
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
		unload_all_sounds();
		
		ptr= malloc(size);
		if (!ptr)
		{
			unload_all_collections();
			
			ptr= malloc(size);
		}
	}
	
	return ptr;
}


/*
 *  Called regularly during event loops
 */

void global_idle_proc(void)
{
	music_idle_proc();
	network_speaker_idle_proc();
	sound_manager_idle_proc();
}


/* ------------------------- cheating (not around if FINAL is defined) */
// LP change: implementing Benad's "cheats always on"
// #ifndef FINAL

#define MAXIMUM_KEYWORD_LENGTH 20
#define NUMBER_OF_KEYWORDS (sizeof(keywords)/sizeof(struct keyword_data))

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

/* ---------- resources */

/* ---------- structures */
struct keyword_data
{
	short tag;
	char *keyword; /* in uppercase */
};

/* ---------- globals */
static struct keyword_data keywords[]=
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
static char keyword_buffer[MAXIMUM_KEYWORD_LENGTH+1];


static int process_keyword_key(
	char key)
{
	short i;
	short tag = NONE;

	/* copy the buffer down and insert the new character */
	for (i=0;i<MAXIMUM_KEYWORD_LENGTH-1;++i)
	{
		keyword_buffer[i]= keyword_buffer[i+1];
	}
	keyword_buffer[MAXIMUM_KEYWORD_LENGTH-1]= toupper(key);
	keyword_buffer[MAXIMUM_KEYWORD_LENGTH]= 0;
	
	/* any matches? */
	for (i=0; i<NUMBER_OF_KEYWORDS; ++i)
	{
		if (!strcmp(keywords[i].keyword, keyword_buffer+MAXIMUM_KEYWORD_LENGTH-strlen(keywords[i].keyword)))
		{
			/* wipe the buffer if we have a match */
			memset(keyword_buffer, 0, MAXIMUM_KEYWORD_LENGTH);
			tag= keywords[i].tag;
			break;
		}
	}
	
	return tag;
}

// LP additions:
// Macros for code below:
// Here, MaxNumber is the maximum number of items that will be added;
// the final number is at least MaxNumber
inline void AddItemsToPlayer(short ItemType, short MaxNumber)
{
	for (int i=0; i<MaxNumber; i++)
		try_and_add_player_item(local_player_index,ItemType);
}

// Here, only one is added, unless the number of items is at least as great as MaxNumber
inline void AddOneItemToPlayer(short ItemType, short MaxNumber)
{
	local_player->items[ItemType] = MAX(local_player->items[ItemType],0);
	if (local_player->items[ItemType] < MaxNumber)
		try_and_add_player_item(local_player_index,ItemType);
}

static void handle_keyword(
	int tag)
{
	bool cheated= true;

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
				short index;
				
				for(index= 0; index<sizeof(items)/sizeof(short); ++index)
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
				short index;
				
				for(index= 0; index<sizeof(items)/sizeof(short); ++index)
				{
					switch(get_item_kind(items[index]))
					{
						case _weapon:
							if(items[index]==_i_shotgun || items[index]==_i_magnum)
							{
								AddOneItemToPlayer(items[index],2);
								/*
								assert(items[index]>=0 && items[index]<NUMBER_OF_ITEMS);
								if(local_player->items[items[index]]==NONE)
								{
									local_player->items[items[index]]= 1;
								} else {
									local_player->items[items[index]]++;
								}
								*/
							} else {	
								AddItemsToPlayer(items[index],1);
								// local_player->items[items[index]]= 1;
							}
							break;
							
						case _ammunition:
							AddItemsToPlayer(items[index],10);
							// local_player->items[items[index]]= 10;
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
			cheated= false;
			break;
	}

//	/* can't use computer terminals or save in the final version if we've cheated */
//	if (cheated) SET_PLAYER_HAS_CHEATED(local_player);
#if 0
	if (cheated)
	{
		long final_ticks;
		
		SetSoundVol(7);
		play_local_sound(20110);
		Delay(45, &final_ticks);
		play_local_sound(20110);
		Delay(45, &final_ticks);
		play_local_sound(20110);
	}
#endif
	
	return;
}
// LP change: implementing Benad's "cheats always on"
// #endif


// LP addition: XML support for controlling whether cheats are active
class XML_CheatsParser: public XML_ElementParser
{
	
public:
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_CheatsParser(): XML_ElementParser("cheats") {}
};

bool XML_CheatsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"on") == 0)
	{
		return ReadBooleanValue(Value,CheatsActive);
	}
	UnrecognizedTag();
	return false;
}

static XML_CheatsParser CheatsParser;


XML_ElementParser *Cheats_GetParser()
{
	return &CheatsParser;
}

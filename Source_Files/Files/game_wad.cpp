/*
GAME_WAD.C

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

Sunday, July 3, 1994 10:45:17 PM

Routines for loading an entire game.

Sunday, September 25, 1994 5:03:54 PM  (alain)
	call recalculate_redundant_endpoint_data() upon restoring saved game since
	the redundant data isn't saved.
Sunday, November 6, 1994 5:35:34 PM
	added support for the unified platforms/doors, cleaned up some old code of mine...
Saturday, August 26, 1995 2:28:56 PM
	made portable.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 6, 2000 (Loren Petrich):
	Added loading and saving of physics models in savegames and from map files

Feb 12, 2000 (Loren Petrich):
	Added MARATHON_INFINITY_DATA_VERSION where appropriate

Feb 14, 2000 (Loren Petrich):
	Added more Pfhorte-friendly error checking to reading in of
	map-info ('Minf') chunk; allowing it to be 2 bytes shorter.

Feb 17, 2000 (Loren Petrich):
	Hides cursor after warning user about loading non-Bungie map files
	(strERRORS, warningExternalMapsFile)

Feb 19, 2000 (Loren Petrich):
	Fixed off-by-one asserts in load_***() routines;

Feb 26, 2000 (Loren Petrich):
	Added chase-cam initialization

June 15, 2000 (Loren Petrich):
	Added supprt for Chris Pruett's Pfhortran

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 25, 2000 (Loren Petrich):
	Cleared errors (game_errors.c/h) produced by Pfhortran
	and by checking on a scenario's image files

Aug 28, 2000 (Loren Petrich):
	Started on using new pack/unpack routines

Nov 26, 2000 (Loren Petrich):
	Movied a RunLevelScript() before some other stuff, such as entering_map(),
	so that textures to be loaded can be specified before they actually get loaded.

Feb 15, 2002 (Br'fin (Jeremy Parsons)):
	Additional save data is now applied to the Temporary file instead of the original
	(Old level preview info is now saved under Macintosh again)
*/

// This needs to do the right thing on save game, which is storing the precalculated crap.

#include "cseries.h"

#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "monsters.h"
#include "network.h"
#include "projectiles.h"
#include "effects.h"
#include "player.h"
#include "platforms.h"
#include "flood_map.h"
#include "scenery.h"
#include "lightsource.h"
#include "media.h"
#include "weapons.h"
#include "shell.h"
#include "preferences.h"
#include "FileHandler.h"

#include "editor.h"
#include "tags.h"
#include "wad.h"
#include "game_wad.h"
#include "interface.h"
#include "game_window.h"
#include "game_errors.h"
#include "computer_interface.h" // for loading/saving terminal state.
#include "images.h"
#include "shell.h"
#include "preferences.h"
#include "SoundManager.h"
#include "Plugins.h"
#include "ephemera.h"

// LP change: added chase-cam init and render allocation
#include "ChaseCam.h"
#include "render.h"

#include "XML_LevelScript.h"

// For packing and unpacking some of the stuff
#include "Packing.h"

#include "motion_sensor.h"	// ZZZ for reset_motion_sensor()

#include "Music.h"

// unify the save game code into one structure.

/* -------- local globals */
FileSpecifier MapFileSpec;
static bool file_is_set= false;

// LP addition: was a physics model loaded from the previous level loaded?
static bool PhysicsModelLoadedEarlier = false;

// The following local globals are for handling games that need to be restored.
struct revert_game_info
{
	bool game_is_from_disk;
	struct game_data game_information;
	struct player_start_data player_start;
	struct entry_point entry_point;
	FileSpecifier SavedGame;
};
static struct revert_game_info revert_game_data;

/* -------- static functions */
static void scan_and_add_scenery(void);
static void complete_restoring_level(struct wad_data *wad);
static void load_redundant_map_data(short *redundant_data, size_t count);
static void allocate_map_structure_for_map(struct wad_data *wad);
static wad_data *build_export_wad(wad_header *header, int32 *length);
static struct wad_data *build_save_game_wad(struct wad_header *header, int32 *length);

static void allocate_map_for_counts(size_t polygon_count, size_t side_count,
	size_t endpoint_count, size_t line_count);
static void load_points(uint8 *points, size_t count);
static void load_lines(uint8 *lines, size_t count);
static void load_sides(uint8 *sides, size_t count, short version);
static void load_polygons(uint8 *polys, size_t count, short version);
static void load_lights(uint8 *_lights, size_t count, short version);
static void load_annotations(uint8 *annotations, size_t count);
static void load_objects(uint8 *map_objects, size_t count, short version);
static void load_media(uint8 *_medias, size_t count);
static void load_map_info(uint8 *map_info);
static void load_ambient_sound_images(uint8 *data, size_t count);
static void load_random_sound_images(uint8 *data, size_t count);
static void load_terminal_data(uint8 *data, size_t length);

/* Used _ONLY_ by game_wad.c internally and precalculate.c. */
// ZZZ: hmm, no longer true, now using when resuming a network saved-game... hope that's ok?...
//static bool process_map_wad(struct wad_data *wad, bool restoring_game, short version);

/* Final three calls, must be in this order! */
static void recalculate_redundant_map(void);
static void scan_and_add_platforms(uint8 *platform_static_data, size_t count, short version);
static void complete_loading_level(short *_map_indexes, size_t map_index_count, 
	uint8 *_platform_data, size_t platform_data_count,
	uint8 *actual_platform_data, size_t actual_platform_data_count, short version);

static uint8 *unpack_directory_data(uint8 *Stream, directory_data *Objects, size_t Count);
//static uint8 *pack_directory_data(uint8 *Stream, directory_data *Objects, int Count);

/* ------------------------ Net functions */
int32 get_net_map_data_length(
	void *data) 
{
	return get_flat_data_length(data);
}

/* Note that this frees it as well */
bool process_net_map_data(
	void *data) 
{
	struct wad_header header;
	struct wad_data *wad;
	bool success= false;
	
	wad= inflate_flat_data(data, &header);
	if(wad)
	{
		success= process_map_wad(wad, false, header.data_version);
		free_wad(wad); /* Note that the flat data points into the wad. */
	}
	
	return success;
}

/* This will have to do some interesting voodoo with union wads, methinks */
void *get_map_for_net_transfer(
	struct entry_point *entry)
{
	assert(file_is_set);
	
	/* false means don't use union maps.. */
	return get_flat_data(MapFileSpec, false, entry->level_number);
}

/* ---------------------- End Net Functions ----------- */

/* This takes a cstring */
void set_map_file(FileSpecifier& File, bool loadScripts)
{
	// Do whatever parameter restoration is specified before changing the file
	if (file_is_set) RunRestorationScript();

	MapFileSpec = File;
	set_scenario_images_file(File);
	// Only need to do this here
	if(loadScripts) LoadLevelScripts(File);

	// Don't care whether there was an error when checking on the file's scenario images
	clear_game_error();

	file_is_set= true;
}

/* Set to the default map.. (Only if no map doubleclicked upon on startup.. */
void set_to_default_map(
	void)
{
	FileSpecifier NewMapFile;
	
	get_default_map_spec(NewMapFile);
	set_map_file(NewMapFile);
}

/* Return true if it finds the file, and it sets the mapfile to that file. */
/* Otherwise it returns false, meaning that we need have the file sent to us. */
bool use_map_file(
	uint32 checksum)
{
	FileSpecifier File;
	bool success= false;

	if(find_wad_file_that_has_checksum(File, _typecode_scenario, strPATHS, checksum))
	{
		set_map_file(File);
		success= true;
	}

	return success;
}

dynamic_data get_dynamic_data_from_save(FileSpecifier& File)
{
	OpenedFile MapFile;
	dynamic_data dynamic_data_return;

	if (open_wad_file_for_reading(File, MapFile))
	{
		wad_header header;
		if (read_wad_header(MapFile, &header))
		{
			auto wad = read_indexed_wad_from_file(MapFile, &header, 0, true);
			if (wad)
			{
				get_dynamic_data_from_wad(wad, &dynamic_data_return);
				free_wad(wad);
			}
		}

		close_wad_file(MapFile);
	}

	return dynamic_data_return;
}

bool load_level_from_map(
	short level_index)
{
	OpenedFile OFile;
	struct wad_header header;
	struct wad_data *wad;
	short index_to_load;
	bool restoring_game= false;

	if(file_is_set)
	{
		/* Determine what we are trying to do.. */
		if(level_index==NONE)
		{
			restoring_game= true;
			index_to_load= 0; /* Saved games are always index 0 */
		} else {
			index_to_load= level_index;
		}
		
		OpenedFile MapFile;
		if (open_wad_file_for_reading(MapFileSpec,MapFile))
		{
			/* Read the file */
			if(read_wad_header(MapFile, &header))
			{
				if(index_to_load>=0 && index_to_load<header.wad_count)
				{
                        
					wad= read_indexed_wad_from_file(MapFile, &header, index_to_load, true);
					if (wad)
					{
						/* Process everything... */
						process_map_wad(wad, restoring_game, header.data_version);
		
						/* Nuke our memory... */
						free_wad(wad);
					} else {
						// error code has been set...
					}
				} else {
					set_game_error(gameError, errWadIndexOutOfRange);
				}
			} else {
				// error code has been set...
			}
		
			/* Close the file.. */
			close_wad_file(MapFile);
		} else {
			// error code has been set..
		}
	} else {
		set_game_error(gameError, errMapFileNotSet);
	}

	/* ... and bail */
	return (!error_pending());
}

// keep these around for level export
static std::vector<static_platform_data> static_platforms;

extern bool ok_to_reset_scenery_solidity;

/* Hopefully this is in the correct order of initialization... */
/* This sucks, beavis. */
void complete_loading_level(
	short *_map_indexes,
	size_t map_index_count,
	uint8 *_platform_data,
	size_t platform_data_count,
	uint8 *actual_platform_data,
	size_t actual_platform_data_count,
	short version)
{
	/* Scan, add the doors, recalculate, and generally tie up all loose ends */
	/* Recalculate the redundant data.. */
	load_redundant_map_data(_map_indexes, map_index_count);

	static_platforms.clear();

	/* Add the platforms. */
	if(_platform_data || (_platform_data==NULL && actual_platform_data==NULL))
	{
		scan_and_add_platforms(_platform_data, platform_data_count, version);
	} else {
		assert(actual_platform_data);
		PlatformList.resize(actual_platform_data_count);
		unpack_platform_data(actual_platform_data,platforms,actual_platform_data_count);
		assert(actual_platform_data_count == static_cast<size_t>(static_cast<int16>(actual_platform_data_count)));
		assert(0 <= static_cast<int16>(actual_platform_data_count));
		dynamic_world->platform_count= static_cast<int16>(actual_platform_data_count);
	}

	scan_and_add_scenery();
	ok_to_reset_scenery_solidity = true;
	
	/* Gotta do this after recalculate redundant.. */
	if(version==MARATHON_ONE_DATA_VERSION)
	{
		short loop;
		
		for(loop= 0; loop<dynamic_world->side_count; ++loop)
		{
			guess_side_lightsource_indexes(loop);
			if (static_world->environment_flags&_environment_vacuum)
			{
				side_data *side= get_side_data(loop);
				if (side->flags&_side_is_control_panel)
					side->flags |= _side_is_m1_lighted_switch;
			}
		}
	}
}

/* Call with location of NULL to get the number of start locations for a */
/* given team or player */
short get_player_starting_location_and_facing(
	short team, 
	short index, 
	struct object_location *location)
{
	short ii;
	struct map_object *saved_object;
	short count= 0;
	bool done= false;
	
	saved_object= saved_objects;
	for(ii=0; !done && ii<dynamic_world->initial_objects_count; ++ii)
	{
		if(saved_object->type==_saved_player)
		{
			/* index=NONE means use any starting location */
			if(saved_object->index==team || team==NONE)
			{
				if(location && count==index)
				{
					location->p= saved_object->location;
					location->polygon_index= saved_object->polygon_index;
					location->yaw= saved_object->facing;
					location->pitch= 0;
					location->flags= saved_object->flags;
					done= true;
				}
				count++;
			}
		}
		++saved_object;
	}
	
	/* If they asked for a valid location, make sure that we gave them one */
	if(location) vassert(done, csprintf(temporary, "Tried to place: %d only %d starting pts.", index, count));
	
	return count;
}

uint32 get_current_map_checksum(
	void)
{
	// fileref file_handle;
	struct wad_header header;

	assert(file_is_set);
	OpenedFile MapFile;
	open_wad_file_for_reading(MapFileSpec, MapFile);
	assert(MapFile.IsOpen());

	/* Read the file */
	read_wad_header(MapFile, &header);
	
	/* Close the file.. */
	close_wad_file(MapFile);	
	
	return header.checksum;
}

// ZZZ: split this out from new_game for sharing
void set_saved_game_name_to_default()
{
	revert_game_data.SavedGame.SetToSavedGamesDir();
	revert_game_data.SavedGame += getcstr(temporary, strFILENAMES, filenameDEFAULT_SAVE_GAME);
}

extern void ResetPassedLua();

bool new_game(
	short number_of_players, 
	bool network,
	struct game_data *game_information,
	struct player_start_data *player_start_information,
	struct entry_point *entry_point)
{
	assert(!network || number_of_players == NetGetNumberOfPlayers());
	
	const short intended_local_player_index = network ? NetGetLocalPlayerIndex() : 0;
	
	short player_index, i;
	bool success= true;

	ResetPassedLua();

	/* Make sure our code is synchronized.. */
	assert(MAXIMUM_PLAYER_START_NAME_LENGTH==MAXIMUM_PLAYER_NAME_LENGTH);

	/* Initialize the global network going flag... */
	game_is_networked= network;
	
	/* If we want to save it, this is an untitled map.. */
        set_saved_game_name_to_default();

	/* Set the random seed. */
	set_random_seed(game_information->initial_random_seed);

	/* Initialize the players to a known state. This must be done before goto_level */
	/*  because it sets dynamic_world->player_count to 0, which is crucial for when */
	/*  I try to recreate the players... */
	initialize_map_for_new_game(); // memsets dynamic_world to 0

	/* Copy the game data into the dynamic_world */
	/* ajr-this used to be done only when we successfully loaded the map. however, goto_level
	 * will place the initial monsters on a level, which calls new_monster, which relies
	 * on this information being setup properly, so we do it here instead. */
	obj_copy(dynamic_world->game_information, *game_information);

	/* Load the level */	
	assert(file_is_set);
	success= goto_level(entry_point, true, number_of_players);
	/* If we were able to load the map... */
	if(success)
	{
		/* Initialize the players-> note there may be more than one player in a */
		/* non-network game, for playback.. */
		for (i=0;i<number_of_players;++i)
		{
			new_player_flags flags = (i == intended_local_player_index ? new_player_make_local_and_current : 0);
			player_index= new_player(player_start_information[i].team,
				player_start_information[i].color, player_start_information[i].identifier, flags);
			assert(player_index==i);

			/* Now copy in the name of the player.. */
			assert(strlen(player_start_information[i].name)<=MAXIMUM_PLAYER_NAME_LENGTH);
			strncpy(players[i].name, player_start_information[i].name, MAXIMUM_PLAYER_NAME_LENGTH+1);
		}

		/* we need to alert the function that reverts the game of the game setup so that
		 * new game can be called if the user wants to revert later.
		 */
		setup_revert_game_info(game_information, player_start_information, entry_point);
		
		// Reset the player queues (done here and in load_game)
		reset_action_queues();
		
		/* Load the collections */
		/* entering map might fail if NetSync() fails.. */
		success= entering_map(false);
		
                // ZZZ: set motion sensor to sane state - needs to come after entering_map() (which calls load_collections())
                reset_motion_sensor(current_player_index);
	}
	
	// LP change: adding chase-cam initialization
	ChaseCam_Initialize();

	return success;
}

bool get_indexed_entry_point(
	struct entry_point *entry_point, 
	short *index, 
	int32 type)
{
	short actual_index;
	
	// Open map file
	assert(file_is_set);
	OpenedFile MapFile;
	if (!open_wad_file_for_reading(MapFileSpec,MapFile))
		return false;

	// Read header
	wad_header header;
	if (!read_wad_header(MapFile, &header)) {
		close_wad_file(MapFile);
		return false;
	}
    
	bool success = false;
	if (header.application_specific_directory_data_size == SIZEOF_directory_data)
	{

		// New style wad
		void *total_directory_data= read_directory_data(MapFile, &header);

		assert(total_directory_data);
		for(actual_index= *index; actual_index<header.wad_count; ++actual_index)
		{
			uint8 *p = (uint8 *)get_indexed_directory_data(&header, actual_index, total_directory_data);
			directory_data directory;
			unpack_directory_data(p, &directory, 1);

			/* Find the flags that match.. */
			if(directory.entry_point_flags & type)
			{
				/* This one is valid! */
				entry_point->level_number= actual_index;
				strncpy(entry_point->level_name, directory.level_name, 66);
			
				*index= actual_index+1;
				success= true;
				break; /* Out of the for loop */
			}
		}
		free(total_directory_data);

	} else {

		// Old style wad, find the index
		for(actual_index= *index; !success && actual_index<header.wad_count; ++actual_index)
		{
			struct wad_data *wad;

			/* Read the file */
			wad= read_indexed_wad_from_file(MapFile, &header, actual_index, true);
			if (wad)
			{
				/* IF this has the proper type.. */
				size_t length;
				uint8 *p = (uint8 *)extract_type_from_wad(wad, MAP_INFO_TAG, &length);
				assert(length == SIZEOF_static_data);
				static_data map_info;
				unpack_static_data(p, &map_info, 1);

				// single-player Marathon 1 levels aren't always marked
				if (header.data_version == MARATHON_ONE_DATA_VERSION &&
				    map_info.entry_point_flags == 0)
					map_info.entry_point_flags = _single_player_entry_point;

				// Marathon 1 handled (then-unused) coop flag differently
				if (header.data_version == MARATHON_ONE_DATA_VERSION)
				{
					if (map_info.entry_point_flags & _single_player_entry_point)
						map_info.entry_point_flags |= _multiplayer_cooperative_entry_point;
					if (map_info.entry_point_flags & _multiplayer_carnage_entry_point)
						map_info.entry_point_flags &= ~_multiplayer_cooperative_entry_point;
				}

				if(map_info.entry_point_flags & type)
				{
					/* This one is valid! */
					entry_point->level_number= actual_index;
					assert(strlen(map_info.level_name)<LEVEL_NAME_LENGTH);
					strncpy(entry_point->level_name, map_info.level_name, 66);
		
					*index= actual_index+1;
					success= true;
				}
				
				free_wad(wad);
			}
		}
	}

	return success;
}

// Get vector of map entry points matching given type
bool get_entry_points(vector<entry_point> &vec, int32 type)
{
	vec.clear();

	// Open map file
	assert(file_is_set);
	OpenedFile MapFile;
	if (!open_wad_file_for_reading(MapFileSpec,MapFile))
		return false;

	// Read header
	wad_header header;
	if (!read_wad_header(MapFile, &header)) {
		close_wad_file(MapFile);
		return false;
	}

	bool success = false;
	if (header.application_specific_directory_data_size == SIZEOF_directory_data) {

		// New style wad, read directory data
		void *total_directory_data = read_directory_data(MapFile, &header);
		assert(total_directory_data);

		// Push matching directory entries into vector
		for (int i=0; i<header.wad_count; i++) {
			uint8 *p = (uint8 *)get_indexed_directory_data(&header, i, total_directory_data);
			directory_data directory;
			unpack_directory_data(p, &directory, 1);

			if (directory.entry_point_flags & type) {

				// This one is valid
				entry_point point;
				point.level_number = i;
				strncpy(point.level_name, directory.level_name, 66);
				vec.push_back(point);
				success = true;
			}
		}
		free(total_directory_data);

	} else {

		// Old style wad
		for (int i=0; i<header.wad_count; i++) {

			wad_data *wad = read_indexed_wad_from_file(MapFile, &header, i, true);
			if (!wad)
				continue;

			// Read map_info data
			size_t length;
			uint8 *p = (uint8 *)extract_type_from_wad(wad, MAP_INFO_TAG, &length);
			assert(length == SIZEOF_static_data);
			static_data map_info;
			unpack_static_data(p, &map_info, 1);

			// single-player Marathon 1 levels aren't always marked
			if (header.data_version == MARATHON_ONE_DATA_VERSION &&
			    map_info.entry_point_flags == 0)
				map_info.entry_point_flags = _single_player_entry_point;

			// Marathon 1 handled (then-unused) coop flag differently
			if (header.data_version == MARATHON_ONE_DATA_VERSION)
			{
				if (map_info.entry_point_flags & _single_player_entry_point)
					map_info.entry_point_flags |= _multiplayer_cooperative_entry_point;
				if (map_info.entry_point_flags & _multiplayer_carnage_entry_point)
					map_info.entry_point_flags &= ~_multiplayer_cooperative_entry_point;
			}

			if (map_info.entry_point_flags & type) {

				// This one is valid
				entry_point point;
				point.level_number = i;
				assert(strlen(map_info.level_name) < LEVEL_NAME_LENGTH);
				strncpy(point.level_name, map_info.level_name, 66);
				vec.push_back(point);
				success = true;
			}
				
			free_wad(wad);
		}
	}

	return success;
}

extern void LoadSoloLua();
extern void LoadReplayNetLua();
extern void LoadStatsLua();
extern bool RunLuaScript();

/* This is called when the game level is changed somehow */
/* The only thing that has to be valid in the entry point is the level_index */

/* Returns a short that is an OSErr... */
bool goto_level(
	struct entry_point *entry, 
	bool new_game,
	short number_of_players)
{
	bool success= true;

	if(!new_game)
	{
		/* Clear the current map */
		leaving_map();

		// ghs: hack to get new MML-specified sounds loaded
		SoundManager::instance()->UnloadAllSounds();
	}

	// LP: doing this here because level-specific MML may specify which level-specific
	// textures to load.
	ResetLevelScript();
	if (!game_is_networked || use_map_file(((game_info*)NetGetGameData())->parent_checksum))
	{
		RunLevelScript(entry->level_number);
	}

#if !defined(DISABLE_NETWORKING)
	/* If the game is networked, then I must call the network code to do the right */
	/* thing with the map.. */
	if(game_is_networked)
	{
		/* This function, if it is a server, calls get_map_for_net_transfer, and */
		/* then calls process_map_wad on it. Non-server receives the map and then */
		/* calls process_map_wad on it. */
		success= NetChangeMap(entry);
	} 
	else 
#endif // !defined(DISABLE_NETWORKING)
	{
		/* Load it and then rock.. */
		load_level_from_map(entry->level_number);
		if(error_pending()) success= false;
	}
	
	if (success)
	{
		// Being careful to carry over errors so that Pfhortran errors can be ignored
		short SavedType, SavedError = get_game_error(&SavedType);
		if (!game_is_networked && number_of_players == 1)
		{
			LoadSoloLua();
		}
		else if (!game_is_networked)
		{
			LoadReplayNetLua();
		}
		LoadStatsLua();

		Music::instance()->PreloadLevelMusic();
		set_game_error(SavedType,SavedError);
		
		if (!new_game)
		{
			recreate_players_for_new_level();
		}
		
		/* Load the collections */
		dynamic_world->current_level_number= entry->level_number;

		// ghs: this runs very early now
		// we want to be before place_initial_objects, and
		// before MarkLuaCollections
		RunLuaScript();

		if (film_profile.early_object_initialization)
		{
			place_initial_objects();
			initialize_control_panels_for_level();
		}

		if (!new_game) 
		{
			
			/* entering_map might fail if netsync fails, but we will have already displayed */
			/* the error.. */
			success= entering_map(false);
		}

		if (!film_profile.early_object_initialization && success)
		{
			place_initial_objects();
			initialize_control_panels_for_level();
		}
		
	}
	
//	if(!success) alert_user(fatalError, strERRORS, badReadMap, -1);
	
	/* We be done.. */
	return success;
}

/* -------------------- Private or map editor functions */
void allocate_map_for_counts(
	size_t polygon_count, 
	size_t side_count,
	size_t endpoint_count,
	size_t line_count)
{
	//long cumulative_length= 0;
	size_t automap_line_count, automap_polygon_count, map_index_count;
	// long automap_line_length, automap_polygon_length, map_index_length;

	/* Give the map indexes a whole bunch of memory (cause we can't calculate it) */
	// map_index_length= (polygon_count*32+1024)*sizeof(int16);
	map_index_count= (polygon_count*32+1024);
	
	/* Automap lines. */
	// automap_line_length= (line_count/8+((line_count%8)?1:0))*sizeof(byte);
	automap_line_count= (line_count/8+((line_count%8)?1:0));
	
	/* Automap Polygons */
	// automap_polygon_length= (polygon_count/8+((polygon_count%8)?1:0))*sizeof(byte);
	automap_polygon_count= (polygon_count/8+((polygon_count%8)?1:0));

	// cumulative_length+= polygon_count*sizeof(struct polygon_data);
	// cumulative_length+= side_count*sizeof(struct side_data);
	// cumulative_length+= endpoint_count*sizeof(struct endpoint_data);
	// cumulative_length+= line_count*sizeof(struct line_data);
	// cumulative_length+= map_index_length;
	// cumulative_length+= automap_line_length;
	// cumulative_length+= automap_polygon_length;

	/* Okay, we now have the length.  Allocate our block.. */
	// reallocate_map_structure_memory(cumulative_length);

	/* Tell the recalculation data how big it is.. */
	// set_map_index_buffer_size(map_index_length);

	/* Setup our pointers. */
	// map_polygons= (struct polygon_data *) get_map_structure_chunk(polygon_count*sizeof(struct polygon_data));
	// map_sides= (struct side_data *) get_map_structure_chunk(side_count*sizeof(struct side_data));
	// map_endpoints= (struct endpoint_data *) get_map_structure_chunk(endpoint_count*sizeof(struct endpoint_data));
	// map_lines= (struct line_data *) get_map_structure_chunk(line_count*sizeof(struct line_data));
	// map_indexes= (short *) get_map_structure_chunk(map_index_length);
	// automap_lines= (uint8 *) get_map_structure_chunk(automap_line_length);
	// automap_polygons= (uint8 *) get_map_structure_chunk(automap_polygon_length);
	
	// Most of the other stuff: reallocate here
	EndpointList.resize(endpoint_count);
	LineList.resize(line_count);
	SideList.resize(side_count);
	PolygonList.resize(polygon_count);
	AutomapLineList.resize(automap_line_count);
	AutomapPolygonList.resize(automap_polygon_count);
	
	// Map indexes: start off with none of them (of course),
	// but reserve a size equal to the map index length
	MapIndexList.clear();
	MapIndexList.reserve(map_index_count);
	dynamic_world->map_index_count= 0;
	
	// Stuff that needs the max number of polygons
	allocate_render_memory();
	allocate_flood_map_memory();
}

void load_points(
	uint8 *points,
	size_t count)
{
	size_t loop;
	
	// OK to modify input-data pointer since it's called by value
	for(loop=0; loop<count; ++loop)
	{
		world_point2d& vertex = map_endpoints[loop].vertex;
		StreamToValue(points,vertex.x);
		StreamToValue(points,vertex.y);
	}
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->endpoint_count= static_cast<int16>(count);
}

void load_lines(
	uint8 *lines, 
	size_t count)
{
	// assert(count>=0 && count<=MAXIMUM_LINES_PER_MAP);
	unpack_line_data(lines,map_lines,count);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->line_count= static_cast<int16>(count);
}

void load_sides(
	uint8 *sides, 
	size_t count,
	short version)
{
	size_t loop;

	bool reserved_side_flag = false;
	
	// assert(count>=0 && count<=MAXIMUM_SIDES_PER_MAP);

	unpack_side_data(sides,map_sides,count);

	for(loop=0; loop<count; ++loop)
	{
		// whatever editor created Siege of Nor'Khor left all kinds of unused
		// side flags set; try to detect them and clear them out
		if (map_sides[loop].flags & _reserved_side_flag)
		{
			reserved_side_flag = true;
		}
		
		if(version==MARATHON_ONE_DATA_VERSION)
		{
			map_sides[loop].transparent_texture.texture= UNONE;
			map_sides[loop].ambient_delta= 0;
			map_sides[loop].flags |= _side_item_is_optional;
		}
		++sides;
	}

	if (reserved_side_flag)
	{
		for (loop = 0; loop < count; ++loop)
		{
			static constexpr int m2_side_flags_mask = 0x007f;
			map_sides[loop].flags &= m2_side_flags_mask;
		}
	}

	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->side_count= static_cast<int16>(count);
}

void load_polygons(
	uint8 *polys, 
	size_t count,
	short version)
{
	size_t loop;

	// assert(count>=0 && count<=MAXIMUM_POLYGONS_PER_MAP);
	
	unpack_polygon_data(polys,map_polygons,count);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->polygon_count= static_cast<int16>(count);

	/* Allow for backward compatibility! */
	switch(version)
	{
		case MARATHON_ONE_DATA_VERSION:
			for(loop= 0; loop<count; ++loop)
			{
				map_polygons[loop].media_index= NONE;
				map_polygons[loop].floor_origin.x= map_polygons[loop].floor_origin.y= 0;
				map_polygons[loop].ceiling_origin.x= map_polygons[loop].ceiling_origin.y= 0;
                
                switch (map_polygons[loop].type)
                {
                    case _polygon_is_hill:
                        map_polygons[loop].type = _polygon_is_minor_ouch;
                        break;
                    case _polygon_is_base:
                        map_polygons[loop].type = _polygon_is_major_ouch;
                        break;
                    case _polygon_is_zone_border:
                        map_polygons[loop].type = _polygon_is_glue;
                        break;
                    case _polygon_is_goal:
                        map_polygons[loop].type = _polygon_is_glue_trigger;
                        break;
                    case _polygon_is_visible_monster_trigger:
                        map_polygons[loop].type = _polygon_is_superglue;
                        break;
                    case _polygon_is_invisible_monster_trigger:
                        map_polygons[loop].type = _polygon_must_be_explored;
                        break;
                    case _polygon_is_dual_monster_trigger:
                        map_polygons[loop].type = _polygon_is_automatic_exit;
                        break;
                }
			}
			break;
			
		case MARATHON_TWO_DATA_VERSION:
		// LP addition:
		case MARATHON_INFINITY_DATA_VERSION:
			break;
			
		default:
			assert(false);
			break;
	}
}

void load_lights(
	uint8 *_lights, 
	size_t count,
	short version)
{
	unsigned short loop, new_index;
	
	LightList.resize(count);
	objlist_clear(lights,count);
	// vassert(count>=0 && count<=MAXIMUM_LIGHTS_PER_MAP, csprintf(temporary, "Light count: %d vers: %d",
	//	count, version));
	
	old_light_data *OldLights;
	
	switch(version)
	{
	case MARATHON_ONE_DATA_VERSION: {
		
		// Unpack the old lights into a temporary array
		OldLights = new old_light_data[count];
		unpack_old_light_data(_lights,OldLights,count);
		
		old_light_data *OldLtPtr = OldLights;
		for(loop= 0; loop<count; ++loop, OldLtPtr++)
		{
			static_light_data TempLight;
			convert_old_light_data_to_new(&TempLight, OldLtPtr, 1);
			
			new_index = new_light(&TempLight);
			assert(new_index==loop);
		}
		delete []OldLights;
		break;			
	}
		
	case MARATHON_TWO_DATA_VERSION:
	case MARATHON_INFINITY_DATA_VERSION:
		// OK to modify the data pointer since it was passed by value
		for(loop= 0; loop<count; ++loop)
		{
			static_light_data TempLight;
			_lights = unpack_static_light_data(_lights, &TempLight, 1);
			
			new_index = new_light(&TempLight);
			assert(new_index==loop);
		}
		break;			
		
	default:
		assert(false);
		break;
	}
}

void load_annotations(
	uint8 *annotations, 
	size_t count)
{
	// assert(count>=0 && count<=MAXIMUM_ANNOTATIONS_PER_MAP);
	MapAnnotationList.resize(count);
	unpack_map_annotation(annotations,map_annotations,count);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->default_annotation_count= static_cast<int16>(count);
}

void load_objects(uint8 *map_objects, size_t count, short version)
{
	// assert(count>=0 && count<=MAXIMUM_SAVED_OBJECTS);
	SavedObjectList.resize(count);
        unpack_map_object(map_objects,saved_objects,count, version);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->initial_objects_count= static_cast<int16>(count);
}

void load_map_info(
	uint8 *map_info)
{
	unpack_static_data(map_info,static_world,1);
	static_world->ball_in_play = false;
}

void load_media(
	uint8 *_medias,
	size_t count)
{
	// struct media_data *media= _medias;
	size_t ii;
	
	MediaList.resize(count);
	objlist_clear(medias,count);
	// assert(count>=0 && count<=MAXIMUM_MEDIAS_PER_MAP);
	
	for(ii= 0; ii<count; ++ii)
	{
		media_data TempMedia;
		_medias = unpack_media_data(_medias,&TempMedia,1);
		
		size_t new_index = new_media(&TempMedia);
		assert(new_index==ii);
	}
}

void load_ambient_sound_images(
	uint8 *data,
	size_t count)
{
	// assert(count>=0 &&count<=MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP);
	AmbientSoundImageList.resize(count);
	unpack_ambient_sound_image_data(data,ambient_sound_images,count);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->ambient_sound_image_count= static_cast<int16>(count);
}

void load_random_sound_images(
	uint8 *data,
	size_t count)
{
	// assert(count>=0 &&count<=MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP);
	RandomSoundImageList.resize(count);
	unpack_random_sound_image_data(data,random_sound_images,count);
	assert(count == static_cast<size_t>(static_cast<int16>(count)));
	assert(0 <= static_cast<int16>(count));
	dynamic_world->random_sound_image_count= static_cast<int16>(count);
}

/* Recalculate all the redundant crap- must be done before platforms/doors/etc.. */
void recalculate_redundant_map(
	void)
{
	short loop;

	for(loop=0;loop<dynamic_world->polygon_count;++loop) recalculate_redundant_polygon_data(loop);
	for(loop=0;loop<dynamic_world->line_count;++loop) recalculate_redundant_line_data(loop);
	for(loop=0;loop<dynamic_world->endpoint_count;++loop) recalculate_redundant_endpoint_data(loop);
}

bool load_game_from_file(FileSpecifier& File, bool run_scripts)
{
	bool success= false;

	ResetPassedLua();
	ResetLevelScript();

	/* Setup for a revert.. */
	revert_game_data.game_is_from_disk = true;
	revert_game_data.SavedGame = File;

	uint32 parent_checksum = read_wad_file_parent_checksum(File);
	bool found_map = use_map_file(parent_checksum); /* Find the original scenario this saved game was a part of.. */

	FileSpecifier map_parent;
	if (found_map) {
		map_parent = get_map_file();
		auto dynamic_data = get_dynamic_data_from_save(File);
		RunLevelScript(dynamic_data.current_level_number);
	}

	/* Use the save game file.. */
	set_map_file(File, false);
	/* Load the level from the map */
	success= load_level_from_map(NONE); /* Save games are ALWAYS index NONE */
	if (success)
	{	
		if(found_map)
			set_map_file(map_parent, false);
		else
		{
			/* Tell the user theyÕre screwed when they try to leave this level. */
			alert_user(infoError, strERRORS, cantFindMap, 0);

			// LP addition: makes the game look normal
			hide_cursor();
		
			/* Set to the default map. */
			set_to_default_map();
		}
		
		if (run_scripts)
		{
			// LP: getting the level scripting off of the map file
			// Being careful to carry over errors so that Pfhortran errors can be ignored
			short SavedType, SavedError = get_game_error(&SavedType);
			if (!game_is_networked)
			{
				LoadSoloLua();
			}
			LoadStatsLua();
			set_game_error(SavedType,SavedError);
		}
	}

	return success;
}

void setup_revert_game_info(
	struct game_data *game_info, 
	struct player_start_data *start, 
	struct entry_point *entry)
{
	revert_game_data.game_is_from_disk = false;
	obj_copy(revert_game_data.game_information, *game_info);
	obj_copy(revert_game_data.player_start, *start);
	obj_copy(revert_game_data.entry_point, *entry);
}

extern void reset_messages();

bool revert_game(
	void)
{
	bool successful;
	
	assert(dynamic_world->player_count==1);

	leaving_map();
	
	if (revert_game_data.game_is_from_disk)
	{
		/* Reload their last saved game.. */
		successful= load_game_from_file(revert_game_data.SavedGame, true);
		if (successful) 
		{
			Music::instance()->PreloadLevelMusic();
			RunLuaScript();
			
			// LP: added for loading the textures if one had died on another level;
			// this gets around WZ's moving of this line into make_restored_game_relevant()
			successful = entering_map(true /*restoring game*/);
		}

		/* And they don't get to continue. */
		stop_recording();
	}
	else
	{
		/* This was the totally evil line discussed above. */
		successful= new_game(1, false, &revert_game_data.game_information, &revert_game_data.player_start, 
			&revert_game_data.entry_point);
			
		/* And rewind so that the last player is used. */
		rewind_recording();
	}

	if(successful)
	{
		update_interface(NONE);
		ChaseCam_Reset();
		ResetFieldOfView();
		reset_messages();
		ReloadViewContext();
	}
	
	return successful;
}

bool export_level(FileSpecifier& File)
{
	struct wad_header header;
	short err = 0;
	bool success = false;
	int32 offset, wad_length;
	struct directory_entry entry;
	struct wad_data *wad;

	FileSpecifier TempFile;
	TempFile.SetTempName(File);

	/* Fill in the default wad header (we are using File instead of TempFile to get the name right in the header) */
	fill_default_wad_header(File, CURRENT_WADFILE_VERSION, MARATHON_TWO_DATA_VERSION, 1, 0, &header);

	if (create_wadfile(TempFile, _typecode_scenario))
	{
		OpenedFile SaveFile;
		if (open_wad_file_for_writing(TempFile, SaveFile))
		{
			/* Write out the new header */
			if (write_wad_header(SaveFile, &header))
			{
				offset = SIZEOF_wad_header;
				
				wad = build_export_wad(&header, &wad_length);
				if (wad)
				{
					set_indexed_directory_offset_and_length(&header, &entry, 0, offset, wad_length, 0);
					
					if (write_wad(SaveFile, &header, wad, offset))
					{
						/* Update the new header */
												offset+= wad_length;
						header.directory_offset= offset;
						if (write_wad_header(SaveFile, &header) && write_directorys(SaveFile, &header, &entry))
						{
							/* We win. */
							success= true;
						} 
					}
					
					free_wad(wad);
				}
			}

			err = SaveFile.GetError();
			calculate_and_store_wadfile_checksum(SaveFile);
			close_wad_file(SaveFile);
		}

		if (!err)
		{
			// We can't delete open files on Windows, so close
			// the current level before we overwrite it.
			bool restore_images = false;
			if (File == get_map_file())
			{
				unset_scenario_images_file();
				restore_images = true;
			}
			if (!TempFile.Rename(File))
			{
				err = 1;
			}
			if (restore_images)
			{
				set_scenario_images_file(File);
				clear_game_error();
			}
		}
	}
	
	if (err || error_pending())
	{	
		success = false;
	}


	return success;
	
}

void get_current_saved_game_name(FileSpecifier& File)
{
	File = revert_game_data.SavedGame;
}

/* The current mapfile should be set to the save game file... */
bool save_game_file(FileSpecifier& File, const std::string& metadata, const std::string& imagedata)
{
	struct wad_header header;
	short err = 0;
	bool success= false;
	int32 offset, wad_length;
	struct directory_entry entries[2];
	struct wad_data *wad, *meta_wad;

	/* Save off the random seed. */
	dynamic_world->random_seed= get_random_seed();

	/* Setup to revert the game properly */
	revert_game_data.game_is_from_disk= true;
	revert_game_data.SavedGame = File;

	// LP: add a file here; use temporary file for a safe save.
	// Write into the temporary file first
	FileSpecifier TempFile;
	TempFile.SetTempName(File);
	
	/* Fill in the default wad header (we are using File instead of TempFile to get the name right in the header) */
	fill_default_wad_header(File, CURRENT_WADFILE_VERSION, EDITOR_MAP_VERSION, 2, 0, &header);
		
	/* Assume that we confirmed on save as... */
	if (create_wadfile(TempFile,_typecode_savegame))
	{
		OpenedFile SaveFile;
		if(open_wad_file_for_writing(TempFile,SaveFile))
		{
			/* Write out the new header */
			if (write_wad_header(SaveFile, &header))
			{
				offset= SIZEOF_wad_header;
		
				wad= build_save_game_wad(&header, &wad_length);
				if (wad)
				{
					/* Set the entry data.. */
					set_indexed_directory_offset_and_length(&header, 
						entries, 0, offset, wad_length, 0);
					
					/* Save it.. */
					if (write_wad(SaveFile, &header, wad, offset))
					{
						/* Update the new header */
						offset+= wad_length;
						header.directory_offset= offset;
						header.parent_checksum= read_wad_file_checksum(MapFileSpec);
						
						/* Create metadata wad */
						meta_wad = build_meta_game_wad(metadata, imagedata, &header, &wad_length);
						if (meta_wad)
						{
							set_indexed_directory_offset_and_length(&header,
								entries, 1, offset, wad_length, SAVE_GAME_METADATA_INDEX);
							
							if (write_wad(SaveFile, &header, meta_wad, offset))
							{
								offset+= wad_length;
								header.directory_offset= offset;
						
								if (write_wad_header(SaveFile, &header) && write_directorys(SaveFile, &header, entries))
								{
									/* We win. */
									success= true;
								}
							}
							
							free_wad(meta_wad);
						}
					}

					free_wad(wad);
				}
			}

			err = SaveFile.GetError();
			close_wad_file(SaveFile);
		}
		
		if (!err)
		{
			if (!TempFile.Rename(File))
			{
				err = 1;
			}
		}
	}
	
	if(err || error_pending())
	{
		if(!err) err= get_game_error(NULL);
		alert_user(infoError, strERRORS, fileError, err);
		clear_game_error();
		success= false;
	}
	
	return success;
}

/* -------- static functions */
static void scan_and_add_platforms(
	uint8 *platform_static_data,
	size_t count,
	short version)
{
	struct polygon_data *polygon;
	short loop;

	PlatformList.resize(count);
	objlist_clear(platforms,count);

	static_platforms.resize(count);
	unpack_static_platform_data(platform_static_data, static_platforms.data(), count);

	polygon= map_polygons;
	for(loop=0; loop<dynamic_world->polygon_count; ++loop)
	{
		if (polygon->type==_polygon_is_platform)
		{
			/* Search and find the extra data.  If it is not there, use the permutation for */
			/* backwards compatibility! */

			size_t platform_static_data_index;
			for(platform_static_data_index = 0; platform_static_data_index<count; ++platform_static_data_index)
			{
				if (static_platforms[platform_static_data_index].polygon_index == loop)
				{
					new_platform(&static_platforms[platform_static_data_index], loop, version);
					break;
				}
			}
			
			/* DIdn't find it- use a standard platform */
			if(platform_static_data_index==count)
			{
				polygon->permutation= 1;
				new_platform(get_defaults_for_platform_type(polygon->permutation), loop, version);
			}	
		}
		++polygon;
	}
}


extern void unpack_lua_states(uint8*, size_t);

/* Load a level from a wad-> mainly used by the net stuff. */
bool process_map_wad(
	struct wad_data *wad, 
	bool restoring_game,
	short version)
{
	size_t data_length;
	uint8 *data;
	size_t count;
	bool is_preprocessed_map= false;

	assert(version==MARATHON_INFINITY_DATA_VERSION || version==MARATHON_TWO_DATA_VERSION || version==MARATHON_ONE_DATA_VERSION);

	/* zero everything so no slots are used */	
	initialize_map_for_new_level();

	/* Calculate the length (for reallocate map) */
	allocate_map_structure_for_map(wad);

	/* Extract points */
	data= (uint8 *)extract_type_from_wad(wad, POINT_TAG, &data_length);
	count= data_length/SIZEOF_world_point2d;
	assert(data_length == count*SIZEOF_world_point2d);
	
	if(count)
	{
		load_points(data, count);
	} else {
         
		data= (uint8 *)extract_type_from_wad(wad, ENDPOINT_DATA_TAG, &data_length);
		count= data_length/SIZEOF_endpoint_data;
		assert(data_length == count*SIZEOF_endpoint_data);
		// assert(count>=0 && count<MAXIMUM_ENDPOINTS_PER_MAP);

		/* Slam! */
		unpack_endpoint_data(data,map_endpoints,count);
		assert(count == static_cast<size_t>(static_cast<int16>(count)));
		assert(0 <= static_cast<int16>(count));
		dynamic_world->endpoint_count= static_cast<int16>(count);

		if (version > MARATHON_ONE_DATA_VERSION)
			is_preprocessed_map= true;
	}

	/* Extract lines */
	data= (uint8 *)extract_type_from_wad(wad, LINE_TAG, &data_length);
	count = data_length/SIZEOF_line_data;
	assert(data_length == count*SIZEOF_line_data);
	load_lines(data, count);

	/* Order is important! */
	data= (uint8 *)extract_type_from_wad(wad, SIDE_TAG, &data_length);
	count = data_length/SIZEOF_side_data;
	assert(data_length == count*SIZEOF_side_data);
	load_sides(data, count, version);

	/* Extract polygons */
	data= (uint8 *)extract_type_from_wad(wad, POLYGON_TAG, &data_length);
	count = data_length/SIZEOF_polygon_data;
	assert(data_length == count*SIZEOF_polygon_data);
	load_polygons(data, count, version);
	
	/* Extract the lightsources */
	if(restoring_game)
	{
		// Slurp them in
		data= (uint8 *)extract_type_from_wad(wad, LIGHTSOURCE_TAG, &data_length);
		count = data_length/SIZEOF_light_data;
		assert(data_length == count*SIZEOF_light_data);
		LightList.resize(count);
		unpack_light_data(data,lights,count);
	}
	else
	{
		/* When you are restoring a game, the actual light structure is set. */
		data= (uint8 *)extract_type_from_wad(wad, LIGHTSOURCE_TAG, &data_length);
		if(version==MARATHON_ONE_DATA_VERSION) 
		{
			/* We have an old style light */
			count= data_length/SIZEOF_old_light_data;
			assert(count*SIZEOF_old_light_data==data_length);
			load_lights(data, count, version);
		} else {
			count= data_length/SIZEOF_static_light_data;
			assert(count*SIZEOF_static_light_data==data_length);
			load_lights(data, count, version);
		}

		//	HACK!!!!!!!!!!!!!!! vulcan doesnÕt NONE .first_object field after adding scenery
		{
			for (count= 0; count<static_cast<size_t>(dynamic_world->polygon_count); ++count)
			{
				map_polygons[count].first_object= NONE;
			}
		}
	}

	/* Extract the annotations */
	data= (uint8 *)extract_type_from_wad(wad, ANNOTATION_TAG, &data_length);
	count = data_length/SIZEOF_map_annotation;
	assert(data_length == count*SIZEOF_map_annotation);
	load_annotations(data, count);

	/* Extract the objects */
	data= (uint8 *)extract_type_from_wad(wad, OBJECT_TAG, &data_length);
	count = data_length/SIZEOF_map_object;
	assert(data_length == count*static_cast<size_t>(SIZEOF_map_object));
	load_objects(data, count, version);

	/* Extract the map info data */
	data= (uint8 *)extract_type_from_wad(wad, MAP_INFO_TAG, &data_length);
	// LP change: made this more Pfhorte-friendly
	assert(static_cast<size_t>(SIZEOF_static_data)==data_length 
		|| static_cast<size_t>(SIZEOF_static_data-2)==data_length);
	load_map_info(data);
    if (version == MARATHON_ONE_DATA_VERSION)
    {
        if (static_world->mission_flags & _mission_exploration)
        {
            static_world->mission_flags &= ~_mission_exploration;
            static_world->mission_flags |= _mission_exploration_m1;
        }
        if (static_world->mission_flags & _mission_rescue)
        {
            static_world->mission_flags &= ~_mission_rescue;
            static_world->mission_flags |= _mission_rescue_m1;
        }
        if (static_world->mission_flags & _mission_repair)
        {
            static_world->mission_flags &= ~_mission_repair;
            static_world->mission_flags |= _mission_repair_m1;
        }
        if (static_world->environment_flags & _environment_rebellion)
        {
            static_world->environment_flags &= ~_environment_rebellion;
            static_world->environment_flags |= _environment_rebellion_m1;
        }
        static_world->environment_flags |= _environment_glue_m1|_environment_ouch_m1|_environment_song_index_m1|_environment_terminals_stop_time|_environment_activation_ranges|_environment_m1_weapons;
        
    }

    if (static_world->environment_flags & _environment_song_index_m1) {
	    Music::instance()->SetClassicLevelMusic(static_world->song_index);
    }

	/* Extract the game difficulty info.. */
	data= (uint8 *)extract_type_from_wad(wad, ITEM_PLACEMENT_STRUCTURE_TAG, &data_length);
	// In case of an absent placement chunk...
	if (data_length == 0)
	{
		data = new uint8[2*MAXIMUM_OBJECT_TYPES*SIZEOF_object_frequency_definition];
		memset(data,0,2*MAXIMUM_OBJECT_TYPES*SIZEOF_object_frequency_definition);
	}
	else
		assert(data_length == 2*MAXIMUM_OBJECT_TYPES*SIZEOF_object_frequency_definition);
	load_placement_data(data + MAXIMUM_OBJECT_TYPES*SIZEOF_object_frequency_definition, data);
	if (data_length == 0)
		delete []data;
	
	/* Extract the terminal data. */
	data= (uint8 *)extract_type_from_wad(wad, TERMINAL_DATA_TAG, &data_length);
	load_terminal_data(data, data_length);

	/* Extract the media definitions */
	if(restoring_game)
	{
		// Slurp it in
		data= (uint8 *)extract_type_from_wad(wad, MEDIA_TAG, &data_length);
		count= data_length/SIZEOF_media_data;
		assert(count*SIZEOF_media_data==data_length);
		MediaList.resize(count);
		unpack_media_data(data,medias,count);
	}
	else
	{
		data= (uint8 *)extract_type_from_wad(wad, MEDIA_TAG, &data_length);
		count= data_length/SIZEOF_media_data;
		assert(count*SIZEOF_media_data==data_length);
		load_media(data, count);
	}

	/* Extract the ambient sound images */
	data= (uint8 *)extract_type_from_wad(wad, AMBIENT_SOUND_TAG, &data_length);
	count = data_length/SIZEOF_ambient_sound_image_data;
	assert(data_length == count*SIZEOF_ambient_sound_image_data);
	load_ambient_sound_images(data, count);
	load_ambient_sound_images(data, data_length/SIZEOF_ambient_sound_image_data);

	/* Extract the random sound images */
	data= (uint8 *)extract_type_from_wad(wad, RANDOM_SOUND_TAG, &data_length);
	count = data_length/SIZEOF_random_sound_image_data;
	assert(data_length == count*SIZEOF_random_sound_image_data);
	load_random_sound_images(data, count);

	/* Extract embedded shapes */
	data= (uint8 *)extract_type_from_wad(wad, SHAPE_PATCH_TAG, &data_length);
	set_shapes_patch_data(data, data_length);

	/* Extract MMLS */
	data= (uint8 *)extract_type_from_wad(wad, MMLS_TAG, &data_length);
	SetMMLS(data, data_length);

	/* Extract LUAS */
	data= (uint8 *)extract_type_from_wad(wad, LUAS_TAG, &data_length);
	SetLUAS(data, data_length);

	/* Extract saved Lua state */
	data =(uint8 *)extract_type_from_wad(wad, LUA_STATE_TAG, &data_length);
	unpack_lua_states(data, data_length);

	// LP addition: load the physics-model chunks (all fixed-size)
	bool PhysicsModelLoaded = false;
	
	data= (uint8 *)extract_type_from_wad(wad, MONSTER_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_monster_definition;
	assert(count*SIZEOF_monster_definition == data_length);
	assert(count <= NUMBER_OF_MONSTER_TYPES);
	if (data_length > 0)
	{
		if (!PhysicsModelLoaded) init_physics_wad_data();
		PhysicsModelLoaded = true;
		unpack_monster_definition(data,count);
	}
	
	data= (uint8 *)extract_type_from_wad(wad, EFFECTS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_effect_definition;
	assert(count*SIZEOF_effect_definition == data_length);
	assert(count <= NUMBER_OF_EFFECT_TYPES);
	if (data_length > 0)
	{
		if (!PhysicsModelLoaded) init_physics_wad_data();
		PhysicsModelLoaded = true;
		unpack_effect_definition(data,count);
	}
	
	data= (uint8 *)extract_type_from_wad(wad, PROJECTILE_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_projectile_definition;
	assert(count*SIZEOF_projectile_definition == data_length);
	assert(count <= NUMBER_OF_PROJECTILE_TYPES);
	if (data_length > 0)
	{
		if (!PhysicsModelLoaded) init_physics_wad_data();
		PhysicsModelLoaded = true;
		unpack_projectile_definition(data,count);
	}
	
	data= (uint8 *)extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_physics_constants;
	assert(count*SIZEOF_physics_constants == data_length);
	assert(count <= get_number_of_physics_models());
	if (data_length > 0)
	{
		if (!PhysicsModelLoaded) init_physics_wad_data();
		PhysicsModelLoaded = true;
		unpack_physics_constants(data,count);
	}
	
	data= (uint8 *)extract_type_from_wad(wad, WEAPONS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_weapon_definition;
	assert(count*SIZEOF_weapon_definition == data_length);
	assert(count <= get_number_of_weapon_types());
	if (data_length > 0)
	{
		if (!PhysicsModelLoaded) init_physics_wad_data();
		PhysicsModelLoaded = true;
		unpack_weapon_definition(data,count);
	}
	
	// LP addition: Reload the physics model if it had been loaded in the previous level,
	// but not in the current level. This avoids the persistent-physics bug.
	// ghs: always reload the physics model if there isn't one merged
	if (PhysicsModelLoadedEarlier && !PhysicsModelLoaded && !game_is_networked)
		import_definition_structures();
	PhysicsModelLoadedEarlier = PhysicsModelLoaded;
	
	RunScriptChunks();

	init_ephemera(dynamic_world->polygon_count);

	/* If we are restoring the game, then we need to add the dynamic data */
	if(restoring_game)
	{
		// Slurp it all in...
		data= (uint8 *)extract_type_from_wad(wad, MAP_INDEXES_TAG, &data_length);
		count= data_length/sizeof(short);
		assert(count*int32(sizeof(short))==data_length);
		MapIndexList.resize(count);
		StreamToList(data,map_indexes,count);
		
		data= (uint8 *)extract_type_from_wad(wad, PLAYER_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_player_data;
		assert(count*SIZEOF_player_data==data_length);
		unpack_player_data(data,players,count);
		team_damage_from_player_data();
		
		get_dynamic_data_from_wad(wad, dynamic_world);
		
		data= (uint8 *)extract_type_from_wad(wad, OBJECT_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_object_data;
		assert(count*SIZEOF_object_data==data_length);
		vassert(count <= MAXIMUM_OBJECTS_PER_MAP,
			csprintf(temporary,"Number of map objects %zu > limit %u",count,MAXIMUM_OBJECTS_PER_MAP));
		unpack_object_data(data,objects,count);
		
		// Unpacking is E-Z here...
		data= (uint8 *)extract_type_from_wad(wad, AUTOMAP_LINES, &data_length);
		memcpy(automap_lines,data,data_length);
		data= (uint8 *)extract_type_from_wad(wad, AUTOMAP_POLYGONS, &data_length);
		memcpy(automap_polygons,data,data_length);

		data= (uint8 *)extract_type_from_wad(wad, MONSTERS_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_monster_data;
		assert(count*SIZEOF_monster_data==data_length);
		vassert(count <= MAXIMUM_MONSTERS_PER_MAP,
			csprintf(temporary,"Number of monsters %zu > limit %u",count,MAXIMUM_MONSTERS_PER_MAP));
		unpack_monster_data(data,monsters,count);

		data= (uint8 *)extract_type_from_wad(wad, EFFECTS_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_effect_data;
		assert(count*SIZEOF_effect_data==data_length);
		vassert(count <= MAXIMUM_EFFECTS_PER_MAP,
			csprintf(temporary,"Number of effects %zu > limit %u",count,MAXIMUM_EFFECTS_PER_MAP));
		unpack_effect_data(data,effects,count);

		data= (uint8 *)extract_type_from_wad(wad, PROJECTILES_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_projectile_data;
		assert(count*SIZEOF_projectile_data==data_length);
		vassert(count <= MAXIMUM_PROJECTILES_PER_MAP,
			csprintf(temporary,"Number of projectiles %zu > limit %u",count,MAXIMUM_PROJECTILES_PER_MAP));
		unpack_projectile_data(data,projectiles,count);
		
		data= (uint8 *)extract_type_from_wad(wad, PLATFORM_STRUCTURE_TAG, &data_length);
		count= data_length/SIZEOF_platform_data;
		assert(count*SIZEOF_platform_data==data_length);
		PlatformList.resize(count);
		unpack_platform_data(data,platforms,count);
		
		data= (uint8 *)extract_type_from_wad(wad, WEAPON_STATE_TAG, &data_length);
		count= data_length/SIZEOF_player_weapon_data;
		assert(count*SIZEOF_player_weapon_data==data_length);
		unpack_player_weapon_data(data,count);
		
		data= (uint8 *)extract_type_from_wad(wad, TERMINAL_STATE_TAG, &data_length);
		count= data_length/SIZEOF_player_terminal_data;
		assert(count*SIZEOF_player_terminal_data==data_length);
		unpack_player_terminal_data(data,count);
		
		complete_restoring_level(wad);
	} else {
		uint8 *map_index_data;
		size_t map_index_count;
		uint8 *platform_structures;
		size_t platform_structure_count;

		if(version==MARATHON_ONE_DATA_VERSION)
		{
			/* Force precalculation */
			map_index_data= NULL;
			map_index_count= 0; 
		} else {
			map_index_data= (uint8 *)extract_type_from_wad(wad, MAP_INDEXES_TAG, &data_length);
			map_index_count= data_length/sizeof(short);
			assert(map_index_count*sizeof(short)==data_length);
		}

		assert((is_preprocessed_map && map_index_count) || (!is_preprocessed_map && !map_index_count));

		data= (uint8 *)extract_type_from_wad(wad, PLATFORM_STATIC_DATA_TAG, &data_length);
		count= data_length/SIZEOF_static_platform_data;
		assert(count*SIZEOF_static_platform_data==data_length);
		
		platform_structures= (uint8 *)extract_type_from_wad(wad, PLATFORM_STRUCTURE_TAG, &data_length);
		platform_structure_count= data_length/SIZEOF_platform_data;
		assert(platform_structure_count*SIZEOF_platform_data==data_length);
		
		complete_loading_level((short *) map_index_data, map_index_count,
			data, count, platform_structures,
			platform_structure_count, version);

	}
	
	/* ... and bail */
	return true;
}

void get_dynamic_data_from_wad(wad_data* wad, dynamic_data* dest)
{
	size_t data_length;
	uint8* data = (uint8*)extract_type_from_wad(wad, DYNAMIC_STRUCTURE_TAG, &data_length);
	assert(data_length == SIZEOF_dynamic_data);
	unpack_dynamic_data(data, dest, 1);
}

static void allocate_map_structure_for_map(
	struct wad_data *wad)
{
	size_t data_length;
	size_t line_count, polygon_count, side_count, endpoint_count;

	/* Extract points */
	extract_type_from_wad(wad, POINT_TAG, &data_length);
	endpoint_count= data_length/SIZEOF_world_point2d;
	if(endpoint_count*SIZEOF_world_point2d!=data_length) alert_corrupted_map(0x7074); // 'pt'
	
	if(!endpoint_count)
	{
		extract_type_from_wad(wad, ENDPOINT_DATA_TAG, &data_length);
		endpoint_count= data_length/SIZEOF_endpoint_data;
		if(endpoint_count*SIZEOF_endpoint_data!=data_length) alert_corrupted_map(0x6570); // 'ep'
	}

	/* Extract lines */
	extract_type_from_wad(wad, LINE_TAG, &data_length);
	line_count= data_length/SIZEOF_line_data;
	if(line_count*SIZEOF_line_data!=data_length) alert_corrupted_map(0x6c69); // 'li'

	/* Sides.. */
	extract_type_from_wad(wad, SIDE_TAG, &data_length);
	side_count= data_length/SIZEOF_side_data;
	if(side_count*SIZEOF_side_data!=data_length) alert_corrupted_map(0x7369); // 'si'

	/* Extract polygons */
	extract_type_from_wad(wad, POLYGON_TAG, &data_length);
	polygon_count= data_length/SIZEOF_polygon_data;
	if(polygon_count*SIZEOF_polygon_data!=data_length) alert_corrupted_map(0x7369); // 'si'

	allocate_map_for_counts(polygon_count, side_count, endpoint_count, line_count);
}

/* Note that we assume the redundant data has already been recalculated... */
static void load_redundant_map_data(
	short *redundant_data,
	size_t count)
{
	if (redundant_data)
	{
		// assert(redundant_data && map_indexes);
		uint8 *Stream = (uint8 *)redundant_data;
		MapIndexList.resize(count);
		StreamToList(Stream,map_indexes,count);
		assert(count == static_cast<size_t>(static_cast<int16>(count)));
		assert(0 <= static_cast<int16>(count));
		dynamic_world->map_index_count= static_cast<int16>(count);
	}
	else
	{
		recalculate_redundant_map();
		precalculate_map_indexes();
	}
}

void load_terminal_data(
	uint8 *data, 
	size_t length)
{
	/* I would really like it if I could get these into computer_interface.c statically */
	unpack_map_terminal_data(data,length);
}

static void scan_and_add_scenery(
	void)
{
	short ii;
	struct map_object *saved_object;
	
	saved_object= saved_objects;
	for(ii=0; ii<dynamic_world->initial_objects_count; ++ii)
	{
		if (saved_object->type==_saved_object)
		{
			struct object_location location;
			
			location.p= saved_object->location;
			location.flags= saved_object->flags;
			location.yaw= saved_object->facing;
			location.polygon_index= saved_object->polygon_index;
			new_scenery(&location, saved_object->index);
		}
		
		++saved_object;
	} 
}

struct save_game_data 
{
	uint32 tag;
	short unit_size;
	bool loaded_by_level;
};

#define NUMBER_OF_EXPORT_ARRAYS (sizeof(export_data)/sizeof(struct save_game_data))
save_game_data export_data[]=
{
	{ POINT_TAG, SIZEOF_world_point2d, true },
	{ LINE_TAG, SIZEOF_line_data, true },
	{ POLYGON_TAG, SIZEOF_polygon_data, true },
	{ SIDE_TAG, SIZEOF_side_data, true },
	{ LIGHTSOURCE_TAG, SIZEOF_static_light_data, true, },
	{ ANNOTATION_TAG, SIZEOF_map_annotation, true },
	{ OBJECT_TAG, SIZEOF_map_object, true },
	{ MAP_INFO_TAG, SIZEOF_static_data, true },
	{ ITEM_PLACEMENT_STRUCTURE_TAG, SIZEOF_object_frequency_definition, true },
	{ PLATFORM_STATIC_DATA_TAG, SIZEOF_static_platform_data, true },
	{ TERMINAL_DATA_TAG, sizeof(byte), true },
	{ MEDIA_TAG, SIZEOF_media_data, true }, // false },
	{ AMBIENT_SOUND_TAG, SIZEOF_ambient_sound_image_data, true },
	{ RANDOM_SOUND_TAG, SIZEOF_random_sound_image_data, true },
	{ SHAPE_PATCH_TAG, sizeof(byte), true },
//	{ PLATFORM_STRUCTURE_TAG, SIZEOF_platform_data, true },
};

#define NUMBER_OF_SAVE_ARRAYS (sizeof(save_data)/sizeof(struct save_game_data))
struct save_game_data save_data[]=
{
	{ ENDPOINT_DATA_TAG, SIZEOF_endpoint_data, true },
	{ LINE_TAG, SIZEOF_line_data, true },
	{ SIDE_TAG, SIZEOF_side_data, true },
	{ POLYGON_TAG, SIZEOF_polygon_data, true },
	{ LIGHTSOURCE_TAG, SIZEOF_light_data, true }, // false },
	{ ANNOTATION_TAG, SIZEOF_map_annotation, true },
	{ OBJECT_TAG, SIZEOF_map_object, true },
	{ MAP_INFO_TAG, SIZEOF_static_data, true },
	{ ITEM_PLACEMENT_STRUCTURE_TAG, SIZEOF_object_frequency_definition, true },
	{ MEDIA_TAG, SIZEOF_media_data, true }, // false },
	{ AMBIENT_SOUND_TAG, SIZEOF_ambient_sound_image_data, true },
	{ RANDOM_SOUND_TAG, SIZEOF_random_sound_image_data, true },
	{ TERMINAL_DATA_TAG, sizeof(byte), true },
	
	// LP addition: handling of physics models
	{ MONSTER_PHYSICS_TAG, SIZEOF_monster_definition, true},
	{ EFFECTS_PHYSICS_TAG, SIZEOF_effect_definition, true},
	{ PROJECTILE_PHYSICS_TAG, SIZEOF_projectile_definition, true},
	{ PHYSICS_PHYSICS_TAG, SIZEOF_physics_constants, true},
	{ WEAPONS_PHYSICS_TAG, SIZEOF_weapon_definition, true},

	// GHS: save the new embedded shapes
	{ SHAPE_PATCH_TAG, sizeof(byte), true },

	{ MMLS_TAG, sizeof(byte), true },
	{ LUAS_TAG, sizeof(byte), true },

	{ MAP_INDEXES_TAG, sizeof(short), true }, // false },
	{ PLAYER_STRUCTURE_TAG, SIZEOF_player_data, true }, // false },
	{ DYNAMIC_STRUCTURE_TAG, SIZEOF_dynamic_data, true }, // false },
	{ OBJECT_STRUCTURE_TAG, SIZEOF_object_data, true }, // false },
	{ AUTOMAP_LINES, sizeof(byte), true }, // false },
	{ AUTOMAP_POLYGONS, sizeof(byte), true }, // false },
	{ MONSTERS_STRUCTURE_TAG, SIZEOF_monster_data, true }, // false },
	{ EFFECTS_STRUCTURE_TAG, SIZEOF_effect_data, true }, // false },
	{ PROJECTILES_STRUCTURE_TAG, SIZEOF_projectile_data, true }, // false },
	{ PLATFORM_STRUCTURE_TAG, SIZEOF_platform_data, true }, // false },
	{ WEAPON_STATE_TAG, SIZEOF_player_weapon_data, true }, // false },
	{ TERMINAL_STATE_TAG, SIZEOF_player_terminal_data, true }, // false }

	{ LUA_STATE_TAG, sizeof(byte), true },
};

static uint8 *export_tag_to_global_array_and_size(
	uint32 tag,
	size_t *size
	)
{
	uint8 *array = NULL;
	size_t unit_size = 0;
	size_t count = 0;
	unsigned index;

	for (index=0; index<NUMBER_OF_EXPORT_ARRAYS; ++index)
	{
		if(export_data[index].tag==tag)
		{
			unit_size= export_data[index].unit_size;
			break;
		}
	}
	assert(index != NUMBER_OF_EXPORT_ARRAYS);

	switch (tag)
	{
	case POINT_TAG:
		count = dynamic_world->endpoint_count;
		break;

	case LIGHTSOURCE_TAG:
		count = dynamic_world->light_count;
		break;

	case PLATFORM_STATIC_DATA_TAG:
		count = dynamic_world->platform_count;
		break;

	case POLYGON_TAG:
		count = dynamic_world->polygon_count;
		break;

	default:
		assert(false);
		break;
	}

	// Allocate a temporary packed-data chunk;
	// indicate if there is nothing to be written
	*size= count*unit_size;
	if (*size > 0)
		array = new byte[*size];
	else
		return NULL;

	objlist_clear(array, *size);
	
	// An OK-to-alter version of that array pointer
	uint8 *temp_array = array;

	switch (tag)
	{
	case POINT_TAG:
		for (size_t loop = 0; loop < count; ++loop)
		{
			world_point2d& vertex = map_endpoints[loop].vertex;
			ValueToStream(temp_array, vertex.x);
			ValueToStream(temp_array, vertex.y);
		}
		break;

	case LIGHTSOURCE_TAG:
		for (size_t loop = 0; loop < count; ++loop)
		{
			temp_array = pack_static_light_data(temp_array, &lights[loop].static_data, 1);
		}
		break;

	case PLATFORM_STATIC_DATA_TAG:
		if (static_platforms.size() == count)
		{
			// export them directly as they came in
			pack_static_platform_data(array, &static_platforms[0], count);
		}
		else
		{
			for (size_t loop = 0; loop < count; ++loop)
			{
				// ghs: this belongs somewhere else
				static_platform_data platform;
				obj_clear(platform);
				platform.type = platforms[loop].type;
				platform.speed = platforms[loop].speed;
				platform.delay = platforms[loop].delay;
				if (PLATFORM_GOES_BOTH_WAYS(&platforms[loop]))
				{
					platform.maximum_height = platforms[loop].maximum_ceiling_height;
					platform.minimum_height = platforms[loop].minimum_floor_height;
				}
				else if (PLATFORM_COMES_FROM_FLOOR(&platforms[loop]))
				{
					platform.maximum_height = platforms[loop].maximum_floor_height;
					platform.minimum_height = platforms[loop].minimum_floor_height;
				}
				else
				{
					platform.maximum_height = platforms[loop].maximum_ceiling_height;
					platform.minimum_height = platforms[loop].minimum_floor_height;
				}
				platform.static_flags = platforms[loop].static_flags;
				platform.polygon_index = platforms[loop].polygon_index;
				platform.tag = platforms[loop].tag;

				temp_array = pack_static_platform_data(temp_array, &platform, 1);
			}
		}
		break;

	case POLYGON_TAG:
		for (size_t loop = 0; loop < count; ++loop)
		{
			// Forge visual mode crashes if we don't do this
			polygon_data polygon = PolygonList[loop];
			polygon.first_object = NONE;
			temp_array = pack_polygon_data(temp_array, &polygon, 1);
		}
		break;

	default:
		assert(false);
		break;
	}

	return array;
}

extern size_t save_lua_states();
extern void pack_lua_states(uint8*, size_t);
		

/* the sizes are the sizes to save in the file, be aware! */
static uint8 *tag_to_global_array_and_size(
	uint32 tag, 
	size_t *size
	)
{
	uint8 *array= NULL;
	size_t unit_size = 0;
	size_t count = 0;
	unsigned index;
	
	for (index=0; index<NUMBER_OF_SAVE_ARRAYS; ++index)
	{
		if(save_data[index].tag==tag)
		{
			unit_size= save_data[index].unit_size;
			break;
		}
	}
	assert(index != NUMBER_OF_SAVE_ARRAYS);
	
	// LP: had fixed off-by-one error in medias saving,
	// and had added physics-model saving
	
	switch (tag)
	{
		case ENDPOINT_DATA_TAG:
			count= dynamic_world->endpoint_count;
			break;
		case LINE_TAG:
			count= dynamic_world->line_count;
			break;
		case SIDE_TAG:
			count= dynamic_world->side_count;
			break;
		case POLYGON_TAG:
			count= dynamic_world->polygon_count;
			break;
		case LIGHTSOURCE_TAG:
			count= dynamic_world->light_count;
			break;
		case ANNOTATION_TAG:
			count= dynamic_world->default_annotation_count;
			break;
		case OBJECT_TAG:
			count= dynamic_world->initial_objects_count;
			break;
		case MAP_INFO_TAG:
			count= 1;
			break;
		case PLAYER_STRUCTURE_TAG:
			count= dynamic_world->player_count;
			break;
		case DYNAMIC_STRUCTURE_TAG:
			count= 1;
			break;
		case OBJECT_STRUCTURE_TAG:
			count= dynamic_world->object_count;
			break;
		case MAP_INDEXES_TAG:
			count= static_cast<unsigned short>(dynamic_world->map_index_count);
			break;
		case AUTOMAP_LINES:
			count= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0)); 
			break;
		case AUTOMAP_POLYGONS:
			count= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0));
			break;
		case MONSTERS_STRUCTURE_TAG:
			count= dynamic_world->monster_count;
			break;
		case EFFECTS_STRUCTURE_TAG:
			count= dynamic_world->effect_count;
			break;
		case PROJECTILES_STRUCTURE_TAG:
			count= dynamic_world->projectile_count;
			break;
		case MEDIA_TAG:
			count= count_number_of_medias_used();
			break;
		case ITEM_PLACEMENT_STRUCTURE_TAG:
			count= 2*MAXIMUM_OBJECT_TYPES;
			break;
		case PLATFORM_STRUCTURE_TAG:
			count= dynamic_world->platform_count;
			break;
		case AMBIENT_SOUND_TAG:
			count= dynamic_world->ambient_sound_image_count;
			break;
		case RANDOM_SOUND_TAG:
			count= dynamic_world->random_sound_image_count;
			break;
		case TERMINAL_DATA_TAG:
			count= calculate_packed_terminal_data_length();
			break;
		case WEAPON_STATE_TAG:
			count= dynamic_world->player_count;
			break;
		case TERMINAL_STATE_TAG:
			count= dynamic_world->player_count;
			break;
		case MONSTER_PHYSICS_TAG:
			count= NUMBER_OF_MONSTER_TYPES;
			break;
		case EFFECTS_PHYSICS_TAG:
			count= NUMBER_OF_EFFECT_TYPES;
			break;
		case PROJECTILE_PHYSICS_TAG:
			count= NUMBER_OF_PROJECTILE_TYPES;
			break;
		case PHYSICS_PHYSICS_TAG:
			count= get_number_of_physics_models();
			break;
		case WEAPONS_PHYSICS_TAG:
			count= get_number_of_weapon_types();
			break;
	        case SHAPE_PATCH_TAG:
			get_shapes_patch_data(count);
			break;
	        case MMLS_TAG:
			GetMMLS(count);
			break;
	        case LUAS_TAG:
			GetLUAS(count);
			break;
		case LUA_STATE_TAG:
			count= save_lua_states();
			break;
		default:
			assert(false);
			break;
	}
	
	// Allocate a temporary packed-data chunk;
	// indicate if there is nothing to be written
	*size= count*unit_size;
	if (*size > 0)
		array = new byte[*size];
	else
		return NULL;

	objlist_clear(array, *size);
	
	// An OK-to-alter version of that array pointer
	uint8 *temp_array = array;
	
	switch (tag)
	{
		case ENDPOINT_DATA_TAG:
			pack_endpoint_data(array,map_endpoints,count);
			break;
		case LINE_TAG:
			pack_line_data(array,map_lines,count);
			break;
		case SIDE_TAG:
			pack_side_data(array,map_sides,count);
			break;
		case POLYGON_TAG:
			pack_polygon_data(array,map_polygons,count);
			break;
		case LIGHTSOURCE_TAG:
			pack_light_data(array,lights,count);
			break;
		case ANNOTATION_TAG:
			pack_map_annotation(array,map_annotations,count);
			break;
		case OBJECT_TAG:
			pack_map_object(array,saved_objects,count);
			break;
		case MAP_INFO_TAG:
			pack_static_data(array,static_world,count);
			break;
		case PLAYER_STRUCTURE_TAG:
			pack_player_data(array,players,count);
			break;
		case DYNAMIC_STRUCTURE_TAG:
			pack_dynamic_data(array,dynamic_world,count);
			break;
		case OBJECT_STRUCTURE_TAG:
			pack_object_data(array,objects,count);
			break;
		case MAP_INDEXES_TAG:
			ListToStream(temp_array,map_indexes,count); // E-Z packing here...
			break;
		case AUTOMAP_LINES:
			memcpy(array,automap_lines,*size);
			break;
		case AUTOMAP_POLYGONS:
			memcpy(array,automap_polygons,*size);
			break;
		case MONSTERS_STRUCTURE_TAG:
			pack_monster_data(array,monsters,count);
			break;
		case EFFECTS_STRUCTURE_TAG:
			pack_effect_data(array,effects,count);
			break;
		case PROJECTILES_STRUCTURE_TAG:
			pack_projectile_data(array,projectiles,count);
			break;
		case MEDIA_TAG:
			pack_media_data(array,medias,count);
			break;
		case ITEM_PLACEMENT_STRUCTURE_TAG:
			pack_object_frequency_definition(array,get_placement_info(),count);
			break;
		case PLATFORM_STRUCTURE_TAG:
			pack_platform_data(array,platforms,count);
			break;
		case AMBIENT_SOUND_TAG:
			pack_ambient_sound_image_data(array,ambient_sound_images,count);
			break;
		case RANDOM_SOUND_TAG:
			pack_random_sound_image_data(array,random_sound_images,count);
			break;
		case TERMINAL_DATA_TAG:
			pack_map_terminal_data(array,count);
			break;
		case WEAPON_STATE_TAG:
			pack_player_weapon_data(array,count);
			break;
		case TERMINAL_STATE_TAG:
			pack_player_terminal_data(array,count);
			break;
		case MONSTER_PHYSICS_TAG:
			pack_monster_definition(array,count);
			break;
		case EFFECTS_PHYSICS_TAG:
			pack_effect_definition(array,count);
			break;
		case PROJECTILE_PHYSICS_TAG:
			pack_projectile_definition(array,count);
			break;
		case PHYSICS_PHYSICS_TAG:
			pack_physics_constants(array,count);
			break;
		case WEAPONS_PHYSICS_TAG:
			pack_weapon_definition(array,count);
			break;
	        case SHAPE_PATCH_TAG:
			memcpy(array, get_shapes_patch_data(count), count);
			break;
	        case MMLS_TAG:
			memcpy(array, GetMMLS(count), count);
			break;
	        case LUAS_TAG:
			memcpy(array, GetLUAS(count), count);
			break;
		case LUA_STATE_TAG:
			pack_lua_states(array, count);
			break;
		default:
			assert(false);
			break;
	}
	
	return array;
}

static wad_data *build_export_wad(wad_header *header, int32 *length)
{
	struct wad_data *wad= NULL;
	uint8 *array_to_slam;
	size_t size;

	wad= create_empty_wad();
	if(wad)
	{
		recalculate_map_counts();

		// try to divine initial platform/polygon states
		vector<platform_data> SavedPlatforms = PlatformList;
		vector<polygon_data> SavedPolygons = PolygonList;
		vector<line_data> SavedLines = LineList;

		for (size_t loop = 0; loop < PlatformList.size(); ++loop)
		{
			platform_data *platform = &PlatformList[loop];
			// reset the polygon heights
			if (PLATFORM_COMES_FROM_FLOOR(platform))
			{
				platform->floor_height = platform->minimum_floor_height;
				PolygonList[platform->polygon_index].floor_height = platform->floor_height;
			}
			if (PLATFORM_COMES_FROM_CEILING(platform))
			{
				platform->ceiling_height = platform->maximum_ceiling_height;
				PolygonList[platform->polygon_index].ceiling_height = platform->ceiling_height;
			}
		}

		for (size_t loop = 0; loop < LineList.size(); ++loop)
		{
			line_data *line = &LineList[loop];
			if (LINE_IS_VARIABLE_ELEVATION(line))
			{
				SET_LINE_VARIABLE_ELEVATION(line, false);
				SET_LINE_SOLIDITY(line, false);
				SET_LINE_TRANSPARENCY(line, true);
			}
		}

		for(unsigned loop= 0; loop<NUMBER_OF_EXPORT_ARRAYS; ++loop)
		{
			/* If there is a conversion function, let it handle it */
			switch (export_data[loop].tag)
			{
			case POINT_TAG:
			case LIGHTSOURCE_TAG:
			case PLATFORM_STATIC_DATA_TAG:
			case POLYGON_TAG:
				array_to_slam= export_tag_to_global_array_and_size(export_data[loop].tag, &size);
				break;
			default:
				array_to_slam= tag_to_global_array_and_size(export_data[loop].tag, &size);
			}
	
			/* Add it to the wad.. */
			if(size)
			{
				wad= append_data_to_wad(wad, export_data[loop].tag, array_to_slam, size, 0);
				delete []array_to_slam;
			}
		}

		PlatformList = SavedPlatforms;
		PolygonList = SavedPolygons;
		LineList = SavedLines;

		if(wad) *length= calculate_wad_length(header, wad);
	}
	
	return wad;
}

/* Build the wad, with all the crap */
static struct wad_data *build_save_game_wad(
	struct wad_header *header, 
	int32 *length)
{
	struct wad_data *wad= NULL;
	uint8 *array_to_slam;
	size_t size;

	wad= create_empty_wad();
	if(wad)
	{
		recalculate_map_counts();
		for(unsigned loop= 0; loop<NUMBER_OF_SAVE_ARRAYS; ++loop)
		{
			/* If there is a conversion function, let it handle it */
			array_to_slam= tag_to_global_array_and_size(save_data[loop].tag, &size);
	
			/* Add it to the wad.. */
			if(size)
			{
				wad= append_data_to_wad(wad, save_data[loop].tag, array_to_slam, size, 0);
				delete []array_to_slam;
			}
		}
		if(wad) *length= calculate_wad_length(header, wad);
	}
	
	return wad;
}

/* Build save game wad holding metadata and preview image */
struct wad_data *build_meta_game_wad(
	const std::string& metadata,
	const std::string& imagedata,
	struct wad_header *header,
	int32 *length)
{
	struct wad_data *wad= NULL;
	
	wad= create_empty_wad();
	if(wad)
	{
		size_t size = metadata.length();
		if (size)
		{
			wad= append_data_to_wad(wad, SAVE_META_TAG, metadata.c_str(), size, 0);
		}

		size_t imgsize = imagedata.length();
		if (imgsize)
		{
			wad= append_data_to_wad(wad, SAVE_IMG_TAG, imagedata.c_str(), imgsize, 0);
		}
		if(wad) *length= calculate_wad_length(header, wad);
	}
	
	return wad;
}

/* Load and slam all of the arrays */
static void complete_restoring_level(
	struct wad_data *wad)
{
	ok_to_reset_scenery_solidity = false;
	/* Loading games needs this done. */
	reset_action_queues();
}


/* CP Addition: get_map_file returns a pointer to the current map file */
FileSpecifier& get_map_file()
{
	return MapFileSpec;
}

void level_has_embedded_physics_lua(int Level, bool& HasPhysics, bool& HasLua)
{
	// load the wad file and look for chunks !!??
	wad_header header;
	wad_data* wad;
	OpenedFile MapFile;
	if (open_wad_file_for_reading(get_map_file(), MapFile))
	{
		if (read_wad_header(MapFile, &header))
		{
			wad = read_indexed_wad_from_file(MapFile, &header, Level, true);
			if (wad)
			{
				size_t data_length;
				extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
				HasPhysics = data_length > 0;

				extract_type_from_wad(wad, LUAS_TAG, &data_length);
				HasLua = data_length > 0;
				free_wad(wad);
			}
		}
		close_wad_file(MapFile);
	}
}


/*
 *  Unpacking/packing functions
 */

static uint8 *unpack_directory_data(uint8 *Stream, directory_data *Objects, size_t Count)
{
	uint8* S = Stream;
	directory_data* ObjPtr = Objects;

	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->mission_flags);
		StreamToValue(S,ObjPtr->environment_flags);
		StreamToValue(S,ObjPtr->entry_point_flags);
		StreamToBytes(S,ObjPtr->level_name,LEVEL_NAME_LENGTH);
	}

	assert((S - Stream) == SIZEOF_directory_data);
	return S;
}

// ZZZ: gnu cc swears this is currently unused, and I don't see any sneaky #includes that might need it...
/*
static uint8 *pack_directory_data(uint8 *Stream, directory_data *Objects, int Count)
{
	uint8* S = Stream;
	directory_data* ObjPtr = Objects;

	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->mission_flags);
		ValueToStream(S,ObjPtr->environment_flags);
		ValueToStream(S,ObjPtr->entry_point_flags);
		BytesToStream(S,ObjPtr->level_name,LEVEL_NAME_LENGTH);
	}

	assert((S - Stream) == SIZEOF_directory_data);
	return S;
}
*/

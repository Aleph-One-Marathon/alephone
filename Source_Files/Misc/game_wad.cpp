/*
GAME_WAD.C
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
*/

// This needs to do the right thing on save game, which is storing the precalculated crap.

#include "cseries.h"
#include "byte_swapping.h"

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

// LP change: added chase-cam init
#include "ChaseCam.h"

// CP Addition:  scripting.h necessary for script loading
#include "scripting.h"

// For packing and unpacking some of the stuff
#include "Packing.h"

// LP addition: for physics-model stuff, we need these pointers to definitions
/* sadly extern'ed from their respective files */
extern byte monster_definitions[];
extern byte projectile_definitions[];
extern byte effect_definitions[];
extern byte weapon_definitions[];
extern byte physics_models[];

#ifdef env68k
#pragma segment file_io
#endif

// unify the save game code into one structure.

/* -------- local globals */
FileSpecifier MapFileSpec;
static boolean file_is_set= FALSE;

// LP addition: was a physics model loaded from the previous level loaded?
static bool PhysicsModelLoadedEarlier = false;

// The following local globals are for handling games that need to be restored.
struct revert_game_info
{
	boolean game_is_from_disk;
	struct game_data game_information;
	struct player_start_data player_start;
	struct entry_point entry_point;
	FileSpecifier SavedGame;
};
static struct revert_game_info revert_game_data;

/* Borrowed from the old lightsource.h, to allow Marathon II to open/use Marathon I maps */
struct old_light_data {
	word flags;
	
	short type;
	short mode; /* on, off, etc. */
	short phase;
	
	fixed minimum_intensity, maximum_intensity;
	short period; /* on, in ticks (turning on and off periods are always the same for a given light type,
		or else are some function of this period) */
	
	fixed intensity; /* current intensity */
	
	short unused[5];	
};

enum /* old light types */
{
	_light_is_normal,
	_light_is_rheostat,
	_light_is_flourescent,
	_light_is_strobe, 
	_light_flickers,
	_light_pulsates,
	_light_is_annoying,
	_light_is_energy_efficient
};

/* -------- definitions for byte-swapping */
static _bs_field _bs_directory_data[] = { // 74 bytes
	_2byte, _2byte, _4byte, LEVEL_NAME_LENGTH,
};

static _bs_field _bs_world_point_2d[] = { // 4 bytes
	_2byte, _2byte
};

static _bs_field _bs_endpoint_data[] = { // 16 bytes
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte,
	_2byte
};

static _bs_field _bs_line_data[] = { // 32 bytes
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte,
	6*sizeof(int16)
};

static _bs_field _bs_saved_side[] = { // 64 bytes
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte
};

static _bs_field _bs_side_data[] = { // 6
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_4byte,
	_2byte
};

static _bs_field _bs_polygon_data[] = { // 128 bytes
	_2byte, _2byte, _2byte,
	_2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_4byte,
	_2byte,
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte,
	_2byte, _2byte,
	_2byte
};

static _bs_field _bs_saved_static_light_data[] = { // 100 bytes
    _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte,
    4*sizeof(int16)
};

static _bs_field _bs_static_light_data[] = { // 100 bytes
    _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte, _2byte, _2byte, _4byte, _4byte,
    _2byte,
    4*sizeof(int16)
};

static _bs_field _bs_saved_static_platform_data[] = { // 32 bytes
	_2byte, _2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte,
	_2byte,
	7*sizeof(int16)
};

static _bs_field _bs_static_platform_data[] = { // 32 bytes
	_2byte, _2byte, _2byte, _2byte, _2byte,
	_4byte,
	_2byte,
	_2byte,
	7*sizeof(int16)
};

static _bs_field _bs_saved_platform_data[] = { // 140 bytes
    _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte,
    _2byte,
    22*sizeof(int16)
};

static _bs_field _bs_platform_data[] = { // 140 bytes
    _2byte, _4byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte, _2byte, _2byte, _2byte,
    _2byte,
    _2byte,
    22*sizeof(int16)
};

static _bs_field _bs_map_annotation[] = { // 72 bytes
	_2byte,
	_2byte, _2byte, _2byte,
	MAXIMUM_ANNOTATION_TEXT_LENGTH
};

static _bs_field _bs_map_object[] = { // 16 bytes
	_2byte, _2byte, _2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte
};

static _bs_field _bs_static_data[] = { // 88 bytes
	_2byte,
	_2byte, _2byte, _2byte, _2byte,
	4*sizeof(int16), LEVEL_NAME_LENGTH,
	_4byte
};

static _bs_field _bs_media_data[] = { // 32 bytes
	_2byte, _2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte,
	_2byte, _2byte, _2byte,
	_4byte, _2byte, _2byte,
	2*sizeof(int16)
};

static _bs_field _bs_ambient_sound_image_data[] = { // 16 bytes
	_2byte, _2byte, _2byte, 5*sizeof(int16)
};

static _bs_field _bs_random_sound_image_data[] = { // 32 bytes
	_2byte, _2byte,
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte,
	_4byte, _4byte,
	_2byte,
	3*sizeof(int16)
};

/* -------- static functions */
static void scan_and_add_scenery(void);
static void complete_restoring_level(struct wad_data *wad);
static void load_redundant_map_data(short *redundant_data, short count);
static void allocate_map_structure_for_map(struct wad_data *wad);
static struct wad_data *build_save_game_wad(struct wad_header *header, long *length);

#ifdef LP
static void allocate_map_for_counts(short polygon_count, short side_count,
	short endpoint_count, short line_count, long terminal_data_length);
static void load_points(byte *points, short count);
static void load_lines(byte *lines, short count);
static void load_sides(byte *sides, short count, short version);
static void load_polygons(byte *polys, short count, short version);
static void load_lights(saved_static_light *_lights, short count, short version);
static void load_annotations(byte *annotations, short count);
static void load_objects(byte *map_objects, short count);
static void load_media(struct media_data *media, short count);
static void load_map_info(byte *map_info);
static void load_ambient_sound_images(byte *data, short count);
static void load_random_sound_images(byte *data, short count);
static void load_terminal_data(byte *data, long length);
/*
static void load_points(saved_map_pt *points, short count);
static void load_lines(saved_line *lines, short count);
static void load_sides(saved_side *sides, short count, short version);
static void load_polygons(saved_poly *polys, short count, short version);
static void load_lights(saved_static_light *_lights, short count, short version);
static void load_annotations(saved_annotation *annotations, short count);
static void load_objects(saved_object *map_objects, short count);
static void load_media(struct media_data *media, short count);
static void load_map_info(saved_map_data *map_info);
static void load_ambient_sound_images(struct ambient_sound_image_data *data, short count);
static void load_random_sound_images(struct random_sound_image_data *data, short count);
static void load_terminal_data(byte *data, long length);
*/

/* Used _ONLY_ by game_wad.c internally and precalculate.c. */
static boolean process_map_wad(struct wad_data *wad, boolean restoring_game, short version);

/* Final three calls, must be in this order! */
static void recalculate_redundant_map(void);
static void scan_and_add_platforms(struct static_platform_data *platform_static_data, short count);
static void complete_loading_level(short *map_indexes, short map_index_count, 
	struct static_platform_data *platform_data, short platform_data_count,
	struct platform_data *actual_platform_data, short actual_platform_data_count, short version);

#endif
#ifdef CB
static void allocate_map_for_counts(short polygon_count, short side_count,
	short endpoint_count, short line_count, long terminal_data_length);
static void load_points(saved_map_pt *points, short count);
static void load_lines(saved_line *lines, short count);
static void load_sides(saved_side *sides, short count, short version);
static void load_polygons(saved_poly *polys, short count, short version);
static void load_lights(struct saved_static_light_data *lights, short count, short version);
static void load_annotations(saved_annotation *annotations, short count);
static void load_objects(saved_object *map_objects, short count);
static void load_media(struct media_data *media, short count);
static void load_map_info(saved_map_data *map_info);
static void load_ambient_sound_images(struct ambient_sound_image_data *data, short count);
static void load_random_sound_images(struct random_sound_image_data *data, short count);
static void load_terminal_data(byte *data, long length);
static void scan_and_add_scenery(void);
static void complete_restoring_level(struct wad_data *wad);
static void load_redundant_map_data(short *redundant_data, short count);
static boolean process_map_wad(struct wad_data *wad, boolean restoring_game, short version);
static void allocate_map_structure_for_map(struct wad_data *wad);
static struct wad_data *build_save_game_wad(struct wad_header *header, long *length);

/* Used _ONLY_ by game_wad.c internally and precalculate.c. */
boolean process_map_wad(struct wad_data *wad, boolean restoring_game, short version);

// Final three calls, must be in this order!
static void recalculate_redundant_map(void);
static void scan_and_add_platforms(struct saved_static_platform_data *platform_static_data,
	short count);
static void complete_loading_level(short *map_indexes, short map_index_count,
	struct saved_static_platform_data *platform_data, short platform_data_count,
	struct saved_platform_data *actual_platform_data, short actual_platform_data_count, short version);

#endif
/* ------------------------ Net functions */
long get_net_map_data_length(
	void *data) 
{
	return get_flat_data_length(data);
}

/* Note that this frees it as well */
boolean process_net_map_data(
	void *data) 
{
	struct wad_header header;
	struct wad_data *wad;
	boolean success= FALSE;
	
	wad= inflate_flat_data(data, &header);
	if(wad)
	{
		success= process_map_wad(wad, FALSE, header.data_version);
		free_wad(wad); /* Note that the flat data points into the wad. */
	}
	
	return success;
}

/* This will have to do some interesting voodoo with union wads, methinks */
void *get_map_for_net_transfer(
	struct entry_point *entry)
{
	assert(file_is_set);
	
	/* FALSE means don't use union maps.. */
	return get_flat_data(MapFileSpec, FALSE, entry->level_number);
	// return get_flat_data(&current_map_file, FALSE, entry->level_number);
}

/* ---------------------- End Net Functions ----------- */

/* This takes a cstring */
void set_map_file(FileSpecifier& File)
{
	MapFileSpec = File;
	set_scenario_images_file(File);

	// Don't care whether there was an error when checking on the file's scenario images
	clear_game_error();

	file_is_set= TRUE;
}

/* Set to the default map.. (Only if no map doubleclicked upon on startup.. */
void set_to_default_map(
	void)
{
	FileSpecifier NewMapFile;
	
	get_default_map_spec(NewMapFile);
	set_map_file(NewMapFile);
}

/* Return TRUE if it finds the file, and it sets the mapfile to that file. */
/* Otherwise it returns FALSE, meaning that we need have the file sent to us. */
boolean use_map_file(
	long checksum) /* Should be unsigned long */
{
	FileSpecifier File;
	// FileDesc file;
	boolean success= FALSE;

	if(find_wad_file_that_has_checksum(File, _typecode_scenario, strPATHS, checksum))
	// if(find_wad_file_that_has_checksum(&file, SCENARIO_FILE_TYPE, strPATHS, checksum))
	{
		set_map_file(File);
		// set_map_file(&file);
		success= TRUE;
	}

	return success;
}

boolean load_level_from_map(
	short level_index)
{
	OpenedFile OFile;
	struct wad_header header;
	struct wad_data *wad;
	short index_to_load;
	boolean restoring_game= FALSE;
	short error= 0;
	if(file_is_set)
	{
		/* Determine what we are trying to do.. */
		if(level_index==NONE)
		{
			restoring_game= TRUE;
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
					wad= read_indexed_wad_from_file(MapFile, &header, index_to_load, TRUE);
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
			
			// LP: carry over errors
			short SavedType, SavedError;
			SavedError = get_game_error(&SavedType);
			
			//CP Addition: load any scripts available
			// if (load_script(1000+level_index) < 0)
				;  //this sucks.
			
			// Restore carried-over error so that Pfhortran errors
			// will not cause trouble
			set_game_error(SavedType,SavedError);
		} else {
			// error code has been set..
		}
	} else {
		set_game_error(gameError, errMapFileNotSet);
	}

	/* ... and bail */
	return (!error_pending());
}

/* Hopefully this is in the correct order of initialization... */
/* This sucks, beavis. */
void complete_loading_level(
	short *map_indexes,
	short map_index_count,
#ifdef LP
	struct static_platform_data *platform_data,
#endif
#ifdef CB
	struct saved_static_platform_data *platform_data,
#endif
	short platform_data_count,
#ifdef LP
	struct platform_data *actual_platform_data,
#endif
#ifdef CB
	struct saved_platform_data *actual_platform_data,
#endif
	short actual_platform_data_count,
	short version)
{
	/* Scan, add the doors, recalculate, and generally tie up all loose ends */
	/* Recalculate the redundant data.. */
	load_redundant_map_data(map_indexes, map_index_count);

	/* Add the platforms. */
	if(platform_data || (platform_data==NULL && actual_platform_data==NULL))
	{
		scan_and_add_platforms(platform_data, platform_data_count);
	} else {
		assert(actual_platform_data);
#ifdef SDL
		// CB: convert saved_platform_data to platform_data
		for (int i=0; i<actual_platform_data_count; i++) {
			struct platform_data *p = platforms + i;
			struct saved_platform_data *q = actual_platform_data + i;
			p->type = q->type;
			p->static_flags = (q->static_flags_hi << 16) | q->static_flags_lo;
			p->speed = q->speed;
			p->delay = q->delay;
			p->minimum_floor_height = q->minimum_floor_height;
			p->maximum_floor_height = q->maximum_floor_height;
			p->minimum_ceiling_height = q->minimum_ceiling_height;
			p->maximum_ceiling_height = q->maximum_ceiling_height;
			p->polygon_index = q->polygon_index;
			p->dynamic_flags = q->dynamic_flags;
			p->floor_height = q->floor_height;
			p->ceiling_height = q->ceiling_height;
			p->ticks_until_restart = q->ticks_until_restart;
			for (int j=0; j<MAXIMUM_VERTICES_PER_POLYGON; j++)
				p->endpoint_owners[j] = q->endpoint_owners[j];
			p->parent_platform_index = q->parent_platform_index;
			p->tag = q->tag;
		}
#else
		objlist_copy(platforms, actual_platform_data, actual_platform_data_count);
#endif
		dynamic_world->platform_count= actual_platform_data_count;
	}

	scan_and_add_scenery();
	
	/* Gotta do this after recalculate redundant.. */
	if(version==MARATHON_ONE_DATA_VERSION)
	{
		short loop;
		
		for(loop= 0; loop<dynamic_world->polygon_count; ++loop)
		{
			guess_side_lightsource_indexes(loop);
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
#if 1
	short ii;
	struct map_object *saved_object;
	short count= 0;
	boolean done= FALSE;
	
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
					done= TRUE;
				}
				count++;
			}
		}
		++saved_object;
	}
	
	/* If they asked for a valid location, make sure that we gave them one */
	if(location) vassert(done, csprintf(temporary, "Tried to place: %d only %d starting pts.", index, count));
	
	return count;
#else
	location->x= 0x14e9;
	location->y= 0x1ba0;
	*facing= 0x00;
	*polygon_index= 0x6f;

	return 1;
#endif
}

unsigned long get_current_map_checksum(
	void)
{
	// fileref file_handle;
	struct wad_header header;

	assert(file_is_set);
	OpenedFile MapFile;
	assert(open_wad_file_for_reading(MapFileSpec, MapFile));

	/* Read the file */
	read_wad_header(MapFile, &header);
	
	/* Close the file.. */
	close_wad_file(MapFile);	
	
	return header.checksum;
}

boolean new_game(
	short number_of_players, 
	boolean network,
	struct game_data *game_information,
	struct player_start_data *player_start_information,
	struct entry_point *entry_point)
{
	short player_index, i;
	boolean success= TRUE;

	/* Make sure our code is synchronized.. */
	assert(MAXIMUM_PLAYER_START_NAME_LENGTH==MAXIMUM_PLAYER_NAME_LENGTH);

	/* Initialize the global network going flag... */
	game_is_networked= network;
	
	/* If we want to save it, this is an untitled map.. */
#if defined(mac)
	revert_game_data.SavedGame.SetToApp();
	revert_game_data.SavedGame.SetName(getcstr(temporary, strFILENAMES, filenameDEFAULT_SAVE_GAME),_typecode_savegame);
#elif defined(SDL)
	revert_game_data.SavedGame.SetToLocalDataDir();
	revert_game_data.SavedGame.AddPart(getcstr(temporary, strFILENAMES, filenameDEFAULT_SAVE_GAME));
#endif

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
	success= goto_level(entry_point, TRUE);

	/* If we were able to load the map... */
	if(success)
	{
		/* Initialize the players-> note there may be more than one player in a */
		/* non-network game, for playback.. */
		for (i=0;i<number_of_players;++i)
		{
			player_index= new_player(player_start_information[i].team,
				player_start_information[i].color, player_start_information[i].identifier);
			assert(player_index==i);

			/* Now copy in the name of the player.. */
			assert(strlen(player_start_information[i].name)<=MAXIMUM_PLAYER_NAME_LENGTH);
			strcpy(players[i].name, player_start_information[i].name);
		}
	
		if(game_is_networked)
		{
			/* Make sure we can count. */
			assert(number_of_players==NetGetNumberOfPlayers());
			
			set_local_player_index(NetGetLocalPlayerIndex());
			set_current_player_index(NetGetLocalPlayerIndex());
		}
		else
		{
			set_local_player_index(0);
			set_current_player_index(0);
		}
		
		/* we need to alert the function that reverts the game of the game setup so that
		 * new game can be called if the user wants to revert later.
		 */
		setup_revert_game_info(game_information, player_start_information, entry_point);
		
		// Reset the player queues (done here and in load_game)
		reset_player_queues();
		
		/* Load the collections */
		/* entering map might fail if NetSync() fails.. */
		success= entering_map();
	}
	
	// LP change: adding chase-cam initialization
	ChaseCam_Initialize();

	return success;
}

boolean get_indexed_entry_point(
	struct entry_point *entry_point, 
	short *index, 
	long type)
{
	// short file_handle;
	struct wad_header header;
	short actual_index;
	boolean success= FALSE;
	
	assert(file_is_set);

	OpenedFile MapFile;
	if (!open_wad_file_for_reading(MapFileSpec,MapFile)) return false;
	{
		if (read_wad_header(MapFile, &header))
		{
			/* If this is a new style */
			if(header.application_specific_directory_data_size==SIZEOF_directory_data)
			{
				void *total_directory_data= read_directory_data(MapFile, &header);

				assert(total_directory_data);
				for(actual_index= *index; actual_index<header.wad_count; ++actual_index)
				{
					struct directory_data *directory;
					
					directory= (struct directory_data *)get_indexed_directory_data(&header, actual_index, 
						total_directory_data);
					byte_swap_object(*directory, _bs_directory_data);

					/* Find the flags that match.. */
					if(directory->entry_point_flags & type)
					{
						/* This one is valid! */
						entry_point->level_number= actual_index;
						strcpy(entry_point->level_name, directory->level_name);
			
						*index= actual_index+1;
						success= TRUE;
						break; /* Out of the for loop */
					}
				}
				free(total_directory_data);
			} else {
				/* Old style */
				/* Find the index.. */
				for(actual_index= *index; !success && actual_index<header.wad_count; ++actual_index)
				{
					struct wad_data *wad;

					/* Read the file */
					// wad= read_indexed_wad_from_file(file_handle, &header, actual_index, TRUE);
					wad= read_indexed_wad_from_file(MapFile, &header, actual_index, TRUE);
					if (wad)
					{
						struct static_data *map_info;
						long length;

						/* IF this has the proper type.. */
						map_info= (struct static_data *)extract_type_from_wad(wad, MAP_INFO_TAG, &length);
						assert(length==sizeof(struct static_data));
						if(map_info->entry_point_flags & type)
						{
							/* This one is valid! */
							entry_point->level_number= actual_index;
							assert(strlen(map_info->level_name)<LEVEL_NAME_LENGTH);
							strcpy(entry_point->level_name, map_info->level_name);
				
							*index= actual_index+1;
							success= TRUE;
						}
				
						free_wad(wad);
					}
				}
			}
		}

		/* Close the file.. */
		close_wad_file(MapFile);
	}

	return success;
}

/* This is called when the game level is changed somehow */
/* The only thing that has to be valid in the entry point is the level_index */

/* Returns a short that is an OSErr... */
boolean goto_level(
	struct entry_point *entry, 
	boolean new_game)
{
	boolean success= TRUE;

	if(!new_game)
	{
		/* Clear the current map */
		leaving_map();
	}

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
	{
		/* Load it and then rock.. */
		load_level_from_map(entry->level_number);
		if(error_pending()) success= FALSE;
	}

	if (success)
	{
		if (!new_game)
		{
			recreate_players_for_new_level();
	
			/* Load the collections */
			dynamic_world->current_level_number= entry->level_number;

			/* entering_map might fail if netsync fails, but we will have already displayed */
			/* the error.. */
			success= entering_map();
		}

		if(success) /* successfully switched. (guaranteed except in net games) */
		{
			/* place_initial_objects should be called in complete_loading_level(), but
			 * it needs to be called after recreate_players_for_new_level() if we're switching
			 * levels (the placement stuff calls point_is_player_visible) and loading the game
			 * wad munges the monster information (which p_i_p_v() needs) */
			place_initial_objects();
	
			initialize_control_panels_for_level(); /* must be called after the players are initialized */
	
			dynamic_world->current_level_number= entry->level_number;
		} else {
//			assert(error_pending());
		}
	}
//	if(!success) alert_user(fatalError, strERRORS, badReadMap, -1);

	/* We be done.. */
	return success;
}

/* -------------------- Private or map editor functions */
void allocate_map_for_counts(
	short polygon_count, 
	short side_count,
	short endpoint_count,
	short line_count,
	long terminal_data_length)
{
	long cumulative_length= 0;
	long automap_line_length, automap_polygon_length, map_index_length;

	/* Give the map indexes a whole bunch of memory (cause we can't calculate it) */
	map_index_length= (polygon_count*32+1024)*sizeof(int16);
	
	/* Automap lines. */
	automap_line_length= (line_count/8+((line_count%8)?1:0))*sizeof(byte);
	
	/* Automap Polygons */
	automap_polygon_length= (polygon_count/8+((polygon_count%8)?1:0))*sizeof(byte);

	cumulative_length+= polygon_count*sizeof(struct polygon_data);
	cumulative_length+= side_count*sizeof(struct side_data);
	cumulative_length+= endpoint_count*sizeof(struct endpoint_data);
	cumulative_length+= line_count*sizeof(struct line_data);
	cumulative_length+= map_index_length;
	cumulative_length+= automap_line_length;
	cumulative_length+= automap_polygon_length;
	cumulative_length+= terminal_data_length;

	/* Okay, we now have the length.  Allocate our block.. */
	reallocate_map_structure_memory(cumulative_length);

	/* Tell the recalculation data how big it is.. */
	set_map_index_buffer_size(map_index_length);

	/* Setup our pointers. */
	map_polygons= (struct polygon_data *) get_map_structure_chunk(polygon_count*sizeof(struct polygon_data));
	map_sides= (struct side_data *) get_map_structure_chunk(side_count*sizeof(struct side_data));
	map_endpoints= (struct endpoint_data *) get_map_structure_chunk(endpoint_count*sizeof(struct endpoint_data));
	map_lines= (struct line_data *) get_map_structure_chunk(line_count*sizeof(struct line_data));
	map_indexes= (short *) get_map_structure_chunk(map_index_length);
	automap_lines= (byte *) get_map_structure_chunk(automap_line_length);
	automap_polygons= (byte *) get_map_structure_chunk(automap_polygon_length);
	map_terminal_data= (byte *) get_map_structure_chunk(terminal_data_length);

	return;
}

void load_points(
	byte *points,
	short count)
{
	short loop;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_ENDPOINTS_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_ENDPOINTS_PER_MAP);
	// OK to modify input-data pointer since it's called by value
	for(loop=0; loop<count; ++loop)
	{
		world_point2d& vertex = map_endpoints[loop].vertex;
		StreamToValue(points,vertex.x);
		StreamToValue(points,vertex.y);
		/*
		map_endpoints[loop].vertex= *points;
		byte_swap_object(map_endpoints[loop].vertex, _bs_world_point_2d);
		++points;
		*/
	}
	dynamic_world->endpoint_count= count;
}

void load_lines(
	byte *lines, 
	short count)
{
	// short loop;

	assert(count>=0 && count<=MAXIMUM_LINES_PER_MAP);
	
	unpack_line_data(lines,map_lines,count);
	/*
	for(loop=0; loop<count; ++loop)
	{
		map_lines[loop]= *lines;
		byte_swap_object(map_lines[loop], _bs_line_data);
		++lines;
	}
	*/
	dynamic_world->line_count= count;
}

void load_sides(
	byte *sides, 
	short count,
	short version)
{
	short loop;
	
	assert(count>=0 && count<=MAXIMUM_SIDES_PER_MAP);

	unpack_side_data(sides,map_sides,count);

#if 0
	for(loop=0; loop<count; ++loop)
	{
#ifdef LP
		// map_sides[loop]= *sides;
		byte_swap_object(*sides, _bs_side_data);
		unpack_side_data(*sides,map_sides[loop]);
#endif
#ifdef CB
#ifdef SDL
		map_sides[loop]= *(side_data *)sides;
		byte_swap_data(map_sides + loop, SIZEOF_saved_side, 1, _bs_saved_side);
		map_sides[loop].ambient_delta = (sides->ambient_delta_hi << 16) | sides->ambient_delta_lo;
#else
		map_sides[loop]= *sides;
		byte_swap_object(map_sides[loop], _bs_side_data);
#endif
#endif
#endif

	for(loop=0; loop<count; ++loop)
	{
		if(version==MARATHON_ONE_DATA_VERSION)
		{
			map_sides[loop].transparent_texture.texture= NONE;
			map_sides[loop].ambient_delta= 0;
		}
		++sides;
	}

	dynamic_world->side_count= count;
}

void load_polygons(
	byte *polys, 
	short count,
	short version)
{
	short loop;

	assert(count>=0 && count<=MAXIMUM_POLYGONS_PER_MAP);
	
	unpack_polygon_data(polys,map_polygons,count);
	/*
	for(loop=0; loop<count; ++loop)
	{
		map_polygons[loop]= *polys;
		byte_swap_object(map_polygons[loop], _bs_polygon_data);
		++polys;
	}
	*/
	dynamic_world->polygon_count= count;

	/* Allow for backward compatibility! */
	switch(version)
	{
		case MARATHON_ONE_DATA_VERSION:
			for(loop= 0; loop<count; ++loop)
			{
				map_polygons[loop].media_index= NONE;
				map_polygons[loop].floor_origin.x= map_polygons[loop].floor_origin.y= 0;
				map_polygons[loop].ceiling_origin.x= map_polygons[loop].ceiling_origin.y= 0;
			}
			break;
			
		case MARATHON_TWO_DATA_VERSION:
		// LP addition:
		case MARATHON_INFINITY_DATA_VERSION:
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
}

static void convert_lighting_function_spec(lighting_function_specification &dst, const saved_lighting_function_specification &src)
{
	dst.function = src.function;
	dst.period = src.period;
	dst.delta_period = src.delta_period;
	dst.intensity = (src.intensity_hi << 16) | src.intensity_lo;
	dst.delta_intensity = (src.delta_intensity_hi << 16) | src.delta_intensity_lo;
}

void load_lights(
#ifdef LP
	saved_static_light *_lights, 
#endif
#ifdef CB
	struct saved_static_light_data *lights,
#endif
	short count,
	short version)
{
	short loop;

	vassert(count>=0 && count<=MAXIMUM_LIGHTS_PER_MAP, csprintf(temporary, "Light count: %d vers: %d",
		count, version));
	
	switch(version)
	{
		case MARATHON_ONE_DATA_VERSION:
			{	
				struct old_light_data *light= (struct old_light_data *) _lights;
				
				/* As time goes on, we should add functions below to make the lights */
				/*  behave more like their bacward compatible cousins. */
				for(loop= 0; loop<count; ++loop)
				{
					short new_index;

					/* Do the best we can.. */
					switch(SDL_SwapBE16(light->type))
					{
						case _light_is_normal:
						case _light_is_annoying:
						case _light_is_energy_efficient:
						case _light_is_rheostat:
						case _light_is_flourescent:
							new_index= new_light(get_defaults_for_light_type(_normal_light));
							break;
							
						case _light_is_strobe:
						case _light_flickers:
						case _light_pulsates:
							new_index= new_light(get_defaults_for_light_type(_strobe_light));
							break;
							
						default:
							// LP change:
							assert(false);
							// halt();
							break;
					}
					assert(new_index==loop);
					light++;
				}
			}
			break;
			
		case MARATHON_TWO_DATA_VERSION:
		// LP addition:
		case MARATHON_INFINITY_DATA_VERSION:
			{
#ifdef LP
				struct saved_static_light *light= _lights;
#endif
#ifdef CB
				struct saved_static_light_data *light= lights;
#endif				
				for(loop= 0; loop<count; ++loop)
				{
					short new_index;
#ifdef LP
					byte_swap_object(light, _bs_static_light_data);
					static_light_data temp_light;
					unpack_light_data(*light,temp_light);
					
					new_index= new_light(&temp_light);
#endif
#ifdef CB
#ifdef SDL
					// CB: convert saved_static_light_data to static_light_data
					saved_static_light_data tmp = *light;
					byte_swap_data(&tmp, SIZEOF_saved_static_light_data, 1, _bs_saved_static_light_data);
					static_light_data tmp2;
					tmp2.type = tmp.type;
					tmp2.flags = tmp.flags;
					tmp2.phase = tmp.phase;
					convert_lighting_function_spec(tmp2.primary_active, tmp.primary_active);
					convert_lighting_function_spec(tmp2.secondary_active, tmp.secondary_active);
					convert_lighting_function_spec(tmp2.becoming_active, tmp.becoming_active);
					convert_lighting_function_spec(tmp2.primary_inactive, tmp.primary_inactive);
					convert_lighting_function_spec(tmp2.secondary_inactive, tmp.secondary_inactive);
					convert_lighting_function_spec(tmp2.becoming_inactive, tmp.becoming_inactive);
					tmp2.tag = tmp.tag;
					new_index = new_light(&tmp2);
#else
					byte_swap_object(*light, _bs_static_light_data);
					new_index = new_light(light);
#endif
#endif
					assert(new_index==loop);
					light++;
				}
			}
			break;
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return;
}

void load_annotations(
	byte *annotations, 
	short count)
{
	// short ii;
	
	assert(count>=0 && count<=MAXIMUM_ANNOTATIONS_PER_MAP);
	
	unpack_map_annotation(annotations,map_annotations,count);
	/*
	for(ii=0; ii<count; ++ii)
	{
		map_annotations[ii]= annotations[ii];
		byte_swap_object(map_annotations[ii], _bs_map_annotation);
	}
	*/
	dynamic_world->default_annotation_count= count;
	
	return;
}

void load_objects(byte *map_objects, short count)
{
	// short ii;
	
	assert(count>=0 && count<=MAXIMUM_SAVED_OBJECTS);
	
	unpack_map_object(map_objects,saved_objects,count);
	/*
	for(ii=0; ii<count; ++ii)
	{
		saved_objects[ii]= map_objects[ii];	
		byte_swap_object(saved_objects[ii], _bs_map_object);
	}
	*/
	dynamic_world->initial_objects_count= count;
}

void load_map_info(
	byte *map_info)
{
	unpack_static_data(map_info,static_world);
	// memcpy(static_world, map_info, sizeof(struct static_data));
	// byte_swap_data(static_world, SIZEOF_static_data, 1, _bs_static_data);
}

void load_media(
	struct media_data *medias,
	short count)
{
	struct media_data *media= medias;
	short ii;
	
	assert(count>=0 && count<=MAXIMUM_MEDIAS_PER_MAP);

	for(ii= 0; ii<count; ++ii)
	{
#ifdef SDL
		media_data tmp = *media;
		byte_swap_data(&tmp, SIZEOF_media_data, 1, _bs_media_data);
		short new_index = new_media(&tmp);
#else
		byte_swap_object(media, _bs_media_data);
		short new_index= new_media(media);
#endif
		
		assert(new_index==ii);
		media++;
	}
	
	return;
}

void load_ambient_sound_images(
	byte *data,
	short count)
{
	assert(count>=0 &&count<=MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP);

	unpack_ambient_sound_image_data(data,ambient_sound_images,count);
#if 0
#ifdef SDL
	memcpy(ambient_sound_images, data, count*SIZEOF_ambient_sound_image_data);
	byte_swap_data(ambient_sound_images, SIZEOF_ambient_sound_image_data, count, _bs_ambient_sound_image_data);
#else
	objlist_copy(ambient_sound_images, data, count);
	byte_swap_object_list(ambient_sound_images, count, _bs_ambient_sound_image_data);
#endif
#endif

	dynamic_world->ambient_sound_image_count= count;
}

void load_random_sound_images(
	byte *data,
	short count)
{
	assert(count>=0 &&count<=MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP);

	unpack_random_sound_image_data(data,random_sound_images,count);
#if 0
#ifdef SDL
	memcpy(random_sound_images, data, count*sizeof(struct random_sound_image_data));
	byte_swap_data(random_sound_images, SIZEOF_random_sound_image_data, count, _bs_random_sound_image_data);
#else
	objlist_copy(random_sound_images, data, count);
	byte_swap_object_list(random_sound_images, count, _bs_random_sound_image_data);
#endif
#endif

	dynamic_world->random_sound_image_count= count;
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

extern boolean load_game_from_file(FileSpecifier& File);

boolean load_game_from_file(FileSpecifier& File)
{
	boolean success= FALSE;
	
	// LP: verify sizes: (of on-disk structures only!)
	assert(sizeof(map_object) == SIZEOF_map_object);
	assert(sizeof(directory_data) == SIZEOF_directory_data);
	assert(sizeof(map_annotation) == SIZEOF_map_annotation);
	assert(sizeof(ambient_sound_image_data) == SIZEOF_ambient_sound_image_data);
	assert(sizeof(random_sound_image_data) == SIZEOF_random_sound_image_data);
	assert(sizeof(endpoint_data) == SIZEOF_endpoint_data);
	assert(sizeof(object_frequency_definition) == SIZEOF_object_frequency_definition);
	assert(sizeof(static_data) == SIZEOF_static_data);
#ifdef LP
	assert(sizeof(saved_static_light) == SIZEOF_static_light_data);
#endif
#ifdef CB
	assert(sizeof(saved_static_light_data) == SIZEOF_saved_static_light_data);
#endif
	assert(sizeof(media_data) == SIZEOF_media_data);
#ifdef LP
	assert(sizeof(saved_static_platform) == SIZEOF_static_platform_data);
	assert(sizeof(saved_platform) == SIZEOF_platform_data);
#endif
#ifdef CB
	assert(sizeof(saved_static_platform_data) == SIZEOF_saved_static_platform_data);
	assert(sizeof(saved_platform_data) == SIZEOF_saved_platform_data);
#endif

	/* Must reset this, in case they played a net game before this one. */
	game_is_networked= FALSE;

	/* Setup for a revert.. */
	revert_game_data.game_is_from_disk = TRUE;
	revert_game_data.SavedGame = File;

	/* Use the save game file.. */
	set_map_file(File);
	
	/* Load the level from the map */
	success= load_level_from_map(NONE); /* Save games are ALWAYS index NONE */
	if (success)
	{
		unsigned long parent_checksum;
	
		/* Set the non-saved data.... */
		set_current_player_index(0);
		set_local_player_index(0);

		/* Find the original scenario this saved game was a part of.. */
		parent_checksum= read_wad_file_parent_checksum(File);
		if(!use_map_file(parent_checksum))
		{
			/* Tell the user they’re screwed when they try to leave this level. */
			alert_user(infoError, strERRORS, cantFindMap, 0);

			// LP addition: makes the game look normal
			hide_cursor();
		
			/* Set to the default map. */
			set_to_default_map();
		}

		/* Set the random seed. */
		set_random_seed(dynamic_world->random_seed);
		
		/* Load the shapes and whatnot.. */		
		entering_map();
	} 

	return success;
}

void setup_revert_game_info(
	struct game_data *game_info, 
	struct player_start_data *start, 
	struct entry_point *entry)
{
	revert_game_data.game_is_from_disk = FALSE;
	obj_copy(revert_game_data.game_information, *game_info);
	obj_copy(revert_game_data.player_start, *start);
	obj_copy(revert_game_data.entry_point, *entry);
	
	return;
}

boolean revert_game(
	void)
{
	boolean successful;
	
	assert(dynamic_world->player_count==1);

	leaving_map();
	
	if (revert_game_data.game_is_from_disk)
	{
		/* Reload their last saved game.. */
		successful= load_game_from_file(revert_game_data.SavedGame);
		// successful= load_game_from_file(&revert_game_data.saved_game);

		/* And they don't get to continue. */
		stop_recording();
	}
	else
	{
		/* This was the totally evil line discussed above. */
		successful= new_game(1, FALSE, &revert_game_data.game_information, &revert_game_data.player_start, 
			&revert_game_data.entry_point);
			
		/* And rewind so that the last player is used. */
		rewind_recording();
	}

	if(successful)
	{
		update_interface(NONE);
		ChaseCam_Reset();
		ResetFieldOfView();
		ReloadViewContext();
	}
	
	return successful;
}

void get_current_saved_game_name(FileSpecifier& File)
{
	File = revert_game_data.SavedGame;
}

/* The current mapfile should be set to the save game file... */
boolean save_game_file(FileSpecifier& File)
{
	struct wad_header header;
	short err;
	boolean success= FALSE;
	long offset, wad_length;
	struct directory_entry entry;
	struct wad_data *wad;

	/* Save off the random seed. */
	dynamic_world->random_seed= get_random_seed();

	/* Setup to revert the game properly */
	revert_game_data.game_is_from_disk= TRUE;
	revert_game_data.SavedGame = File;
	
	// LP: add a file here
	
	/* Fill in the default wad header */
	fill_default_wad_header(File, CURRENT_WADFILE_VERSION, EDITOR_MAP_VERSION, 1, 0, &header);
		
	/* Assume that we confirmed on save as... */
	if (create_wadfile(File,_typecode_savegame))
	{
		OpenedFile SaveFile;
		if(open_wad_file_for_writing(File,SaveFile))
		{
			/* Write out the new header */
			if (write_wad_header(SaveFile, &header))
			{
				offset= sizeof(struct wad_header);
		
				wad= build_save_game_wad(&header, &wad_length);
				if (wad)
				{
					/* Set the entry data.. */
					set_indexed_directory_offset_and_length(&header, 
						&entry, 0, offset, wad_length, 0);
					
					/* Save it.. */
					if (write_wad(SaveFile, &header, wad, offset))
					{
						/* Update the new header */
						offset+= wad_length;
						header.directory_offset= offset;
						header.parent_checksum= read_wad_file_checksum(MapFileSpec);
						if (write_wad_header(SaveFile, &header) && write_directorys(SaveFile, &header, &entry))
						{
							/* This function saves the overhead map as a thumbnail, as well */
							/*  as adding the string resource that tells you what it is when */
							/*  it is clicked on & Marathon2 isn't installed.  Obviously, both */
							/*  of these are superfluous for a dos environment. */
							add_finishing_touches_to_save_file(File);

							/* We win. */
							success= TRUE;
						} 
					}

					free_wad(wad);
				}
			}

			err = SaveFile.GetError();
			close_wad_file(SaveFile);
		}
	}
	
	if(err || error_pending())
	{
		if(!err) err= get_game_error(NULL);
		alert_user(infoError, strERRORS, fileError, err);
		clear_game_error();
		success= FALSE;
	}
	
	return success;
}

/* -------- static functions */
static void scan_and_add_platforms(
#ifdef LP
	struct static_platform_data *platform_static_data,
#endif
#ifdef CB
	struct saved_static_platform_data *platform_static_data,
#endif
	short count)
{
	struct polygon_data *polygon;
	short loop;
#ifdef LP
	struct static_platform_data *static_data;
#endif
#ifdef CB
	struct saved_static_platform_data *static_data;
#endif
	short platform_static_data_index;
	
	polygon= map_polygons;
	for(loop=0; loop<dynamic_world->polygon_count; ++loop)
	{
		if (polygon->type==_polygon_is_platform)
		{
			/* Search and find the extra data.  If it is not there, use the permutation for */
			/* backwards compatibility! */
			static_data= platform_static_data;
			for(platform_static_data_index= 0; platform_static_data_index<count; ++platform_static_data_index)
			{
				if(static_data->polygon_index==loop)
				{
#ifdef SDL
					// CB: convert saved_static_platform_data to static_platform_data
					static_platform_data tmp;
					tmp.type = static_data->type;
					tmp.speed = static_data->speed;
					tmp.delay = static_data->delay;
					tmp.maximum_height = static_data->maximum_height;
					tmp.minimum_height = static_data->minimum_height;
					tmp.static_flags = (static_data->static_flags_hi << 16) | static_data->static_flags_lo;
					tmp.polygon_index = static_data->polygon_index;
					tmp.tag = static_data->tag;
					new_platform(&tmp, loop);
#else
					new_platform(static_data, loop);
#endif
					break;
				}
				static_data++;
			}
			
			/* DIdn't find it- use a standard platform */
			if(platform_static_data_index==count)
			{
				polygon->permutation= 1;
				new_platform(get_defaults_for_platform_type(polygon->permutation), loop);
			}	
		}
		++polygon;
	}
}


/* Load a level from a wad-> mainly used by the net stuff. */
boolean process_map_wad(
	struct wad_data *wad, 
	boolean restoring_game,
	short version)
{
	long data_length;
	byte *data;
	short count;
	boolean is_preprocessed_map= FALSE;
	
	assert(version==MARATHON_INFINITY_DATA_VERSION || version==MARATHON_TWO_DATA_VERSION || version==MARATHON_ONE_DATA_VERSION);

	/* zero everything so no slots are used */	
	initialize_map_for_new_level();

	/* Calculate the length (for reallocate map) */
	allocate_map_structure_for_map(wad);

	/* Extract points */
	data= (unsigned char *)extract_type_from_wad(wad, POINT_TAG, &data_length);
	count= data_length/SIZEOF_world_point2d;
	assert(data_length == count*SIZEOF_world_point2d);
	
	if(count)
	{
		load_points(data, count);
	} else {
		data= (unsigned char *)extract_type_from_wad(wad, ENDPOINT_DATA_TAG, &data_length);
		count= data_length/SIZEOF_endpoint_data;
		assert(data_length == count*SIZEOF_endpoint_data);
		assert(count>=0 && count<MAXIMUM_ENDPOINTS_PER_MAP);

		/* Slam! */
		unpack_endpoint_data(data,map_endpoints,count);
		/*
		memcpy(map_endpoints, data, count*sizeof(endpoint_data));
		byte_swap_object_list(map_endpoints, count, _bs_endpoint_data);
		*/
		dynamic_world->endpoint_count= count;

		is_preprocessed_map= TRUE;
	}

	/* Extract lines */
	data= (unsigned char *)extract_type_from_wad(wad, LINE_TAG, &data_length);
	count = data_length/SIZEOF_line_data;
	assert(data_length == count*SIZEOF_line_data);
	load_lines(data, count);

	/* Order is important! */
	data= (unsigned char *)extract_type_from_wad(wad, SIDE_TAG, &data_length);
	count = data_length/SIZEOF_side_data;
	assert(data_length == count*SIZEOF_side_data);
	load_sides(data, count, version);

	/* Extract polygons */
	data= (unsigned char *)extract_type_from_wad(wad, POLYGON_TAG, &data_length);
	count = data_length/SIZEOF_polygon_data;
	assert(data_length == count*SIZEOF_polygon_data);
	load_polygons(data, count, version);

	/* Extract the lightsources */
	if(!restoring_game)
	{
		/* When you are restoring a game, the actual light structure is set. */
		data= (unsigned char *)extract_type_from_wad(wad, LIGHTSOURCE_TAG, &data_length);
		if(version==MARATHON_ONE_DATA_VERSION) 
		{
			/* We have an old style light */
			count= data_length/sizeof(struct old_light_data);
			assert(count*sizeof(struct old_light_data)==data_length);
#ifdef LP
			load_lights((saved_static_light *) data, count, version);
#endif
#ifdef CB
			load_lights((struct saved_static_light_data *) data, count, version);
#endif
		} else {
#ifdef LP
			count= data_length/sizeof(saved_static_light);
			assert(count*sizeof(saved_static_light)==data_length);
			load_lights((saved_static_light *) data, count, version);
#endif
#ifdef CB
			count= data_length/sizeof(struct saved_static_light_data);
			assert(count*sizeof(struct saved_static_light_data)==data_length);
			load_lights((struct saved_static_light_data *) data, count, version);
#endif
		}

		//	HACK!!!!!!!!!!!!!!! vulcan doesn’t NONE .first_object field after adding scenery
		{
			for (count= 0; count<dynamic_world->polygon_count; ++count)
			{
				map_polygons[count].first_object= NONE;
			}
		}
	}

	/* Extract the annotations */
	data= (unsigned char *)extract_type_from_wad(wad, ANNOTATION_TAG, &data_length);
	count = data_length/SIZEOF_map_annotation;
	assert(data_length == count*SIZEOF_map_annotation);
	load_annotations(data, count);

	/* Extract the objects */
	data= (unsigned char *)extract_type_from_wad(wad, OBJECT_TAG, &data_length);
	count = data_length/SIZEOF_map_object;
	assert(data_length == count*SIZEOF_map_object);
	load_objects(data, count);

	/* Extract the map info data */
	data= (unsigned char *)extract_type_from_wad(wad, MAP_INFO_TAG, &data_length);
	// LP change: made this more Pfhorte-friendly
	assert(SIZEOF_static_data==data_length || (SIZEOF_static_data-2)==data_length);
	load_map_info(data);

	/* Extract the game difficulty info.. */
	data= (unsigned char *)extract_type_from_wad(wad, ITEM_PLACEMENT_STRUCTURE_TAG, &data_length);
	load_placement_data(((struct object_frequency_definition *) data)+MAXIMUM_OBJECT_TYPES,
		(struct object_frequency_definition *) data);

	/* Extract the terminal data. */
	data= (unsigned char *)extract_type_from_wad(wad, TERMINAL_DATA_TAG, &data_length);
	load_terminal_data(data, data_length);

	/* Extract the media definitions */
	if(!restoring_game)
	{
		data= (unsigned char *)extract_type_from_wad(wad, MEDIA_TAG, &data_length);
		count= data_length/sizeof(struct media_data);
		assert(count*sizeof(struct media_data)==data_length);
		load_media((struct media_data *) data, count);
	}

	/* Extract the ambient sound images */
	data= (unsigned char *)extract_type_from_wad(wad, AMBIENT_SOUND_TAG, &data_length);
	count = data_length/SIZEOF_ambient_sound_image_data;
	assert(data_length == count*SIZEOF_ambient_sound_image_data);
	load_ambient_sound_images(data, count);
	load_ambient_sound_images(data, data_length/SIZEOF_ambient_sound_image_data);

	/* Extract the random sound images */
	data= (unsigned char *)extract_type_from_wad(wad, RANDOM_SOUND_TAG, &data_length);
	count = data_length/SIZEOF_random_sound_image_data;
	assert(data_length == count*SIZEOF_random_sound_image_data);
	load_random_sound_images(data, count);

	// LP addition: load the physics-model chunks (all fixed-size)
	bool PhysicsModelLoaded = false;
	
#ifdef mac	//!! most of these structures have non-portable alignment requirements; to be fixed later
	data= (unsigned char *)extract_type_from_wad(wad, MONSTER_PHYSICS_TAG, &data_length);
	// dprintf("Monsters: %d %d: %d",NUMBER_OF_MONSTER_TYPES,get_monster_defintion_size(),data_length);
	count = data_length/get_monster_defintion_size();
	assert(count*get_monster_defintion_size() == data_length);
	assert(count <= NUMBER_OF_MONSTER_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(monster_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, EFFECTS_PHYSICS_TAG, &data_length);
	// dprintf("Effects: %d %d: %d",NUMBER_OF_EFFECT_TYPES,get_effect_defintion_size(),data_length);
	count = data_length/get_effect_defintion_size();
	assert(count*get_effect_defintion_size() == data_length);
	assert(count <= NUMBER_OF_EFFECT_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(effect_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PROJECTILE_PHYSICS_TAG, &data_length);
	// dprintf("Projectiles: %d %d: %d",NUMBER_OF_PROJECTILE_TYPES,get_projectile_definition_size(),data_length);
	count = data_length/get_projectile_definition_size();
	assert(count*get_projectile_definition_size() == data_length);
	assert(count <= NUMBER_OF_PROJECTILE_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(projectile_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
	// dprintf("Physics: %d %d: %d",get_number_of_physics_models(),get_physics_definition_size(),data_length);
	count = data_length/get_physics_definition_size();
	assert(count*get_physics_definition_size() == data_length);
	assert(count <= get_number_of_physics_models());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(physics_models,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, WEAPONS_PHYSICS_TAG, &data_length);
	// dprintf("Weapons: %d %d: %d",get_number_of_weapons(),get_weapon_defintion_size(),data_length);
	count = data_length/get_weapon_defintion_size();
	assert(count*get_weapon_defintion_size() == data_length);
	assert(count <= get_number_of_weapons());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(weapon_definitions,data,data_length);
	}
#endif
	
	// LP addition: Reload the physics model if it had been loaded in the previous level,
	// but not in the current level. This avoids the persistent-physics bug.
	if (PhysicsModelLoadedEarlier && !PhysicsModelLoaded)
		import_definition_structures();
	PhysicsModelLoadedEarlier = PhysicsModelLoaded;
	
	/* If we are restoring the game, then we need to add the dynamic data */
	if(restoring_game)
	{
		complete_restoring_level(wad);
	} else {
		byte *map_index_data;
		short map_index_count;
#ifdef LP
		saved_platform *platform_structures;
#endif
#ifdef CB
		struct saved_platform_data *platform_structures;
#endif
		short platform_structure_count;

		if(version==MARATHON_ONE_DATA_VERSION)
		{
			/* Force precalculation */
			map_index_data= NULL;
			map_index_count= 0; 
		} else {
			map_index_data= (unsigned char *)extract_type_from_wad(wad, MAP_INDEXES_TAG, &data_length);
			map_index_count= data_length/sizeof(short);
			assert(map_index_count*sizeof(short)==data_length);
		}

		assert(is_preprocessed_map&&map_index_count || !is_preprocessed_map&&!map_index_count);

#ifdef SDL
		data= (unsigned char *)extract_type_from_wad(wad, PLATFORM_STATIC_DATA_TAG, &data_length);
		count= data_length/SIZEOF_saved_static_platform_data;
		assert(count*SIZEOF_saved_static_platform_data==data_length);
		byte_swap_data(data, SIZEOF_saved_static_platform_data, count, _bs_saved_static_platform_data);

		platform_structures= (struct saved_platform_data *)extract_type_from_wad(wad, PLATFORM_STRUCTURE_TAG, &data_length);
		platform_structure_count= data_length/SIZEOF_saved_platform_data;
		assert(platform_structure_count*SIZEOF_saved_platform_data==data_length);
		byte_swap_data(platform_structures, SIZEOF_saved_platform_data, platform_structure_count, _bs_saved_platform_data);
#else
		data= (unsigned char *)extract_type_from_wad(wad, PLATFORM_STATIC_DATA_TAG, &data_length);
		count= data_length/sizeof(saved_static_platform);
		assert(count*sizeof(saved_static_platform)==data_length);
		byte_swap_object_list(data, count, _bs_static_platform_data);
		
		static_platform_data *unpacked_static_platform_structures = NULL;
		if (count > 0)
		{
			unpacked_static_platform_structures = new static_platform_data[count];
			saved_static_platform *static_platform_structures = (saved_static_platform *)data;
			for (int i=0; i<count; i++)
				unpack_static_platform_data(static_platform_structures[i], unpacked_static_platform_structures[i]);
		}

		platform_structures= (saved_platform *)extract_type_from_wad(wad, PLATFORM_STRUCTURE_TAG, &data_length);
		platform_structure_count= data_length/sizeof(saved_platform);
		assert(platform_structure_count*sizeof(saved_platform)==data_length);
		byte_swap_object_list(platform_structures, platform_structure_count, _bs_platform_data);
#ifdef LP		
		platform_data *unpacked_platform_structures = NULL;
		if (platform_structure_count > 0)
		{
			unpacked_platform_structures = new platform_data[platform_structure_count];
			for (int i=0; i<platform_structure_count; i++)
				unpack_platform_data(platform_structures[i], unpacked_platform_structures[i]);
		}
		
#endif
#endif
		complete_loading_level((short *) map_index_data, map_index_count,
#ifdef LP
			unpacked_static_platform_structures, count, unpacked_platform_structures, 
#endif
#ifdef CB
			(struct saved_static_platform_data *) data, count, platform_structures,
#endif
			platform_structure_count, version);
		
		if (unpacked_static_platform_structures) delete []unpacked_static_platform_structures;
		if (unpacked_platform_structures) delete []unpacked_platform_structures;
	}

	/* ... and bail */
	return TRUE;
}

static void allocate_map_structure_for_map(
	struct wad_data *wad)
{
	byte *data;
	long data_length;
	short line_count, polygon_count, side_count, endpoint_count;
	long terminal_data_length;

	/* Extract points */
	data= (unsigned char *)extract_type_from_wad(wad, POINT_TAG, &data_length);
	endpoint_count= data_length/sizeof(saved_map_pt);
	if(endpoint_count*sizeof(saved_map_pt)!=data_length) alert_user(fatalError, strERRORS, corruptedMap, 'pt');
	
	if(!endpoint_count)
	{
		data= (unsigned char *)extract_type_from_wad(wad, ENDPOINT_DATA_TAG, &data_length);
		endpoint_count= data_length/sizeof(struct endpoint_data);
		if(endpoint_count*sizeof(struct endpoint_data)!=data_length) alert_user(fatalError, strERRORS, corruptedMap, 'ep');
	}

	/* Extract lines */
	data= (unsigned char *)extract_type_from_wad(wad, LINE_TAG, &data_length);
	line_count= data_length/sizeof(saved_line);
	if(line_count*sizeof(saved_line)!=data_length) alert_user(fatalError, strERRORS, corruptedMap, 'li');

	/* Sides.. */
	data= (unsigned char *)extract_type_from_wad(wad, SIDE_TAG, &data_length);
	side_count= data_length/sizeof(saved_side);
	if(side_count*sizeof(saved_side)!=data_length) alert_user(fatalError, strERRORS, corruptedMap, 'si');

	/* Extract polygons */
	data= (unsigned char *)extract_type_from_wad(wad, POLYGON_TAG, &data_length);
	polygon_count= data_length/sizeof(saved_poly);
	if(polygon_count*sizeof(saved_poly)!=data_length) alert_user(fatalError, strERRORS, corruptedMap, 'si');

	/* Extract the terminal junk */
	data= (unsigned char *)extract_type_from_wad(wad, TERMINAL_DATA_TAG, &terminal_data_length);

	allocate_map_for_counts(polygon_count, side_count, endpoint_count, line_count, terminal_data_length);

	return;
}

/* Note that we assume the redundant data has already been recalculated... */
static void load_redundant_map_data(
	short *redundant_data,
	short count)
{
	if (redundant_data)
	{
		assert(redundant_data && map_indexes);
		objlist_copy(map_indexes, redundant_data, count);
		byte_swap_memory(map_indexes, _2byte, count);
		dynamic_world->map_index_count= count;
	}
	else
	{
		boolean have_been_warned= FALSE;

#if !defined(ALPHA) && !defined(BETA)
		if(!have_been_warned)
		{
			/* Only warn the gatherer.. */
			if(!game_is_networked || (game_is_networked && local_player_index==0))
			{
				alert_user(infoError, strERRORS, warningExternalMapsFile, -1);
				// LP addition: makes the game look normal
				hide_cursor();
			}
			have_been_warned= TRUE;
		}
#endif

		recalculate_redundant_map();
		precalculate_map_indexes();
	}
// dynamic_world->map_index_count= 0;
// recalculate_redundant_map();
// precalculate_map_indexes();

	return;
}

void load_terminal_data(
	byte *data, 
	long length)
{
	/* I would really like it if I could get these into computer_interface.c statically */
	memcpy(map_terminal_data, data, length);
	byte_swap_terminal_data(map_terminal_data, length);
	map_terminal_data_length= length;
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
	
	return;
}

struct save_game_data 
{
	long tag;
	short unit_size;
	boolean loaded_by_level;
};

#define NUMBER_OF_SAVE_ARRAYS (sizeof(save_data)/sizeof(struct save_game_data))
struct save_game_data save_data[]=
{
	{ ENDPOINT_DATA_TAG, sizeof(struct endpoint_data), TRUE },
	{ LINE_TAG, sizeof(saved_line), TRUE },
	{ SIDE_TAG, sizeof(saved_side), TRUE },
	{ POLYGON_TAG, sizeof(saved_poly), TRUE },
	{ LIGHTSOURCE_TAG, sizeof(struct light_data), FALSE },
	{ ANNOTATION_TAG, sizeof(saved_annotation), TRUE },
	{ OBJECT_TAG, sizeof(saved_object), TRUE },
	{ MAP_INFO_TAG, sizeof(struct static_data), TRUE },
	{ ITEM_PLACEMENT_STRUCTURE_TAG, MAXIMUM_OBJECT_TYPES*sizeof(struct object_frequency_definition)*2, TRUE },
	{ MEDIA_TAG, sizeof(struct media_data), FALSE },
	{ AMBIENT_SOUND_TAG, sizeof(struct ambient_sound_image_data), TRUE },
	{ RANDOM_SOUND_TAG, sizeof(struct random_sound_image_data), TRUE },
	{ TERMINAL_DATA_TAG, sizeof(byte), TRUE },
	
	// LP addition: handling of physics models
	{ MONSTER_PHYSICS_TAG, sizeof(byte), TRUE},
	{ EFFECTS_PHYSICS_TAG, sizeof(byte), TRUE},
	{ PROJECTILE_PHYSICS_TAG, sizeof(byte), TRUE},
	{ PHYSICS_PHYSICS_TAG, sizeof(byte), TRUE},
	{ WEAPONS_PHYSICS_TAG, sizeof(byte), TRUE},

	{ MAP_INDEXES_TAG, sizeof(short), FALSE },
	{ PLAYER_STRUCTURE_TAG, sizeof(struct player_data), FALSE },
	{ DYNAMIC_STRUCTURE_TAG, sizeof(struct dynamic_data), FALSE },
	{ OBJECT_STRUCTURE_TAG, sizeof(struct object_data), FALSE },
	{ AUTOMAP_LINES, sizeof(byte), FALSE },
	{ AUTOMAP_POLYGONS, sizeof(byte), FALSE },
	{ MONSTERS_STRUCTURE_TAG, sizeof(struct monster_data), FALSE },
	{ EFFECTS_STRUCTURE_TAG, sizeof(struct effect_data), FALSE },
	{ PROJECTILES_STRUCTURE_TAG, sizeof(struct projectile_data), FALSE },
	{ PLATFORM_STRUCTURE_TAG, sizeof(struct platform_data), FALSE },
	{ WEAPON_STATE_TAG, sizeof(byte), FALSE },
	{ TERMINAL_STATE_TAG, sizeof(byte), FALSE }
};

/* the sizes are the sizes to save in the file, be aware! */
static void *tag_to_global_array_and_size(
	long tag, 
	long *size
	)
{
	void *array= NULL;
	short unit_size, index;
	
	for(index= 0; index<NUMBER_OF_SAVE_ARRAYS; ++index)
	{
		if(save_data[index].tag==tag)
		{
			unit_size= save_data[index].unit_size;
			break;
		}
	}
	assert(index != NUMBER_OF_SAVE_ARRAYS);

	switch (tag)
	{
		case ENDPOINT_DATA_TAG:
			array= map_endpoints;
			*size= dynamic_world->endpoint_count*sizeof(struct endpoint_data);
			break;
		case LINE_TAG:
			array= map_lines;
			assert(sizeof(saved_line)==sizeof(struct line_data));
			*size= dynamic_world->line_count*unit_size;
			break;
		case SIDE_TAG:
			array= map_sides;
			assert(sizeof(saved_side)==sizeof(struct side_data));
			*size= dynamic_world->side_count*unit_size;
			break;
		case POLYGON_TAG:
			array= map_polygons;
			assert(sizeof(saved_poly)==sizeof(struct polygon_data));
			*size= dynamic_world->polygon_count*unit_size;
			break;
		case LIGHTSOURCE_TAG:
			assert(sizeof(saved_static_light)==sizeof(struct static_light_data));
			array= lights;
			*size= dynamic_world->light_count*unit_size;
			break;
		case ANNOTATION_TAG:
			array= map_annotations;
			assert(sizeof(struct map_annotation)==sizeof(saved_annotation));
			*size= dynamic_world->default_annotation_count*unit_size;
			break;
		case OBJECT_TAG:
			array= saved_objects;
			assert(sizeof(struct map_object)==sizeof(saved_object));
			*size= dynamic_world->initial_objects_count*unit_size;
			break;
		case MAP_INFO_TAG:
			array= static_world;
			*size= unit_size;
			break;
		case PLAYER_STRUCTURE_TAG:
			array= players;
			*size= unit_size*dynamic_world->player_count;
			break;
		case DYNAMIC_STRUCTURE_TAG:
			array= dynamic_world;
			*size= unit_size;
			break;
		case OBJECT_STRUCTURE_TAG:
			array= objects;
			*size= unit_size*dynamic_world->object_count;
			break;
		case MAP_INDEXES_TAG:
			array= map_indexes;
			*size= unit_size*dynamic_world->map_index_count;
			break;
		case AUTOMAP_LINES:
			array= automap_lines;
			*size= (dynamic_world->line_count/8+((dynamic_world->line_count%8)?1:0))*sizeof(byte); 
			break;
		case AUTOMAP_POLYGONS:
			array= automap_polygons;
			*size= (dynamic_world->polygon_count/8+((dynamic_world->polygon_count%8)?1:0))*sizeof(byte);
			break;
		case MONSTERS_STRUCTURE_TAG:
			array= monsters;
			*size= unit_size*dynamic_world->monster_count;
			break;
		case EFFECTS_STRUCTURE_TAG:
			array= effects;
			*size= unit_size*dynamic_world->effect_count;
			break;
		case PROJECTILES_STRUCTURE_TAG:
			array= projectiles;
			*size= unit_size*dynamic_world->projectile_count;
			break;
		case MEDIA_TAG:
			array= medias;
			// LP change: fixed saving of media data;
			// off-by-one error now gone
			*size= unit_size*count_number_of_medias_used();
			// *size= unit_size*(MAXIMUM_MEDIAS_PER_MAP-1);
			break;
		case ITEM_PLACEMENT_STRUCTURE_TAG:
			array= get_placement_info();
			*size= unit_size;
			break;
		case PLATFORM_STRUCTURE_TAG:
			array= platforms;
			*size= unit_size*dynamic_world->platform_count;
			break;
		case AMBIENT_SOUND_TAG:
			array= ambient_sound_images;
			*size= unit_size*dynamic_world->ambient_sound_image_count;
			break;
		case RANDOM_SOUND_TAG:
			array= random_sound_images;
			*size= unit_size*dynamic_world->random_sound_image_count;
			break;
		case TERMINAL_DATA_TAG:
			array= get_terminal_information_array();
			*size= calculate_terminal_information_length();
			break;
		case WEAPON_STATE_TAG:
			array= get_weapon_array();
			*size= calculate_weapon_array_length();
			break;
		case TERMINAL_STATE_TAG:
			array= get_terminal_data_for_save_game();
			*size= calculate_terminal_data_length();
			break;
		// LP addition: handling of physics models
		case MONSTER_PHYSICS_TAG:
			array= monster_definitions;
			*size= NUMBER_OF_MONSTER_TYPES*get_monster_defintion_size();
			break;
		case EFFECTS_PHYSICS_TAG:
			array= effect_definitions;
			*size= NUMBER_OF_EFFECT_TYPES*get_effect_defintion_size();
			break;
		case PROJECTILE_PHYSICS_TAG:
			array= projectile_definitions;
			*size= NUMBER_OF_PROJECTILE_TYPES*get_projectile_definition_size();
			break;
		case PHYSICS_PHYSICS_TAG:
			array= physics_models;
			*size= get_number_of_physics_models()*get_physics_definition_size();
			break;
		case WEAPONS_PHYSICS_TAG:
			array= weapon_definitions;
			*size= get_number_of_weapons()*get_weapon_defintion_size();
			break;
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
	
	return array;
}

/* Build the wad, with all the crap */
static struct wad_data *build_save_game_wad(
	struct wad_header *header, 
	long *length)
{
	struct wad_data *wad= NULL;
	short loop;
	byte *array_to_slam;
	long size;

	wad= create_empty_wad();
	if(wad)
	{
		recalculate_map_counts();
		for(loop= 0; loop<NUMBER_OF_SAVE_ARRAYS; ++loop)
		{
			/* If there is a conversion function, let it handle it */
			array_to_slam= (unsigned char *)tag_to_global_array_and_size(save_data[loop].tag, &size);
	
			/* Add it to the wad.. */
			if(size)
			{
				wad= append_data_to_wad(wad, save_data[loop].tag, array_to_slam, size, 0l);
			}
		}
		if(wad) *length= calculate_wad_length(header, wad);
	}
	
	return wad;
}

/* Load and slam all of the arrays */
static void complete_restoring_level(
	struct wad_data *wad)
{
	short loop;
	void *array;
	byte *data;
	long size, data_length;
	short count;
	
	for(loop= 0; loop<NUMBER_OF_SAVE_ARRAYS; ++loop)
	{
		/* If it hasn't already been loaded */
		if(!save_data[loop].loaded_by_level)
		{
			/* Size is invalid at this point.. */
			array= tag_to_global_array_and_size(save_data[loop].tag, &size);
			data= (unsigned char *)extract_type_from_wad(wad, save_data[loop].tag, &data_length);	
			count= data_length/save_data[loop].unit_size;
			assert(count*save_data[loop].unit_size==data_length);

			/* Copy the data to the proper array.. */
			memcpy(array, data, data_length);
		}
	}
	
	/* Loading games needs this done. */
	reset_player_queues();
}


/* CP Addition: get_map_file returns a pointer to the current map file */
//FileDesc *get_map_file(
//	void)
FileSpecifier& get_map_file()
{
	return MapFileSpec; // (FileDesc *)&MapFile.Spec;
}

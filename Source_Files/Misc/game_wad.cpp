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

// LP change: moved this into main directory:
#include "editor.h"
// #include ":editor code:editor.h"
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
static FileDesc current_map_file;
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
	FileDesc saved_game;
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

/* -------- static functions */
static void scan_and_add_scenery(void);
static void complete_restoring_level(struct wad_data *wad);
static void load_redundant_map_data(short *redundant_data, short count);
void scan_and_add_platforms(struct static_platform_data *platform_static_data,
	short count);
static void allocate_map_structure_for_map(struct wad_data *wad);
static struct wad_data *build_save_game_wad(struct wad_header *header, long *length);

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
	return get_flat_data(&current_map_file, FALSE, entry->level_number);
}

/* ---------------------- End Net Functions ----------- */

/* This takes a cstring */
void set_map_file(
	FileDesc *file)
{
	memcpy(&current_map_file, file, sizeof(FileDesc));
	set_scenario_images_file(file);
	file_is_set= TRUE;
	
	return;
}

/* Set to the default map.. (Only if no map doubleclicked upon on startup.. */
void set_to_default_map(
	void)
{
	FileDesc new_map;
	
	get_default_map_spec(&new_map);
	set_map_file(&new_map);
	
	return;
}

/* Return TRUE if it finds the file, and it sets the mapfile to that file. */
/* Otherwise it returns FALSE, meaning that we need have the file sent to us. */
boolean use_map_file(
	long checksum) /* Should be unsigned long */
{
	FileDesc file;
	boolean success= FALSE;

	if(find_wad_file_that_has_checksum(&file, SCENARIO_FILE_TYPE, strPATHS, checksum))
	{
		set_map_file(&file);
		success= TRUE;
	}

	return success;
}

boolean load_level_from_map(
	short level_index)
{
	fileref file_handle;
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
	
//		file_handle= open_union_wad_file_for_reading(&current_map_file);
//		if(file_handle!=0)
		file_handle= open_wad_file_for_reading(&current_map_file);
		if(file_handle!=NONE)
		{
			/* Read the file */
			if(read_wad_header(file_handle, &header))
			{
				if(index_to_load>=0 && index_to_load<header.wad_count)
				{
					wad= read_indexed_wad_from_file(file_handle, &header, index_to_load, TRUE);
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
			close_wad_file(file_handle);
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
	struct static_platform_data *platform_data,
	short platform_data_count,
	struct platform_data *actual_platform_data,
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
		memcpy(platforms, actual_platform_data, actual_platform_data_count*sizeof(struct platform_data));
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
	fileref file_handle;
	struct wad_header header;

	assert(file_is_set);
	file_handle= open_wad_file_for_reading(&current_map_file);
	assert(file_handle != -1);	

	/* Read the file */
	read_wad_header(file_handle, &header);
	
	/* Close the file.. */
	close_wad_file(file_handle);	
	
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
	get_application_filedesc(&revert_game_data.saved_game);
	getpstr(revert_game_data.saved_game.name, strFILENAMES, filenameDEFAULT_SAVE_GAME);

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
	memcpy(&dynamic_world->game_information, game_information, sizeof(struct game_data));

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
	short file_handle;
	struct wad_header header;
	short actual_index;
	boolean success= FALSE;
	
	assert(file_is_set);

	file_handle= open_wad_file_for_reading(&current_map_file);
	if (file_handle != -1)
	{
		if (read_wad_header(file_handle, &header))
		{
			/* If this is a new style */
			if(header.application_specific_directory_data_size==sizeof(struct directory_data))
			{
				void *total_directory_data= read_directory_data(file_handle, &header);

				assert(total_directory_data);
				for(actual_index= *index; actual_index<header.wad_count; ++actual_index)
				{
					struct directory_data *directory;
					
					directory= (struct directory_data *)get_indexed_directory_data(&header, actual_index, 
						total_directory_data);

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
					wad= read_indexed_wad_from_file(file_handle, &header, actual_index, TRUE);
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
		close_wad_file(file_handle);
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
	map_index_length= (polygon_count*32+1024)*sizeof(short);
	
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
	saved_map_pt *points, 
	short count)
{
	short loop;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_ENDPOINTS_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_ENDPOINTS_PER_MAP);
	for(loop=0; loop<count; ++loop)
	{
		map_endpoints[loop].vertex= *points;
		++points;
	}
	dynamic_world->endpoint_count= count;
}

void load_lines(
	saved_line *lines, 
	short count)
{
	short loop;

	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_LINES_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_LINES_PER_MAP);
	for(loop=0; loop<count; ++loop)
	{
		map_lines[loop]= *lines;
		++lines;
	}
	dynamic_world->line_count= count;
}

void load_sides(
	saved_side *sides, 
	short count,
	short version)
{
	short loop;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_SIDES_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_SIDES_PER_MAP);
	for(loop=0; loop<count; ++loop)
	{
		map_sides[loop]= *sides;

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
	saved_poly *polys, 
	short count,
	short version)
{
	short loop;

	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_POLYGONS_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_POLYGONS_PER_MAP);
	for(loop=0; loop<count; ++loop)
	{
		map_polygons[loop]= *polys;
		++polys;
	}
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

void load_lights(
	struct static_light_data *lights, 
	short count,
	short version)
{
	short loop;

	// LP change: fixed off-by-one error
	vassert(count>=0 && count<=MAXIMUM_LIGHTS_PER_MAP, csprintf(temporary, "Light count: %d vers: %d",
		count, version));
	// vassert(count>=0 && count<MAXIMUM_LIGHTS_PER_MAP, csprintf(temporary, "Light count: %d vers: %d",
	//	count, version));
	
	switch(version)
	{
		case MARATHON_ONE_DATA_VERSION:
			{	
				struct old_light_data *light= (struct old_light_data *) lights;
				
				/* As time goes on, we should add functions below to make the lights */
				/*  behave more like their bacward compatible cousins. */
				for(loop= 0; loop<count; ++loop)
				{
					short new_index;
					
					/* Do the best we can.. */
					switch(light->type)
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
				struct static_light_data *light= lights;
				
				for(loop= 0; loop<count; ++loop)
				{
					short new_index;
					
					new_index= new_light(light);
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
	saved_annotation *annotations, 
	short count)
{
	short ii;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_ANNOTATIONS_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_ANNOTATIONS_PER_MAP);
	
	for(ii=0; ii<count; ++ii)
	{
		map_annotations[ii]= annotations[ii];
	}
	dynamic_world->default_annotation_count= count;
	
	return;
}

void load_objects(saved_object *map_objects, short count)
{
	short ii;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_SAVED_OBJECTS);
	// assert(count>=0 && count<MAXIMUM_SAVED_OBJECTS);
	
	for(ii=0; ii<count; ++ii)
	{
		saved_objects[ii]= map_objects[ii];	
	} 
	dynamic_world->initial_objects_count= count;
}

void load_map_info(
	saved_map_data *map_info)
{
	memcpy(static_world, map_info, sizeof(struct static_data));
}

void load_media(
	struct media_data *medias,
	short count)
{
	struct media_data *media= medias;
	short ii;
	
	// LP change: fixed off-by-one error
	assert(count>=0 && count<=MAXIMUM_MEDIAS_PER_MAP);
	// assert(count>=0 && count<MAXIMUM_MEDIAS_PER_MAP);
	for(ii= 0; ii<count; ++ii)
	{
		short new_index= new_media(media);
		
		assert(new_index==ii);
		media++;
	}
	
	return;
}

void load_ambient_sound_images(
	struct ambient_sound_image_data *data,
	short count)
{
	// LP change: fixed off-by-one error
	assert(count>=0 &&count<=MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP);
	// assert(count>=0 &&count<MAXIMUM_AMBIENT_SOUND_IMAGES_PER_MAP);
	memcpy(ambient_sound_images, data, count*sizeof(struct ambient_sound_image_data));
	dynamic_world->ambient_sound_image_count= count;
}

void load_random_sound_images(
	struct random_sound_image_data *data,
	short count)
{
	// LP change: fixed off-by-one error
	assert(count>=0 &&count<=MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP);
	// assert(count>=0 &&count<MAXIMUM_RANDOM_SOUND_IMAGES_PER_MAP);
	memcpy(random_sound_images, data, count*sizeof(struct random_sound_image_data));
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

extern boolean load_game_from_file(FileDesc *file);

boolean load_game_from_file(
	FileDesc *file)
{
	boolean success= FALSE;

	/* Must reset this, in case they played a net game before this one. */
	game_is_networked= FALSE;

	/* Setup for a revert.. */
	revert_game_data.game_is_from_disk = TRUE;
	memcpy(&revert_game_data.saved_game, file, sizeof(FileDesc));

	/* Use the save game file.. */
	set_map_file(file);
	
	/* Load the level from the map */
	success= load_level_from_map(NONE); /* Save games are ALWAYS index NONE */

	if (success)
	{
		unsigned long parent_checksum;
	
		/* Set the non-saved data.... */
		set_current_player_index(0);
		set_local_player_index(0);

		/* Find the original scenario this saved game was a part of.. */
		parent_checksum= read_wad_file_parent_checksum(file);
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
	memcpy(&revert_game_data.game_information, game_info, sizeof(struct game_data));
	memcpy(&revert_game_data.player_start, start, sizeof(struct player_start_data));
	memcpy(&revert_game_data.entry_point, entry, sizeof(struct entry_point));
	
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
		successful= load_game_from_file(&revert_game_data.saved_game);

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

void get_current_saved_game_name(
	unsigned char *file_name)
{
	memcpy(file_name, revert_game_data.saved_game.name, revert_game_data.saved_game.name[0]+1);
}

/* The current mapfile should be set to the save game file... */
boolean save_game_file(
	FileDesc *file)
{
	struct wad_header header;
	FileError err;
	boolean success= FALSE;
	short file_ref;
	long offset, wad_length;
	struct directory_entry entry;
	struct wad_data *wad;

	/* Save off the random seed. */
	dynamic_world->random_seed= get_random_seed();

	/* Setup to revert the game properly */
	revert_game_data.game_is_from_disk= TRUE; 
	memcpy(&revert_game_data.saved_game, file, sizeof(FileDesc));

	/* Fill in the default wad header */
	fill_default_wad_header(file, CURRENT_WADFILE_VERSION, EDITOR_MAP_VERSION, 1, 0, &header);
		
	/* Assume that we confirmed on save as... */
	err= create_wadfile(file, SAVE_GAME_TYPE);
	
	if(!err)
	{
		file_ref= open_wad_file_for_writing(file); /* returns -1 on error */
		if (file_ref>=0)
		{
			/* Write out the new header */
			if (write_wad_header(file_ref, &header))
			{
				offset= sizeof(struct wad_header);
		
				wad= build_save_game_wad(&header, &wad_length);
				if (wad)
				{
					/* Set the entry data.. */
					set_indexed_directory_offset_and_length(&header, 
						&entry, 0, offset, wad_length, 0);
					
					/* Save it.. */
					if (write_wad(file_ref, &header, wad, offset))
					{
						/* Update the new header */
						offset+= wad_length;
						header.directory_offset= offset;
						header.parent_checksum= read_wad_file_checksum(&current_map_file);
						if (write_wad_header(file_ref, &header) && write_directorys(file_ref, &header, &entry))
						{
							/* This function saves the overhead map as a thumbnail, as well */
							/*  as adding the string resource that tells you what it is when */
							/*  it is clicked on & Marathon2 isn't installed.  Obviously, both */
							/*  of these are superfluous for a dos environment. */
							add_finishing_touches_to_save_file(file);

							/* We win. */
							success= TRUE;
						} 
					}

					free_wad(wad);
				}
			}

			close_wad_file(file_ref);
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
void scan_and_add_platforms(
	struct static_platform_data *platform_static_data,
	short count)
{
	struct polygon_data *polygon;
	short loop;
	struct static_platform_data *static_data;
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
					new_platform(static_data, loop);
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
	count= data_length/sizeof(saved_map_pt);
	if(count)
	{
		load_points((saved_map_pt *) data, count);
	} else {
		data= (unsigned char *)extract_type_from_wad(wad, ENDPOINT_DATA_TAG, &data_length);
		count= data_length/sizeof(struct endpoint_data);
		assert(count>=0 && count<MAXIMUM_ENDPOINTS_PER_MAP);

		/* Slam! */
		memcpy(map_endpoints, data, count*sizeof(struct endpoint_data));
		dynamic_world->endpoint_count= count;

		is_preprocessed_map= TRUE;
	}

	/* Extract lines */
	data= (unsigned char *)extract_type_from_wad(wad, LINE_TAG, &data_length);
	load_lines((saved_line *) data, data_length/sizeof(saved_line));

	/* Order is important! */
	data= (unsigned char *)extract_type_from_wad(wad, SIDE_TAG, &data_length);
	load_sides((saved_side *) data, data_length/sizeof(saved_side), version);

	/* Extract polygons */
	data= (unsigned char *)extract_type_from_wad(wad, POLYGON_TAG, &data_length);
	load_polygons((saved_poly *) data, data_length/sizeof(saved_poly), version);

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
			load_lights((struct static_light_data *) data, count, version);
		} else {
			count= data_length/sizeof(struct static_light_data);
			assert(count*sizeof(struct static_light_data)==data_length);
			load_lights((struct static_light_data *) data, count, version);
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
	count= data_length/sizeof(saved_annotation);
	assert(count*sizeof(saved_annotation)==data_length);
	load_annotations((saved_annotation *) data, count);

	/* Extract the objects */
	data= (unsigned char *)extract_type_from_wad(wad, OBJECT_TAG, &data_length);
	count= data_length/sizeof(saved_object);
	assert(count*sizeof(saved_object)==data_length);
	load_objects((saved_object *) data, count);

	/* Extract the map info data */
	data= (unsigned char *)extract_type_from_wad(wad, MAP_INFO_TAG, &data_length);
	// LP change: made this more Pfhorte-friendly
	assert(sizeof(saved_map_data)==data_length || (sizeof(saved_map_data)-2)==data_length);
	// assert(sizeof(saved_map_data)==data_length);
	load_map_info((saved_map_data *) data);

	/* Extract the game difficulty info.. */
	data= (unsigned char *)extract_type_from_wad(wad, ITEM_PLACEMENT_STRUCTURE_TAG, &data_length);
	vassert(data_length==MAXIMUM_OBJECT_TYPES*sizeof(struct object_frequency_definition)*2,
		csprintf(temporary, "data length for placement stuff is wrong (it's %d bytes)", data_length));
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
	count= data_length/sizeof(struct ambient_sound_image_data);
	assert(count*sizeof(struct ambient_sound_image_data)==data_length);
	load_ambient_sound_images((struct ambient_sound_image_data *) data, count);

	/* Extract the random sound images */
	data= (unsigned char *)extract_type_from_wad(wad, RANDOM_SOUND_TAG, &data_length);
	count= data_length/sizeof(struct random_sound_image_data);
	assert(count*sizeof(struct random_sound_image_data)==data_length);
	load_random_sound_images((struct random_sound_image_data *) data, count);

	// LP addition: load the physics-model chunks (all fixed-size)
	int NumChunks;
	bool PhysicsModelLoaded = false;
	
	data= (unsigned char *)extract_type_from_wad(wad, MONSTER_PHYSICS_TAG, &data_length);
	// dprintf("Monsters: %d %d: %d",NUMBER_OF_MONSTER_TYPES,get_monster_defintion_size(),data_length);
	NumChunks = data_length/get_monster_defintion_size();
	assert(NumChunks*get_monster_defintion_size() == data_length);
	assert(NumChunks <= NUMBER_OF_MONSTER_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(monster_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, EFFECTS_PHYSICS_TAG, &data_length);
	// dprintf("Effects: %d %d: %d",NUMBER_OF_EFFECT_TYPES,get_effect_defintion_size(),data_length);
	NumChunks = data_length/get_effect_defintion_size();
	assert(NumChunks*get_effect_defintion_size() == data_length);
	assert(NumChunks <= NUMBER_OF_EFFECT_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(effect_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PROJECTILE_PHYSICS_TAG, &data_length);
	// dprintf("Projectiles: %d %d: %d",NUMBER_OF_PROJECTILE_TYPES,get_projectile_definition_size(),data_length);
	NumChunks = data_length/get_projectile_definition_size();
	assert(NumChunks*get_projectile_definition_size() == data_length);
	assert(NumChunks <= NUMBER_OF_PROJECTILE_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(projectile_definitions,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
	// dprintf("Physics: %d %d: %d",get_number_of_physics_models(),get_physics_definition_size(),data_length);
	NumChunks = data_length/get_physics_definition_size();
	assert(NumChunks*get_physics_definition_size() == data_length);
	assert(NumChunks <= get_number_of_physics_models());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(physics_models,data,data_length);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, WEAPONS_PHYSICS_TAG, &data_length);
	// dprintf("Weapons: %d %d: %d",get_number_of_weapons(),get_weapon_defintion_size(),data_length);
	NumChunks = data_length/get_weapon_defintion_size();
	assert(NumChunks*get_weapon_defintion_size() == data_length);
	assert(NumChunks <= get_number_of_weapons());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		memcpy(weapon_definitions,data,data_length);
	}
	
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
		struct platform_data *platform_structures;
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

		data= (unsigned char *)extract_type_from_wad(wad, PLATFORM_STATIC_DATA_TAG, &data_length);
		count= data_length/sizeof(struct static_platform_data);
		assert(count*sizeof(struct static_platform_data)==data_length);

		platform_structures= (struct platform_data *)extract_type_from_wad(wad, PLATFORM_STRUCTURE_TAG, &data_length);
		platform_structure_count= data_length/sizeof(struct platform_data);
		assert(platform_structure_count*sizeof(struct platform_data)==data_length);

		complete_loading_level((short *) map_index_data, map_index_count,
			(struct static_platform_data *) data, count, platform_structures, 
			platform_structure_count, version);
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
		memcpy(map_indexes, redundant_data, count*sizeof(short));
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
	long *size)
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

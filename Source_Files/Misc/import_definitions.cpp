/*
IMPORT_DEFINITIONS.C
Sunday, October 2, 1994 1:25:23 PM  (Jason')

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 31, 2000 (Loren Petrich):
	Added unpacking code for the physics models
*/

#include "cseries.h"
#include <string.h>

#include "tags.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"
#include "wad.h"
#include "game_errors.h"
// LP addition
#include "FileHandler.h"

// LP: get all the unpacker definitions
#include "monsters.h"
#include "effects.h"
#include "projectiles.h"
#include "player.h"
#include "weapons.h"

/* ---------- globals */

/* sadly extern'ed from their respective files */
// LP: no longer necessary
/*
extern byte monster_definitions[];
extern byte projectile_definitions[];
extern byte effect_definitions[];
extern byte weapon_definitions[];
extern byte physics_models[];
*/

#define IMPORT_STRUCTURE
#include "extensions.h"

/* ---------- local globals */
static FileSpecifier PhysicsFileSpec;

/* ---------- local prototype */
static struct wad_data *get_physics_wad_data(boolean *bungie_physics);
static void import_physics_wad_data(struct wad_data *wad);

/* ---------- code */
void set_physics_file(FileSpecifier& File)
{
	PhysicsFileSpec = File;
	// memcpy(&physics_file, file, sizeof(FileDesc));
	
	return;
}

void set_to_default_physics_file(
	void)
{
	get_default_physics_spec(PhysicsFileSpec);

//	dprintf("Set to: %d %d %.*s", physics_file.vRefNum, physics_file.parID, physics_file.name[0], physics_file.name+1);

	return;
}

void import_definition_structures(
	void)
{
	struct wad_data *wad;
	boolean bungie_physics;
	static boolean warned_about_physics= FALSE;

	wad= get_physics_wad_data(&bungie_physics);
	if(wad)
	{
		if(!bungie_physics && !warned_about_physics)
		{
			/* warn the user that external physics models are Bad Thingª */
			alert_user(infoError, strERRORS, warningExternalPhysicsModel, 0);
			warned_about_physics= TRUE;
		}
		
		/* Actually load it in.. */		
		import_physics_wad_data(wad);
		
		free_wad(wad);
	}

	return;
}

void *get_network_physics_buffer(
	long *physics_length)
{
	void *data= get_flat_data(PhysicsFileSpec, FALSE, 0);
	
	if(data)
	{
		*physics_length= get_flat_data_length(data);
	} else {
		*physics_length= 0;
	}
	
	return data;
}

void process_network_physics_model(
	void *data)
{
	if(data)
	{
		struct wad_header header;
		struct wad_data *wad;
		boolean success= FALSE;
	
		wad= inflate_flat_data(data, &header);
		if(wad)
		{
			import_physics_wad_data(wad);
			free_wad(wad); /* Note that the flat data points into the wad. */
		}
	}
	
	return;
}

/* --------- local code */
static struct wad_data *get_physics_wad_data(
	boolean *bungie_physics)
{
	struct wad_data *wad= NULL;
	
//	dprintf("Open is: %d %d %.*s", physics_file.vRefNum, physics_file.parID, physics_file.name[0], physics_file.name+1);

	OpenedFile PhysicsFile;
	if(open_wad_file_for_reading(PhysicsFileSpec,PhysicsFile));
	{
		struct wad_header header;

		if(read_wad_header(PhysicsFile, &header))
		{
			if(header.data_version==BUNGIE_PHYSICS_DATA_VERSION || header.data_version==PHYSICS_DATA_VERSION)
			{
				wad= read_indexed_wad_from_file(PhysicsFile, &header, 0, TRUE);
				if(header.data_version==BUNGIE_PHYSICS_DATA_VERSION)
				{
					*bungie_physics= TRUE;
				} else {
					*bungie_physics= FALSE;
				}
			}
		}

		close_wad_file(PhysicsFile);
	} 
	
	/* Reset any errors that might have occurred.. */
	set_game_error(systemError, errNone);

	return wad;
}

static void import_physics_wad_data(
	struct wad_data *wad)
{
	// LP: this code is copied out of game_wad.c
	long data_length;
	byte *data;
	short count;
	bool PhysicsModelLoaded = false;
	
	data= (unsigned char *)extract_type_from_wad(wad, MONSTER_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_monster_definition;
	assert(count*SIZEOF_monster_definition == data_length);
	assert(count <= NUMBER_OF_MONSTER_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		unpack_monster_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, EFFECTS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_effect_definition;
	assert(count*SIZEOF_effect_definition == data_length);
	assert(count <= NUMBER_OF_EFFECT_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		unpack_effect_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PROJECTILE_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_projectile_definition;
	assert(count*SIZEOF_projectile_definition == data_length);
	assert(count <= NUMBER_OF_PROJECTILE_TYPES);
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		unpack_projectile_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_physics_constants;
	assert(count*SIZEOF_physics_constants == data_length);
	assert(count <= get_number_of_physics_models());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		unpack_physics_constants(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, WEAPONS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_weapon_definition;
	assert(count*SIZEOF_weapon_definition == data_length);
	assert(count <= get_number_of_weapon_types());
	if (data_length > 0)
	{
		PhysicsModelLoaded = true;
		unpack_weapon_definition(data,count);
	}
	
#if 0
	short index;
	
	for(index= 0; index<NUMBER_OF_DEFINITIONS; ++index)
	{
		long length;
		struct definition_data *definition= definitions+index;
		void *data;
		
		/* Given a wad, extract the given tag from it */
		data= extract_type_from_wad(wad, definition->tag, &length);
		if(data)
		{
			/* Copy it into the proper array */
			memcpy(definition->data, data, length);
		}
	}
#endif
	
	return;
}


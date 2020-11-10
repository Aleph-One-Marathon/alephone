/*
IMPORT_DEFINITIONS.C

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
#include "shell.h"
#include "preferences.h"
#include "FileHandler.h"
#include "shell.h"
#include "preferences.h"

// LP: get all the unpacker definitions
#include "monsters.h"
#include "effects.h"
#include "projectiles.h"
#include "player.h"
#include "weapons.h"
#include "physics_models.h"

#include "AStream.h"

/* ---------- globals */

#define IMPORT_STRUCTURE
#include "extensions.h"

/* ---------- local globals */
static FileSpecifier PhysicsFileSpec;

/* ---------- local prototype */
static struct wad_data *get_physics_wad_data(bool *bungie_physics);
static void import_physics_wad_data(struct wad_data *wad);
static void import_m1_physics_data();
static void import_m1_physics_data_from_network(uint8 *data, uint32 length);

/* ---------- code */
void set_physics_file(FileSpecifier& File)
{
	PhysicsFileSpec = File;
}

void set_to_default_physics_file(
	void)
{
	get_default_physics_spec(PhysicsFileSpec);
//	dprintf("Set to: %d %d %.*s", physics_file.vRefNum, physics_file.parID, physics_file.name[0], physics_file.name+1);
}

void init_physics_wad_data()
{
	init_monster_definitions();
	init_effect_definitions();
	init_projectile_definitions();
	init_physics_constants();
	init_weapon_definitions();
}

bool physics_file_is_m1(void)
{
    bool m1_physics = false;
    
    // check for M1 physics
    OpenedFile PhysicsFile;
    if (PhysicsFileSpec.Open(PhysicsFile))
    {
        uint32 tag = SDL_ReadBE32(PhysicsFile.GetRWops());
        switch (tag)
        {
            case M1_MONSTER_PHYSICS_TAG:
            case M1_EFFECTS_PHYSICS_TAG:
            case M1_PROJECTILE_PHYSICS_TAG:
            case M1_PHYSICS_PHYSICS_TAG:
            case M1_WEAPONS_PHYSICS_TAG:
                m1_physics = true;
                break;
            default:
                break;
        }
        
        PhysicsFile.Close();
    }
    return m1_physics;
}

void import_definition_structures(
	void)
{
	init_physics_wad_data();

	if (physics_file_is_m1())
	{
		import_m1_physics_data();
	}
	else
	{
		struct wad_data *wad;
		bool bungie_physics;
		
		wad= get_physics_wad_data(&bungie_physics);
		if(wad)
		{
			/* Actually load it in.. */		
			import_physics_wad_data(wad);
			
			free_wad(wad);
		}
	}
}

#define M1_PHYSICS_MAGIC_COOKIE (0xDEAFDEAF)

void *get_network_physics_buffer(
	int32 *physics_length)
{
	if (physics_file_is_m1())
	{
		bool success = false;
		uint8 *data = NULL;
		OpenedFile PhysicsFile;
		if (PhysicsFileSpec.Open(PhysicsFile) &&
		    PhysicsFile.GetLength(*physics_length))
		{
			*physics_length += 8;
			data = (uint8 *)malloc(*physics_length);
            SDL_RWops *ops = SDL_RWFromMem(data, *physics_length);
            success = SDL_WriteBE32(ops, uint32(M1_PHYSICS_MAGIC_COOKIE));
            if (success)
                success = SDL_WriteBE32(ops, uint32(*physics_length - 8));
            SDL_RWclose(ops);
            if (success)
                success = PhysicsFile.Read(*physics_length - 8, &data[8]);
			if (!success)
				free(data);
		}
		if (!success)
		{
			*physics_length = 0;
			return NULL;
		}
		return data;
	}
	
	short SavedType, SavedError = get_game_error(&SavedType);
	void *data= get_flat_data(PhysicsFileSpec, false, 0);
	set_game_error(SavedType, SavedError);
	
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
	init_physics_wad_data();

	if(data)
	{
		// check for M1 physics
		SDL_RWops *ops = SDL_RWFromConstMem(data, 8);
		uint32 cookie = SDL_ReadBE32(ops);
		if(cookie == M1_PHYSICS_MAGIC_COOKIE)
		{
			uint32 length= SDL_ReadBE32(ops);
			SDL_RWclose(ops);
			uint8 *s= (uint8 *)data;
			import_m1_physics_data_from_network(&s[8], length);
			return;
		}
		else
		{
			SDL_RWclose(ops);
		}

		struct wad_header header;
		struct wad_data *wad;
	
		wad= inflate_flat_data(data, &header);
		if(wad)
		{
			import_physics_wad_data(wad);
			free_wad(wad); /* Note that the flat data points into the wad. */
		}
	}
}

/* --------- local code */
static struct wad_data *get_physics_wad_data(
	bool *bungie_physics)
{
	struct wad_data *wad= NULL;
	
//	dprintf("Open is: %d %d %.*s", physics_file.vRefNum, physics_file.parID, physics_file.name[0], physics_file.name+1);

	OpenedFile PhysicsFile;
	if(open_wad_file_for_reading(PhysicsFileSpec,PhysicsFile))
	{
		struct wad_header header;

		if(read_wad_header(PhysicsFile, &header))
		{
			if(header.data_version==BUNGIE_PHYSICS_DATA_VERSION || header.data_version==PHYSICS_DATA_VERSION)
			{
				wad= read_indexed_wad_from_file(PhysicsFile, &header, 0, true);
				if(header.data_version==BUNGIE_PHYSICS_DATA_VERSION)
				{
					*bungie_physics= true;
				} else {
					*bungie_physics= false;
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
	size_t data_length;
	byte *data;
	size_t count;
	
	data= (unsigned char *)extract_type_from_wad(wad, MONSTER_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_monster_definition;
	assert(count*SIZEOF_monster_definition == data_length);
	assert(count <= NUMBER_OF_MONSTER_TYPES);
	if (data_length > 0)
	{
		unpack_monster_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, EFFECTS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_effect_definition;
	assert(count*SIZEOF_effect_definition == data_length);
	assert(count <= NUMBER_OF_EFFECT_TYPES);
	if (data_length > 0)
	{
		unpack_effect_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PROJECTILE_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_projectile_definition;
	assert(count*SIZEOF_projectile_definition == data_length);
	assert(count <= NUMBER_OF_PROJECTILE_TYPES);
	if (data_length > 0)
	{
		unpack_projectile_definition(data,count);
	}
	
	data= (unsigned char *)extract_type_from_wad(wad, PHYSICS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_physics_constants;
	assert(count*SIZEOF_physics_constants == data_length);
	assert(count <= get_number_of_physics_models());
	if (data_length > 0)
	{
		unpack_physics_constants(data,count);
	}
	
	data= (unsigned char*) extract_type_from_wad(wad, WEAPONS_PHYSICS_TAG, &data_length);
	count = data_length/SIZEOF_weapon_definition;
	assert(count*SIZEOF_weapon_definition == data_length);
	assert(count <= get_number_of_weapon_types());
	if (data_length > 0)
	{
		unpack_weapon_definition(data,count);
	}
}

static void import_m1_physics_data()
{
	OpenedFile PhysicsFile;
	if (!PhysicsFileSpec.Open(PhysicsFile)) 
	{
		return;
	}

	int32 position  = 0;
	int32 length;
	PhysicsFile.GetLength(length);

	while (position < length)
	{
		std::vector<uint8> header(12);
		PhysicsFile.Read(header.size(), &header[0]);
		AIStreamBE header_stream(&header[0], header.size());

		uint32 tag;
		uint16 count;
		uint16 size;

		header_stream >> tag;
		header_stream.ignore(4); // unused
		header_stream >> count;
		header_stream >> size;

		std::vector<uint8> data(count * size);
		PhysicsFile.Read(data.size(), &data[0]);
		switch (tag) 
		{
		case M1_MONSTER_PHYSICS_TAG:
			unpack_m1_monster_definition(&data[0], count);
			break;
		case M1_EFFECTS_PHYSICS_TAG:
			unpack_m1_effect_definition(&data[0], count);
			break;
		case M1_PROJECTILE_PHYSICS_TAG:
			unpack_m1_projectile_definition(&data[0], count);
			break;
		case M1_PHYSICS_PHYSICS_TAG:
			unpack_m1_physics_constants(&data[0], count);
			break;
		case M1_WEAPONS_PHYSICS_TAG:
			unpack_m1_weapon_definition(&data[0], count);
			break;
		}

		PhysicsFile.GetPosition(position);
	}
}

static void import_m1_physics_data_from_network(uint8 *data, uint32 length)
{
	int32 position = 0;
	while (position < length)
	{
		AIStreamBE header_stream(&data[position], 12);
		position += 12;

		uint32 tag;
		uint16 count;
		uint16 size;

		header_stream >> tag;
		header_stream.ignore(4); // unused
		header_stream >> count;
		header_stream >> size;

		switch (tag)
		{
			case M1_MONSTER_PHYSICS_TAG:
				unpack_m1_monster_definition(&data[position], count);
				break;
			case M1_EFFECTS_PHYSICS_TAG:
				unpack_m1_effect_definition(&data[position], count);
				break;
			case M1_PROJECTILE_PHYSICS_TAG:
				unpack_m1_projectile_definition(&data[position], count);
				break;
			case M1_PHYSICS_PHYSICS_TAG:
				unpack_m1_physics_constants(&data[position], count);
				break;
			case M1_WEAPONS_PHYSICS_TAG:
				unpack_m1_weapon_definition(&data[position], count);
				break;
		}
		position += count * size;
	}
}


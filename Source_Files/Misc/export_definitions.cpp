/*
EXPORT_DEFINITIONS.C

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

Sunday, October 2, 1994 12:35:58 PM  (Jason')
Tuesday, October 31, 1995 11:02:24 AM (Ryan)
*/

#include "macintosh_cseries.h"

#include "map.h"
#include "effects.h"
#include "projectiles.h"
#include "monsters.h"
#include "weapons.h"
#include "wad.h"
#include "items.h"
#include "SoundManager.h"
#include "media.h"
#include "tags.h"

/* ---------- globals */

#define EXPORT_STRUCTURE 1

#include "weapon_definitions.h"
#include "monster_definitions.h"
#include "projectile_definitions.h"
#include "effect_definitions.h"
#include "physics_models.h"

#include "extensions.h"

/* ---------- private code */
static bool create_physics_file(FileDesc *file);

/* ---------- code */

void main(
	int argc,
	char **argv)
{
	if (argc<=1)
	{
		fprintf(stderr, "Usage: %s <destination>\n", argv[0]);
		exit(1);
	}
	else
	{
		FSSpec physics_spec;
		OSErr error;

		initialize_debugger(true);

		/* Get the Marathon FSSpec */
		error= get_my_fsspec(&physics_spec);
		strcpy(temporary, argv[1]);
		c2pstr(temporary);
		error= FSMakeFSSpec(0, 0, temporary, &physics_spec);
		if(!error || error==fnfErr)
		{
			if(!create_physics_file((FileDesc *) &physics_spec))
			{
				fprintf(stderr, "Unable to create the physics file!\n");
			}
		} 
		else
		{
			fprintf(stderr, "%s: Error opening '%s' (Err: %d)", argv[0], argv[1], error);
			exit(1);
		}
	}
	
	exit(0);
}

/* ---------- private code */
static bool create_physics_file(
	FileDesc *file)
{
	bool success= false;
	FileError error= 0;

	error= create_wadfile(file, PHYSICS_FILE_TYPE);
	if(!error)
	{
		fileref file_ref;
		
		file_ref= open_wad_file_for_writing(file);
		if(file_ref != NONE)
		{
			struct wad_header header;
			struct directory_entry entries[MAXIMUM_DIRECTORY_ENTRIES_PER_FILE];

			/* Create the header.. */		
			fill_default_wad_header(file, CURRENT_WADFILE_VERSION,
				BUNGIE_PHYSICS_DATA_VERSION, 1, 0, &header);
			
			if(write_wad_header(file_ref, &header))
			{
				struct wad_data *wad= create_empty_wad();

				if(wad)
				{
					short index;
					
					for (index= 0; index<NUMBER_OF_DEFINITIONS; ++index)
					{
						struct definition_data *definition= definitions+index;
						
						wad= append_data_to_wad(wad, definition->tag, definition->data,
							definition->size*definition->count, 0);
					}
				
					if(write_wad(file_ref, &header, wad, sizeof(struct wad_header)))
					{
						/* Update the header.. */
						entries[0].offset_to_start= sizeof(struct wad_header);
						entries[0].index= 0;
						entries[0].length= calculate_wad_length(&header, wad);

						/* Write the directory.. */
						header.directory_offset= sizeof(struct wad_header)+entries[0].length;
						header.wad_count= 1;
						write_wad_header(file_ref, &header);

						write_directorys(file_ref, &header, entries);
					
						/* We win.. */
						success= true;
					}
				} else {
					error= memory_error();
				}
			}

			/* Save the crc.. */
			calculate_and_store_wadfile_checksum(file_ref);

			close_wad_file(file_ref);

			fprintf(stderr, "Physics File: %P Checksum: 0x%x\n", file->name, 
				read_wad_file_checksum(file));
		}
	}
	
	return success;
}

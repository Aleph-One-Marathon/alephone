/*

	physics_patches.c
	Tuesday, October 31, 1995 9:47:29 PM- rdm created.

	Given two physics files, this compares and builds a patch file.
*/

#include "macintosh_cseries.h"

#include "map.h"
#include "effects.h"
#include "projectiles.h"
#include "monsters.h"
#include "weapons.h"
#include "wad.h"
#include "items.h"
#include "mysound.h"
#include "media.h"
#include "tags.h"

/* ---------- globals */

/* only so we can use the definitions.h file... */
byte monster_definitions[1];
byte projectile_definitions[1];
byte effect_definitions[1];
byte weapon_definitions[1];
byte physics_models[1];

#include "extensions.h"

/* ---------- private code */
static boolean create_physics_file(FileDesc *file);
static boolean create_delta_wad_file(FileDesc *original, FileDesc *new,
	FileDesc *delta, unsigned long delta_file_type);

/* ---------- code */

void main(
	int argc,
	char **argv)
{
	if (argc<=1)
	{
		fprintf(stderr, "Usage: %s <original> <new> <patchname>\n", argv[0]);
		exit(1);
	}
	else
	{
		FSSpec original, new, delta;
		OSErr error;
		
		strcpy(temporary, argv[1]);
		c2pstr(temporary);
		/* Boot drive, desktop.. */
		error= FSMakeFSSpec(0, 0, temporary, &original);
		if(!error)
		{
			strcpy(temporary, argv[2]);
			c2pstr(temporary);
			
			/* Boot drive, desktop.. */
			error= FSMakeFSSpec(0, 0, temporary, &new);
			if(!error)
			{
				strcpy(temporary, argv[3]);
				c2pstr(temporary);
				
				/* Boot drive, desktop.. */
				error= FSMakeFSSpec(0, 0, temporary, &delta);
				if(!error || error==fnfErr)
				{
					if(!create_delta_wad_file((FileDesc *)&original, (FileDesc *)&new, (FileDesc *)&delta, PATCH_FILE_TYPE))
					{
						fprintf(stderr, "Unable to create delta wad.", argv[0], argv[1], error);
						exit(1);
					}
				}
			}
			else
			{
				fprintf(stderr, "%s: Error opening '%s' (Err: %d)", argv[0], argv[2], error);
				exit(1);
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
static struct wad_data *get_physics_wad_from_file(
	FileDesc *file,
	unsigned long *checksum)
{
	struct wad_data *wad= NULL;
	short file_id;
	
	file_id= open_wad_file_for_reading(file);
	if(file_id != NONE)
	{
		struct wad_header header;

		if(read_wad_header(file_id, &header))
		{
			if(header.data_version==PHYSICS_DATA_VERSION)
			{
				wad= read_indexed_wad_from_file(file_id, &header, 0, TRUE);
				if(!wad)
				{
					fprintf(stderr, "Couldn't read wad! (%P)\n", file->name);
				}
				if(checksum) *checksum= header.checksum;
			} else {
				fprintf(stderr, "Bad version! (%P)\n", file->name);
			}
		} else {
			fprintf(stderr, "Unable to read header! (%P)\n", file->name);
		}

		close_wad_file(file_id);
	} else {
		fprintf(stderr, "Unable to open file! (%P)\n", file->name);
	}

	return wad;
}

static boolean write_patchfile(
	FileDesc *file,
	struct wad_data *wad,
	short data_version,
	unsigned long parent_checksum,
	unsigned long patch_filetype)
{
	boolean success= FALSE;
	FileError error= 0;

	error= create_wadfile(file, patch_filetype);
	if(!error)
	{
		fileref file_ref;
		
		file_ref= open_wad_file_for_writing(file);
		if(file_ref != NONE)
		{
			struct wad_header header;
			struct directory_entry entries[1];

			/* Create the header.. */		
			fill_default_wad_header(file, CURRENT_WADFILE_VERSION,
				data_version, 1, 0, &header);

			header.parent_checksum= parent_checksum;
			
			if(write_wad_header(file_ref, &header))
			{
				if(write_wad(file_ref, &header, wad, sizeof(struct wad_header)))
				{
					/* We win.. */
					success= TRUE;
				}
			}

			/* Update the header.. */
			entries[0].offset_to_start= sizeof(struct wad_header);
			entries[0].index= 0;
			entries[0].length= calculate_wad_length(&header, wad);
	
			/* Write the directory.. */
			header.directory_offset= sizeof(struct wad_header)+entries[0].length;
			header.wad_count= 1;
			write_wad_header(file_ref, &header);
	
			write_directorys(file_ref, &header, entries);
			
			calculate_and_store_wadfile_checksum(file_ref);

			close_wad_file(file_ref);
		}
	}
	
	return success;
}

static boolean create_delta_wad_file(
	FileDesc *original, 
	FileDesc *new,
	FileDesc *delta,
	unsigned long delta_file_type)
{
	boolean success= FALSE;
	struct wad_data *original_wad, *new_wad;
	unsigned long parent_checksum;

	original_wad= get_physics_wad_from_file(original, &parent_checksum);
	if(!original_wad)
	{
		fprintf(stderr, "Didn't get original wad!\n");
	}

	new_wad= get_physics_wad_from_file(new, NULL);
	if(!new_wad)
	{
		fprintf(stderr, "Didn't get new wad!\n");
	}

	if(original_wad && new_wad)
	{
		short index;
		boolean had_deltas= FALSE;
		struct wad_data *delta_wad= create_empty_wad();

		fprintf(stderr, "Opened both files…\n");

		/* Loop through all the wads. */		
		for(index= 0; index<NUMBER_OF_DEFINITIONS; ++index)
		{
			long original_length, new_length;
			struct definition_data *definition= definitions+index;
			byte *original_data, *new_data;			
			long first_nonmatching_offset, nonmatching_length, offset;

			/* Given a wad, extract the given tag from it */
			original_data= extract_type_from_wad(original_wad, definition->tag, &original_length);
			new_data= extract_type_from_wad(new_wad, definition->tag, &new_length);

			/* The both _HAVE_ to be full files.. */
			assert(original_data && new_data && new_length==original_length);
			
			/* Now compare them.. */
			first_nonmatching_offset= NONE;
			nonmatching_length= 0;
			for(offset= 0; offset<original_length; ++offset)
			{
				if(original_data[offset]!=new_data[offset])
				{	
					if(first_nonmatching_offset==NONE)
					{
						first_nonmatching_offset= offset;
						had_deltas= TRUE;
					}
					
					nonmatching_length= (offset-first_nonmatching_offset)+1;
				}
			}

			fprintf(stderr, "Tag: %.4s Non matching: %d Length: %d\n",	
				(char *) &definition->tag, first_nonmatching_offset,
				nonmatching_length);

			if(first_nonmatching_offset!=NONE)
			{
				delta_wad= append_data_to_wad(delta_wad, definition->tag, 
					&new_data[first_nonmatching_offset],
					nonmatching_length, first_nonmatching_offset);
			}
		}

		/* If we had the deltas.. */	
		if(had_deltas)
		{
			fprintf(stderr, "File had deltas, writing patchfile\n");
			success= write_patchfile(delta, delta_wad, PHYSICS_DATA_VERSION,	
				parent_checksum, delta_file_type);
				
			if(success)
			{
				fprintf(stderr, "Delta file successfully written..\n");
			} else {
				fprintf(stderr, "Unable to write Delta file..\n");
			}
		} else {
			fprintf(stderr, "The files were identical!\n");
			success= TRUE;
		}

		free_wad(delta_wad);
	}
	
	if(new_wad)	free_wad(new_wad);
	if(original_wad) free_wad(original_wad);

	return success;
}

void *level_transition_malloc(
	size_t size)
{
	return malloc(size);
}

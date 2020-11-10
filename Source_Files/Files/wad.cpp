/*
	WAD.C

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

	Thursday, June 30, 1994 10:54:39 PM

	Tuesday, December 13, 1994 4:31:46 PM- allowed for application specific data in the directory
		data.  This lets me put the names and entry flags in one tight logical place.

	Sunday, February 5, 1995 1:55:01 AM- allow for offset for inplace creation of data, added
		version control so that I can read things that are old, implemented a checksum value 
		for the header, allowed for poor man's search criteria.
		
		Three open calls:
			1) open_wad_file_for_reading -> standard open call.
			2) open_union_wad_file_for_reading-> opens multiple files, based on file type and 
				alphabetical.
			3) open_union_wad_file_for_reading_by_list-> opens multiple files, based on array 
				passed in.

Future Options:
ðÊReentrancy
ðÊUse malloc to actually make it possible to read the data as changes are made in the future?

	Saturday, October 28, 1995 1:13:38 PM- whoops.  Had a huge memory leak in the inflate wad
		data.  Iâm fired.
	
Jan 30, 2000 (Loren Petrich):
	Did some typecasts

Feb 3, 2000 (Loren Petrich):
	Added WADFILE_HAS_INFINITY_STUFF to list of recognized wad types,
		for Marathon Infinity compatibility.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 15, 2000 (Loren Petrich):
	Suppressed union-wad stuff; that was probably some abortive attempt at creating some sort of
	fancy archive format.

Aug 25, 2000 (Loren Petrich):
	Fixed a stupid bug in my reworking of the file handling --
		"if (!open_...)" is now "if (open...)" -- checksumming should now work correctly.

Sep 11, 2000 (Loren Petrich):
	Made get_flat_data() and inflate_flat_data() pack and unpack properly...

July 6, 2001 (Loren Petrich):
	Added Thomas Herzog's more careful wad-version error checking
	
Sep 30, 2001 (Loren Petrich):
	Added support for reading Marathon 1 map files
	(not sure if anyone really wants to write them);
	also added a "between levels" flag so that this may be used with 3D models.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Adjusted Carbon flow to avoid a p2cstr
*/

// Note that level_transition_malloc is specific to marathon...

#include "cseries.h"

#include <string.h>
#include <stdlib.h>

#include "wad.h"
#include "tags.h"
#include "crc.h"
#include "game_errors.h"
#include "interface.h" // for strERRORS

#include "FileHandler.h"
#include "Packing.h"

// Formerly in portable_files.h
inline short memory_error() {return 0;}

/* ---------------- private structures */
// LP: no more union wads

// Indicates whether the wad is being loaded between levels;
// should be "false" for something that may be cleared by some other
// "between-levels" loading, such as 3D models.
bool BetweenLevels = true;

/* ---------------- private global data */
struct wad_internal_data *internal_data[MAXIMUM_OPEN_WADFILES]= {NULL, NULL, NULL};

/* ---------------- private prototypes */
static int32 calculate_directory_offset(struct wad_header *header, short index);
static short get_directory_base_length(struct wad_header *header);
static short get_entry_header_length(struct wad_header *header);
static bool read_indexed_directory_data(OpenedFile& OFile, struct wad_header *header,
	short index, struct directory_entry *entry);
static int32 calculate_raw_wad_length(struct wad_header *file_header, uint8 *wad);
static bool read_indexed_wad_from_file_into_buffer(OpenedFile& OFile, 
	struct wad_header *header, short index, void *buffer, int32 *length);
static short count_raw_tags(uint8 *raw_wad);
static struct wad_data *convert_wad_from_raw(struct wad_header *header, uint8 *data,	int32 wad_start_offset,
	int32 raw_length);
static struct wad_data *convert_wad_from_raw_modifiable(struct wad_header *header, uint8 *raw_wad, int32 raw_length);
//static void patch_wad_from_raw(struct wad_header *header, uint8 *raw_wad, struct wad_data *read_wad);
static bool size_of_indexed_wad(OpenedFile& OFile, struct wad_header *header, short index, 
	int32 *length);

static bool write_to_file(OpenedFile& OFile, int32 offset, void *data, int32 length);
static bool read_from_file(OpenedFile& OFile, int32 offset, void *data, int32 length);

// LP: routines for packing and unpacking the data from streams of bytes
static uint8 *unpack_wad_header(uint8 *Stream, wad_header *Objects, size_t Count);
static uint8 *pack_wad_header(uint8 *Stream, wad_header *Objects, size_t Count);
static uint8 *unpack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, size_t Count);
static uint8 *pack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, size_t Count);
static uint8 *unpack_directory_entry(uint8 *Stream, directory_entry *Objects, size_t Count);
static uint8 *pack_directory_entry(uint8 *Stream, directory_entry *Objects, size_t Count);
//static uint8 *unpack_old_entry_header(uint8 *Stream, old_entry_header *Objects, size_t Count);
static uint8 *pack_old_entry_header(uint8 *Stream, old_entry_header *Objects, size_t Count);
static uint8 *unpack_entry_header(uint8 *Stream, entry_header *Objects, size_t Count);
static uint8 *pack_entry_header(uint8 *Stream, entry_header *Objects, size_t Count);

/* ------------------ Code Begins */

bool read_wad_header(
	OpenedFile& OFile, 
	struct wad_header *header)
{
	int error = 0;
	bool success= true;
	
	uint8 buffer[SIZEOF_wad_header];
	error = !read_from_file(OFile, 0, buffer, SIZEOF_wad_header);
	unpack_wad_header(buffer,header,1);
	
	if(error)
	{
		set_game_error(systemError, error);
		success= false;
	} else {
		// Thomas Herzog made this error checking more careful
		if((header->version>CURRENT_WADFILE_VERSION) || (header->data_version > 2) || (header->wad_count < 1))
		{
			set_game_error(gameError, errUnknownWadVersion);
			success= false;
		}
	}
	
	return success;
}

extern void *level_transition_malloc(size_t size);

/* This could be improved.  Under the current implementation, it requires 2X sizeof level worth */
/*  of memory to load... (This makes writing wads easier, but isn't really useful for loading */
/* Note that this does the correct thing for union wadfiles... */
struct wad_data *read_indexed_wad_from_file(
	OpenedFile& OFile, 
	struct wad_header *header, 
	short index,
	bool read_only)
{
	struct wad_data *read_wad= (struct wad_data *) NULL;
	uint8 *raw_wad = NULL;
     int32 length = 0;
	int error = 0;

	// if(file_id>=0) /* NOT a union wadfile... */
	{
		if (size_of_indexed_wad(OFile, header, index, &length))
		{
			// The padding is so that one can use later-Marathon entry-header reading
			// on Marathon 1 wadfiles, which have a shorter entry header
			int32 padded_length = length + (SIZEOF_entry_header-SIZEOF_old_entry_header);

			raw_wad= BetweenLevels ?
				(uint8 *) level_transition_malloc(padded_length) :
				(uint8 *) malloc(padded_length);
			
			if(raw_wad)
			{
				/* Read into the buffer */
				if (read_indexed_wad_from_file_into_buffer(OFile, header, index, raw_wad, &length))
				{
					/* Got the raw wad. Convert it into our internal representation... */
					if(read_only)
					{
						read_wad= convert_wad_from_raw(header, raw_wad, 0, length);
					} else {
						read_wad= convert_wad_from_raw_modifiable(header, raw_wad, length);
					}
					if(!read_wad)
					{
						/* Error.. */
						error= memory_error();
					}
					if(!read_wad || !read_only)
					{
						free(raw_wad);
						raw_wad = NULL;
					}
				}
				else
				{
					free(raw_wad);
					raw_wad = NULL;
				}
			} else {
				error= memory_error();
			}
		}
	}

	if(error)
	{
		set_game_error(systemError, error);
	}
	
	return read_wad;
}

void *extract_type_from_wad(
	struct wad_data *wad,
	WadDataType type, 
	size_t *length)
{
	void *return_value= NULL;
	short index;
	
	*length= 0;
	
	assert(wad);
	for(index= 0; index<wad->tag_count; ++index)
	{
		if(wad->tag_data[index].tag==type)
		{
			return_value= wad->tag_data[index].data;
			*length= wad->tag_data[index].length;
			assert(wad->tag_data[index].length >= 0);
			break;
		}
	}
	
	return return_value;
}

bool wad_file_has_checksum(
	FileSpecifier& File, 
	uint32 checksum)
{
	bool has_checksum= false;

	if(checksum==read_wad_file_checksum(File))
	{
		has_checksum= true;
	}
	
	return has_checksum;
}

uint32 read_wad_file_checksum(FileSpecifier& File)
{
	struct wad_header header;
	uint32 checksum= 0;
	
	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	{
		if(read_wad_header(OFile, &header))
		{
			checksum= header.checksum;
		}
		
		close_wad_file(OFile);
	}
	
	return checksum;
}

uint32 read_wad_file_parent_checksum(FileSpecifier& File)
{
	// fileref file_id;
	struct wad_header header;
	uint32 checksum= 0;

	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	{
		if(read_wad_header(OFile, &header))
		{
			checksum= header.parent_checksum;
		}
		
		close_wad_file(OFile);
	}
	
	return checksum;
}

bool wad_file_has_parent_checksum(
	FileSpecifier& File, 
	uint32 parent_checksum)
{
	// fileref file_id;
	bool has_checksum= false;
	struct wad_header header;

	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	// file_id= open_wad_file_for_reading(file);
	// if(file_id>=0)
	{
		if(read_wad_header(OFile, &header))
		// if(read_wad_header(file_id, &header))
		{
			if(header.parent_checksum==parent_checksum)
			{
				/* Found a match!  */
				has_checksum= true;
			}
		}
		
		close_wad_file(OFile);
		// close_wad_file(file_id);
	}
	
	return has_checksum;
}

/* ------------ Writing functions */
struct wad_data *create_empty_wad(void)
{
	struct wad_data *wad;

	wad= (struct wad_data *) malloc(sizeof(struct wad_data));
	if(wad)
	{
		obj_clear(*wad); /* IMPORTANT! */
	}

	return wad;
} 

void fill_default_wad_header(
	FileSpecifier& File, 
	short wadfile_version,
	short data_version, 
	short wad_count,
	short application_directory_data_size,
	struct wad_header *header)
{
	obj_clear(*header);
	header->version= wadfile_version;
	header->data_version= data_version;
	File.GetName(header->file_name);
	header->wad_count= wad_count;
	header->application_specific_directory_data_size= application_directory_data_size;					

	header->entry_header_size= get_entry_header_length(header);
	if(!header->entry_header_size) 
	{
		/* Default.. */
		header->entry_header_size = SIZEOF_entry_header;
	}

	header->directory_entry_base_size= get_directory_base_length(header);
	if(!header->directory_entry_base_size)
	{
		header->directory_entry_base_size = SIZEOF_directory_entry;
	}

	/* Things left for caller to fill in: */
	/* uint32 checksum, int32 directory_offset, uint32 parent_checksum */
}

bool write_wad_header(
	OpenedFile& OFile, 
	struct wad_header *header)
{
	bool success= true;

	uint8 buffer[SIZEOF_wad_header];
	obj_clear(buffer);
	pack_wad_header(buffer,header,1);
	write_to_file(OFile, 0, buffer, SIZEOF_wad_header);

	return success;
}

// Takes raw, unswapped directory data
bool write_directorys(
	OpenedFile& OFile, 
	struct wad_header *header,
	void *entries)
{
	int32 size_to_write= get_size_of_directory_data(header);
	bool success= true;
	
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	write_to_file(OFile, header->directory_offset, entries, 
		size_to_write);

	return success;
}

/* Note wad_count better be correct! */
int32 get_size_of_directory_data(
	struct wad_header *header)
{
	short base_entry_size= get_directory_base_length(header);

	assert(header->wad_count);
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY || header->application_specific_directory_data_size==0);

	return (header->wad_count*
		(header->application_specific_directory_data_size+base_entry_size));
}

// Takes raw, unswapped directory data
void *get_indexed_directory_data(
	struct wad_header *header,
	short index,
	void *directories)
{
	// LP: changed "char *" to "uint8 *"
	uint8 *data_ptr= (uint8 *)directories;
	short base_entry_size= get_directory_base_length(header);

	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	assert(index>=0 && index<header->wad_count);
	data_ptr += index*(header->application_specific_directory_data_size+base_entry_size);
	data_ptr += base_entry_size; /* Because the application specific junk follows the standard entries */

	return ((void *) data_ptr);
}

void set_indexed_directory_offset_and_length(
	struct wad_header *header,
	void *entries,
	short index,
	int32 offset,
	int32 length,
	short wad_index)
{
	uint8 *data_ptr= (uint8 *)entries;
	int32 data_offset;
	
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	
	/* calculate_directory_offset is for the file, by subtracting the base, we get the actual offset.. */
	data_offset= calculate_directory_offset(header, index) - header->directory_offset;

	data_ptr+= data_offset;
	
	// LP: eliminating this dangerous sort of casting;
	// should work correctly for wadfiles with size more than 1
	/*
	entry= (struct directory_entry *) data_ptr;
	
	entry->length= length;
	entry->offset_to_start= offset;
	
	if(header->version>=WADFILE_SUPPORTS_OVERLAYS)
	{
		entry->index= wad_index;
	}
	*/
	
	// LP: should be correct for packing also
	if (header->version>=WADFILE_SUPPORTS_OVERLAYS)
	{
		directory_entry entry;
		
		entry.length = length;
		entry.offset_to_start = offset;
		entry.index = wad_index;
		
		pack_directory_entry(data_ptr, &entry, 1);
	}
	else
	{
		old_directory_entry entry;
		
		entry.length = length;
		entry.offset_to_start = offset;
		
		pack_old_directory_entry(data_ptr, &entry, 1);
	}
}

// Returns raw, unswapped directory data
void *read_directory_data(
	OpenedFile& OFile,
	struct wad_header *header)
{
	int32 size;
	uint8 *data;
	
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	
	size= get_size_of_directory_data(header);
	data= (uint8 *)malloc(size);
	if(data)
	{
		read_from_file(OFile, header->directory_offset, data, size);
	}

	return data;
}

struct wad_data *append_data_to_wad(
	struct wad_data *wad, 
	WadDataType type, 
	const void *data,
	size_t size,
	size_t offset) /* Allows for inplace creation of wadfiles */
{
	short index;

	assert(size); /* You can't append zero length data anymore! */
	assert(wad);
	assert(!wad->read_only_data);

	/* Find the index to replace */
	for(index= 0; index<wad->tag_count; ++index)
	{
		if(wad->tag_data[index].tag==type) 
		{
			/* Just free it, and let it go. */
			free(wad->tag_data[index].data);
			break;
		}
	}
	
	/* If we are appending... */
	if(index==wad->tag_count)
	{
		struct tag_data *old_data= wad->tag_data;

		wad->tag_count++;
		wad->tag_data= (struct tag_data *) malloc(wad->tag_count*sizeof(struct tag_data));

		if(!wad->tag_data)
		{
			alert_out_of_memory();
		}

		assert(wad->tag_data);
		objlist_clear(wad->tag_data, wad->tag_count);
		if(old_data)
		{
			objlist_copy(wad->tag_data, old_data, (wad->tag_count-1));
			free(old_data);
		}
	}

	/* Copy it in.. */
	assert(index>=0 && index<wad->tag_count);
	wad->tag_data[index].data= (uint8 *) malloc(size);
	if(!wad->tag_data[index].data)
	{
		alert_out_of_memory();
	}
	assert(wad->tag_data[index].data);
		
	memcpy(wad->tag_data[index].data, data, size);

	/* Setup the tag data. */
	wad->tag_data[index].tag= type;
	wad->tag_data[index].length= size;
	wad->tag_data[index].offset= offset;

	return wad;
}

void remove_tag_from_wad(
	struct wad_data *wad, 
	WadDataType type)
{
	short index;

	assert(wad);
	assert(!wad->read_only_data);

	/* Find the index to replace */
	for(index= 0; index<wad->tag_count; ++index)
	{
		if(wad->tag_data[index].tag==type) break;
	}
	
	/* If we are appending... */
	if(index!=wad->tag_count)
	{
		struct tag_data *old_data= wad->tag_data;

		wad->tag_count-= 1;
		wad->tag_data= (struct tag_data *) malloc(wad->tag_count*sizeof(struct tag_data));
		
		if(!wad->tag_data)
		{
			alert_out_of_memory();
		}

		assert(wad->tag_data);
		objlist_clear(wad->tag_data, wad->tag_count);
		if(old_data)
		{
			/* Copy the stuff below it. */
			objlist_copy(wad->tag_data, old_data, index);
			
			/* Copy the stuff above it. */
			objlist_copy(&wad->tag_data[index], &old_data[index+1], (wad->tag_count-index));
			
			free(old_data);
		}
	}
}

/* Now uses CRC to checksum.. */
void calculate_and_store_wadfile_checksum(OpenedFile& OFile)
{
	struct wad_header header;
	
	/* read the header */
	read_wad_header(OFile, &header);

	/* Make sure we don't checksum the checksum value.. */
	header.checksum= 0l;
	write_wad_header(OFile, &header);
	
	/* Unused bytes better ALWAYS be initialized to zero.. */
	header.checksum= calculate_crc_for_opened_file(OFile);

	/* Save it.. */
	write_wad_header(OFile, &header);
}

bool write_wad(
	OpenedFile& OFile, 
	struct wad_header *file_header,
	struct wad_data *wad, 
        int32 offset)
{
	int error = 0;
	bool success;
	short entry_header_length= get_entry_header_length(file_header);
	short index;
	struct entry_header header;
	int32 running_offset= 0l;

	assert(wad);
	assert(!wad->read_only_data);

	for(index=0; !error && index<wad->tag_count; ++index)
	{
		header.tag= wad->tag_data[index].tag;
		header.length= wad->tag_data[index].length;

		/* On older versions, this will get overwritten by the copy.. */
		header.offset= wad->tag_data[index].offset;
	
		if(index==wad->tag_count-1)
		{
			/* Last one's next offset is zero.. */
			header.next_offset= 0;
		} else {
			running_offset+= header.length+entry_header_length;
			header.next_offset= running_offset;
		}

		/* Write this to the file... */
		uint8 buffer[MAX(SIZEOF_old_entry_header,SIZEOF_entry_header)];
		switch (entry_header_length)
		{
		case SIZEOF_old_entry_header:
			pack_old_entry_header(buffer,(old_entry_header *)&header,1);
			break;
		case SIZEOF_entry_header:
			pack_entry_header(buffer,&header,1);
			break;
		default:
			vassert(false,csprintf(temporary,"Unrecognized entry-header length: %d",entry_header_length));
		}
		if (write_to_file(OFile, offset, buffer, entry_header_length))
		{
			offset+= entry_header_length;
		
			/* Write the data.. */
			write_to_file(OFile, offset, wad->tag_data[index].data, wad->tag_data[index].length);
			{
				offset+= wad->tag_data[index].length;
			}
		}
	}
	
	if(error)
	{
		success= false;
		set_game_error(systemError, error);
	} else {
		success= true;
	}
	
	return success;
}

short number_of_wads_in_file(FileSpecifier& File)
{
	short count= NONE;
	
	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	{
		struct wad_header header;
		
		/* read the header */
		read_wad_header(OFile, &header);
		
		count= header.wad_count;
		
		close_wad_file(OFile);
	}
	
	return count;
}

void free_wad(
	struct wad_data *wad)
{
	short ii;
	
	assert(wad);
	
	/* Free all of the tags */
	if(wad->read_only_data)
	{
		/* Read only wad.. */
		free(wad->read_only_data);
		free(wad->tag_data);
	} else {
		/* Modifiable */
		for(ii=0; ii<wad->tag_count; ++ii)
		{
			assert(wad->tag_data[ii].data);
			free(wad->tag_data[ii].data);
		}
		free(wad->tag_data);
	}
	
	/* And free the total data.. */
	free(wad);
}

int32 calculate_wad_length(
	struct wad_header *file_header, 
	struct wad_data *wad)
{
	short ii;
	short header_length= get_entry_header_length(file_header);
	int32 running_length= 0l;

	for(ii= 0; ii<wad->tag_count; ++ii)
	{
		running_length += wad->tag_data[ii].length + header_length;
	}
	
	return running_length;
}

/* ------------ Transfer type functions */
#define CURRENT_FLAT_MAGIC_COOKIE (0xDEADDEAD)

/*
	LP: ought not to use such a struct directly, because this is supposed to be packed data
	Format:
	4 bytes -- magic cookie
	4 bytes -- length
	SIZEOF_wad_header -- packed wad header
*/
const int SIZEOF_encapsulated_wad_data = 2*4 + SIZEOF_wad_header;
	

void *get_flat_data(
	FileSpecifier& File, 
	bool use_union, 
	short wad_index)
{
	struct wad_header header;
	bool success= false;
	uint8 *data= NULL;
	
	assert(!use_union);
	
	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	{
		/* Read the file */
		success= read_wad_header(OFile, &header);

		if (success)
		{
			int32 length;
			int error = 0;
		
			/* Allocate the conglomerate data.. */
			if (size_of_indexed_wad(OFile, &header, wad_index, &length))
			{
				data= (uint8 *)malloc(length+SIZEOF_encapsulated_wad_data);
				if(data)
				{
					uint8 *buffer= data + SIZEOF_encapsulated_wad_data;
					
					// Pack the encapsulated header
					uint8 *S = data;
					ValueToStream(S,uint32(CURRENT_FLAT_MAGIC_COOKIE));
					ValueToStream(S,int32(length + SIZEOF_encapsulated_wad_data));
					S = pack_wad_header(S,&header,1);
					assert((S - data) == SIZEOF_encapsulated_wad_data);
					
					/* Read into our buffer... */
					success = read_indexed_wad_from_file_into_buffer(OFile, &header, wad_index, 
						buffer, &length);
					
					if (!success)
					{
						/* Error-> didn't get it.. */
						free(data);
						data= NULL;
						error = OFile.GetError();
					}
				} 
				else 
				{
					error= memory_error();
				}
			}

			set_game_error(systemError, error);
		}
		
		/* Close the file.. */
		close_wad_file(OFile);
	}

	return data;
}

int32 get_flat_data_length(
	void *data)
{
	int32 Length;
	uint8 *S = (uint8 *)data;
	S += 4;
	StreamToValue(S,Length);
	return Length;
}

/* This is how you dispose of it-> you inflate it, then use free_wad() */
struct wad_data *inflate_flat_data(
	void *data, 
	struct wad_header *header)
{
	struct wad_data *wad= NULL;
	uint8 *buffer= ((uint8 *) data)+SIZEOF_encapsulated_wad_data;
	int32 raw_length;

	assert(data);
	assert(header);
	
	uint32 MagicCookie;
	uint8 *S = (uint8 *)data;
	StreamToValue(S,MagicCookie);
	assert(MagicCookie==CURRENT_FLAT_MAGIC_COOKIE);
	
	// Get the length here, where it's convenient
	int32 Length;
	StreamToValue(S,Length);
	
	S = unpack_wad_header(S,header,1);
	assert((S - (uint8 *)data) == SIZEOF_encapsulated_wad_data);

	raw_length= calculate_raw_wad_length(header, buffer);
	assert(raw_length==Length-SIZEOF_encapsulated_wad_data);
	
	/* Now inflate.. */
	wad= convert_wad_from_raw(header, (uint8 *)data, SIZEOF_encapsulated_wad_data, raw_length);
	
	return wad;
}

/* ---------- debugging routines. */
void dump_wad(
	struct wad_data *wad)
{
	short index;
	struct tag_data *tag= wad->tag_data;

	dprintf("---Dumping---");
	dprintf("Tag Count: %d", wad->tag_count);
	for(index= 0; index<wad->tag_count; ++index)
	{
		assert(tag);
		dprintf("Tag: %x data: %p length: %d offset: %d", tag->tag, tag->data, tag->length,
			tag->offset);
		tag++;
	}
	dprintf("---End of Dump---");
}

/* ---------- file management routines */
bool create_wadfile(FileSpecifier& File, Typecode Type)
{
	return File.Create(Type);
}

static bool open_wad_file_or_set_error(FileSpecifier& File, OpenedFile& OFile, bool Writable)
{
	if (!File.Open(OFile, Writable))
	{
		set_game_error(systemError, File.GetError());
		return false;
	}
	return true;
}

bool open_wad_file_for_reading(FileSpecifier& File, OpenedFile& OFile)
{
	return open_wad_file_or_set_error(File, OFile, false);
}

bool open_wad_file_for_writing(FileSpecifier& File, OpenedFile& OFile)
{
	return open_wad_file_or_set_error(File, OFile, true);
}

void close_wad_file(OpenedFile& File)
{
	File.Close();
}

/* ------------------------------ Private Code --------------- */
static bool size_of_indexed_wad(
	OpenedFile& OFile, 
	struct wad_header *header, 
	short index, 
	int32 *length)
{
	struct directory_entry entry;
	// FileError error;
	
	// assert(file_id>=0); /* No union wads! */
	
	if (read_indexed_directory_data(OFile, header, index, &entry))
	{
		*length= entry.length;
	}
	else return false;
	
	return true;
}

static int32 calculate_directory_offset(
	struct wad_header *header, 
	short index)
{
	int32 offset;
	int32 unit_size;

	switch(header->version)
	{
		case PRE_ENTRY_POINT_WADFILE_VERSION:
			assert(header->application_specific_directory_data_size==0);
			// OK for Marathon 1		
		case WADFILE_HAS_DIRECTORY_ENTRY:
		case WADFILE_SUPPORTS_OVERLAYS:
		// LP addition:
		case WADFILE_HAS_INFINITY_STUFF:
			assert(header->application_specific_directory_data_size>=0);
			unit_size= header->application_specific_directory_data_size+get_directory_base_length(header);
			break;
			
		default:
			vhalt(csprintf(temporary, "what is version %d?", header->version));
			break;
	}

	/* Now actually calculate it (Note that the directory_entry data is first) */
	offset= header->directory_offset+(index*unit_size);
	
	return offset;
}

static short get_entry_header_length(
	struct wad_header *header)
{
	short size;

	assert(header);
	
	switch(header->version)
	{
		case PRE_ENTRY_POINT_WADFILE_VERSION:
		case WADFILE_HAS_DIRECTORY_ENTRY:
			size = SIZEOF_old_entry_header;
			break;

		default:
			/* After this point, I stored it. */
			size = header->entry_header_size;
			break;
	}
	
	return size;
}

static short get_directory_base_length(
	struct wad_header *header)
{
	short size;
	
	assert(header);
	assert(header->version<=CURRENT_WADFILE_VERSION);

	switch(header->version)
	{
		case PRE_ENTRY_POINT_WADFILE_VERSION:
		case WADFILE_HAS_DIRECTORY_ENTRY:
			size = SIZEOF_old_directory_entry;
			break;

		default:
			/* After this point, I stored it. */
			size = header->directory_entry_base_size;
			break;
	}
		
	return size;
}

/* This searches the directories for the given index, to allow for special replacements. */
static bool read_indexed_directory_data(
	OpenedFile& OFile,
	struct wad_header *header,
	short index,
	struct directory_entry *entry)
{
	short base_entry_size;
	int32 offset;

	/* Get the sizes of the data structures */
	base_entry_size= get_directory_base_length(header);
	
	/* For old files, the index==the actual index */
	if(header->version<=WADFILE_HAS_DIRECTORY_ENTRY) 
	{
		/* Calculate the offset */
		offset= calculate_directory_offset(header, index);

		/* Read it! */
		assert(base_entry_size<=SIZEOF_directory_entry);
		
		uint8 buffer[MAX(SIZEOF_old_directory_entry,SIZEOF_directory_entry)];
		if (!read_from_file(OFile, offset, buffer, base_entry_size))
			return false;
		switch (base_entry_size)
		{
		case SIZEOF_old_directory_entry:
			unpack_old_directory_entry(buffer,(old_directory_entry *)entry,1);
			break;
		case SIZEOF_directory_entry:
			unpack_directory_entry(buffer,entry,1);
			break;
		default:
			vassert(false,csprintf(temporary,"Unrecognized base-entry length: %d",base_entry_size));
		}
		return true;

	} else {

		short directory_index;

		/* Pin it, so we can try to read future file formats */
		if(base_entry_size>SIZEOF_directory_entry) 
		{
			base_entry_size= SIZEOF_directory_entry;
		}
	
		/* We have to loop.. */
		for(directory_index= 0; directory_index<header->wad_count; ++directory_index)
		{
			/* We use a hint, that the index is the real index, to help make this have */
			/* a "hit" on the first try */
			short test_index= (index+directory_index)%header->wad_count;
		
			/* Calculate the offset */
			offset= calculate_directory_offset(header, test_index);

			/* Read it.. */
			uint8 buffer[MAX(SIZEOF_old_directory_entry,SIZEOF_directory_entry)];
			if (!read_from_file(OFile, offset, buffer, base_entry_size))
				return false;
			switch (base_entry_size)
			{
			case SIZEOF_old_directory_entry:
				unpack_old_directory_entry(buffer,(old_directory_entry *)entry,1);
				break;
			case SIZEOF_directory_entry:
				unpack_directory_entry(buffer,entry,1);
				break;
			default:
				vassert(false,csprintf(temporary,"Unrecognized base-entry length: %d",base_entry_size));
			}
			if(entry->index==index) 
			{
				return true; /* Got it */
			}
		}
	}

	/* Not found */
	return false;
}

/* Internal function.. */
static bool read_indexed_wad_from_file_into_buffer(
	OpenedFile& OFile, 
	struct wad_header *header, 
	short index,
	void *buffer,
	int32 *length) /* Length of maximum buffer on entry, actual length on return */
{
	struct directory_entry entry;
	bool success = false;

	/* Read the directory entry first */
	if (read_indexed_directory_data(OFile, header, index, &entry))
	{
		/* Some sanity checks */
		assert(*length<=entry.length);
		assert(buffer);
		
		/* Set the length */
		*length= entry.length;

		/* Read into it. */
		if (entry.length > 0) {
			success = read_from_file(OFile, entry.offset_to_start, buffer, entry.length);

			/* Veracity Check */
			/* ! an error, it has a length non-zero and calculated != actual */
			assert(entry.length==calculate_raw_wad_length(header, (uint8 *)buffer));
		}
	}
	
	return success;
}

/* This *MUST* be a base wad.. */
static struct wad_data *convert_wad_from_raw(
	struct wad_header *header, 
	uint8 *data,
	int32 wad_start_offset,
	int32 raw_length)
{
	struct wad_data *wad;
	uint8 *raw_wad;

	/* In case we are somewhere else, like, for example, in a net transferred level.. */
	raw_wad= data+wad_start_offset;
	
	wad= (struct wad_data *) malloc(sizeof(struct wad_data));
	if(wad)
	{
		short tag_count;

		/* Clear it */
		obj_clear(*wad);

		/* If the wad is of non-zero length... */
		if(raw_length) 
		{
			/* Count the tags */
			tag_count= count_raw_tags(raw_wad);
			
			/* Allocate the tags.. */
			wad->tag_count= tag_count;
			wad->tag_data= (struct tag_data *) malloc(tag_count * sizeof(struct tag_data));
			if(wad->tag_data)
			{
				short index;
				short entry_header_size;
			
				/* Clear it */
				objlist_clear(wad->tag_data, tag_count);
				
				entry_header_size= get_entry_header_length(header);
				entry_header wad_entry_header;
				uint8 *raw_wad_entry_header = raw_wad;
				// Will work OK for Marathon 1
				unpack_entry_header(raw_wad_entry_header, &wad_entry_header, 1);
				
				/* Note that this is a read only wad.. */	
				wad->read_only_data= data;
	
				for(index= 0; index<tag_count; ++index)
				{
					assert(header->version<WADFILE_SUPPORTS_OVERLAYS || wad_entry_header.offset == 0);
					wad->tag_data[index].tag = wad_entry_header.tag;
					wad->tag_data[index].length = wad_entry_header.length;
					wad->tag_data[index].offset = 0;
					wad->tag_data[index].data = raw_wad_entry_header + entry_header_size;

					raw_wad_entry_header = raw_wad + wad_entry_header.next_offset;
					// Will work OK for Marathon 1
					unpack_entry_header(raw_wad_entry_header, &wad_entry_header, 1);
				} 
			} else {
				alert_out_of_memory();
			}
		}
	}
	
	return wad;
}

/* This *MUST* be a base wad.. */
static struct wad_data *convert_wad_from_raw_modifiable(
	struct wad_header *header, 
	uint8 *raw_wad,
	int32 raw_length)
{
	struct wad_data *wad;

	wad= (struct wad_data *) malloc(sizeof(struct wad_data));
	if(wad)
	{
		short tag_count;

		/* Clear it */
		obj_clear(*wad);

		/* If the wad is of non-zero length... */
		if(raw_length) 
		{
			/* Count the tags */
			tag_count= count_raw_tags(raw_wad);
	
			/* Allocate the tags.. */
			wad->tag_count= tag_count;
			wad->tag_data= (struct tag_data *) malloc(tag_count * sizeof(struct tag_data));
			if(wad->tag_data)
			{
				short index;
				short entry_header_size;
			
				/* Clear it */
				objlist_clear(wad->tag_data, tag_count);
				
				entry_header_size= get_entry_header_length(header);
				entry_header wad_entry_header;
				uint8 *raw_wad_entry_header = raw_wad;
				// Will work OK for Marathon 1
				unpack_entry_header(raw_wad_entry_header, &wad_entry_header, 1);
				
				for(index= 0; index<tag_count; ++index)
				{
					wad->tag_data[index].tag = wad_entry_header.tag;
					wad->tag_data[index].length = wad_entry_header.length;
					wad->tag_data[index].data = (uint8 *) malloc(wad->tag_data[index].length);
					if(!wad->tag_data[index].data)
					{
						alert_out_of_memory();
					}
					wad->tag_data[index].offset= 0l;
					
					/* This MUST be a base! */
					assert(header->version<WADFILE_SUPPORTS_OVERLAYS || wad_entry_header.offset == 0);
	
					/* Copy the data.. */
					memcpy(wad->tag_data[index].data, raw_wad_entry_header + entry_header_size, wad->tag_data[index].length);
					raw_wad_entry_header = raw_wad + wad_entry_header.next_offset;
					// Will work OK for Marathon 1
					unpack_entry_header(raw_wad_entry_header, &wad_entry_header, 1);
				} 
			}
		}
	}
	
	return wad;
}

// Will work OK for Marathon 1
static short count_raw_tags(
	uint8 *raw_wad)
{
	int tag_count = 0;

	entry_header header;
	unpack_entry_header(raw_wad, &header, 1);
	while (true) {
		tag_count++;
		uint32 next_offset = header.next_offset;
		if (next_offset == 0)
			break;
		unpack_entry_header(raw_wad + next_offset, &header, 1);
	}

	return tag_count;
}

// Will work OK for Marathon 1
static int32 calculate_raw_wad_length(
	struct wad_header *file_header,
	uint8 *wad)
{
	int entry_header_size = get_entry_header_length(file_header);
	int32 length = 0;

	entry_header header;
	unpack_entry_header(wad, &header, 1);
	while (true) {
		length += header.length + entry_header_size;
		uint32 next_offset = header.next_offset;
		if (next_offset == 0)
			break;
		unpack_entry_header(wad + next_offset, &header, 1);
	}

	return length;
}

static bool write_to_file(
	OpenedFile& OFile, 
	int32 offset, 
	void *data, 
	int32 length)
{
	if (!OFile.SetPosition(offset)) return false;
	return OFile.Write(length, data);
}

static bool read_from_file(
	OpenedFile& OFile, 
	int32 offset, 
	void *data, 
	int32 length)
{
	if (!OFile.SetPosition(offset)) return false;
	return OFile.Read(length, data);
}

static uint8 *unpack_wad_header(uint8 *Stream, wad_header *Objects, size_t Count)
{
	uint8* S = Stream;
	wad_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->version);
		StreamToValue(S,ObjPtr->data_version);
		StreamToBytes(S,ObjPtr->file_name,MAXIMUM_WADFILE_NAME_LENGTH);
		StreamToValue(S,ObjPtr->checksum);
		StreamToValue(S,ObjPtr->directory_offset);
		StreamToValue(S,ObjPtr->wad_count);
		StreamToValue(S,ObjPtr->application_specific_directory_data_size);
		StreamToValue(S,ObjPtr->entry_header_size);
		StreamToValue(S,ObjPtr->directory_entry_base_size);
		StreamToValue(S,ObjPtr->parent_checksum);
		S += 2*20;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_wad_header));
	return S;
}

static uint8 *pack_wad_header(uint8 *Stream, wad_header *Objects, size_t Count)
{
	uint8* S = Stream;
	wad_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->version);
		ValueToStream(S,ObjPtr->data_version);
		BytesToStream(S,ObjPtr->file_name,MAXIMUM_WADFILE_NAME_LENGTH);
		ValueToStream(S,ObjPtr->checksum);
		ValueToStream(S,ObjPtr->directory_offset);
		ValueToStream(S,ObjPtr->wad_count);
		ValueToStream(S,ObjPtr->application_specific_directory_data_size);
		ValueToStream(S,ObjPtr->entry_header_size);
		ValueToStream(S,ObjPtr->directory_entry_base_size);
		ValueToStream(S,ObjPtr->parent_checksum);
		S += 2*20;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_wad_header));
	return S;
}


static uint8 *unpack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, size_t Count)
{
	uint8* S = Stream;
	old_directory_entry* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->offset_to_start);
		StreamToValue(S,ObjPtr->length);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_directory_entry));
	return S;
}

static uint8 *pack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, size_t Count)
{
	uint8* S = Stream;
	old_directory_entry* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->offset_to_start);
		ValueToStream(S,ObjPtr->length);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_directory_entry));
	return S;
}


static uint8 *unpack_directory_entry(uint8 *Stream, directory_entry *Objects, size_t Count)
{
	uint8* S = Stream;
	directory_entry* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->offset_to_start);
		StreamToValue(S,ObjPtr->length);
		StreamToValue(S,ObjPtr->index);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_directory_entry));
	return S;
}

static uint8 *pack_directory_entry(uint8 *Stream, directory_entry *Objects, size_t Count)
{
	uint8* S = Stream;
	directory_entry* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->offset_to_start);
		ValueToStream(S,ObjPtr->length);
		ValueToStream(S,ObjPtr->index);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_directory_entry));
	return S;
}


#if 0
static uint8 *unpack_old_entry_header(uint8 *Stream, old_entry_header *Objects, size_t Count)
{
	uint8* S = Stream;
	old_entry_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->tag);
		StreamToValue(S,ObjPtr->next_offset);
		StreamToValue(S,ObjPtr->length);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_entry_header));
	return S;
}
#endif

static uint8 *pack_old_entry_header(uint8 *Stream, old_entry_header *Objects, size_t Count)
{
	uint8* S = Stream;
	old_entry_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->tag);
		ValueToStream(S,ObjPtr->next_offset);
		ValueToStream(S,ObjPtr->length);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_entry_header));
	return S;
}


static uint8 *unpack_entry_header(uint8 *Stream, entry_header *Objects, size_t Count)
{
	uint8* S = Stream;
	entry_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->tag);
		StreamToValue(S,ObjPtr->next_offset);
		StreamToValue(S,ObjPtr->length);
		StreamToValue(S,ObjPtr->offset);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_entry_header));
	return S;
}

static uint8 *pack_entry_header(uint8 *Stream, entry_header *Objects, size_t Count)
{
	uint8* S = Stream;
	entry_header* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->tag);
		ValueToStream(S,ObjPtr->next_offset);
		ValueToStream(S,ObjPtr->length);
		ValueToStream(S,ObjPtr->offset);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_entry_header));
	return S;
}

void SetBetweenlevels(bool _BetweenLevels) {BetweenLevels = _BetweenLevels;}
bool IsBetweenLevels() {return BetweenLevels;}

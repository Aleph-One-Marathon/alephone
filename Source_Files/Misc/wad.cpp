/*
	WAD.C
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
¥ÊReentrancy
¥ÊUse malloc to actually make it possible to read the data as changes are made in the future?

	Saturday, October 28, 1995 1:13:38 PM- whoops.  Had a huge memory leak in the inflate wad
		data.  IÕm fired.
	
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
*/

// Note that level_transition_malloc is specific to marathon...

#include "cseries.h"
#include "byte_swapping.h"

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
#ifdef mac
inline short memory_error() {return MemError();}
#else
inline short memory_error() {return 0;}
#endif

#ifdef env68k
#pragma segment file_io
#endif

/* ---------------- private structures */
// LP: no more union wads
/*
struct wad_internal_data {
	short file_count;
	FileDesc files[MAXIMUM_UNION_WADFILES];
};
*/

/* ---------------- definitions for byte-swapping */
static _bs_field _bs_wad_header[] = { // 128 bytes
	_2byte, _2byte, MAXIMUM_WADFILE_NAME_LENGTH, _4byte, _4byte,
	_2byte, _2byte, _2byte, _2byte, _4byte, 20*sizeof(int16)
};

static _bs_field _bs_directory_entry[] = { // 10 bytes
	_4byte, _4byte, _2byte
};

static _bs_field _bs_entry_header[] = { // 16 bytes
	_4byte, _4byte, _4byte, _4byte
};

/* ---------------- private global data */
struct wad_internal_data *internal_data[MAXIMUM_OPEN_WADFILES]= {NULL, NULL, NULL};

/* ---------------- private prototypes */
static long calculate_directory_offset(struct wad_header *header, short index);
static void dump_raw_wad(byte *wad);
static short get_directory_base_length(struct wad_header *header);
static short get_entry_header_length(struct wad_header *header);
static bool read_indexed_directory_data(OpenedFile& OFile, struct wad_header *header,
	short index, struct directory_entry *entry);
static long calculate_raw_wad_length(struct wad_header *file_header, byte *wad);
static bool read_indexed_wad_from_file_into_buffer(OpenedFile& OFile, 
	struct wad_header *header, short index, void *buffer, long *length);
static short count_raw_tags(byte *raw_wad);
static struct wad_data *convert_wad_from_raw(struct wad_header *header, byte *data,	long wad_start_offset,
	long raw_length);
static struct wad_data *convert_wad_from_raw_modifiable(struct wad_header *header, byte *raw_wad, long raw_length);
static void patch_wad_from_raw(struct wad_header *header, byte *raw_wad, struct wad_data *read_wad);
static void patch_wad_from_raw(struct wad_header *header, byte *raw_wad, struct wad_data *read_wad);
static bool size_of_indexed_wad(OpenedFile& OFile, struct wad_header *header, short index, 
	long *length);

static bool write_to_file(OpenedFile& OFile, long offset, void *data, long length);
static bool read_from_file(OpenedFile& OFile, long offset, void *data, long length);

// LP: routines for packing and unpacking the data from streams of bytes
static uint8 *unpack_wad_header(uint8 *Stream, wad_header *Objects, int Count = 1);
static uint8 *pack_wad_header(uint8 *Stream, wad_header *Objects, int Count = 1);
static uint8 *unpack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, int Count = 1);
static uint8 *pack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, int Count = 1);
static uint8 *unpack_directory_entry(uint8 *Stream, directory_entry *Objects, int Count = 1);
static uint8 *pack_directory_entry(uint8 *Stream, directory_entry *Objects, int Count = 1);
static uint8 *unpack_old_entry_header(uint8 *Stream, old_entry_header *Objects, int Count = 1);
static uint8 *pack_old_entry_header(uint8 *Stream, old_entry_header *Objects, int Count = 1);
static uint8 *unpack_entry_header(uint8 *Stream, entry_header *Objects, int Count = 1);
static uint8 *pack_entry_header(uint8 *Stream, entry_header *Objects, int Count = 1);

/* ------------------ Code Begins */

boolean read_wad_header(
	OpenedFile& OFile, 
	struct wad_header *header)
{
	boolean union_file= FALSE;
	int error = 0;
	boolean success= TRUE;
	
	uint8 buffer[SIZEOF_wad_header];
	read_from_file(OFile, 0, buffer, SIZEOF_wad_header);
	unpack_wad_header(buffer,header);
	/*
	read_from_file(OFile, 0, header, sizeof(wad_header));
	byte_swap_object(*header, _bs_wad_header);
	*/
	
	if(error)
	{
		set_game_error(systemError, error);
		success= FALSE;
	} else {
		if(header->version>CURRENT_WADFILE_VERSION)
		{
			set_game_error(gameError, errUnknownWadVersion);
			success= FALSE;
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
	boolean read_only)
{
	struct wad_data *read_wad= (struct wad_data *) NULL;
	byte *raw_wad;
	long length;
	int error = 0;

	// if(file_id>=0) /* NOT a union wadfile... */
	{
		if (size_of_indexed_wad(OFile, header, index, &length))
		{
#if 0
			raw_wad= (byte *) malloc(length);
#else
			raw_wad= (byte *) level_transition_malloc(length);
#endif
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

						/* Free it! */
						free(raw_wad);
					}

					if(!read_wad)
					{
						/* Error.. */
						error= memory_error();
					}
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
	long *length)
{
	void *return_value= NULL;
	short index;
	
	*length= 0l;
	
	assert(wad);
	for(index= 0; index<wad->tag_count; ++index)
	{
		if(wad->tag_data[index].tag==type)
		{
			return_value= wad->tag_data[index].data;
			*length= wad->tag_data[index].length;
			break;
		}
	}
	
	return return_value;
}

boolean wad_file_has_checksum(
	FileSpecifier& File, 
	unsigned long checksum)
{
	boolean has_checksum= FALSE;

	if(checksum==read_wad_file_checksum(File))
	{
		has_checksum= TRUE;
	}
	
	return has_checksum;
}

unsigned long read_wad_file_checksum(FileSpecifier& File)
{
	struct wad_header header;
	unsigned long checksum= 0l;
	
	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	// file_id= open_wad_file_for_reading(file);
	// if(file_id>=0)
	{
		// if(read_wad_header(file_id, &header))
		if(read_wad_header(OFile, &header))
		{
			checksum= header.checksum;
		}
		
		close_wad_file(OFile);
		// close_wad_file(file_id);
	}
	
	return checksum;
}

unsigned long read_wad_file_parent_checksum(FileSpecifier& File)
{
	// fileref file_id;
	struct wad_header header;
	unsigned long checksum= 0l;

	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	// file_id= open_wad_file_for_reading(file);
	// if(file_id>=0)
	{
		if(read_wad_header(OFile, &header))
		// if(read_wad_header(file_id, &header))
		{
			checksum= header.parent_checksum;
		}
		
		close_wad_file(OFile);
		// close_wad_file(file_id);
	}
	
	return checksum;
}

boolean wad_file_has_parent_checksum(
	FileSpecifier& File, 
	unsigned long parent_checksum)
{
	// fileref file_id;
	boolean has_checksum= FALSE;
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
				has_checksum= TRUE;
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
#ifdef mac
	// LP: being sure to create a Pascal-format filename
	char Name[256];
	File.GetName(Name);
	header->file_name[0] = strlen(Name);
	strncpy(header->file_name+1,Name,MAXIMUM_WADFILE_NAME_LENGTH-1);
	p2cstr((unsigned char *)header->file_name);
#else
	File.GetLastPart(header->file_name);
#endif
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
	/* unsigned long checksum, long directory_offset, unsigned long parent_checksum */
}

boolean write_wad_header(
	OpenedFile& OFile, 
	struct wad_header *header)
{
	boolean success= TRUE;

	uint8 buffer[SIZEOF_wad_header];
	pack_wad_header(buffer,header);
	write_to_file(OFile, 0, buffer, SIZEOF_wad_header);
	/*	
	wad_header tmp = *header;
	byte_swap_object(tmp, _bs_wad_header);
	write_to_file(OFile, 0, &tmp, sizeof(wad_header));
	*/

	return success;
}

// Takes raw, unswapped directory data
boolean write_directorys(
	OpenedFile& OFile, 
	struct wad_header *header,
	void *entries)
{
	long size_to_write= get_size_of_directory_data(header);
	boolean success= TRUE;
	
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	write_to_file(OFile, header->directory_offset, entries, 
		size_to_write);

	return success;
}

/* Note wad_count better be correct! */
long get_size_of_directory_data(
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
	// LP: changed "char *" to "byte *"
	byte *data_ptr= (byte *)directories;
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
	long offset,
	long length,
	short wad_index)
{
	byte *data_ptr= (byte *)entries;
	struct directory_entry *entry;
	long data_offset;

	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	
	/* calculate_directory_offset is for the file, by subtracting the base, we get the actual offset.. */
	data_offset= calculate_directory_offset(header, index) - header->directory_offset;

	data_ptr+= data_offset;
	entry= (struct directory_entry *) data_ptr;
	
	entry->length= length;
	entry->offset_to_start= offset;
	
	if(header->version>=WADFILE_SUPPORTS_OVERLAYS)
	{
		entry->index= wad_index;
	}
	
	// LP: should be correct for packing also
	if (header->version>=WADFILE_SUPPORTS_OVERLAYS)
	{
		uint8 buffer[SIZEOF_old_directory_entry];
		memcpy(buffer,data_ptr,SIZEOF_old_directory_entry);
		unpack_old_directory_entry(buffer,(old_directory_entry *)data_ptr);
	}
	else
	{
		uint8 buffer[SIZEOF_directory_entry];
		memcpy(buffer,data_ptr,SIZEOF_directory_entry);
		unpack_directory_entry(buffer,(directory_entry *)data_ptr);
	}
	/*
	byte_swap_data(data_ptr, header->version>=WADFILE_SUPPORTS_OVERLAYS ? SIZEOF_old_directory_entry : SIZEOF_directory_entry, 1, _bs_directory_entry);
	*/
}

// Returns raw, unswapped directory data
void *read_directory_data(
	OpenedFile& OFile,
	struct wad_header *header)
{
	long size;
	byte *data;
	
	assert(header->version>=WADFILE_HAS_DIRECTORY_ENTRY);
	
	size= get_size_of_directory_data(header);
	data= (byte *)malloc(size);
	if(data)
	{
		read_from_file(OFile, header->directory_offset, data, size);
	}

	return data;
}

struct wad_data *append_data_to_wad(
	struct wad_data *wad, 
	WadDataType type, 
	void *data, 
	long size,
	long offset) /* Allows for inplace creation of wadfiles */
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
			alert_user(fatalError, strERRORS, outOfMemory, memory_error());
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
	wad->tag_data[index].data= (byte *) malloc(size);
	if(!wad->tag_data[index].data)
	{
		alert_user(fatalError, strERRORS, outOfMemory, memory_error());
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
			alert_user(fatalError, strERRORS, outOfMemory, memory_error());
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

	return;
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

boolean write_wad(
	OpenedFile& OFile, 
	struct wad_header *file_header,
	struct wad_data *wad, 
	long offset)
{
	int error = 0;
	boolean success;
	short entry_header_length= get_entry_header_length(file_header);
	short index;
	struct entry_header header;
	long running_offset= 0l;

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
			pack_old_entry_header(buffer,(old_entry_header *)&header);
			break;
		case SIZEOF_entry_header:
			pack_entry_header(buffer,&header);
			break;
		default:
			vassert(false,csprintf(temporary,"Unrecognized entry-header length: %d",entry_header_length));
		}
		if (write_to_file(OFile, offset, buffer, entry_header_length));
		/*
		byte_swap_data(&header, entry_header_length, 1, _bs_entry_header);
		if (write_to_file(OFile, offset, &header, entry_header_length))
		*/
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
		success= FALSE;
		set_game_error(systemError, error);
	} else {
		success= TRUE;
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

long calculate_wad_length(
	struct wad_header *file_header, 
	struct wad_data *wad)
{
	short ii;
	short header_length= get_entry_header_length(file_header);
	long running_length= 0l;

	for(ii= 0; ii<wad->tag_count; ++ii)
	{
		running_length += wad->tag_data[ii].length + header_length;
	}
	
	return running_length;
}

/* ------------ Transfer type functions */
#define CURRENT_FLAT_MAGIC_COOKIE (0xDEADDEAD)
struct encapsulated_wad_data {
	long magic_cookie;			/* Simple version control */
	long length;				/* Length, including everything.. */
	struct wad_header header;	/* The 128 byte header */
	/* Flat wad.. */
};

void *get_flat_data(
	FileSpecifier& File, 
	boolean use_union, 
	short wad_index)
{
	struct wad_header header;
	boolean success= FALSE;
	struct encapsulated_wad_data *data= NULL;
	
	assert(!use_union);
	
	OpenedFile OFile;
	if (open_wad_file_for_reading(File,OFile))
	{
		/* Read the file */
		success= read_wad_header(OFile, &header);

		if (success)
		{
			long length;
			int error = 0;
		
			/* Allocate the conglomerate data.. */
			if (size_of_indexed_wad(OFile, &header, wad_index, &length))
			{
				data= (struct encapsulated_wad_data *) malloc(length+sizeof(struct encapsulated_wad_data));
				if(data)
				{
					byte *buffer= ((byte *) data)+sizeof(struct encapsulated_wad_data);
				
					data->magic_cookie= CURRENT_FLAT_MAGIC_COOKIE;
					data->length= length+sizeof(struct encapsulated_wad_data);
					obj_copy(data->header, header);

					/* Read into our buffer... */
					error= read_indexed_wad_from_file_into_buffer(OFile, &header, wad_index, 
						buffer, &length);

					if(error)
					{
						/* Error-> didn't get it.. */
						free(data);
						data= (struct encapsulated_wad_data *) NULL;
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

long get_flat_data_length(
	void *data)
{
	struct encapsulated_wad_data *d= (struct encapsulated_wad_data *) data;
	return d->length;
}

/* This is how you dispose of it-> you inflate it, then use free_wad() */
struct wad_data *inflate_flat_data(
	void *data, 
	struct wad_header *header)
{
	struct encapsulated_wad_data *d= (struct encapsulated_wad_data *) data;
	struct wad_data *wad= NULL;
	byte *buffer= ((byte *) data)+sizeof(struct encapsulated_wad_data);
	long raw_length;

	assert(data);
	assert(header);
	assert(d->magic_cookie==CURRENT_FLAT_MAGIC_COOKIE);

	obj_copy(*header, (d->header));

	raw_length= calculate_raw_wad_length(header, buffer);
	assert(raw_length==d->length-sizeof(struct encapsulated_wad_data));
	
	/* Now inflate.. */
	wad= convert_wad_from_raw(header, (byte *)data, sizeof(struct encapsulated_wad_data), raw_length);
	
	return wad;
}

/* ---------- debugging routines. */
void dump_wad(
	struct wad_data *wad)
{
	short index;
	struct tag_data *tag= wad->tag_data;

	dprintf("---Dumping---;g");
	dprintf("Tag Count: %d;g", wad->tag_count);
	for(index= 0; index<wad->tag_count; ++index)
	{
		assert(tag);
		dprintf("Tag: %x data: %x length: %d offset: %d;g", tag->tag, tag->data, tag->length,
			tag->offset);
		tag++;
	}
	dprintf("---End of Dump---");
}

/* ---------- file management routines */
bool create_wadfile(FileSpecifier& File, int Type)
{
	return File.Create(Type);
}

bool open_wad_file_for_reading(FileSpecifier& File, OpenedFile& OFile)
{
	return File.Open(OFile);
}

bool open_wad_file_for_writing(FileSpecifier& File, OpenedFile& OFile)
{
	return File.Open(OFile,true);
}

void close_wad_file(OpenedFile& File)
{
	File.Close();
#if 0
	if(file_id>=0) /* It is a real file descriptor */
	{
		close_file(file_id);
	} else { /* It is the index of the internal data -1 */
		// LP: suppressing union-wad stuff
		// free_internal_data_for_file_id(file_id);
	}
#endif
	
	return;
}

/* ------------------------------ Private Code --------------- */
static bool size_of_indexed_wad(
	OpenedFile& OFile, 
	struct wad_header *header, 
	short index, 
	long *length)
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

static long calculate_directory_offset(
	struct wad_header *header, 
	short index)
{
	long offset;
	long unit_size;
	long additional_offset;

	switch(header->version)
	{
		case PRE_ENTRY_POINT_WADFILE_VERSION:
			assert(header->application_specific_directory_data_size==0);
		case WADFILE_HAS_DIRECTORY_ENTRY:
		case WADFILE_SUPPORTS_OVERLAYS:
		// LP addition:
		case WADFILE_HAS_INFINITY_STUFF:
			assert(header->application_specific_directory_data_size>=0);
			unit_size= header->application_specific_directory_data_size+get_directory_base_length(header);
			additional_offset= header->application_specific_directory_data_size;
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
	long offset;
	int error = 0;

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
		error= read_from_file(OFile, offset, buffer, base_entry_size);
		switch (base_entry_size)
		{
		case SIZEOF_old_directory_entry:
			unpack_old_directory_entry(buffer,(old_directory_entry *)entry);
			break;
		case SIZEOF_directory_entry:
			unpack_directory_entry(buffer,entry);
			break;
		default:
			vassert(false,csprintf(temporary,"Unrecognized base-entry length: %d",base_entry_size));
		}
		/*
		error= read_from_file(OFile, offset, entry, base_entry_size);
		byte_swap_data(entry, base_entry_size, 1, _bs_directory_entry);
		*/
	} else {
		short directory_index;

		/* Pin it, so we can try to read future file formats */
		if(base_entry_size>SIZEOF_directory_entry) 
		{
			base_entry_size= SIZEOF_directory_entry;
		}
	
		/* We have to loop.. */
		for(directory_index= 0; !error && directory_index<header->wad_count; ++directory_index)
		{
			/* We use a hint, that the index is the real index, to help make this have */
			/* a ÒhitÓ on the first try */
			short test_index= (index+directory_index)%header->wad_count;
		
			/* Calculate the offset */
			offset= calculate_directory_offset(header, test_index);

			/* Read it.. */
			uint8 buffer[MAX(SIZEOF_old_directory_entry,SIZEOF_directory_entry)];
			error= read_from_file(OFile, offset, buffer, base_entry_size);
			switch (base_entry_size)
			{
			case SIZEOF_old_directory_entry:
				unpack_old_directory_entry(buffer,(old_directory_entry *)entry);
				break;
			case SIZEOF_directory_entry:
				unpack_directory_entry(buffer,entry);
				break;
			default:
				vassert(false,csprintf(temporary,"Unrecognized base-entry length: %d",base_entry_size));
			}
			/*
			error= read_from_file(OFile, offset, entry, base_entry_size);
			byte_swap_data(entry, base_entry_size, 1, _bs_directory_entry);
			*/
			if(entry->index==index) 
			{
				break; /* Got it! */
			}
		}
	}

	/* File error has precedence. */
	return error;
}

/* Internal function.. */
static bool read_indexed_wad_from_file_into_buffer(
	OpenedFile& OFile, 
	struct wad_header *header, 
	short index,
	void *buffer,
	long *length) /* Length of maximum buffer on entry, actual length on return */
{
	struct directory_entry entry;
	int error = 0;

	/* Read the directory entry first */
	if (read_indexed_directory_data(OFile, header, index, &entry))
	{
		/* Check the index stored in the entry, and assert if they don't match! */
		assert((get_directory_base_length(header)==sizeof(old_directory_entry)) || entry.index==index);
		assert(*length<=entry.length);
		assert(buffer);

		/* Read into it. */
		error= read_from_file(OFile, entry.offset_to_start, buffer, entry.length);

		/* Set the length */
		*length= entry.length;

		/* Veracity Check */
		/* ! an error, it has a length non-zero and calculated != actual */
		assert(entry.length==calculate_raw_wad_length(header, (byte *)buffer));
	}
	
	return error;
}

/* This *MUST* be a base wad.. */
static struct wad_data *convert_wad_from_raw(
	struct wad_header *header, 
	byte *data,
	long wad_start_offset,
	long raw_length)
{
	struct wad_data *wad;
	byte *raw_wad;

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
				struct entry_header *wad_entry_header;
				short index;
				short entry_header_size;
			
				/* Clear it */
				objlist_clear(wad->tag_data, tag_count);
				
				entry_header_size= get_entry_header_length(header);
				wad_entry_header= (struct entry_header *) raw_wad;

				/* Note that this is a read only wad.. */	
				wad->read_only_data= data;
	
				for(index= 0; index<tag_count; ++index)
				{
					assert(header->version<WADFILE_SUPPORTS_OVERLAYS || SDL_SwapBE32(wad_entry_header->offset)==0l);
					wad->tag_data[index].tag= SDL_SwapBE32(wad_entry_header->tag);
					wad->tag_data[index].length= SDL_SwapBE32(wad_entry_header->length);
					wad->tag_data[index].offset= 0l;
					wad->tag_data[index].data= ((byte *) wad_entry_header)+entry_header_size;
					wad_entry_header= (struct entry_header *) (raw_wad + SDL_SwapBE32(wad_entry_header->next_offset));
#ifdef OBSOLETE					
					if(wad->tag_data[index].data)
					{
						/* This MUST be a base! */
						assert(header->version<WADFILE_SUPPORTS_OVERLAYS || wad_entry_header->offset==0l);
		
						/* Copy the data.. */
						memcpy(wad->tag_data[index].data, 
							(((byte *) wad_entry_header)+entry_header_size), 
							wad_entry_header->length);
						wad_entry_header= (struct entry_header *) (raw_wad + wad_entry_header->next_offset);
					} else {
						alert_user(fatalError, strERRORS, outOfMemory, memory_error());
					}
#endif
				} 
			} else {
				alert_user(fatalError, strERRORS, outOfMemory, memory_error());
			}
		}
	}
	
	return wad;
}

/* This *MUST* be a base wad.. */
static struct wad_data *convert_wad_from_raw_modifiable(
	struct wad_header *header, 
	byte *raw_wad,
	long raw_length)
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
				struct entry_header *wad_entry_header;
				short index;
				short entry_header_size;
			
				/* Clear it */
				objlist_clear(wad->tag_data, tag_count);
				
				entry_header_size= get_entry_header_length(header);
				wad_entry_header= (struct entry_header *) raw_wad;
	
				for(index= 0; index<tag_count; ++index)
				{
					wad->tag_data[index].tag= SDL_SwapBE32(wad_entry_header->tag);
					wad->tag_data[index].length= SDL_SwapBE32(wad_entry_header->length);
					wad->tag_data[index].data= (byte *) malloc(wad->tag_data[index].length);
					if(!wad->tag_data[index].data)
					{
						alert_user(fatalError, strERRORS, outOfMemory, memory_error());
					}
					wad->tag_data[index].offset= 0l;
					
					/* This MUST be a base! */
					assert(header->version<WADFILE_SUPPORTS_OVERLAYS || SDL_SwapBE32(wad_entry_header->offset)==0l);
	
					/* Copy the data.. */
					memcpy(wad->tag_data[index].data, 
						(((byte *) wad_entry_header)+entry_header_size), 
						wad->tag_data[index].length);
					wad_entry_header= (struct entry_header *) (raw_wad + SDL_SwapBE32(wad_entry_header->next_offset));
				} 
			}
		}
	}
	
	return wad;
}

static short count_raw_tags(
	byte *raw_wad)
{
	int tag_count = 0;
	entry_header *header = (entry_header *)raw_wad;
	while (true) {
		tag_count++;
		uint32 next_offset = SDL_SwapBE32(header->next_offset);
		if (next_offset == 0)
			break;
		header = (entry_header *)((uint8 *)raw_wad + next_offset);
	}

	return tag_count;
}

/* Patch it! */
static void patch_wad_from_raw(
	struct wad_header *header, 
	byte *raw_wad, 
	struct wad_data *read_wad)
{
	short tag_count;
	struct entry_header *wad_entry_header;
	short index;
	short entry_header_size= get_entry_header_length(header);

#ifdef SDL
printf("+++ patch_wad_from_raw\n");	//!!
abort();
#endif
	
	/* Count the tags */
	tag_count= count_raw_tags(raw_wad);

	/* On patch file.. */
	assert(header->version>= WADFILE_SUPPORTS_OVERLAYS);
	wad_entry_header= (struct entry_header *) raw_wad;
	for(index= 0; index<tag_count; ++index)
	{
		short actual_index;
		
		for(actual_index=0; actual_index<read_wad->tag_count; ++actual_index)
		{
			if(read_wad->tag_data[actual_index].tag==wad_entry_header->tag)
			{
				/* MATCH! */
				assert(wad_entry_header->offset+wad_entry_header->length<=
					read_wad->tag_data[actual_index].length);
				
				/* Copy it in.. */
				memcpy(read_wad->tag_data[actual_index].data+wad_entry_header->offset,
					(((byte *) wad_entry_header)+entry_header_size),
					wad_entry_header->length);
				
				break;
			}
		}

		if(actual_index==read_wad->tag_count)
		{
			/* This is an overload of a tag that wasn't previously existant for this level... */
			dprintf("New tags are not supported!");
		}
		
		wad_entry_header= (struct entry_header *) (raw_wad + wad_entry_header->next_offset);
	} 
}

static long calculate_raw_wad_length(
	struct wad_header *file_header,
	byte *wad)
{
	int entry_header_size = get_entry_header_length(file_header);

	long length = 0;
	entry_header *header = (entry_header *)wad;
	while (true) {
		length += SDL_SwapBE32(header->length) + entry_header_size;
		uint32 next_offset = SDL_SwapBE32(header->next_offset);
		if (next_offset == 0)
			break;
		header = (entry_header *)((uint8 *)wad + next_offset);
	}

	return length;
}

/* -------- debug privates */
static void dump_raw_wad(
	byte *wad)
{
	struct entry_header *header;
	long offset;
	short tag_count;

	assert(wad);
	
	offset= tag_count= 0;
	header= (struct entry_header *) wad;
	
	while(header->next_offset)
	{
		offset = header->next_offset;
		dprintf("%d Tag: %x Length: %d Next Offset: %d", tag_count, header->tag, header->length, header->next_offset);
		tag_count++;
		header= (struct entry_header *) (((byte *) wad) + offset);
	}
	dprintf("%d Tag: %x Length: %d Next Offset: %d", tag_count, header->tag, header->length, header->next_offset);
}

// LP: won't bother to convert the union-wad stuff file handling
#if 0
static void dump_union_ref(
	fileref union_ref)
{
	struct wad_internal_data *data= get_internal_data_for_file_id(union_ref);
	short ii;
	
	assert(data);

	if(data->file_count>1)
	{
		dprintf("Dump for union ref: %d", union_ref);
		for(ii= 0; ii<data->file_count; ++ii)
		{
			dprintf("File %d VRef: %d parID: %d Name: %.*s", ii, data->files[ii].vRefNum,
				data->files[ii].parID, data->files[ii].name[0], data->files[ii].name+1);
		}
		dprintf("End Dump for union ref: %d", union_ref);
	}
}

/* -------- union static functions */
static struct wad_internal_data *get_internal_data_for_file_id(
	fileref file_id)
{
	short actual_index= (-file_id)-1;

	assert(actual_index>=0 && actual_index<MAXIMUM_OPEN_WADFILES);
	return internal_data[(-file_id)-1];
}

static struct wad_internal_data *allocate_internal_data_for_file_id(
	fileref file_id)
{
	short actual_index= (-file_id)-1;

	assert(actual_index>=0 && actual_index<MAXIMUM_OPEN_WADFILES);

	/* Make sure it isn't already allocated */
	assert(!internal_data[actual_index]);

	internal_data[actual_index]= (struct wad_internal_data *)malloc(sizeof(struct wad_internal_data));
	if(!internal_data[actual_index]) alert_user(fatalError, strERRORS, outOfMemory, memory_error());

	/* If we got it.. */
	if(internal_data[actual_index])
		internal_data[actual_index]->file_count= 0;
	
	return internal_data[actual_index];
}

/* Not static because mac_wad.c needs it. (gross hack) */
void free_internal_data_for_file_id(
	fileref file_id)
{
	short actual_index= (-file_id)-1;

	assert(actual_index>=0 && actual_index<MAXIMUM_OPEN_WADFILES);

	/* Make sure it is already allocated */
	assert(internal_data[actual_index]);
	free(internal_data[actual_index]);
	internal_data[actual_index]= NULL;
}

static fileref find_available_union_refnum(
	void)
{
	short i;
	fileref ref;
	
	for(i= 0; i<MAXIMUM_OPEN_WADFILES; ++i) 
	{
		if(!internal_data[i]) break;
	}
	
	if(i==MAXIMUM_OPEN_WADFILES) 
	{
		/* No more files! */
		ref = FILEREF_NONE;
	} else {
		/* Got one! */
		ref= -1 - i;
	}
	
	return ref;
}
#endif

static bool write_to_file(
	OpenedFile& OFile, 
	// fileref file_id, 
	long offset, 
	void *data, 
	long length)
{
	if (!OFile.SetPosition(offset)) return false;
	return OFile.Write(length, data);

	/*
	FileError err;

	assert(file_id != FILEREF_NONE);
	err= set_fpos(file_id, offset);
	if (!err)
	{
		err= write_file(file_id, length,  data);
		assert(!err);
	}
	
	return err;
	*/
}

static bool read_from_file(
	OpenedFile& OFile, 
	// fileref file_id, 
	long offset, 
	void *data, 
	long length)
{
	if (!OFile.SetPosition(offset)) return false;
	return OFile.Read(length, data);

	/*
	FileError err;

	assert(file_id != FILEREF_NONE);
	err= set_fpos(file_id, offset);
	if (!err)
	{
		err= read_file(file_id, length, data);
	}
	
	return err;
	*/
}

static uint8 *unpack_wad_header(uint8 *Stream, wad_header *Objects, int Count)
{
	uint8* S = Stream;
	wad_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
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
	
	assert((S - Stream) == Count*SIZEOF_wad_header);
	return S;
}

static uint8 *pack_wad_header(uint8 *Stream, wad_header *Objects, int Count)
{
	uint8* S = Stream;
	wad_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
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
	
	assert((S - Stream) == Count*SIZEOF_wad_header);
	return S;
}


static uint8 *unpack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, int Count)
{
	uint8* S = Stream;
	old_directory_entry* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->offset_to_start);
		StreamToValue(S,ObjPtr->length);
	}
	
	assert((S - Stream) == Count*SIZEOF_old_directory_entry);
	return S;
}

static uint8 *pack_old_directory_entry(uint8 *Stream, old_directory_entry *Objects, int Count)
{
	uint8* S = Stream;
	old_directory_entry* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->offset_to_start);
		ValueToStream(S,ObjPtr->length);
	}
	
	assert((S - Stream) == Count*SIZEOF_old_directory_entry);
	return S;
}


static uint8 *unpack_directory_entry(uint8 *Stream, directory_entry *Objects, int Count)
{
	uint8* S = Stream;
	directory_entry* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->offset_to_start);
		StreamToValue(S,ObjPtr->length);
		StreamToValue(S,ObjPtr->index);
	}
	
	assert((S - Stream) == Count*SIZEOF_directory_entry);
	return S;
}

static uint8 *pack_directory_entry(uint8 *Stream, directory_entry *Objects, int Count)
{
	uint8* S = Stream;
	directory_entry* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->offset_to_start);
		ValueToStream(S,ObjPtr->length);
		ValueToStream(S,ObjPtr->index);
	}
	
	assert((S - Stream) == Count*SIZEOF_directory_entry);
	return S;
}


static uint8 *unpack_old_entry_header(uint8 *Stream, old_entry_header *Objects, int Count)
{
	uint8* S = Stream;
	old_entry_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->tag);
		StreamToValue(S,ObjPtr->next_offset);
		StreamToValue(S,ObjPtr->length);
	}
	
	assert((S - Stream) == Count*SIZEOF_old_entry_header);
	return S;
}

static uint8 *pack_old_entry_header(uint8 *Stream, old_entry_header *Objects, int Count)
{
	uint8* S = Stream;
	old_entry_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->tag);
		ValueToStream(S,ObjPtr->next_offset);
		ValueToStream(S,ObjPtr->length);
	}
	
	assert((S - Stream) == Count*SIZEOF_old_entry_header);
	return S;
}


static uint8 *unpack_entry_header(uint8 *Stream, entry_header *Objects, int Count)
{
	uint8* S = Stream;
	entry_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->tag);
		StreamToValue(S,ObjPtr->next_offset);
		StreamToValue(S,ObjPtr->length);
		StreamToValue(S,ObjPtr->offset);
	}
	
	assert((S - Stream) == Count*SIZEOF_entry_header);
	return S;
}

static uint8 *pack_entry_header(uint8 *Stream, entry_header *Objects, int Count)
{
	uint8* S = Stream;
	entry_header* ObjPtr = Objects;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->tag);
		ValueToStream(S,ObjPtr->next_offset);
		ValueToStream(S,ObjPtr->length);
		ValueToStream(S,ObjPtr->offset);
	}
	
	assert((S - Stream) == Count*SIZEOF_entry_header);
	return S;
}

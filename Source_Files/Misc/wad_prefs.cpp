/*

	 wad_prefs.c
	 Wednesday, March 22, 1995 12:08:47 AM- rdm created.
	 	
Jan 30, 2000 (Loren Petrich)
	Did some typecasts

Mar 5, 2000 (Loren Petrich)
	Added more graceful degradation on prefs-wad read-in in load_preferences()

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

#include "cseries.h"
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

#include "map.h"
#include "shell.h"
#include "wad.h"
#include "game_errors.h"

#include "wad_prefs.h"

#include "FileHandler.h"

#ifdef env68k
	#pragma segment file_io
#endif

/* ------ local defines */
#define CURRENT_PREF_WADFILE_VERSION 0

/* ------ local globals */
struct preferences_info *prefInfo= NULL;

/* ------ local prototypes */
static void load_preferences(void);

// LP: fake portable-files stuff
#ifdef mac
inline short memory_error() {return MemError();}
#else
inline short memory_error() {return 0;}
#endif

/* ------------ Entry point */
/* Open the file, and allocate whatever internal structures are necessary in the */
/*  preferences pointer.. */
boolean w_open_preferences_file(
	char *PrefName,
	int Type)
{
	int error = 0;
	boolean success= TRUE;

	/* allocate space for our global structure to keep track of the prefs file */
	prefInfo = NULL;
	try {
		prefInfo = new preferences_info;

#if defined(mac)
		prefInfo->PrefsFile.SetParentToPreferences();
		prefInfo->PrefsFile.SetName(PrefName,Type);
		/* check for the preferences folder using FindFolder, creating it if necessary */
#elif defined(SDL)
		prefInfo->PrefsFile = local_data_dir;
		prefInfo->PrefsFile.AddPart(PrefName);
#endif

		/* does the preferences file exist? */
		load_preferences(); /* Uses prefInfo.. */
	
		if(error_pending())
		{
			short type;
		
			error= get_game_error(&type);
			if(type==systemError)
			{
				if (!prefInfo->PrefsFile.Exists())
				{
					prefInfo->PrefsFile.Create(Type);
					prefInfo->wad = create_empty_wad();
					set_game_error(systemError,prefInfo->PrefsFile.GetError());
					w_write_preferences_file();
				}
			} else {
				/* Something was invalid.. */
				if (prefInfo->PrefsFile.Delete())
				{
					prefInfo->wad= create_empty_wad();
					set_game_error(systemError, error);
					w_write_preferences_file();
				}
				set_game_error(systemError, errNone);
			}
		}
	} catch (...) {
		set_game_error(systemError, memory_error());
	}
	
	if (error)
	{
		/* if something is broken, make sure we at least return valid prefs */
		if(prefInfo && !prefInfo->wad) prefInfo->wad= create_empty_wad();
	}

	/* Gotta bail... */
	if(!prefInfo || !prefInfo->wad)
	{
		success= FALSE;
	}
	
// dump_wad(prefInfo->wad);
	
	return success;
}

/* Caller should assert this. */
void *w_get_data_from_preferences(
	WadDataType tag,					/* Tag to read */
	short expected_size,				/* Data size */
	prefs_initializer initialize,	/* Call if I have to allocate it.. */
	prefs_validater validate)	/* Verify function-> fixes if bad and returns TRUE */
{
	void *data;
	long length;
	
	assert(prefInfo->wad);
	
	data= extract_type_from_wad(prefInfo->wad, tag, &length);
	/* If we got the data, but it was the wrong size, or we didn't get the data... */
	if((data && length != expected_size) || (!data))
	{
		/* I have a copy of this pointer in the wad-> it's okay to do this */
		data= malloc(expected_size);
		if(data)
		{
			/* Initialize it */
			initialize(data);
			
			/* Append it to the file, for writing out.. */
			append_data_to_wad(prefInfo->wad, tag, data, expected_size, 0);
			
			/* Free our private pointer */
			free(data);
			
			/* Return the real one. */
			data= extract_type_from_wad(prefInfo->wad, tag, &length);
		}
	}
	
	if(data)
	{
		/* Only call the validation function if it is present. */
		if(validate && validate(data))
		{
			char *new_data;
			
			/* We can't hand append_data_to_wad a copy of the data pointer it */
			/* contains */
			new_data= (char *)malloc(expected_size);
			assert(new_data);
			
			memcpy(new_data, data, expected_size);
			
			/* Changed in the validation. Save to our wad. */
			append_data_to_wad(prefInfo->wad, tag, new_data, expected_size, 0);
	
			/* Free the new copy */
			free(new_data);
	
			/* And reextract the pointer.... */
			data= extract_type_from_wad(prefInfo->wad, tag, &length);
		}
	}
	
	return data;
}	

void w_write_preferences_file(
	void)
{
	struct wad_header header;
	OpenedFile OFile;

	/* We can be called atexit. */
	if(error_pending())
	{
		set_game_error(systemError, errNone);
	}
	
	assert(!error_pending());
	OpenedFile PrefsFile;
	if (open_wad_file_for_writing(prefInfo->PrefsFile,PrefsFile))
	{
		struct directory_entry entry;

		fill_default_wad_header(prefInfo->PrefsFile, 
			CURRENT_WADFILE_VERSION, CURRENT_PREF_WADFILE_VERSION, 
			1, 0l, &header);
			
		if (write_wad_header(PrefsFile, &header))
		{
			long wad_length;
			long offset= sizeof(struct wad_header);

			/* Length? */
			wad_length= calculate_wad_length(&header, prefInfo->wad);

			/* Set the entry data..... (and put into correct byte-order for writing) */
			set_indexed_directory_offset_and_length(&header, 
				&entry, 0, offset, wad_length, 0);
			
			/* Now write it.. */
			if (write_wad(PrefsFile, &header, prefInfo->wad, offset))
			{
				offset+= wad_length;
				header.directory_offset= offset;
				if (write_wad_header(PrefsFile, &header) &&
					write_directorys(PrefsFile, &header, &entry))
				{
					/* Success! */
				} else {
					assert(error_pending());
				}
			} 
			else {
				assert(error_pending());
			}

			/* Since we don't free it, it is opened.. */
		} else {
			assert(error_pending());
		}
		close_wad_file(PrefsFile);
	} 
	
	return;
}

static void load_preferences(
	void)
{
	/* If it was already allocated, we are reloading, so free the old one.. */
	if(prefInfo->wad)
	{	
		free_wad(prefInfo->wad);
		prefInfo->wad= NULL;
	}
	
	OpenedFile PrefsFile;
	if (open_wad_file_for_reading(prefInfo->PrefsFile,PrefsFile))
	{
		struct wad_header header;
	
		/* Read the header from the wad file */
		if(read_wad_header(PrefsFile, &header))
		{
			/* Read the indexed wad from the file */
			prefInfo->wad= read_indexed_wad_from_file(PrefsFile, &header, 0, FALSE);
			// LP change: more graceful degradation
			if (!prefInfo->wad) set_game_error(gameError, errUnknownWadVersion);
			// assert(prefInfo->wad);
		}
				
		/* Close the file.. */
		close_wad_file(PrefsFile);
	}
	
	return;
}

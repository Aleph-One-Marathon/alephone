/*

	macintosh_files.c
	Tuesday, August 29, 1995 3:18:54 PM- rdm created.

Feb 3, 2000 (Loren Petrich):
	Suppressed "dprintf" in open_file_for_reading() and open_file_for_writing(),
		since file-opening errors are handled without any need for
		debug interrupts.
*/

#include "macintosh_cseries.h"
#include <Aliases.h>
#include <folders.h>

#include "portable_files.h"
#include "game_errors.h"
#include "tags.h" // for APPLICATION_CREATOR

/* --------------- code! */
FileError create_file(
	FileDesc *file,
	unsigned long file_type) /* Should be OSType, or extension for dos. */
{
	FileError err;

	/* Assume that they already confirmed this is goign to die.. */
	err= FSpDelete((FSSpec *) file);

	if(!err || err==fnfErr)
	{
		err= FSpCreate((FSSpec *) file, APPLICATION_CREATOR, file_type, smSystemScript);
		if(!err)
		{
			FSpCreateResFile((FSSpec *)file, APPLICATION_CREATOR, file_type, smSystemScript);
			err= ResError();
		}
	}

	return err;
}

fileref open_file_for_reading(
	FileDesc *file)
{
	fileref file_id;
	Boolean is_folder, was_aliased;
	FileError error;

	ResolveAliasFile((FSSpec *)file, TRUE, &is_folder, &was_aliased);
	error= FSpOpenDF((FSSpec *)file, fsRdPerm, &file_id);
	if (error) 
	{
		// LP change: suppress
		// dprintf("HOpen('%.*s', #%d, #%d)==#%d;g;", file->name[0], file->name+1, file->vRefNum, file->parID, error);

		file_id= NONE;
		set_game_error(systemError, error);
	}

	return file_id;
}

fileref open_file_for_writing(
	FileDesc *file)
{
	fileref file_id;
	Boolean is_folder, was_aliased;
	FileError error;

	ResolveAliasFile((FSSpec *)file, TRUE, &is_folder, &was_aliased);
	error= FSpOpenDF((FSSpec *)file, fsWrPerm, &file_id);
	if (error) 
	{
		// LP change: suppress
		// dprintf("HOpen('%.*s', #%d, #%d)==#%d;g;", file->name[0], file->name+1, file->vRefNum, file->parID, error);

		file_id= NONE;
		set_game_error(systemError, error);
	}

	return file_id;
}

void close_file(
	fileref refnum)
{
	OSErr error;
	
/*	error= FSFlush(refnum, 0);	why?  closing flushes.   (Bo Lindbergh) */
	error= FSClose(refnum);
	assert(!error);
	
	return;
}

unsigned long get_fpos(
	fileref refnum)
{
	unsigned long position;
	OSErr err;

	err= GetFPos(refnum, (long *) &position);	
	assert(!err);
	
	return position;
}

FileError set_fpos(
	fileref refnum,
	unsigned long offset)
{
	OSErr err;

	err= SetFPos(refnum, fsFromStart, offset);
	
	return err;
}

FileError set_eof(
	fileref refnum,
	unsigned long offset)
{
	OSErr err;
	
	err= SetEOF(refnum, offset);
	
	return err;
}

unsigned long get_file_length(
	fileref refnum)
{
	unsigned long file_length;
	unsigned long initial_position;

	GetFPos(refnum, (long *) &initial_position);	
	SetFPos(refnum, fsFromLEOF, 0l);
	GetFPos(refnum, (long *) &file_length);
	SetFPos(refnum, fsFromStart, initial_position);
	
	return file_length;
}

FileError read_file(
	fileref refnum,
	unsigned long count, 
	void *buffer)
{
	OSErr err;
	
	assert(refnum>=0);
	err= FSRead(refnum, (long *) &count, buffer);
	
	return err;
}

FileError write_file(
	fileref refnum,
	unsigned long count,
	void *buffer)
{
	OSErr err;

	err= FSWrite(refnum, (long *) &count, buffer);

	return err;
}

FileError delete_file(
	FileDesc *file)
{
	FileError error;
	
	error= FSpDelete((FSSpec *) file);
	
	return error;
}

FileError find_preferences_location(
	FileDesc *file)
{
	FileError err;

	err= FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		&file->vRefNum, &file->parID);
		
	return err;
}

/* ------- miscellaneous routines */
FileError add_application_name_to_fsspec(
	FileDesc *file, 
	unsigned char *pascal_name)
{
	FileError err;
	FInfo finder_info;
	short refnum;
	
	err = FSpGetFInfo((FSSpec *) file, &finder_info);
	if(!err)
	{
		refnum= FSpOpenResFile((FSSpec *) file, fsWrPerm);
		err= ResError();
		if(err)
		{
			/* This file doesn't have a res fork.. */
			if(err==fnfErr)
			{
				FSpCreateResFile((FSSpec *) file, finder_info.fdCreator, 
					finder_info.fdType, smSystemScript);
				err= ResError();
				
				if(!err)
				{
					refnum= FSpOpenResFile((FSSpec *) file, fsWrPerm);
					err= ResError();
				}
			}
		}
	}
	
	if(!err)
	{
		Handle resource;
		
		assert(refnum>=0);
								
		/* Add in the application name */
		err= PtrToHand(pascal_name, &resource, pascal_name[0]+1);
		assert(!err && resource);
								
		AddResource(resource, 'STR ', -16396, "\p");
		ReleaseResource(resource);
					
		CloseResFile(refnum);
	}

	return err;
}

void get_application_filedesc( /* this is used to save files in the same directory */
	FileDesc *file)
{
	get_my_fsspec((FSSpec *) file);
}

FileError memory_error(
	void)
{
	return MemError();
}

/*

	wad_macintosh.c
	Saturday, August 26, 1995 2:04:59 PM- rdm created.

	routines from wad.c that are not portable!

Jan 30, 2000 (Loren Petrich)
	Changed "private" to "_private" to make data structures more C++-friendly

Feb 19, 2000 (Loren Petrich):
	Made more searching recurvive;
	in particular, searches for parent files of saved games

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

#include <string.h>

#include "macintosh_cseries.h"
#include "wad.h"
#include "tags.h"
#include "game_errors.h"

#include "find_files.h"
#include "FileHandler.h"

#ifdef env68k
#pragma segment file_io
#endif

struct find_checksum_private_data {
	unsigned long checksum_to_match;
};

struct find_files_private_data { /* used for enumerating wadfiles */
	FileSpecifier BaseFile;
	// FileDesc *base_file;
	long base_checksum;
};

/* ------------ local prototypes */

static Boolean match_wad_checksum_callback(FSSpec *file, void *data);
static Boolean checksum_and_not_base_callback(FSSpec *file, void *data);
static Boolean match_modification_date_callback(FSSpec *file, void *data);
static boolean find_wad_file_with_checksum_in_directory(FSSpec *matching_file, short vRefNum,
	long parID, unsigned long file_type, unsigned long checksum);
static boolean find_file_with_modification_date_in_directory(FSSpec *matching_file, short vRefNum,
	long parID, unsigned long file_type, unsigned long modification_date);

/* ------------- code! */
/* Search all the directories in the path.. */
boolean find_wad_file_that_has_checksum(
	FileSpecifier& File,
	// FileDesc *matching_file,
	unsigned long file_type,
	short path_resource_id,
	unsigned long checksum)
{
	OSErr err;
	FSSpec app_spec;
	boolean file_matched= FALSE;
	
	FSSpec *matching_file = &File.GetSpec();
	// FileDesc *matching_file = (FileDesc *)&(File.Spec);

	/* Look for the files in the same directory that we are in.. */	
	err= get_my_fsspec(&app_spec);
	if(!err)
	{
		file_matched= find_wad_file_with_checksum_in_directory((FSSpec *) matching_file,
			app_spec.vRefNum, app_spec.parID, file_type, checksum);
			
		if(!file_matched)
		{
			short path_count= countstr(path_resource_id);
			short index;
			
			for(index= 0; !file_matched && index<path_count; ++index)
			{
				FSSpec test_directory_spec;
			
				getpstr(ptemporary, path_resource_id, index);
				err= FSMakeFSSpec(app_spec.vRefNum, app_spec.parID, ptemporary, &test_directory_spec);

				if(!err)
				{
					long parID;
					OSErr error;
					
					/* The referenced thing is a directory, and we want to search inside it. */
					/*  therefore we want to get it's parent id.. */
					error= get_directories_parID(&test_directory_spec, &parID);
					if(!error)
					{
						file_matched= find_wad_file_with_checksum_in_directory((FSSpec *)matching_file,
							test_directory_spec.vRefNum, parID, file_type, checksum);
					}
				}
			}
		}
	}

	return file_matched;
}

// LP: begin no-compile
#if 0

/* Find other entries that reference the base wad.  Search is by type, and when one is found, the */
/*  checksums are checked, to make sure that it is modifying the correct file. */
FileError find_other_entries_that_reference_checksum(
	unsigned long checksum,
	FileDesc *files_array,
	short *count)
{
	struct find_file_pb pb;
	OSType file_type;
	struct find_files_private_data private_data;
	FInfo file_info;
	FileError error;

	/* What type of file are we? */
	FSpGetFInfo((FSSpec *) &files_array[0], &file_info);
	file_type= file_info.fdType;

	/* Setup the private data for the callback */
	private_data.base_file= &files_array[0];
	private_data.base_checksum= checksum;
	
	/* Clear out the find_file pb */
	memset(&pb, 0, sizeof(struct find_file_pb));
	
	/* Set the information */
	pb.version= 0;
	pb.flags= _ff_recurse;
	pb.vRefNum= files_array[0].vRefNum;
	pb.directory_id= files_array[0].parID;
	pb.type_to_find= file_type;
	pb.buffer= (FSSpec *) &files_array[1]; /* First one is already set.. */
	pb.max= MAXIMUM_UNION_WADFILES-1;
	pb.callback= checksum_and_not_base_callback;
	pb.user_data= &private_data;

	/* Find them! */
	error= find_files(&pb);
	
	if(!error) 
	{
		*count= pb.count+1; /* +1 because base is already added. */
	} else {
		*count= 1;
	}

	return error;
}
// LP: end no-compile
#endif

boolean find_file_with_modification_date(
	FileSpecifier& MatchingFile,
	// FileDesc *matching_file,
	unsigned long file_type,
	short path_resource_id,
	unsigned long modification_date)
{
	OSErr err;
	FSSpec app_spec;
	boolean file_matched= FALSE;

	/* Look for the files in the same directory that we are in.. */	
	err= get_my_fsspec(&app_spec);
	FSSpec *matching_file = &MatchingFile.GetSpec();
	if(!err)
	{
		file_matched= find_file_with_modification_date_in_directory((FSSpec *) matching_file,
			app_spec.vRefNum, app_spec.parID, file_type, modification_date);
			
		if(!file_matched)
		{
			short path_count= countstr(path_resource_id);
			short index;
			
			for(index= 0; !file_matched && index<path_count; ++index)
			{
				FSSpec test_directory_spec;
			
				getpstr(ptemporary, path_resource_id, index);
				err= FSMakeFSSpec(app_spec.vRefNum, app_spec.parID, ptemporary, &test_directory_spec);

				if(!err)
				{
					long parID;
					OSErr error;
					
					/* The referenced thing is a directory, and we want to search inside it. */
					/*  therefore we want to get it's parent id.. */
					error= get_directories_parID(&test_directory_spec, &parID);
					if(!error)
					{
						file_matched= find_file_with_modification_date_in_directory((FSSpec *)matching_file,
							test_directory_spec.vRefNum, parID, file_type, modification_date);
					}
				}
			}
		}
	}

	return file_matched;
}

/* Return this directorie's parent id.. (the parID field of things inside it..) */
OSErr get_directories_parID(
	FSSpec *directory, 
	long *parID)
{
	CInfoPBRec pb;
	OSErr error;

	/* Clear it.  Always a good thing */
	memset(&pb, 0, sizeof(pb));
	pb.dirInfo.ioNamePtr= directory->name;
	pb.dirInfo.ioVRefNum= directory->vRefNum;
	pb.dirInfo.ioDrDirID= directory->parID;
	pb.dirInfo.ioFDirIndex= 0; /* use ioNamePtr and ioDirID */
	error= PBGetCatInfoSync(&pb);
	
	*parID = pb.dirInfo.ioDrDirID;
	assert(!error && (pb.hFileInfo.ioFlAttrib & 0x10));

	return error;
}

/* -------------- local code */
static Boolean checksum_and_not_base_callback(
	FSSpec *file, 
	void *data)
{
	Boolean add_this_file= FALSE;
	struct find_files_private_data *_private= (struct find_files_private_data *) data;
	
	/* Don't readd the base file.. */
	if(!equal_fsspecs(file, (FSSpec *) &_private->BaseFile.GetSpec()))
	// if(!equal_fsspecs(file, (FSSpec *) _private->base_file))
	{
		/* Do the checksums match? */
		FileSpecifier File;
		File.SetSpec(*file);
		if(wad_file_has_parent_checksum(File, _private->base_checksum))
		// if(wad_file_has_parent_checksum((FileDesc *) file, _private->base_checksum))
		{
			add_this_file= TRUE;
		}
	}
	
	return add_this_file;
}

static Boolean match_wad_checksum_callback(
	FSSpec *file, 
	void *data)
{
	Boolean add_this_file= FALSE;
	struct find_checksum_private_data *_private= (struct find_checksum_private_data *) data;
	
	/* Do the checksums match? */
	FileSpecifier File;
	File.SetSpec(*file);
	if(wad_file_has_checksum(File, _private->checksum_to_match))
	// if(wad_file_has_checksum((FileDesc *) file, _private->checksum_to_match))
	{
		add_this_file= TRUE;
	}
	
	return add_this_file;
}
	
static boolean find_wad_file_with_checksum_in_directory(
	FSSpec *matching_file,
	short vRefNum,
	long parID,
	unsigned long file_type,
	unsigned long checksum)
{
	boolean success= FALSE;
	struct find_file_pb pb;
	struct find_checksum_private_data private_data;
	// FileError error;
	short error;

	/* Setup the private data for the callback */
	private_data.checksum_to_match= checksum;
	
	/* Clear out the find_file pb */
	memset(&pb, 0, sizeof(struct find_file_pb));
	
	/* Set the information */
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse; /* DANGER WILL ROBINSON!!! */
#if 0
#ifdef FINAL
	pb.flags= _ff_recurse; /* DANGER WILL ROBINSON!!! */
#else
	pb.flags= 0; /* DANGER WILL ROBINSON!!! */
#endif
#endif
	pb.vRefNum= vRefNum;
	pb.directory_id= parID;
	pb.type_to_find= file_type;
	pb.buffer= (FSSpec *) matching_file;
	pb.max= 1; /* Only find one.. */
	pb.callback= match_wad_checksum_callback;
	pb.user_data= &private_data;

	/* Find them! */
	error= find_files(&pb);
	
	if(!error) 
	{
		if(pb.count) success= TRUE;
	} else {
	dprintf("Trying to find file, error: %d", error);
	}

	return success;
}

static unsigned long target_modification_date;

static boolean find_file_with_modification_date_in_directory(
	FSSpec *matching_file,
	short vRefNum,
	long parID,
	unsigned long file_type,
	unsigned long modification_date)
{
	boolean success= FALSE;
	struct find_file_pb pb;
	short error;
	// FileError error;

	/* Setup the private data for the callback */
	target_modification_date= modification_date;
	
	/* Clear out the find_file pb */
	memset(&pb, 0, sizeof(struct find_file_pb));
	
	/* Set the information */
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse | _ff_callback_with_catinfo; /* DANGER WILL ROBINSON!!! */
#if 0
#ifdef FINAL
	pb.flags= _ff_recurse | _ff_callback_with_catinfo; /* DANGER WILL ROBINSON!!! */
#else
	pb.flags= _ff_callback_with_catinfo; /* DANGER WILL ROBINSON!!! */
#endif
#endif
	pb.vRefNum= vRefNum;
	pb.directory_id= parID;
	pb.type_to_find= file_type;
	pb.buffer= (FSSpec *) matching_file;
	pb.max= 1; /* Only find one.. */
	pb.callback= match_modification_date_callback;
	pb.user_data= NULL;

	/* Find them! */
	error= find_files(&pb);
	
	if(!error) 
	{
		if(pb.count) success= TRUE;
	} else {
	dprintf("Trying to find file, error: %d", error);
	}

	return success;
}

static Boolean match_modification_date_callback(
	FSSpec *file, 
	void *data)
{
	Boolean add_this_file= FALSE;
	CInfoPBRec *pb= (CInfoPBRec *) data;
	(void) (file);
	if(pb->hFileInfo.ioFlMdDat==target_modification_date)
	{
		add_this_file= TRUE;
	}
	
	return add_this_file;
}

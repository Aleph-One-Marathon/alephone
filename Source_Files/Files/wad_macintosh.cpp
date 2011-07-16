/*
	wad_macintosh.c

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

	Saturday, August 26, 1995 2:04:59 PM- rdm created.

	routines from wad.c that are not portable!

Jan 30, 2000 (Loren Petrich)
	Changed "private" to "_private" to make data structures more C++-friendly

Feb 19, 2000 (Loren Petrich):
	Made more searching recurvive;
	in particular, searches for parent files of saved games

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 25, 2000 (Loren Petrich):
	Confined searches to the app's directory
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
	uint32 checksum_to_match;
};

struct find_files_private_data { /* used for enumerating wadfiles */
	FileSpecifier BaseFile;
	uint32 base_checksum;
};

/* ------------ local prototypes */

static bool match_wad_checksum_callback(FileSpecifier& File, void *data);
//static bool checksum_and_not_base_callback(FileSpecifier& File, void *data);
static bool match_modification_date_callback(FileSpecifier& File, void *data);
// Now intended to use the _typecode_stuff in tags.h (abstract filetypes)
static bool find_wad_file_with_checksum_in_directory(
	FileSpecifier& MatchingFile, DirectorySpecifier& BaseDir,
	Typecode file_type, uint32 checksum);
static bool find_file_with_modification_date_in_directory(
	FileSpecifier& MatchingFile, DirectorySpecifier& BaseDir,
	Typecode file_type, uint32 checksum);

/* ------------- code! */
/* Search all the directories in the path.. */
bool find_wad_file_that_has_checksum(
	FileSpecifier& MatchingFile,
	Typecode file_type,
	short path_resource_id,
	uint32 checksum)
{
	bool file_matched= false;
	
	/* Look for the files in the same directory that we are in.. */	
	
	// LP: for now, will only care about looking in the Marathon app's directory
	DirectorySpecifier BaseDir;
	Files_GetRootDirectory(BaseDir);
	file_matched= find_wad_file_with_checksum_in_directory(MatchingFile, BaseDir,file_type, checksum);

	return file_matched;
}


bool find_file_with_modification_date(
	FileSpecifier& MatchingFile,
	Typecode file_type,
	short path_resource_id,
	TimeType modification_date)
{
	bool file_matched= false;

	/* Look for the files in the same directory that we are in.. */	
	// LP: for now, will only care about looking in the Marathon app's directory
	DirectorySpecifier BaseDir;
	Files_GetRootDirectory(BaseDir);
	file_matched= find_file_with_modification_date_in_directory(MatchingFile, BaseDir,file_type, modification_date);

	return file_matched;
}


/* -------------- local code */
#if 0
static bool checksum_and_not_base_callback(
	FileSpecifier& File,
	void *data)
{
	bool add_this_file= false;
	struct find_files_private_data *_private= (struct find_files_private_data *) data;
	
	/* Don't readd the base file.. */
	if (File != _private->BaseFile)
	{
		/* Do the checksums match? */
		if(wad_file_has_parent_checksum(File, _private->base_checksum))
		{
			add_this_file= true;
		}
	}
	
	return add_this_file;
}
#endif

static bool match_wad_checksum_callback(
	FileSpecifier& File,
	void *data)
{
	bool add_this_file= false;
	struct find_checksum_private_data *_private= (struct find_checksum_private_data *) data;
	
	/* Do the checksums match? */
	if(wad_file_has_checksum(File, _private->checksum_to_match))
	{
		add_this_file= true;
	}
	
	return add_this_file;
}
	
static bool find_wad_file_with_checksum_in_directory(
	FileSpecifier& MatchingFile,
	DirectorySpecifier& BaseDir,
	Typecode file_type,
	uint32 checksum)
{
	bool success= false;
	FileFinder pb;
	struct find_checksum_private_data private_data;

	/* Setup the private data for the callback */
	private_data.checksum_to_match= checksum;
	
	/* Clear out the find_file pb */
	pb.Clear();
	
	/* Set the information */
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse; /* DANGER WILL ROBINSON!!! */
	pb.BaseDir = BaseDir;
	pb.Type= file_type;
	pb.buffer= &MatchingFile;
	pb.max= 1; /* Only find one.. */
	pb.callback= match_wad_checksum_callback;
	pb.user_data= &private_data;

	/* Find them! */
	if (pb.Find())
	{
		if(pb.count) success= true;
	} else {
		dprintf("Trying to find file, error: %d", pb.GetError());
	}

	return success;
}

static TimeType target_modification_date;

static bool find_file_with_modification_date_in_directory(
	FileSpecifier& MatchingFile,
	DirectorySpecifier& BaseDir,
	Typecode file_type,
	TimeType modification_date)
{
	bool success= false;
	FileFinder pb;

	/* Setup the private data for the callback */
	target_modification_date= modification_date;
	
	/* Clear out the find_file pb */
	pb.Clear();
	
	/* Set the information */
	pb.version= 0;
	// LP change: always recurse
	pb.flags= _ff_recurse | _ff_callback_with_catinfo; /* DANGER WILL ROBINSON!!! */
	pb.BaseDir = BaseDir;
	pb.Type= file_type;
	pb.buffer= &MatchingFile;
	pb.max= 1; /* Only find one.. */
	pb.callback= match_modification_date_callback;
	pb.user_data= NULL;

	/* Find them! */
	if (pb.Find())
	{
		if(pb.count) success= true;
	} else {
		dprintf("Trying to find file, error: %d", pb.GetError());
	}

	return success;
}

static bool match_modification_date_callback(
	FileSpecifier& File,
	void *data)
{
	// More general code; does not use the passed data
	bool add_this_file= false;
	if (File.GetDate() == target_modification_date)
		add_this_file = true;
	
	return add_this_file;
}

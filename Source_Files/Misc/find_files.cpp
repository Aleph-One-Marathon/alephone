/*
	find_files.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Wednesday, January 11, 1995 6:44:55 PM

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Aug 25, 2000 (Loren Petrich):
	Abstracting the file handling

Aug 28, 2000 (Loren Petrich):
	Put these functions into the FileFinder object; eradicated the "param_block->" code, etc.
	and did other things to abstract it.
*/
#if defined(mac) || ( defined(SDL) && defined(SDL_RFORK_HACK) )
#include "macintosh_cseries.h"
#include <string.h> /* For memset */
#include <stdlib.h>

#include "find_files.h"

#ifdef env68k
	#pragma segment file_io
#endif

/* --------- local prototypes */
// static int alphabetical_names(void const *a, void const *b);
// static OSErr enumerate_files(struct find_file_pb *param_block, long directory_id);

// Code:

void FileFinder::Clear()
{
	obj_clear(*this);
}


/* ---------------- Parameter Block Version */
// OSErr find_files(
//	struct find_file_pb *param_block)
bool FileFinder::Find()
{
	// Translate the type info:
	type_to_find = (Type != WILDCARD_TYPE) ? get_typecode(Type) : 0;
	
	// Eliminated buffer creation as unused
	
	/* Assert that we got it. */
	assert(search_type==_callback_only || buffer);
	assert(version==0);

	/* Set the variables */
	count= 0;

	Enumerate(BaseDir);
	
	// Eliminated alphabetical order as unused
		
	return (Err == noErr);
}

/*
Boolean equal_fsspecs(
	FSSpec *a, 
	FSSpec *b)
{
	Boolean equal= false;
	
	if(a->vRefNum==b->vRefNum && a->parID==b->parID && 
		EqualString(a->name, b->name, false, false))
	{
		equal= true;
	}
	
	return equal;
}
*/

/* --------- Local Code --------- */
// static OSErr enumerate_files(
//	struct find_file_pb *param_block, 
//	long directory_id) /* Because it is recursive.. */
bool FileFinder::Enumerate(DirectorySpecifier& Dir)
{
	// Set up the temporary file
	TempFile.FromDirectory(Dir);

	// Kludge to make the FSSpec always available
	FSSpec& temp_file = TempFile.GetSpec();
	
	// Local variable
	short index;

	obj_clear(pb);
	
	pb.hFileInfo.ioVRefNum= Dir.Get_vRefNum();
	pb.hFileInfo.ioNamePtr= temp_file.name;
			
	for(Err= noErr, index=1; count<max && Err==noErr; index++) 
	{
		pb.hFileInfo.ioDirID= Dir.Get_parID();
		pb.hFileInfo.ioFDirIndex= index;

		Err= PBGetCatInfo( &pb, false);
		if(Err == noErr)
		{
			if ((pb.hFileInfo.ioFlAttrib & 16) && (flags & _ff_recurse))
			{
				/* Recurse, if you really want to... */
				
				// Set up volume info
				DirectorySpecifier SubDir = Dir;
				// Change the directory
				SubDir.Set_parID(pb.dirInfo.ioDrDirID);
				// Go!
				Enumerate(SubDir);
				// Reset the temporary file's directory to what's appropriate here
				// (it's global to this object)
				TempFile.FromDirectory(Dir);
				
			} else {
				/* Add.. */
				if(Type==WILDCARD_TYPE || 
					pb.hFileInfo.ioFlFndrInfo.fdType==type_to_find)
				{
					/* Only add if there isn't a callback or it returns true */
					switch(search_type)
					{
						case _fill_buffer:
							if(!callback || callback(TempFile, user_data))
							// if(!callback || callback(&temp_file, user_data))
							{
								/* Copy it in.. */
								buffer[count++] = TempFile;
							}
							break;
							
						case _callback_only:
							assert(callback);
							if(flags & _ff_callback_with_catinfo)
							{
								callback(TempFile, &pb);
								// callback(&temp_file, &pb);
							} else {
								callback(TempFile, user_data);
								// callback(&temp_file, user_data);
							}
							break;
							
						default:
							// LP change:
							assert(false);
							// halt();
							break;
					}
				}
			}
		} else {
			/* We ran out of files.. */
		}
	}

	/* If we got a fnfErr, it was because we indexed too far. */	
	///return (Err==fnfErr) ? (noErr) : Err;
	if (Err == fnfErr) Err = noErr;
	
	return (Err == noErr);
}
/*
static int alphabetical_names(
	void const *a, 
	void const *b)
{
	return (IUCompString(((FSSpec const *)a)->name, ((FSSpec const *)b)->name)); 
}
*/
#endif
#ifndef __FIND_FILES_H
#define __FIND_FILES_H

/*

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

Aug 25, 2000 (Loren Petrich):
	Abstracting the file handling
	
Aug 26, 2000 (Loren Petrich):
	Creating an object-oriented file finder

*/

#include "FileHandler.h"

// Finds every type of file
const Typecode WILDCARD_TYPE = _typecode_unknown;

#if defined(mac)

/* Find all files of a given type.. */

enum {
	_fill_buffer, 				/* Fill the buffer with matches */
	_callback_only				/* Ignore the buffer, and call the callback for each. */
};

enum {
	// Filespec buffers are always created from outside,
	// and no alphabetical sorting is ever done
	// _ff_create_buffer= 0x0001,	/* Create the destination buffer, free later w/ Dispose */
	// _ff_alphabetical= 0x0002,	/* Matches are returned in alphabetical order */
	_ff_recurse= 0x0004,		/* Recurse when I find a subdirectory */
	_ff_callback_with_catinfo= 0x0008 /* Callback with CInfoPBRec as second paramter */
};

// File-finder object
class FileFinder
{
	// Temporary stuff:
	FileSpecifier TempFile;
	
	bool Enumerate(DirectorySpecifier& Dir);

public:
	short version;			/* Version Control (Set to 0)		<-  */
	short flags;			/* Search flags 					<-  */
	short search_type;		/* Search type						<-  */
	
	DirectorySpecifier BaseDir;
	Typecode Type;				/* OSType to find					<-  */
	
	FileSpecifier *buffer; 	/* Destination						<-> */
	short max;				/* Maximum matches to return		<-  */
	short count;			/* Count of matches found 			->  */

	/* Callback	functions, if returns true, you add it.  If */
	/*  callback==NULL, adds all found.							<-  */
        // ZZZ semantics change: if _callback_only and callback returns false, terminate the enumeration
	bool (*callback)(FileSpecifier& File, void *data);
	void *user_data;		/* Passed to callback above.		<-  */
        // ZZZ addition: will be called when entering and leaving a subdirectory
        bool (*directory_change_callback)(FileSpecifier& Directory, bool EnteringDirectory, void* data);
	
	// Clears out the memory contents
	void Clear();
	
	// Does the finding
	bool Find();
	
	// Platform-specific members
	short GetError() {return Err;}

private:
	CInfoPBRec pb; /* static to prevent stack overflow.. */
	OSType type_to_find;
	OSErr Err;
};

#elif defined(SDL)

#include <vector>

// File-finder base class
class FileFinder {
public:
	FileFinder() {}
	virtual ~FileFinder() {}

	bool Find(DirectorySpecifier &dir, Typecode type, bool recursive = true) {
		return _Find(dir, type, recursive, 0);
	}

protected:
	bool _Find(DirectorySpecifier& dir, Typecode type, bool recursive, int depth);
	// Gets called for each found file, returns true if search is to be aborted
	virtual bool found(FileSpecifier &file) = 0;
};

// Find all files of given type and append them to a vector
class FindAllFiles : public FileFinder {
public:
	FindAllFiles(vector<FileSpecifier> &v) : dest_vector(v) {dest_vector.clear();}

private:
	bool found(FileSpecifier &file);
	vector<FileSpecifier> &dest_vector;
};

#endif

#endif

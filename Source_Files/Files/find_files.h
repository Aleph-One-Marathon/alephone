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

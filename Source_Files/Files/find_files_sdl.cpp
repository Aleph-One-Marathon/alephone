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

*/

/*
 *  find_files_sdl.cpp - Routines for finding files, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "find_files.h"

#include <vector>
#include <algorithm>


/*
 *  File finder base class
 */

bool FileFinder::_Find(DirectorySpecifier &dir, Typecode type, bool recursive, int depth)
{
	// Get list of entries in directory
	vector<dir_entry> entries;
	if (!dir.ReadDirectory(entries))
		return false;
	sort(entries.begin(), entries.end());

	// Iterate through entries
	vector<dir_entry>::const_iterator i, end = entries.end();
	for (i = entries.begin(); i != end; i++) {

		// Construct full specifier of file/dir
		FileSpecifier file = dir + i->name;

		if (i->is_directory) {

			if (depth == 0 && i->name == "Plugins")
				continue;

			// Recurse into directory
			if (recursive)
				if (_Find(file, type, recursive, depth + 1))
					return true;

		} else {

			// Check file type and call found() function
			if (type == WILDCARD_TYPE || type == file.GetType())
				if (found(file))
					return true;
		}
	}
	return false;
}


/*
 *  Find all files of given type
 */

bool FindAllFiles::found(FileSpecifier &file)
{
	dest_vector.push_back(file);
	return false;
}

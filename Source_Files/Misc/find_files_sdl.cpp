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

void FileFinder::Find(DirectorySpecifier &dir, int type, bool recursive)
{
	// Get list of entries in directory
	vector<dir_entry> entries;
	if (!dir.ReadDirectory(entries))
		return;
	sort(entries.begin(), entries.end());

	// Iterate through entries
	vector<dir_entry>::const_iterator i, end = entries.end();
	for (i = entries.begin(); i != end; i++) {

		// Construct full specifier of file/dir
		FileSpecifier file = dir;
		file.AddPart(i->name);

		if (i->is_directory) {

			// Recurse into directory
			if (recursive)
				Find(file, type, recursive);

		} else {

			// Check file type and call found() function
			if (type == WILDCARD_TYPE || type == file.GetType())
				if (found(file))
					return;
		}
	}
}


/*
 *  Find all files of given type
 */

bool FindAllFiles::found(FileSpecifier &file)
{
	dest_vector.push_back(file);
	return false;
}

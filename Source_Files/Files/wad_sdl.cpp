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
 *  wad_sdl.cpp - Additional map file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "find_files.h"

#include <SDL_endian.h>


// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;


/*
 *  Find map file with specified checksum in path
 */

class FindByChecksum : public FileFinder {
public:
	FindByChecksum(uint32 checksum) : look_for_checksum(checksum) {}
	~FindByChecksum() {}

	FileSpecifier found_what;

private:
	bool found(FileSpecifier &file)
	{
		OpenedFile f;
		if (!file.Open(f))
			return false;
		f.SetPosition(0x44);
		SDL_RWops *p = f.GetRWops();
		uint32 checksum = SDL_ReadBE32(p);
		if (checksum == look_for_checksum) {
			found_what = file;
			return true;
		}
		return false;
	}

	uint32 look_for_checksum;
};

bool find_wad_file_that_has_checksum(FileSpecifier &matching_file, Typecode file_type, short path_resource_id, uint32 checksum)
{
	FindByChecksum finder(checksum);
	vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
	while (i != end) {
		FileSpecifier dir = *i;
		if (finder.Find(dir, file_type)) {
			matching_file = finder.found_what;
			return true;
		}
		i++;
	}
	return false;
}


/*
 *  Find file with specified modification date in path
 */

class FindByDate : public FileFinder {
public:
	FindByDate(time_t date) : look_for_date(date) {}
	~FindByDate() {}

	FileSpecifier found_what;

private:
	bool found(FileSpecifier &file)
	{
		if (file.GetDate() == look_for_date) {
			found_what = file;
			return true;
		}
		return false;
	}

	TimeType look_for_date;
};

bool find_file_with_modification_date(FileSpecifier &matching_file, Typecode file_type, short path_resource_id, TimeType modification_date)
{
	FindByDate finder(modification_date);
	vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
	while (i != end) {
		FileSpecifier dir = *i;
		if (finder.Find(dir, file_type)) {
			matching_file = finder.found_what;
			return true;
		}
		i++;
	}
	return false;
}

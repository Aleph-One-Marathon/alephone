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
 *  preprocess_map_sdl.cpp - Save game routines, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"

#include <vector>

#include "world.h"
#include "map.h"
#include "shell.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"
#include "QuickSave.h"

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;


/*
 *  Get FileSpecifiers for default data files
 */

static bool get_default_spec(FileSpecifier &file, const string &name)
{
	vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
	while (i != end) {
		file = *i + name;
		if (file.Exists())
			return true;
		i++;
	}
	return false;
}

static bool get_default_spec(FileSpecifier &file, int type)
{
	char name[256];
	getcstr(name, strFILENAMES, type);
	return get_default_spec(file, name);
}

bool have_default_files(void)
{
	FileSpecifier file;
	return (get_default_spec(file, filenameDEFAULT_MAP) &&
			(get_default_spec(file, filenameIMAGES) || get_default_spec(file, filenameEXTERNAL_RESOURCES)) &&
			get_default_spec(file, filenameSHAPES8));
}

void get_default_external_resources_spec(FileSpecifier& file)
{
	get_default_spec(file, filenameEXTERNAL_RESOURCES);
}	

void get_default_map_spec(FileSpecifier &file)
{
	if (!get_default_spec(file, filenameDEFAULT_MAP))
		alert_bad_extra_file();
}

void get_default_physics_spec(FileSpecifier &file)
{
	get_default_spec(file, filenamePHYSICS_MODEL);
	// don't care if it does not exist
}

void get_default_shapes_spec(FileSpecifier &file)
{
	if (!get_default_spec(file, filenameSHAPES8))
		alert_bad_extra_file();
}

void get_default_sounds_spec(FileSpecifier &file)
{
	get_default_spec(file, filenameSOUNDS8);
	// don't care if it does not exist
}

bool get_default_music_spec(FileSpecifier &file)
{
	return get_default_spec(file, filenameMUSIC);
}

bool get_default_theme_spec(FileSpecifier &file)
{
	FileSpecifier theme = "Themes";
	theme += getcstr(temporary, strFILENAMES, filenameDEFAULT_THEME);
	return get_default_spec(file, theme.GetPath());
}

/*
 *  Choose saved game for loading
 */

bool choose_saved_game_to_load(FileSpecifier &saved_game)
{
	return load_quick_save_dialog(saved_game);
}


/*
 *  Save game
 */

bool save_game(void)
{
	pause_game();
    bool success = create_quick_save();
    if (success)
        screen_printf("Game saved");
    else
        screen_printf("Save failed");
	resume_game();

	return success;
}


/*
 *  Store additional data in saved game file
 */

void add_finishing_touches_to_save_file(FileSpecifier &file)
{
	// The MacOS version stores an overhead thumbnail and the level name
	// in resources. Do we want to imitate this?
}

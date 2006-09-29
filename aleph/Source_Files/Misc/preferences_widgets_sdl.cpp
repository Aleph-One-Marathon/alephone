/*

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

*/

/*
 *  preferences_widgets_sdl.cpp - Preferences widgets, SDL specific
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Ripped out of preferences_sdl.cpp/preferences_sdl.h into a new
 *  file Mar 1, 2002 by Woody Zenfell, for sharing.
 */

#include    "preferences_widgets_sdl.h"

/*
 *  Environment dialog
 */

void w_env_select::select_item_callback(void* arg) {
    w_env_select* obj = (w_env_select*) arg;
    obj->select_item(obj->parent);
}

void w_env_select::select_item(dialog *parent)
{
	// Find available files
	vector<FileSpecifier> files;
	if (type == _typecode_theme) {

		// Theme, find by theme script
		FindThemes finder(files);
		vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			FileSpecifier dir = *i + "Themes";
			finder.Find(dir, WILDCARD_TYPE);
			i++;
		}

	} else {

		// Map/phyics/shapes/sounds, find by type
		FindAllFiles finder(files);
		vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			FileSpecifier dir = *i;
			finder.Find(dir, type);
			i++;
		}
	}

	// Create structured list of files
	vector<env_item> items;
	vector<FileSpecifier>::const_iterator i = files.begin(), end = files.end();
	string last_base;
	int indent_level = 0;
	for (i = files.begin(); i != end; i++) {
		string base, part;
		i->SplitPath(base, part);
		if (base != last_base) {

			// New directory
			FileSpecifier base_spec = base;
//			if (base_spec != global_dir && base_spec != local_dir) {

				// Subdirectory, insert name as unselectable item, put items on indentation level 1
				items.push_back(env_item(base_spec, 0, false));
				indent_level = 1;

//			} else {
//
//				// Top-level directory, put items on indentation level 0
//				indent_level = 0;
//			}
			last_base = base;
		}
		items.push_back(env_item(*i, indent_level, true));
	}

	// Create dialog
	dialog d;
	d.add(new w_static_text(menu_title, TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_env_list *list_w = new w_env_list(items, item.GetPath(), &d);
	d.add(list_w);
	d.add(new w_spacer());
	d.add(new w_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) { // Accepted
		if (items.size())
			set_path(items[list_w->get_selection()].spec.GetPath());

        if(mCallback)
            mCallback(this);
	}
}

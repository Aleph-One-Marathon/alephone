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
 *  preferences_widgets_sdl.h - Preferences widgets, SDL specific
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Ripped out of preferences_sdl.* into a new file Mar 1, 2002 by Woody Zenfell.
 */

#ifndef PREFERENCES_WIDGETS_SDL_H
#define PREFERENCES_WIDGETS_SDL_H

#include    "cseries.h"
#include    "find_files.h"
#include    "collection_definition.h"
#include    "sdl_widgets.h"
#include    "sdl_fonts.h"
#include    "screen.h"
#include    "screen_drawing.h"
#include    "interface.h"

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;

// Find available themes in directory and append to vector
class FindThemes : public FileFinder {
public:
	FindThemes(vector<FileSpecifier> &v) : dest_vector(v) {dest_vector.clear();}

private:
	bool found(FileSpecifier &file)
	{
		// Look for "theme.mml" files
		string base, part;
		file.SplitPath(base, part);
		if (part == "theme.mml")
			dest_vector.push_back(base);
		return false;
	}

	vector<FileSpecifier> &dest_vector;
};

// Environment item
class env_item {
public:
	env_item() : indent(0), selectable(false)
	{
		name[0] = 0;
	}

	env_item(const FileSpecifier &fs, int i, bool sel) : spec(fs), indent(i), selectable(sel)
	{
		spec.GetName(name);
	}

	FileSpecifier spec;	// Specifier of associated file
	char name[256];		// Last part of file name
	int indent;			// Indentation level
	bool selectable;	// Flag: item refers to selectable file (otherwise to directory name)
};

// Environment file list widget
class w_env_list : public w_list<env_item> {
public:
	w_env_list(const vector<env_item> &items, const char *selection, dialog *d) : w_list<env_item>(items, 400, 15, 0), parent(d)
	{
		vector<env_item>::const_iterator i, end = items.end();
		size_t num = 0;
		for (i = items.begin(); i != end; i++, num++) {
			if (strcmp(i->spec.GetPath(), selection) == 0) {
				set_selection(num);
				break;
			}
		}
	}

	bool is_item_selectable(size_t i)
	{
		return items[i].selectable;
	}

	void item_selected(void)
	{
		parent->quit(0);
	}

	void draw_item(vector<env_item>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
	{
		y += font->get_ascent();

		int color;
		if (i->selectable) {
			color = selected ? ITEM_ACTIVE_COLOR : ITEM_COLOR;
		} else
			color = LABEL_COLOR;

		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, i->name, x + i->indent * 8, y, get_dialog_color(color), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

private:
	dialog *parent;
};

// Environment selection button
// ZZZ: added callback stuff - callback is made if user clicks on an entry in the selection dialog.
class w_env_select;
typedef void (*selection_made_callback_t)(w_env_select* inWidget);

class w_env_select : public w_select_button {
public:
	w_env_select(const char *name, const char *path, const char *m, Typecode t, dialog *d)
		: w_select_button(name, item_name, select_item_callback, NULL),
		parent(d), menu_title(m), type(t), mCallback(NULL)
	{
		set_arg(this);
		set_path(path);
	}
	~w_env_select() {}

    void    set_selection_made_callback(selection_made_callback_t inCallback) {
        mCallback = inCallback;
    }

	void set_path(const char *p)
	{
		item = p;
		item.GetName(item_name);
		set_selection(item_name);
	}

	const char *get_path(void) const
	{
		return item.GetPath();
	}

	FileSpecifier &get_file_specifier(void)
	{
		return item;
	}

private:
	void select_item(dialog *parent);
	static void select_item_callback(void *arg);

    dialog *parent;
	const char *menu_title;	// Selection menu title

	FileSpecifier item;		// File specification
	Typecode type;				// File type
	char item_name[256];	// File name (excluding directory part)

    selection_made_callback_t mCallback;
};

#endif

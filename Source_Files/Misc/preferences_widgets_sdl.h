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
#include "Plugins.h"

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;

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

		uint32 color;
		if (i->selectable) {
			color = selected ? get_theme_color(ITEM_WIDGET, ACTIVE_STATE) : get_theme_color(ITEM_WIDGET, DEFAULT_STATE);
		} else
			color = get_theme_color(LABEL_WIDGET, DEFAULT_STATE);

		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, FileSpecifier::HideExtension(i->name).c_str(), x + i->indent * 8, y, color, font, style, true);
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
w_env_select(const char *path, const char *m, Typecode t, dialog *d)
	: w_select_button(item_name, select_item_callback, NULL, true),
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
		std::string filename = item_name;
		strncpy(item_name, FileSpecifier::HideExtension(filename).c_str(), 256);
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

class w_crosshair_display : public widget {
public:
	enum {
		kSize = 80
	};

	w_crosshair_display();
	~w_crosshair_display();

	void draw(SDL_Surface *s) const;
	bool is_selectable(void) const { return false; }

	bool placeable_implemented() { return true; }

	bool is_dirty() { return true; }

private:
	SDL_Surface *surface;
};

class w_plugins : public w_list_base {
public:
	w_plugins(std::vector<Plugin>& plugins, int width, int numRows) : w_list_base(width, numRows, 0), m_plugins(plugins)
	{
		saved_min_height = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE);
		trough_rect.h = saved_min_height - get_theme_space(LIST_WIDGET, TROUGH_T_SPACE) - get_theme_space(LIST_WIDGET, TROUGH_B_SPACE);
		num_items = m_plugins.size();
		new_items();
	}

	uint16 item_height() const { return 2 * font->get_line_height() + font->get_line_height() / 2 + 2; }

protected:
	void draw_items(SDL_Surface* s) const;
	void item_selected();

private:
	std::vector<Plugin>& m_plugins;
	void draw_item(Plugins::iterator i, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const;
};

#endif

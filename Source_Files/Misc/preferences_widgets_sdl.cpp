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
 *  preferences_widgets_sdl.cpp - Preferences widgets, SDL specific
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Ripped out of preferences_sdl.cpp/preferences_sdl.h into a new
 *  file Mar 1, 2002 by Woody Zenfell, for sharing.
 */

#include    "preferences_widgets_sdl.h"
#include "Crosshairs.h"

extern bool use_lua_hud_crosshairs;

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
	if (type != _typecode_theme) {

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
				// Subdirectory, insert name as unselectable item, put items on indentation level 1
				items.push_back(env_item(base_spec, 0, false));
				indent_level = 1;
			last_base = base;
		}
		items.push_back(env_item(*i, indent_level, true));
	}

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	
	placer->dual_add(new w_title(menu_title), d);
	placer->add(new w_spacer(), true);
	w_env_list *list_w = new w_env_list(items, item.GetPath(), &d);
	placer->dual_add(list_w, d);
	placer->add(new w_spacer(), true);
	placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	d.activate_widget(list_w);
	d.set_widget_placer(placer);

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

w_crosshair_display::w_crosshair_display() : surface(0)
{
	rect.w = kSize;
	rect.y = kSize;

	saved_min_width = kSize;
	saved_min_height = kSize;

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, kSize, kSize, 16, 0x7c00, 0x03e0, 0x001f, 0);
}

w_crosshair_display::~w_crosshair_display()
{
	SDL_FreeSurface(surface);
	surface = 0;
}

void w_crosshair_display::draw(SDL_Surface *s) const
{
	SDL_FillRect(surface, 0, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));

	SDL_Rect r = { 0, 0, surface->w, surface->h };
	draw_rectangle(surface, &r, get_theme_color(DIALOG_FRAME, FRAME_COLOR));
	
	bool old_use_lua_hud_crosshairs = use_lua_hud_crosshairs;
	use_lua_hud_crosshairs = false;
	bool Old_Crosshairs_IsActive = Crosshairs_IsActive();
	Crosshairs_SetActive(true);
	Crosshairs_Render(surface);
	Crosshairs_SetActive(Old_Crosshairs_IsActive);
	use_lua_hud_crosshairs = old_use_lua_hud_crosshairs;
	
	SDL_BlitSurface(surface, 0, s, const_cast<SDL_Rect *>(&rect));
}

void w_plugins::draw_items(SDL_Surface* s) const 
{
	Plugins::iterator i = m_plugins.begin();
	int16 x = rect.x + get_theme_space(LIST_WIDGET, L_SPACE);
	int16 y = rect.y + get_theme_space(LIST_WIDGET, T_SPACE);
	uint16 width = rect.w - get_theme_space(LIST_WIDGET, L_SPACE) - get_theme_space(LIST_WIDGET, R_SPACE);
	
	for (size_t n = 0; n < top_item; ++n)
	{
		++i;
	}
	
	for (size_t n = top_item; n < top_item + MIN(shown_items, num_items); ++n, ++i, y = y + item_height())
		draw_item(i, s, x, y, width, n == selection && active);
}

void w_plugins::item_selected() 
{
	Plugin& plugin = m_plugins[get_selection()];
	plugin.enabled = !plugin.enabled;
	dirty = true;
	get_owning_dialog()->draw_dirty_widgets();
}

void w_plugins::draw_item(Plugins::iterator it, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const 
{
	y += font->get_ascent();
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	uint32 color;
	if (selected)
	{
		color = get_theme_color(ITEM_WIDGET, ACTIVE_STATE);
	} 
	else if (it->enabled && it->compatible() && it->allowed())
	{
		color = get_theme_color(ITEM_WIDGET, DEFAULT_STATE);
	}
	else
	{
		color = get_theme_color(ITEM_WIDGET, DISABLED_STATE);
	}

	std::string enabled;
	if (!it->compatible()) 
	{
		enabled = " Incompatible";
	}
	else if (!it->allowed())
	{
		enabled = " Disallowed";
	}
	else if (it->enabled)
	{
		enabled = " Enabled";
	}
	else 
	{
		enabled = " Disabled";
	}

	int right_text_width = text_width(enabled.c_str(), font, style);

	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width - right_text_width);
	std::string name_and_version = it->name + " " + it->version;
	draw_text(s, name_and_version.c_str(), x, y, color, font, style);

	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	draw_text(s, enabled.c_str(), x + width - right_text_width, y, color, font, style);

	y += font->get_ascent() + 1;
	std::string types;
	if (it->solo_lua.size()) {
		types += ", Solo Lua";
	}
	if (it->hud_lua.size()) {
		types += ", HUD";
	}
	if (it->theme.size()) {
		types += ", Theme";
	}
	if (it->shapes_patches.size()) {
		types += ", Shapes Patch";
	}
	if (it->mmls.size()) {
		types += ", MML";
	}
	if (it->stats_lua.size()) {
		types += ", Stats";
	}
	types.erase(0, 2);
	right_text_width = text_width(types.c_str(), font, style | styleItalic);
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	draw_text(s, types.c_str(), x + width - right_text_width, y, color, font, style | styleItalic);
	
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width - right_text_width);
	if (it->description.size()) {
		draw_text(s, it->description.c_str(), x, y, color, font, style);
	} else {
		draw_text(s, "No description", x, y, color, font, style);
	}
	
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}

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
 *  sdl_dialogs.cpp - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 *
 *  11 Mar 2002 (Woody Zenfell): renamed XML_FrameParser to XML_DFrameParser
 *      to resolve conflict with new animated-model frame parsing code.
 *
 *  5 Feb 2003 (Woody Zenfell): exposed draw_dirty_widgets() functionality
 *	also trying to fix fullscreen drawing problems related to flipping
 */

#include "cseries.h"
#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"

#include "shape_descriptors.h"
#include "screen_drawing.h"
#include "shell.h"
#include "screen.h"
#include "images.h"
#include "world.h"
#include "SoundManager.h"
#include "game_errors.h"
#include "Plugins.h"

// for fixing broken theme paths
#include "interface.h"
#include "preferences.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#include "OGL_Setup.h"
#include "OGL_Blitter.h"
#include "OGL_Render.h"
#endif

#include "InfoTree.h"
#include "Logging.h"

#include <map>
#include <string>

// Global variables
dialog *top_dialog = NULL;

static SDL_Surface *dialog_surface = NULL;

static SDL_Surface *default_image = NULL;

static OpenedResourceFile theme_resources;

struct dialog_image_spec_type {
	string name;
	bool scale;
};

struct theme_state
{
	std::map<int, SDL_Color> colors;
	std::map<int, dialog_image_spec_type> image_specs;
	std::map<int, SDL_Surface *> images;
};

struct theme_widget
{
	std::map<int, theme_state> states;
	font_info *font;
	TextSpec font_spec;
	bool font_set;
	std::map<int, int> spaces;

	theme_widget() : font(0), font_set(false) { }
};

static FileSpecifier theme_path;
static std::map<int, theme_widget> dialog_theme;

// Prototypes
static void shutdown_dialogs(void);
static bool load_theme(FileSpecifier &theme);
static void unload_theme(void);
static void set_theme_defaults(void);

/*
 *  Initialize dialog manager
 */

void initialize_dialogs()
{
	// Allocate surface for dialogs (this surface is needed because when
	// OpenGL is active, we can't write directly to the screen)
	dialog_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0x7c00, 0x03e0, 0x001f, 0);
	assert(dialog_surface);

	// Default image
	default_image = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 24, 0xff0000, 0x00ff00, 0x0000ff, 0);
	assert(default_image);
	uint32 transp = SDL_MapRGB(default_image->format, 0x00, 0xff, 0xff);
	SDL_FillRect(default_image, NULL, transp);
	SDL_SetColorKey(default_image, SDL_TRUE, transp);

	// Load theme from preferences, if it exists
	load_dialog_theme(true);

	atexit(shutdown_dialogs);
}


/*
 *  Shutdown dialog manager
 */

static void shutdown_dialogs(void)
{
	unload_theme();
}


/*
 *  Theme MML parser
 */

static void parse_theme_image(InfoTree root, int type, int state, int max_index)
{
	int index = -1;
	std::string name;
	if (!root.read_attr("file", name) ||
		!root.read_attr_bounded("index", index, 0, max_index))
		return;
	
	bool scale = false;
	root.read_attr("scale", scale);
	dialog_theme[type].states[state].image_specs[index].name = name;
	dialog_theme[type].states[state].image_specs[index].scale = scale;
}

static void parse_theme_images(InfoTree root, int type, int state, int num_items = 1)
{
	BOOST_FOREACH(InfoTree img, root.children_named("image"))
	{
		parse_theme_image(img, type, state, num_items - 1);
	}
}

static void parse_theme_color(InfoTree root, int type, int state, int max_index)
{
	int index = 0;
	root.read_attr_bounded("index", index, 0, max_index);
	
	float red, green, blue;
	if (!root.read_attr("red", red) ||
		!root.read_attr("green", green) ||
		!root.read_attr("blue", blue))
		return;
	
	SDL_Color color;
	color.r = uint8(PIN(255 * red + 0.5, 0, 255));
	color.g = uint8(PIN(255 * green + 0.5, 0, 255));
	color.b = uint8(PIN(255 * blue + 0.5, 0, 255));
	color.a = 0xff;
	dialog_theme[type].states[state].colors[index] = color;
}

static void parse_theme_colors(InfoTree root, int type, int state, int num_items = 1)
{
	BOOST_FOREACH(InfoTree color, root.children_named("color"))
	{
		parse_theme_color(color, type, state, num_items - 1);
	}
}

static void parse_theme_font(InfoTree root, int type)
{
	int size = -1;
	if (!root.read_attr("size", size))
		return;

	int id = kFontIDMonaco;
	bool have_id = root.read_attr("id", id);
	std::string path;
	bool have_path = root.read_attr("file", path);
	if (!have_id && !have_path)
		return;

	dialog_theme[type].font_spec.font = id;
	dialog_theme[type].font_spec.size = size;
	dialog_theme[type].font_spec.normal = path;
	
	dialog_theme[type].font_spec.style = 0;
	root.read_attr("style", dialog_theme[type].font_spec.style);
	dialog_theme[type].font_spec.adjust_height = 0;
	root.read_attr("adjust_height", dialog_theme[type].font_spec.adjust_height);
	root.read_attr("bold_file", dialog_theme[type].font_spec.bold);
	root.read_attr("italic_file", dialog_theme[type].font_spec.oblique);
	root.read_attr("bold_italic_file", dialog_theme[type].font_spec.bold_oblique);
	
	dialog_theme[type].font_set = true;
}

static void parse_theme_fonts(InfoTree root, int type)
{
	BOOST_FOREACH(InfoTree child, root.children_named("font"))
		parse_theme_font(child, type);
}

void start_parse_widget(int theme_widget)
{
	if (dialog_theme.find(theme_widget) != dialog_theme.end())
		dialog_theme[theme_widget].states.clear();
}

static void parse_default(InfoTree root)
{
	start_parse_widget(DEFAULT_WIDGET);
	parse_theme_colors(root, DEFAULT_WIDGET, DEFAULT_STATE, 3);
	parse_theme_fonts(root, DEFAULT_WIDGET);
}

static void parse_frame(InfoTree root)
{
	root.read_attr("top", dialog_theme[DIALOG_FRAME].spaces[T_SPACE]);
	root.read_attr("bottom", dialog_theme[DIALOG_FRAME].spaces[B_SPACE]);
	root.read_attr("left", dialog_theme[DIALOG_FRAME].spaces[L_SPACE]);
	root.read_attr("right", dialog_theme[DIALOG_FRAME].spaces[R_SPACE]);
	
	parse_theme_colors(root, DIALOG_FRAME, DEFAULT_STATE, 3);
	parse_theme_images(root, DIALOG_FRAME, DEFAULT_STATE, 8);
}

static void parse_title(InfoTree root)
{
	parse_theme_colors(root, TITLE_WIDGET, DEFAULT_STATE);
	parse_theme_fonts(root, TITLE_WIDGET);
}

static void parse_spacer(InfoTree root)
{
	root.read_attr("height", dialog_theme[SPACER_WIDGET].spaces[0]);
}

static void parse_button(InfoTree root)
{
	start_parse_widget(BUTTON_WIDGET);
	root.read_attr("top", dialog_theme[BUTTON_WIDGET].spaces[BUTTON_T_SPACE]);
	root.read_attr("left", dialog_theme[BUTTON_WIDGET].spaces[BUTTON_L_SPACE]);
	root.read_attr("right", dialog_theme[BUTTON_WIDGET].spaces[BUTTON_R_SPACE]);
	root.read_attr("height", dialog_theme[BUTTON_WIDGET].spaces[BUTTON_HEIGHT]);
	
	parse_theme_colors(root, BUTTON_WIDGET, DEFAULT_STATE, 3);
	parse_theme_fonts(root, BUTTON_WIDGET);
	parse_theme_images(root, BUTTON_WIDGET, DEFAULT_STATE, 3);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, BUTTON_WIDGET, ACTIVE_STATE, 3);
		parse_theme_images(child, BUTTON_WIDGET, ACTIVE_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, BUTTON_WIDGET, DISABLED_STATE, 3);
		parse_theme_images(child, BUTTON_WIDGET, DISABLED_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("pressed"))
	{
		parse_theme_colors(child, BUTTON_WIDGET, PRESSED_STATE, 3);
		parse_theme_images(child, BUTTON_WIDGET, PRESSED_STATE, 3);
	}
}

static void parse_tiny_button(InfoTree root)
{
	start_parse_widget(TINY_BUTTON);
	root.read_attr("top", dialog_theme[TINY_BUTTON].spaces[BUTTON_T_SPACE]);
	root.read_attr("left", dialog_theme[TINY_BUTTON].spaces[BUTTON_L_SPACE]);
	root.read_attr("right", dialog_theme[TINY_BUTTON].spaces[BUTTON_R_SPACE]);
	root.read_attr("height", dialog_theme[TINY_BUTTON].spaces[BUTTON_HEIGHT]);
	
	parse_theme_colors(root, TINY_BUTTON, DEFAULT_STATE, 3);
	parse_theme_fonts(root, TINY_BUTTON);
	parse_theme_images(root, TINY_BUTTON, DEFAULT_STATE, 3);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, TINY_BUTTON, ACTIVE_STATE, 3);
		parse_theme_images(child, TINY_BUTTON, ACTIVE_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, TINY_BUTTON, DISABLED_STATE, 3);
		parse_theme_images(child, TINY_BUTTON, DISABLED_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("pressed"))
	{
		parse_theme_colors(child, TINY_BUTTON, PRESSED_STATE, 3);
		parse_theme_images(child, TINY_BUTTON, PRESSED_STATE, 3);
	}
}

static void parse_hyperlink(InfoTree root)
{
	start_parse_widget(HYPERLINK_WIDGET);
	
	parse_theme_colors(root, HYPERLINK_WIDGET, DEFAULT_STATE, 3);
	parse_theme_fonts(root, HYPERLINK_WIDGET);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, HYPERLINK_WIDGET, ACTIVE_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, HYPERLINK_WIDGET, DISABLED_STATE, 3);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("pressed"))
	{
		parse_theme_colors(child, HYPERLINK_WIDGET, PRESSED_STATE, 3);
	}
}

static void parse_item(InfoTree root)
{
	start_parse_widget(ITEM_WIDGET);
	root.read_attr("space", dialog_theme[ITEM_WIDGET].spaces[0]);
	
	parse_theme_colors(root, ITEM_WIDGET, DEFAULT_STATE);
	parse_theme_fonts(root, ITEM_WIDGET);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, ITEM_WIDGET, ACTIVE_STATE);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, ITEM_WIDGET, DISABLED_STATE);
	}
}

static void parse_label(InfoTree root)
{
	start_parse_widget(LABEL_WIDGET);
	
	parse_theme_colors(root, LABEL_WIDGET, DEFAULT_STATE);
	parse_theme_fonts(root, LABEL_WIDGET);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, LABEL_WIDGET, ACTIVE_STATE);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, LABEL_WIDGET, DISABLED_STATE);
	}
}

static void parse_message(InfoTree root)
{
	start_parse_widget(MESSAGE_WIDGET);
	parse_theme_colors(root, MESSAGE_WIDGET, DEFAULT_STATE);
	parse_theme_fonts(root, MESSAGE_WIDGET);
}

static void parse_text_entry(InfoTree root)
{
	start_parse_widget(TEXT_ENTRY_WIDGET);
	
	parse_theme_colors(root, TEXT_ENTRY_WIDGET, DEFAULT_STATE);
	parse_theme_fonts(root, TEXT_ENTRY_WIDGET);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, TEXT_ENTRY_WIDGET, ACTIVE_STATE);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, TEXT_ENTRY_WIDGET, DISABLED_STATE);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("cursor"))
	{
		parse_theme_colors(child, TEXT_ENTRY_WIDGET, CURSOR_STATE);
	}
}

static void parse_chat_entry(InfoTree root)
{
	start_parse_widget(CHAT_ENTRY);
	root.read_attr("name_width", dialog_theme[CHAT_ENTRY].spaces[0]);
	
	parse_theme_colors(root, CHAT_ENTRY, DEFAULT_STATE);
	parse_theme_fonts(root, CHAT_ENTRY);
}

static void parse_list(InfoTree root)
{
	start_parse_widget(LIST_WIDGET);
	root.read_attr("top", dialog_theme[LIST_WIDGET].spaces[T_SPACE]);
	root.read_attr("bottom", dialog_theme[LIST_WIDGET].spaces[B_SPACE]);
	root.read_attr("left", dialog_theme[LIST_WIDGET].spaces[L_SPACE]);
	root.read_attr("right", dialog_theme[LIST_WIDGET].spaces[R_SPACE]);
	
	parse_theme_colors(root, LIST_WIDGET, DEFAULT_STATE, 3);
	parse_theme_images(root, LIST_WIDGET, DEFAULT_STATE, 8);
	
	BOOST_FOREACH(InfoTree child, root.children_named("trough"))
	{
		child.read_attr("top", dialog_theme[LIST_WIDGET].spaces[TROUGH_T_SPACE]);
		child.read_attr("bottom", dialog_theme[LIST_WIDGET].spaces[TROUGH_B_SPACE]);
		child.read_attr("right", dialog_theme[LIST_WIDGET].spaces[TROUGH_R_SPACE]);
		child.read_attr("width", dialog_theme[LIST_WIDGET].spaces[TROUGH_WIDTH]);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("thumb"))
	{
		start_parse_widget(LIST_THUMB);
		parse_theme_colors(child, LIST_THUMB, DEFAULT_STATE, 3);
		parse_theme_images(child, LIST_THUMB, DEFAULT_STATE, 5);
	}
}

static void parse_slider(InfoTree root)
{
	start_parse_widget(SLIDER_WIDGET);
	root.read_attr("top", dialog_theme[SLIDER_WIDGET].spaces[SLIDER_T_SPACE]);
	root.read_attr("left", dialog_theme[SLIDER_WIDGET].spaces[SLIDER_L_SPACE]);
	root.read_attr("right", dialog_theme[SLIDER_WIDGET].spaces[SLIDER_R_SPACE]);
	
	parse_theme_colors(root, SLIDER_WIDGET, DEFAULT_STATE, 3);
	parse_theme_images(root, SLIDER_WIDGET, DEFAULT_STATE, 3);
	
	BOOST_FOREACH(InfoTree child, root.children_named("thumb"))
	{
		start_parse_widget(SLIDER_THUMB);
		parse_theme_colors(child, SLIDER_THUMB, DEFAULT_STATE, 3);
		parse_theme_images(child, SLIDER_THUMB, DEFAULT_STATE);
	}
}

static void parse_checkbox(InfoTree root)
{
	start_parse_widget(CHECKBOX);
	root.read_attr("top", dialog_theme[CHECKBOX].spaces[BUTTON_T_SPACE]);
	root.read_attr("height", dialog_theme[CHECKBOX].spaces[BUTTON_HEIGHT]);
	
	parse_theme_fonts(root, CHECKBOX);
	parse_theme_images(root, CHECKBOX, DEFAULT_STATE, 2);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_images(child, CHECKBOX, ACTIVE_STATE, 2);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_images(child, CHECKBOX, DISABLED_STATE, 2);
	}
}

static void parse_tab(InfoTree root)
{
	start_parse_widget(TAB_WIDGET);
	root.read_attr("top", dialog_theme[TAB_WIDGET].spaces[BUTTON_T_SPACE]);
	root.read_attr("left", dialog_theme[TAB_WIDGET].spaces[BUTTON_L_SPACE]);
	root.read_attr("right", dialog_theme[TAB_WIDGET].spaces[BUTTON_R_SPACE]);
	root.read_attr("height", dialog_theme[TAB_WIDGET].spaces[BUTTON_HEIGHT]);
	root.read_attr("inner_left", dialog_theme[TAB_WIDGET].spaces[TAB_LC_SPACE]);
	root.read_attr("inner_right", dialog_theme[TAB_WIDGET].spaces[TAB_RC_SPACE]);
	
	parse_theme_colors(root, TAB_WIDGET, DEFAULT_STATE, 3);
	parse_theme_fonts(root, TAB_WIDGET);
	parse_theme_images(root, TAB_WIDGET, DEFAULT_STATE, 5);
	
	BOOST_FOREACH(InfoTree child, root.children_named("active"))
	{
		parse_theme_colors(child, TAB_WIDGET, ACTIVE_STATE, 3);
		parse_theme_images(child, TAB_WIDGET, ACTIVE_STATE, 5);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("disabled"))
	{
		parse_theme_colors(child, TAB_WIDGET, DISABLED_STATE, 3);
		parse_theme_images(child, TAB_WIDGET, DISABLED_STATE, 5);
	}
	BOOST_FOREACH(InfoTree child, root.children_named("pressed"))
	{
		parse_theme_colors(child, TAB_WIDGET, PRESSED_STATE, 3);
		parse_theme_images(child, TAB_WIDGET, PRESSED_STATE, 5);
	}
}

static void parse_metaserver(InfoTree root)
{
	start_parse_widget(METASERVER_WIDGETS);
	BOOST_FOREACH(InfoTree child, root.children_named("games"))
	{
		start_parse_widget(METASERVER_GAMES);
		child.read_attr("entries", dialog_theme[METASERVER_GAMES].spaces[w_games_in_room::GAME_ENTRIES]);
		child.read_attr("spacing", dialog_theme[METASERVER_GAMES].spaces[w_games_in_room::GAME_SPACING]);
		
		parse_theme_colors(child, METASERVER_GAMES, w_games_in_room::GAME, 3);
		parse_theme_fonts(child, METASERVER_GAMES);
		
		BOOST_FOREACH(InfoTree gtype, child.children_named("selected"))
		{
			parse_theme_colors(gtype, METASERVER_GAMES, w_games_in_room::SELECTED_GAME, 3);
		}
		BOOST_FOREACH(InfoTree gtype, child.children_named("running"))
		{
			parse_theme_colors(gtype, METASERVER_GAMES, w_games_in_room::RUNNING_GAME, 3);
			BOOST_FOREACH(InfoTree stype, gtype.children_named("selected"))
			{
				parse_theme_colors(stype, METASERVER_GAMES, w_games_in_room::SELECTED_RUNNING_GAME, 3);
			}
		}
		BOOST_FOREACH(InfoTree gtype, child.children_named("incompatible"))
		{
			parse_theme_colors(gtype, METASERVER_GAMES, w_games_in_room::INCOMPATIBLE_GAME, 3);
			BOOST_FOREACH(InfoTree stype, gtype.children_named("selected"))
			{
				parse_theme_colors(stype, METASERVER_GAMES, w_games_in_room::SELECTED_INCOMPATIBLE_GAME, 3);
			}
		}
	}
	BOOST_FOREACH(InfoTree child, root.children_named("players"))
	{
		start_parse_widget(METASERVER_PLAYERS);
		child.read_attr("lines", dialog_theme[METASERVER_PLAYERS].spaces[0]);
		
		parse_theme_fonts(child, METASERVER_PLAYERS);
	}
}

static bool parse_theme_file(FileSpecifier& theme_mml)
{
	if (!theme_mml.Exists())
		return false;
	bool success = false;
	try {
		InfoTree root = InfoTree::load_xml(theme_mml).get_child("marathon.theme");
		
		BOOST_FOREACH(InfoTree child, root.children_named("default"))
			parse_default(child);
		BOOST_FOREACH(InfoTree child, root.children_named("frame"))
			parse_frame(child);
		BOOST_FOREACH(InfoTree child, root.children_named("title"))
			parse_title(child);
		BOOST_FOREACH(InfoTree child, root.children_named("spacer"))
			parse_spacer(child);
		BOOST_FOREACH(InfoTree child, root.children_named("button"))
			parse_button(child);
		BOOST_FOREACH(InfoTree child, root.children_named("tiny_button"))
			parse_tiny_button(child);
		BOOST_FOREACH(InfoTree child, root.children_named("hyperlink"))
			parse_hyperlink(child);
		BOOST_FOREACH(InfoTree child, root.children_named("item"))
			parse_item(child);
		BOOST_FOREACH(InfoTree child, root.children_named("label"))
			parse_label(child);
		BOOST_FOREACH(InfoTree child, root.children_named("message"))
			parse_message(child);
		BOOST_FOREACH(InfoTree child, root.children_named("text_entry"))
			parse_text_entry(child);
		BOOST_FOREACH(InfoTree child, root.children_named("chat_entry"))
			parse_chat_entry(child);
		BOOST_FOREACH(InfoTree child, root.children_named("list"))
			parse_list(child);
		BOOST_FOREACH(InfoTree child, root.children_named("slider"))
			parse_slider(child);
		BOOST_FOREACH(InfoTree child, root.children_named("checkbox"))
			parse_checkbox(child);
		BOOST_FOREACH(InfoTree child, root.children_named("tab"))
			parse_tab(child);
		BOOST_FOREACH(InfoTree child, root.children_named("metaserver"))
			parse_metaserver(child);
		
		success = true;
	} catch (InfoTree::parse_error e) {
		logError("error parsing %s: %s", theme_mml.GetPath(), e.what());
	} catch (InfoTree::path_error e) {
		logError("error parsing %s: %s", theme_mml.GetPath(), e.what());
	} catch (InfoTree::data_error e) {
		logError("error parsing %s: %s", theme_mml.GetPath(), e.what());
	} catch (InfoTree::unexpected_error e) {
		logError("error parsing %s: %s", theme_mml.GetPath(), e.what());
	}
	return success;
}

/*
 *  Load theme
 */

extern vector<DirectorySpecifier> data_search_path;

bool load_dialog_theme(bool force_reload)
{
	FileSpecifier new_theme;
	const Plugin* theme_plugin = Plugins::instance()->find_theme();
	if (theme_plugin)
	{
		new_theme = theme_plugin->directory + theme_plugin->theme;
	}
	else
	{
		get_default_theme_spec(new_theme);
	}
	if (force_reload || new_theme != theme_path)
	{
		return load_theme(new_theme);
	}
	return false;
}

bool load_theme(FileSpecifier &theme)
{
	// Unload previous theme
	unload_theme();

	// Set defaults, the theme overrides these
	set_theme_defaults();

	// Parse theme MML script
	FileSpecifier theme_mml = theme + "theme2.mml";
	bool success = parse_theme_file(theme_mml);
	if (success)
	{
		theme_path = theme;

		// Open resource file
		FileSpecifier theme_rsrc = theme + "resources";
		theme_rsrc.Open(theme_resources);
	}
	clear_game_error();

	// Load fonts
	if (success)
		data_search_path.insert(data_search_path.begin(), theme);
	for (std::map<int, theme_widget>::iterator i = dialog_theme.begin(); i != dialog_theme.end(); ++i)
	{
		if (i->second.font_set) {
			i->second.font = load_font(i->second.font_spec);
			if (!i->second.font) {
				TextSpec fallback_spec = { -1, i->second.font_spec.style, i->second.font_spec.size, 0, "mono" };
				i->second.font = load_font(fallback_spec);
			}
		} else
			i->second.font = 0;
	}
	if (success)
		data_search_path.erase(data_search_path.begin());

	// Load images
	for (std::map<int, theme_widget>::iterator i = dialog_theme.begin(); i != dialog_theme.end(); ++i)
	{
		for (std::map<int, theme_state>::iterator j = i->second.states.begin(); j != i->second.states.end(); ++j)
		{
			for (std::map<int, dialog_image_spec_type>::iterator k = j->second.image_specs.begin(); k != j->second.image_specs.end(); ++k)
			{
				FileSpecifier file = theme + k->second.name;
				OpenedFile of;
				if (file.Open(of))
				{
					SDL_Surface *s = SDL_LoadBMP_RW(of.GetRWops(), 0);
					if (s) 
						SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0x00, 0xff, 0xff));
					j->second.images[k->first] = s;
				}
			}
		}
	}

	return success;
}


/*
 *  Set theme default values
 */

static inline SDL_Color make_color(uint8 r, uint8 g, uint8 b)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 0xff;
	return c;
}

static void set_theme_defaults(void)
{
	// new theme defaults
	static const TextSpec default_font_spec = {kFontIDMonaco, styleNormal, 12, 0, "mono"};
	dialog_theme[DEFAULT_WIDGET].font_spec = default_font_spec;
	dialog_theme[DEFAULT_WIDGET].font_set = true;

	dialog_theme[DEFAULT_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xff, 0xff);
	dialog_theme[DEFAULT_WIDGET].states[DEFAULT_STATE].colors[BACKGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[DEFAULT_WIDGET].states[DEFAULT_STATE].colors[FRAME_COLOR] = make_color(0x3f, 0x3f, 0x3f);

	dialog_theme[TITLE_WIDGET].font_spec = dialog_theme[DEFAULT_WIDGET].font_spec;
	dialog_theme[TITLE_WIDGET].font_spec.size = 24;
	dialog_theme[TITLE_WIDGET].font_set = true;

	dialog_theme[DIALOG_FRAME].spaces[T_SPACE] = 8;
	dialog_theme[DIALOG_FRAME].spaces[L_SPACE] = 8;
	dialog_theme[DIALOG_FRAME].spaces[R_SPACE] = 8;
	dialog_theme[DIALOG_FRAME].spaces[B_SPACE] = 8;

	dialog_theme[SPACER_WIDGET].spaces[0] = 8;

	dialog_theme[LABEL_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0xff, 0x0);
	dialog_theme[LABEL_WIDGET].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x9b, 0x0);
	dialog_theme[LABEL_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);

	dialog_theme[ITEM_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0xff, 0x0);
	dialog_theme[ITEM_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[ITEM_WIDGET].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x9b, 0x0);
	dialog_theme[ITEM_WIDGET].spaces[0] = 16;

	dialog_theme[TEXT_ENTRY_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0xff, 0x0);
	dialog_theme[TEXT_ENTRY_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[TEXT_ENTRY_WIDGET].states[CURSOR_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[TEXT_ENTRY_WIDGET].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x9b, 0x0);

	dialog_theme[BUTTON_WIDGET].spaces[BUTTON_T_SPACE] = 4;
	dialog_theme[BUTTON_WIDGET].spaces[BUTTON_L_SPACE] = 4;
	dialog_theme[BUTTON_WIDGET].spaces[BUTTON_R_SPACE] = 4;
	dialog_theme[BUTTON_WIDGET].spaces[BUTTON_HEIGHT] = 24;
	dialog_theme[BUTTON_WIDGET].states[DEFAULT_STATE].colors[BACKGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[BUTTON_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[BUTTON_WIDGET].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x7f, 0x7f, 0x7f);

	dialog_theme[BUTTON_WIDGET].states[PRESSED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[BUTTON_WIDGET].states[PRESSED_STATE].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

	dialog_theme[BUTTON_WIDGET].font_spec = dialog_theme[DEFAULT_WIDGET].font_spec;
	dialog_theme[BUTTON_WIDGET].font_spec.size = 14;
	dialog_theme[BUTTON_WIDGET].font_set = true;

	dialog_theme[SLIDER_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[SLIDER_THUMB].states[DEFAULT_STATE].colors[FRAME_COLOR] = make_color(0x0, 0xff, 0x0);
	dialog_theme[SLIDER_THUMB].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);

	dialog_theme[LIST_THUMB].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[LIST_THUMB].states[DEFAULT_STATE].colors[FRAME_COLOR] = make_color(0x0, 0xff, 0x0);
	dialog_theme[LIST_WIDGET].spaces[T_SPACE] = 2;
	dialog_theme[LIST_WIDGET].spaces[L_SPACE] = 2;
	dialog_theme[LIST_WIDGET].spaces[R_SPACE] = 14;
	dialog_theme[LIST_WIDGET].spaces[B_SPACE] = 2;
	dialog_theme[LIST_WIDGET].spaces[TROUGH_R_SPACE] = 12;
	dialog_theme[LIST_WIDGET].spaces[TROUGH_WIDTH] = 12;

	dialog_theme[TINY_BUTTON].states[DEFAULT_STATE].colors[BACKGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[TINY_BUTTON].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[TINY_BUTTON].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x7f, 0x7f, 0x7f);
	dialog_theme[TINY_BUTTON].spaces[BUTTON_T_SPACE] = 2;
	dialog_theme[TINY_BUTTON].spaces[BUTTON_L_SPACE] = 2;
	dialog_theme[TINY_BUTTON].spaces[BUTTON_R_SPACE] = 2;
	dialog_theme[TINY_BUTTON].spaces[BUTTON_HEIGHT] = 18;
	dialog_theme[TINY_BUTTON].states[PRESSED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[TINY_BUTTON].states[PRESSED_STATE].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

	dialog_theme[HYPERLINK_WIDGET].font_spec = dialog_theme[DEFAULT_WIDGET].font_spec;
	dialog_theme[HYPERLINK_WIDGET].font_spec.style = 4;
	dialog_theme[HYPERLINK_WIDGET].font_set = true;
	dialog_theme[HYPERLINK_WIDGET].states[DEFAULT_STATE].colors[FOREGROUND_COLOR] = make_color(0x7f, 0x7f, 0xff);
	dialog_theme[HYPERLINK_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[HYPERLINK_WIDGET].states[DISABLED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x9b, 0x0);
	dialog_theme[HYPERLINK_WIDGET].states[PRESSED_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

	dialog_theme[CHECKBOX].font_spec = dialog_theme[DEFAULT_WIDGET].font_spec;
	dialog_theme[CHECKBOX].font_spec.size = 22;
	dialog_theme[CHECKBOX].font_set = true;
	dialog_theme[CHECKBOX].spaces[BUTTON_T_SPACE] = 13;
	dialog_theme[CHECKBOX].spaces[BUTTON_HEIGHT] = 15;
	
	dialog_theme[TAB_WIDGET].spaces[BUTTON_T_SPACE] = 4;
	dialog_theme[TAB_WIDGET].spaces[BUTTON_L_SPACE] = 4;
	dialog_theme[TAB_WIDGET].spaces[BUTTON_R_SPACE] = 4;
	dialog_theme[TAB_WIDGET].spaces[BUTTON_HEIGHT] = 24;
	dialog_theme[TAB_WIDGET].spaces[TAB_LC_SPACE] = 4;
	dialog_theme[TAB_WIDGET].spaces[TAB_RC_SPACE] = 4;
	dialog_theme[TAB_WIDGET].font_spec = dialog_theme[DEFAULT_WIDGET].font_spec;
	dialog_theme[TAB_WIDGET].font_spec.size = 14;
	dialog_theme[TAB_WIDGET].font_set = true;
	dialog_theme[TAB_WIDGET].states[DEFAULT_STATE].colors[BACKGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[TAB_WIDGET].states[ACTIVE_STATE].colors[FOREGROUND_COLOR] = make_color(0xff, 0xe7, 0x0);
	dialog_theme[TAB_WIDGET].states[PRESSED_STATE].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[TAB_WIDGET].states[PRESSED_STATE].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

	dialog_theme[CHAT_ENTRY].spaces[0] = 100;

	dialog_theme[METASERVER_PLAYERS].spaces[0] = 8;

	dialog_theme[METASERVER_GAMES].spaces[w_games_in_room::GAME_SPACING] = 4;
	dialog_theme[METASERVER_GAMES].spaces[w_games_in_room::GAME_ENTRIES] = 3;
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::GAME].colors[FOREGROUND_COLOR] = make_color(0xff, 0xff, 0xff);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::INCOMPATIBLE_GAME].colors[FOREGROUND_COLOR] = make_color(0x7f, 0, 0);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::RUNNING_GAME].colors[FOREGROUND_COLOR] = make_color(0x7f, 0x7f, 0x7f);

	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_GAME].colors[FOREGROUND_COLOR] = make_color(0x0, 0x0, 0x0);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_GAME].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_INCOMPATIBLE_GAME].colors[FOREGROUND_COLOR] = make_color(0x7f, 0, 0);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_INCOMPATIBLE_GAME].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_RUNNING_GAME].colors[FOREGROUND_COLOR] = make_color(0x7f, 0x7f, 0x7f);
	dialog_theme[METASERVER_GAMES].states[w_games_in_room::SELECTED_RUNNING_GAME].colors[BACKGROUND_COLOR] = make_color(0xff, 0xff, 0xff);

}

/*
 *  Unload theme
 */

static void unload_theme(void)
{
	// Unload fonts
	for (std::map<int, theme_widget>::iterator i = dialog_theme.begin(); i != dialog_theme.end(); ++i)
	{
		if (i->second.font)
		{
			unload_font(i->second.font);
			i->second.font = 0;
		}
	}
	// Free surfaces

	for (std::map<int, theme_widget>::iterator i = dialog_theme.begin(); i != dialog_theme.end(); ++i)
	{
		for (std::map<int, theme_state>::iterator j = i->second.states.begin(); j != i->second.states.end(); ++j)
		{
			for (std::map<int, SDL_Surface*>::iterator k = j->second.images.begin(); k != j->second.images.end(); ++k)
			{
				if (k->second)
				{
					SDL_FreeSurface(k->second);
					k->second = 0;
				}
			}
		}
	}

	dialog_theme.clear();
	theme_path = FileSpecifier();

	// Close resource file
	theme_resources.Close();
}


/*
 *  Get dialog font/color/image/space from theme
 */


// ZZZ: added this for convenience; taken from w_player_color::draw().
// Obviously, this color does not come from the theme.
uint32 get_dialog_player_color(size_t colorIndex) {
        SDL_Color c;
        _get_interface_color(PLAYER_COLOR_BASE_INDEX + colorIndex, &c);
        return SDL_MapRGB(dialog_surface->format, c.r, c.g, c.b);
}

font_info *get_theme_font(int widget_type, uint16 &style)
{
	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end() && i->second.font)
	{
		style = i->second.font_spec.style;
		return i->second.font;
	}
	else 
	{
		i = dialog_theme.find(DEFAULT_WIDGET);
		style = i->second.font_spec.style;
		return i->second.font;
	}
}

uint32 get_theme_color(int widget_type, int state, int which)
{
	SDL_Color c = dialog_theme[DEFAULT_WIDGET].states[DEFAULT_STATE].colors[which];

	bool found = false;
	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end())
	{
		std::map<int, theme_state>::iterator j = i->second.states.find(state);
		if (j != i->second.states.end())
		{
			std::map<int, SDL_Color>::iterator k = j->second.colors.find(which);
			if (k != j->second.colors.end())
			{
				c = k->second;
				found = true;
			}
		} 

		if (!found)
		{
			j = i->second.states.find(DEFAULT_STATE);
			if (j != i->second.states.end())
			{
				std::map<int, SDL_Color>::iterator k = j->second.colors.find(which);
				if (k != j->second.colors.end())
				{
					c = k->second;
					found = true;
				}
			}
		}
	}
	
	return SDL_MapRGB(dialog_surface->format, c.r, c.g, c.b);
}

SDL_Surface *get_theme_image(int widget_type, int state, int which, int width, int height)
{
	SDL_Surface *s = default_image;
	bool scale = false;
	bool found = false;

	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end())
	{
		std::map<int, theme_state>::iterator j = i->second.states.find(state);
		if (j != i->second.states.end())
		{
			std::map<int, SDL_Surface*>::iterator k = j->second.images.find(which);
			if (k != j->second.images.end())
			{
				s = k->second;
				scale = j->second.image_specs[k->first].scale;
				found = true;
			}
		}

		if (!found)
		{
			j = i->second.states.find(DEFAULT_STATE);
			if (j != i->second.states.end())
			{
				std::map<int, SDL_Surface*>::iterator k = j->second.images.find(which);
				if (k != j->second.images.end())
				{
					s = k->second;
					scale = j->second.image_specs[k->first].scale;
					found = true;
				}
			}
		}
	}

	// If no width and height is given, the surface is returned
	// as-is and must not be freed by the caller
	if (width == 0 && height == 0)
	{
		return s;
	}

	// Otherwise, a new tiled/rescaled surface is created which
	// must be freed by the caller
	int req_width = width ? width : s->w;
	if (req_width < 1)
		req_width = 1;
	int req_height = height ? height : s->h;
	if (req_height < 1)
		req_height = 1;
	SDL_Surface *s2 = scale ? rescale_surface(s, req_width, req_height) : tile_surface(s, req_width, req_height);
	SDL_SetColorKey(s2, SDL_TRUE, SDL_MapRGB(s2->format, 0x00, 0xff, 0xff));
	return s2;

}

bool use_theme_images(int widget_type)
{
	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end())
	{
		std::map<int, theme_state>::iterator j = i->second.states.find(DEFAULT_STATE);
		if (j != i->second.states.end())
		{
			return j->second.image_specs.size();
		}
	}

	return false;
}

bool use_theme_color(int widget_type, int which)
{
	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end())
	{
		std::map<int, theme_state>::iterator j = i->second.states.find(DEFAULT_STATE);
		if (j != i->second.states.end())
		{
			std::map<int, SDL_Color>::iterator k = j->second.colors.find(which);
			if (k != j->second.colors.end())
			{
				return true;
			}
		}
	}
	return false;
}

int get_theme_space(int widget_type, int which)
{
	std::map<int, theme_widget>::iterator i = dialog_theme.find(widget_type);
	if (i != dialog_theme.end())
	{
		std::map<int, int>::iterator j = i->second.spaces.find(which);
		if (j != i->second.spaces.end())
		{
			return j->second;
		}
	}

	return 0;
}


/*
 *  Play dialog sound
 */

int16 dialog_sound_definitions[] = {
	_snd_pattern_buffer,
	_snd_pattern_buffer,
	_snd_defender_hit,
	_snd_spht_door_obstructed,
	_snd_major_fusion_charged,
	_snd_computer_interface_page,
	_snd_computer_interface_page,
	_snd_hummer_attack,
	_snd_compiler_death
};

int16* original_dialog_sound_definitions = NULL;

int number_of_dialog_sounds() { return NUMBER_OF_DIALOG_SOUNDS; }

void play_dialog_sound(int which)
{
	if (dialog_sound_definitions[which] != NONE)
	{
		SoundManager::instance()->PlaySound(dialog_sound_definitions[which], 0, NONE);
	}
}

widget_placer::~widget_placer()
{
	for (std::vector<placeable *>::iterator it = m_owned.begin(); it != m_owned.end(); it++)
	{
		delete (*it);
	}
}

void table_placer::add(placeable *p, bool assume_ownership)
{
	if (m_add == 0)
	{
		m_table.resize(m_table.size() + 1);
		m_table[m_table.size() - 1].resize(m_columns);
	}
	m_table[m_table.size() - 1][m_add++] = p;
	if (m_add == m_columns) m_add = 0;

	if (assume_ownership) this->assume_ownership(p);
}

void table_placer::add_row(placeable *p, bool assume_ownership)
{
	assert(m_add == 0);
	m_table.resize(m_table.size() + 1);
	m_table[m_table.size() - 1].resize(1);

	m_table[m_table.size() - 1][0] = p;
	
	if (assume_ownership) this->assume_ownership(p);
}

void table_placer::dual_add(widget *w, dialog &d)
{
	add(static_cast<placeable *>(w));
	d.add(w);
}

void table_placer::dual_add_row(widget *w, dialog &d)
{
	add_row(static_cast<placeable *>(w));
	d.add(w);
}

int table_placer::min_height()
{
	int height = 0;
	for (int row = 0; row < m_table.size(); ++row)
	{
		height += row_height(row);
	}

	return height;
}

int table_placer::col_width(int col)
{
	int width = 0;
	for (int row = 0; row < m_table.size(); ++row)
	{
		if (m_table[row].size() == 1) continue;
		int min_width = m_table[row][col]->min_width();
		if (min_width > width)
			width = min_width;
	}

	if (m_col_min_widths[col] > width)
		width = m_col_min_widths[col];

	return width;
}

int table_placer::row_height(int row)
{
	int height = 0;
	for (int col = 0; col < m_table[row].size(); ++col)
	{
		int min_height = m_table[row][col]->min_height();
		if (min_height > height)
			height = min_height;
	}

	return height;
}

int table_placer::min_width()
{
	int width = 0;
	if (m_balance_widths)
	{
		for (int col = 0; col < m_columns; ++col)
		{
			int c_width = col_width(col);
			if (c_width > width)
				width = c_width;
		}

		width = width * m_columns + (m_columns - 1) * m_space;
	}
	else
	{
		for (int col = 0; col < m_columns; ++col)
		{
			width += col_width(col);
		}
		
		width += (m_columns - 1) * m_space;
	}

	for (int row = 0; row < m_table.size(); ++row)
	{
		if (m_table[row].size() == 1)
		{
			int min_width = m_table[row][0]->min_width();
			if (min_width > width) width = min_width;
		}
	}

	return width;
}
		
void table_placer::place(const SDL_Rect &r, placement_flags flags)
{
	int w = min_width();

	std::vector<int> column_widths(m_columns);

	if (m_balance_widths)
	{
		if (flags & kFill)
		{
			for (int i = 0; i < column_widths.size(); i++)
			{
				column_widths[i] = (r.w - (m_columns - 1) * m_space) / m_columns;
			}
		}
		else
		{
			int column_width = col_width(0);
			for (int i = 1; i < column_widths.size(); i++)
			{
				if (col_width(i) > column_width)
					column_width = col_width(i);
			}

			for (int i = 0; i < column_widths.size(); i++)
			{
				column_widths[i] = column_width;
			}
		}
	}
	else if (flags & kFill)
	{
		int pool = r.w - (m_columns - 1) * m_space;
		int columns_remaining = m_columns;
		for (int i = 0; i < m_columns; i++)
		{
			if (!(m_col_flags[i] & kFill))
			{
				columns_remaining--;
				column_widths[i] = col_width(i);
				pool -= column_widths[i];
			}
		}

		if (columns_remaining == m_columns || columns_remaining == 0)
		{
			for (int i = 0; i < column_widths.size(); i++)
			{
				column_widths[i] = col_width(i) + (pool / m_columns);
			}
		}
		else
		{
			int remaining_w = r.w / columns_remaining;
			for (int i = 0; i < m_columns; i++)
			{
				if (m_col_flags[i] & kFill)
				{
					int column_width = col_width(i);
					if (column_width > remaining_w)
					{
						pool -= column_width - remaining_w;
						column_widths[i] = column_width;
					}
				}

				if (pool < 0)
				{
					// bail!
					for (int i = 0; i < column_widths.size(); i++)
					{
						column_widths[i] = col_width(i) + (pool / m_columns);
					}
				}
				else
				{
					remaining_w = pool / columns_remaining;
					for (int i = 0; i < column_widths.size(); i++)
					{
						if (m_col_flags[i] & kFill)
						{
							int column_width = col_width(i);
							if (remaining_w > column_width)
							{
								column_widths[i] = remaining_w;
							}
							else
							{
								column_widths[i] = column_width;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < column_widths.size(); i++)
		{
			column_widths[i] = col_width(i);
		}
	}


	int y_offset = 0;
	for (int row = 0; row < m_table.size(); ++row)
	{
		bool full_row = m_table[row].size() == 1;
		int x_offset;
		if ((flags & kFill) || (flags & kAlignLeft))
		{
			x_offset = 0;
		}
		else if (flags & kAlignRight)
		{
			x_offset = r.w - w;
		}
		else
		{
			x_offset = (r.w - w) / 2;
		}

		for (int col = 0; col < m_table[row].size(); ++col)
		{
			SDL_Rect wr;
			wr.w = full_row ? w : column_widths[col];

			wr.h = row_height(row);
			wr.x = r.x + x_offset;
			wr.y = r.y + y_offset;

			m_table[row][col]->place(wr, full_row ? placeable::kDefault : m_col_flags[col]);

			x_offset += wr.w;
			x_offset += m_space;
		}

		y_offset += row_height(row);
	}
}
		
void table_placer::visible(bool visible)
{
	for (int row = 0; row < m_table.size(); ++row)
	{
		for (int col = 0; col < m_table[row].size(); ++col)
		{
			m_table[row][col]->visible(visible);
		}
	}
}

void vertical_placer::add(placeable *p, bool assume_ownership)
{
	m_widgets.push_back(p);
	m_widget_heights.push_back(p->min_height());
	m_placement_flags.push_back(m_add_flags);

	if (assume_ownership) this->assume_ownership(p);
}

void vertical_placer::dual_add(widget *w, dialog &d)
{
	add(static_cast<placeable *>(w));
	d.add(w);
}

int vertical_placer::min_height()
{
	int height = 0;
	for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); it++)
	{
		height += (*it)->min_height();
	}

	if (m_widgets.size()) 
		height += (m_widgets.size() - 1) * m_space;

	return height;
	
}

int vertical_placer::min_width()
{
	if (m_widgets.size())
	{
		int min_width = m_widgets[0]->min_width();
		for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); it++)
		{
			if ((*it)->min_width() > min_width)
				min_width = (*it)->min_width();
		}
		return std::max(min_width, m_min_width);
	}
	else
	{
		return m_min_width;
	}
}

void vertical_placer::place(const SDL_Rect &r, placement_flags flags)
{
	int y_offset = 0;
	int w = (flags & kFill) ? r.w : min_width();
	for (int i = 0; i < m_widgets.size(); i++)
	{
		SDL_Rect wr;
		if ((flags & kFill) || (flags & kAlignLeft))
		{
			wr.x = r.x;
		}
		else if (flags & kAlignRight)
		{
			wr.x = r.x + r.w - w;
		}
		else
		{
			wr.x = r.x + (r.w - w) / 2;
		}
		wr.w = w;
		wr.h = m_widgets[i]->min_height();
		wr.y = r.y + y_offset;

		m_widgets[i]->place(wr, m_placement_flags[i]);

		y_offset += wr.h;
		y_offset += m_space;
		
	}
}

void vertical_placer::visible(bool visible)
{
	for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); ++it)
	{
		(*it)->visible(visible);
	}
	widget_placer::visible(visible);
}

void horizontal_placer::add(placeable *p, bool assume_ownership)
{
	m_widgets.push_back(p);
	m_widget_widths.push_back(p->min_width());
	m_placement_flags.push_back(m_add_flags);
	
	if (assume_ownership) this->assume_ownership(p);
}

void horizontal_placer::dual_add(widget *w, dialog &d)
{
	add(static_cast<placeable *>(w));
	d.add(w);
}

int horizontal_placer::min_height()
{
	if (m_widgets.size())
	{
		int min_height = m_widgets[0]->min_height();
		for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); it++)
		{
			if ((*it)->min_height() > min_height)
 				min_height = (*it)->min_height();
		}
		return min_height;
	}
	else
	{
		return 0;
	}

}

int horizontal_placer::min_width()
{
	int width = 0;
	if (m_balance_widths)
	{
		// find the largest width
		for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); it++)
		{
			if ((*it)->min_width() > width)
				width = (*it)->min_width();
		}

		width = width * m_widgets.size();
	}
	else
	{
		for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); it++)
		{
			width += (*it)->min_width();
		}
	}

	if (m_widgets.size())
		width += (m_widgets.size() - 1) * m_space;

	return width;
}

void horizontal_placer::place(const SDL_Rect &r, placement_flags flags)
{
	int x_offset;
	if ((flags & kAlignLeft) || (flags & kFill))
	{
		x_offset = 0;
	}
	else if (flags & kAlignRight)
	{
		x_offset = r.w - min_width();
	}
	else
	{
		x_offset = (r.w - min_width()) / 2;
	}

	int h = (flags & kFill) ? r.h : min_height();
	int w = 0;

	bool fill_all_widgets = false;
	if (m_balance_widths)
	{
		if (flags & kFill)
		{
			w = r.w / m_widgets.size();
		}
		else
		{
			for (int i = 0; i < m_widgets.size(); i++)
			{
				if (m_widgets[i]->min_width() > w)
					w = m_widgets[i]->min_width();
			}
		}
	} 
	else if (flags & kFill)
	{
		int pool = r.w - (m_widgets.size() - 1) * m_space;
		int widgets_remaining = m_widgets.size();
		for (int i = 0; i < m_widgets.size(); i++)
		{
			if (!(m_placement_flags[i] & kFill))
			{
				widgets_remaining--;
				pool -= m_widgets[i]->min_width();
			}
		}
			
		if (widgets_remaining == m_widgets.size() || widgets_remaining == 0)
		{
			fill_all_widgets = true;
			w = r.w / m_widgets.size();
		}
		else
		{
			w = r.w / widgets_remaining;
			for (int i = 0; i < m_widgets.size(); i++)
			{
				if (m_placement_flags[i] & kFill)
				{
					int min_width = m_widgets[i]->min_width();
					if (min_width > w)
					{
						pool -= min_width - w;
					}
				}
			}
			
			if (pool < 0)
			{
				// bail!
				fill_all_widgets = true;
				w = r.w / m_widgets.size();
			}
			else
			{
				w = pool / widgets_remaining;
			}
		}
	}

	for (int i = 0; i < m_widgets.size(); i++)
	{
		int min_width = m_widgets[i]->min_width();

		SDL_Rect wr;
		if (m_balance_widths || fill_all_widgets || (m_placement_flags[i] & kFill && w > min_width))
		{
			wr.w = w;
		}
		else 
		{
			wr.w = min_width;
		}

		wr.h = h;
		wr.y = r.y;
		wr.x = r.x + x_offset;

		m_widgets[i]->place(wr, m_placement_flags[i]);

		x_offset += wr.w;
		x_offset += m_space;
	}
}
void horizontal_placer::visible(bool visible)
{
	for (std::vector<placeable *>::iterator it = m_widgets.begin(); it != m_widgets.end(); ++it)
	{
		(*it)->visible(visible);
	}
	widget_placer::visible(visible);
}	

void tab_placer::add(placeable *p, bool assume_ownership)
{
	if (m_tabs.size())
		p->visible(false);
	else
		p->visible(true);

	m_tabs.push_back(p);
	
	if (assume_ownership) this->assume_ownership(p);
}

void tab_placer::dual_add(widget *w, dialog& d)
{
	add(static_cast<placeable *>(w));
	d.add(w);
}

int tab_placer::min_height()
{
	int height = 0;
	for (std::vector<placeable *>::iterator it = m_tabs.begin(); it != m_tabs.end(); it++)
	{
		if ((*it)->min_height() > height) 
			height = (*it)->min_height();
	}

	return height;
}

int tab_placer::min_width()
{
	int width = 0;
	for (std::vector<placeable *>::iterator it = m_tabs.begin(); it != m_tabs.end(); it++)
	{
		if ((*it)->min_width() > width) 
			width = (*it)->min_width();
	}

	return width;
}

void tab_placer::choose_tab(int new_tab)
{
	assert(new_tab < m_tabs.size());
	
	m_tabs[m_tab]->visible(false);
	if (visible())
		m_tabs[new_tab]->visible(true);
	m_tab = new_tab;
}

void tab_placer::place(const SDL_Rect& r, placement_flags flags)
{
	int h = (flags & kFill) ? r.h : min_height();

	for (std::vector<placeable *>::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
	{
		SDL_Rect wr;
		wr.w = (*it)->min_width();
		wr.h = h;
		wr.y = r.y;
		int x_offset;
		if (flags & kAlignLeft)
			x_offset = 0;
		else if (flags & kAlignRight)
			x_offset = r.w - (*it)->min_width();
		else 
			x_offset = (r.w - (*it)->min_width()) / 2;

		wr.x = r.x + x_offset;
		
		(*it)->place(wr);
	}
}

void tab_placer::visible(bool visible)
{
	widget_placer::visible(visible);
	if (m_tabs.size())
	{
		m_tabs[m_tab]->visible(visible);
	}
}
		
/*
 *  Dialog constructor
 */

dialog::dialog() : active_widget(NULL), mouse_widget(0), active_widget_num(UNONE), done(false),
            cursor_was_visible(false), parent_dialog(NULL),
		   processing_function(NULL), placer(0), last_redraw(0)
{
}


/*
 *  Dialog destructor
 */

dialog::~dialog()
{
	// Free all widgets
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		delete *i;
		i++;
	}

	if (placer) {
		delete placer;
		placer = 0;
	}
}


/*
 *  Add widget
 */

void dialog::add(widget *w)
{
	widgets.push_back(w);
        w->set_owning_dialog(this);
}

/*
 *  Layout dialog
 */

void dialog::layout()
{
	assert(placer);

	layout_for_fullscreen = get_screen_mode()->fullscreen;

	// Layout all widgets, calculate total width and height
	SDL_Rect placer_rect;
	placer_rect.w = placer->min_width();
	placer_rect.h = placer->min_height();

	rect.w = get_theme_space(DIALOG_FRAME, L_SPACE) + placer_rect.w + get_theme_space(DIALOG_FRAME, R_SPACE);
	rect.h = get_theme_space(DIALOG_FRAME, T_SPACE) + placer_rect.h + get_theme_space(DIALOG_FRAME, B_SPACE);
	
	// Center dialog on menu surface
	int surface_w = MainScreenLogicalWidth();
	int surface_h = MainScreenLogicalHeight();
	if (MainScreenIsOpenGL())
	{
		surface_w = 640;
		surface_h = 480;
	}
	rect.x = (surface_w - rect.w) / 2;
	rect.y = (surface_h - rect.h) / 2;
	
	placer_rect.x = get_theme_space(DIALOG_FRAME, L_SPACE);
	placer_rect.y = get_theme_space(DIALOG_FRAME, T_SPACE);

	placer->place(placer_rect);
}


/*
 *  Update part of dialog on screen
 */

void dialog::update(SDL_Rect r) const
{
#ifdef HAVE_OPENGL
	if (OGL_IsActive()) {
		OGL_Blitter::BoundScreen(false);
		clear_screen(false);
		OGL_Blitter blitter;
		SDL_Rect src = { 0, 0, rect.w, rect.h };
		blitter.Load(*dialog_surface, src);
		blitter.Draw(rect);

		MainScreenSwap();
	} else 
#endif
	{
		SDL_Surface *video = MainScreenSurface();
		SDL_Rect dst_rect = rect;
		SDL_Rect src_rect = { 0, 0, rect.w, rect.h };
		SDL_BlitSurface(dialog_surface, &src_rect, video, &dst_rect);
		MainScreenUpdateRects(1, &dst_rect);

	}
}


/*
 *  Draw dialog
 */

void dialog::draw_widget(widget *w, bool do_update) const
{
	// Clear and redraw widget
	SDL_FillRect(dialog_surface, &w->rect, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));
	w->draw(dialog_surface);
	w->dirty = false;

	// Blit to screen
	if (do_update)
		update(w->rect);
}

static void draw_frame_image(SDL_Surface *s, int x, int y)
{
	SDL_Rect r = {x, y, s->w, s->h};
	SDL_BlitSurface(s, NULL, dialog_surface, &r);
}

void dialog::draw(void)
{
	if (get_screen_mode()->fullscreen != layout_for_fullscreen)
		layout();

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));

	if (use_theme_images(DIALOG_FRAME))
	{
		// Draw frame
		draw_frame_image(frame_tl, 0, 0);
		draw_frame_image(frame_t, frame_tl->w, 0);
		draw_frame_image(frame_tr, frame_tl->w + frame_t->w, 0);
		draw_frame_image(frame_l, 0, frame_tl->h);
		draw_frame_image(frame_r, rect.w - frame_r->w, frame_tr->h);
		draw_frame_image(frame_bl, 0, frame_tl->h + frame_l->h);
		draw_frame_image(frame_b, frame_bl->w, rect.h - frame_b->h);
		draw_frame_image(frame_br, frame_bl->w + frame_b->w, frame_tr->h + frame_r->h);
	}
	else
	{
		uint32 pixel = get_theme_color(DIALOG_FRAME, DEFAULT_STATE, FRAME_COLOR);
		SDL_Rect r = {0, 0, rect.w, rect.h};
		draw_rectangle(dialog_surface, &r, pixel);
	}

	// Draw all visible widgets
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		if ((*i)->visible())
			draw_widget(*i, false);
		i++;
	}

	// Blit to screen
	SDL_Rect r = {0, 0, rect.w, rect.h};
	update(r);
}

void
dialog::draw_dirty_widgets() const
{
	if (top_dialog != this) return;
        for (unsigned i=0; i<widgets.size(); i++)
		if (widgets[i]->is_dirty())
			if (widgets[i]->visible())
				draw_widget(widgets[i]);
	
}       

/*
 *  Deactivate currently active widget
 */

void dialog::deactivate_currently_active_widget()
{
	if (active_widget) {
		active_widget->set_active(false);
		if (active_widget->associated_label)
			active_widget->associated_label->set_active(false);

        active_widget = NULL;
        active_widget_num = UNONE;
	}
}


/*
 *  Activate widget
 */

void dialog::activate_widget(widget *w)
{
	for (size_t i = 0; i < widgets.size(); i++)
	{
		if (widgets[i] == w)
		{
			activate_widget(i);
			return;
		}
	}
}

void dialog::activate_widget(size_t num)
{
	if (num == active_widget_num)
		return;
	// BUG: may crash if num==UNONE or NONE
	if (!widgets[num]->is_selectable())
		return;

	// Deactivate previously active widget
	deactivate_currently_active_widget();
	
	// Activate new widget
	w_label *label = dynamic_cast<w_label *>(widgets[num]);
	if (label && label->associated_widget)
	{
		if (widgets[num + 1 % widgets.size()] == label->associated_widget)
		{
			active_widget = label->associated_widget;
			active_widget_num = num + 1 % widgets.size();
		}
		else if (widgets[num - 1 % widgets.size()] == label->associated_widget)
		{
			active_widget = label->associated_widget;
			active_widget_num = num - 1 % widgets.size();
		}
		else
			// labels must be placed immediately before or
			// after their associated widgets!
			assert(false);
	}
	else
	{
		active_widget = widgets[num];
		active_widget_num = num;
	}
			

	active_widget->set_active(true);
	if (active_widget->associated_label)
		active_widget->associated_label->set_active(true);
}


/*
 *  Activate first selectable widget (don't draw)
 */

void dialog::activate_first_widget(void)
{
	for (size_t i=0; i<widgets.size(); i++) {
		if (widgets[i]->is_selectable() && widgets[i]->visible()) {
			activate_widget(i);
			break;
		}
	}
}


/*
 *  Activate next/previous selectable widget
 */

void dialog::activate_next_widget(void)
{
	if (!active_widget)
	{
		activate_first_widget();
		return;
	}
	size_t i = active_widget_num;
	// BUG: infinate loop if active_widget_num == UNONE or NONE
	do {
		i++;
		if (i >= int(widgets.size()))
			i = 0;
	} while ((!(widgets[i]->is_selectable() && widgets[i]->visible()) || (widgets[i]->associated_label == widgets[active_widget_num] || widgets[active_widget_num]->associated_label == widgets[i])) && i != active_widget_num);

    // Either widgets[i] is selectable, or i == active_widget_num (in which case we wrapped all the way around)
	if (widgets[i]->is_selectable() && widgets[i]->visible())
		activate_widget(i);
	else
		deactivate_currently_active_widget();
}

void dialog::activate_prev_widget(void)
{
	if (!active_widget)
	{
		activate_first_widget();
	}
	
	size_t i = active_widget_num;
	// BUG: infinate loop if active_widget_num == UNONE or NONE
	do {
		if (i == 0)
			i = widgets.size() - 1;
		else
			i--;
	} while ((!(widgets[i]->is_selectable() && widgets[i]->visible()) || (widgets[i]->associated_label == widgets[active_widget_num] || widgets[active_widget_num]->associated_label == widgets[i])) && i != active_widget_num);

    // Either widgets[i] is selectable, or i == active_widget_num (in which case we wrapped all the way around)
	if (widgets[i]->is_selectable() && widgets[i]->visible())
		activate_widget(i);
	else
		deactivate_currently_active_widget();
}


/*
 *  Find widget given video surface coordinates (<0 = none found)
 */

int dialog::find_widget(int x, int y)
{
	// Transform to dialog coordinates
	x -= rect.x;
	y -= rect.y;

	// Find widget
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	int num = 0;
	while (i != end) {
		widget *w = *i;
		if ((*i)->visible())
			if (x >= w->rect.x && y >= w->rect.y && x < w->rect.x + w->rect.w && y < w->rect.y + w->rect.h)
				return num;
		i++; num++;
	}
	return -1;
}


/*
 *  Find widget by its numeric ID
 */

widget *dialog::get_widget_by_id(short inID) const
{
	// Find first matching widget
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		widget *w = *i;
		if (w->get_identifier() == inID)
			return w;
		i++;
	}
	return NULL;
}


/*
 *  Handle event
 */

void dialog::event(SDL_Event &e)
{

  bool handled = false;
  // handle events we do not want widgets to see or modify
  switch (e.type) {
  case SDL_KEYDOWN:
    
    if (e.key.keysym.sym == SDLK_RETURN
	&& ((e.key.keysym.mod & KMOD_ALT) || (e.key.keysym.mod & KMOD_GUI))) {
      toggle_fullscreen(!(get_screen_mode()->fullscreen));
      draw();
      handled = true;
    }
    break;
  case SDL_WINDOWEVENT:
    if (e.window.event == SDL_WINDOWEVENT_EXPOSED) {
		draw();
		handled = true;
	}
    break;
  }
  
  if (!handled) {
	  // First pass event to active widget (which may modify it)
	  if (active_widget)
		  active_widget->event(e);

	  // handle mouse events
	  if (e.type == SDL_MOUSEMOTION)
	  {
		  int x = e.motion.x, y = e.motion.y;
#ifdef HAVE_OPENGL
		  if (OGL_IsActive())
			  OGL_Blitter::WindowToScreen(x, y);
#endif
		  widget *target = 0;
		  if (mouse_widget)
			  target = mouse_widget;
		  else
		  {	  
			  int num = find_widget(x, y);
			  if (num >= 0)
			  {
				  assert(num == (size_t) num);
				  target = widgets[num];
			  }
		  }

		  if (target)
		  {
			  target->event(e);
			  target->mouse_move(x - rect.x - target->rect.x, y - rect.y - target->rect.y);
		  }
	  }
	  else if (e.type == SDL_MOUSEBUTTONDOWN)
	  {
		  int x = e.button.x, y = e.button.y;
#ifdef HAVE_OPENGL
		  if (OGL_IsActive())
			  OGL_Blitter::WindowToScreen(x, y);
#endif
		  int num = find_widget(x, y);
		  if (num >= 0)
		  {
			  assert(num == (size_t) num);
			  mouse_widget = widgets[num];
			  mouse_widget->event(e);
			  if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT)
				  mouse_widget->mouse_down(x - rect.x - mouse_widget->rect.x, y - rect.y - mouse_widget->rect.y);
		  }
	  }
	  else if (e.type == SDL_MOUSEBUTTONUP)
	  {
		  if (mouse_widget)
		  {
			  mouse_widget->event(e);
			  if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT)
			  {
				  int x = e.button.x, y = e.button.y;
#ifdef HAVE_OPENGL
				  if (OGL_IsActive())
					  OGL_Blitter::WindowToScreen(x, y);
#endif
				  mouse_widget->mouse_up(x - rect.x - mouse_widget->rect.x, y - rect.y - mouse_widget->rect.y);
			  }
			  
			  mouse_widget = 0;
		  }
	  }
	  else if (e.type == SDL_KEYDOWN)
	  {
		  switch (e.key.keysym.sym) {
		  case SDLK_ESCAPE:		// ESC = Exit dialog
			  quit(-1);
			  break;
		  case SDLK_UP:			// Up = Activate previous widget
		  case SDLK_LEFT:
			  activate_prev_widget();
			  break;
		  case SDLK_DOWN:			// Down = Activate next widget
		  case SDLK_RIGHT:
			  activate_next_widget();
			  break;
		  case SDLK_TAB:
			  if (e.key.keysym.mod & KMOD_SHIFT)
				  activate_prev_widget();
			  else
				  activate_next_widget();
			  break;
		  case SDLK_RETURN: 		// Return = Action on widget
			  if (active_widget) active_widget->click(0, 0);
			  break;
		  case SDLK_F9:			// F9 = Screen dump
			  dump_screen();
			  break;
		  default:
			  break;
		  }
	  }
	  else if (e.type == SDL_CONTROLLERBUTTONDOWN)
	  {
		  switch (e.cbutton.button) {
			  case SDL_CONTROLLER_BUTTON_B:
				  quit(-1);
				  break;
			  case SDL_CONTROLLER_BUTTON_DPAD_UP:
			  case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				  activate_prev_widget();
				  break;
			  case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				  activate_next_widget();
				  break;
			  case SDL_CONTROLLER_BUTTON_A:
				  if (active_widget) active_widget->click(0, 0);
				  break;
			  default:
				  break;
		  }
	  }
	  else if (e.type == SDL_QUIT)
	  {
		// Quit requested
			exit(0);
	  }
  }
}


/*
 *  Run dialog modally, returns result code (0 = ok, -1 = cancel)
 */

int dialog::run(bool intro_exit_sounds)
{
	// Put dialog on screen
	start(intro_exit_sounds);

	// Run dialog loop
	while (!done) {
		// Process events
		process_events();
		if (done)
			break;

		if (machine_tick_count() > last_redraw + TICKS_PER_SECOND / 30)
		{
			draw_dirty_widgets();
			last_redraw = machine_tick_count();
		}
        
		// Run custom processing function
		if (processing_function)
			processing_function(this);

		// Give time to system
		global_idle_proc();
		yield();
	}

	// Remove dialog from screen
	return finish(intro_exit_sounds);
}


/*
 *  Put dialog on screen
 */

void dialog::start(bool play_sound)
{
	// Make sure nobody tries re-entrancy with us
	assert(!done);

	initial_text_input = SDL_IsTextInputActive();

	// Set new active dialog
	parent_dialog = top_dialog;
	top_dialog = this;

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));

	// Activate first widget
//	activate_first_widget();

	// Layout dialog
	layout();

	// Get frame images
	frame_tl = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, TL_IMAGE);
	frame_tr = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, TR_IMAGE);
	frame_bl = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, BL_IMAGE);
	frame_br = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, BR_IMAGE);
	frame_t = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, T_IMAGE, rect.w - frame_tl->w - frame_tr->w, 0);
	frame_l = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, L_IMAGE, 0, rect.h - frame_tl->h - frame_bl->h);
	frame_r = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, R_IMAGE, 0, rect.h - frame_tr->h - frame_br->h);
	frame_b = get_theme_image(DIALOG_FRAME, DEFAULT_STATE, B_IMAGE, rect.w - frame_bl->w - frame_br->w, 0);

#if (defined(HAVE_OPENGL) && defined(OPENGL_DOESNT_COPY_ON_SWAP))
	if (OGL_IsActive()) {
        // blank both buffers to avoid flickering
        clear_screen();
	}
#endif

	// Draw dialog
	draw();

	// Show cursor
	cursor_was_visible = (SDL_ShowCursor(SDL_ENABLE) == SDL_ENABLE);

	// Welcome sound
	if (play_sound)
		play_dialog_sound(DIALOG_INTRO_SOUND);

	// Prepare for dialog event loop
	result = 0;
	done = false;
}


/*
 *  Process pending dialog events
 */

bool dialog::process_events()
{
	while (!done) {
		SDL_Event e;
		if (SDL_PollEvent(&e))
			event(e);
		else
			break;
	}

	return done;
}


/*
 *  Remove dialog from screen
 */

int dialog::finish(bool play_sound)
{
	if (initial_text_input)
	{
		SDL_StartTextInput();
	}
	else
	{
		SDL_StopTextInput();
	}

	// Farewell sound
	if (play_sound)
		play_dialog_sound(result == 0 ? DIALOG_OK_SOUND : DIALOG_CANCEL_SOUND);

	// Hide cursor
	if (!cursor_was_visible)
		SDL_ShowCursor(false);

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));

#ifdef HAVE_OPENGL
	if (OGL_IsActive()) {
        glColor4f(0, 0, 0, 1);
#ifdef OPENGL_DOESNT_COPY_ON_SWAP
        for (int i = 0; i < 2; i++)  // execute for both buffers
#endif
        {
			OGL_RenderRect(rect);
            MainScreenSwap();
        }
	} else
#endif 
	{

		// Erase dialog from screen
		SDL_Surface *video = MainScreenSurface();
		SDL_FillRect(video, &rect, SDL_MapRGB(video->format, 0, 0, 0));
		MainScreenUpdateRects(1, &rect);
	}

	// Free frame images
	if (frame_t) SDL_FreeSurface(frame_t);
	if (frame_l) SDL_FreeSurface(frame_l);
	if (frame_r) SDL_FreeSurface(frame_r);
	if (frame_b) SDL_FreeSurface(frame_b);

	// Restore active dialog
	top_dialog = parent_dialog;
	parent_dialog = NULL;
	if (top_dialog) {
		clear_screen();
		top_dialog->draw();
	}
        
	// Allow dialog to be run again later
	done = false;

	return result;
}


/*
 *  Quit dialog, return result
 */

void dialog::quit(int r)
{
	result = r;
	done = true;
}


/*
 *  Standard callback functions
 */

void dialog_ok(void *arg)
{
	dialog *d = (dialog *)arg;
	d->quit(0);
}

void dialog_cancel(void *arg)
{
	dialog *d = (dialog *)arg;
	d->quit(-1);
}

// ZZZ: commonly-used callback for text entry enter_pressed_callback
void dialog_try_ok(w_text_entry* text_entry) {
    w_button* ok_button = dynamic_cast<w_button*>(text_entry->get_owning_dialog()->get_widget_by_id(iOK));
    
    // This is arguably a violation of the sdl_dialog/sdl_widget standard behavior since
    // ok_button is clicked without first becoming the active widget.  With the current
    // implementation of w_button::click, should not be a problem...
    if(ok_button != NULL)
        ok_button->click(0,0);
}

// ZZZ: commonly-used callback to enable/disable "OK" based on whether a text_entry has data
void dialog_disable_ok_if_empty(w_text_entry* inTextEntry) {
    modify_control_enabled(inTextEntry->get_owning_dialog(), iOK,
        (inTextEntry->get_text()[0] == 0) ? CONTROL_INACTIVE : CONTROL_ACTIVE);
}

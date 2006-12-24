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
#include "mysound.h"
#include "game_errors.h"

#ifdef HAVE_OPENGL
#include "SDL_opengl.h"
#include "OGL_Setup.h"
#endif

#include "XML_Loader_SDL.h"
#include "XML_ParseTreeRoot.h"

#include <map>

#ifdef __MVCPP__
#include <string>
#endif

// Global variables
dialog *top_dialog = NULL;

static SDL_Surface *dialog_surface = NULL;

static sdl_font_info *default_font = NULL;
static SDL_Surface *default_image = NULL;

static OpenedResourceFile theme_resources;

static TextSpec dialog_font_spec[NUM_DIALOG_FONTS];
static sdl_font_info *dialog_font[NUM_DIALOG_FONTS];
static SDL_Color dialog_color[NUM_DIALOG_COLORS];
static uint16 dialog_space[NUM_DIALOG_SPACES];

static struct dialog_image_spec_type {
	string name;
	bool scale;
} dialog_image_spec[NUM_DIALOG_IMAGES];

static SDL_Surface *dialog_image[NUM_DIALOG_IMAGES];

bool dialog::sKeyRepeatActive = false;

// Prototypes
static void shutdown_dialogs(void);
static void unload_theme(void);
static void set_theme_defaults(void);


/*
 *  Initialize dialog manager
 */

void initialize_dialogs(FileSpecifier &theme)
{
	// Allocate surface for dialogs (this surface is needed because when
	// OpenGL is active, we can't write directly to the screen)
	dialog_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0x7c00, 0x03e0, 0x001f, 0);
	assert(dialog_surface);

	// Default font and image
	static const TextSpec default_font_spec = {kFontIDMonaco, styleNormal, 12};
	default_font = load_font(default_font_spec);
	assert(default_font);
	default_image = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 24, 0xff0000, 0x00ff00, 0x0000ff, 0);
	assert(default_image);
	uint32 transp = SDL_MapRGB(default_image->format, 0x00, 0xff, 0xff);
	SDL_FillRect(default_image, NULL, transp);
	SDL_SetColorKey(default_image, SDL_SRCCOLORKEY, transp);

	// Load theme from preferences
	load_theme(theme);

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

class XML_ImageParser : public XML_ElementParser {
public:
	XML_ImageParser(int base, int num = 1) : XML_ElementParser("image"), base_index(base), max_index(num - 1) {}

	bool Start()
	{
		have_index = have_name = false;
		scale = false;
		return true;
	}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "index")) {
			if (ReadBoundedNumericalValue(value, "%d", index, 0, max_index))
				have_index = true;
			else
				return false;
		} else if (StringsEqual(tag, "file")) {
			name = value;
			have_name = true;
		} else if (StringsEqual(tag, "scale")) {
			return ReadBooleanValue(value, scale);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}

	bool AttributesDone()
	{
		if (!have_index || !have_name) {
			AttribsMissing();
			return false;
		}
		dialog_image_spec[base_index + index].name = name;
		dialog_image_spec[base_index + index].scale = scale;
		return true;
	}

private:
	int base_index, max_index;

	bool have_index, have_name;

	int index;
	string name;
	bool scale;
};

static XML_ImageParser FrameImageParser(FRAME_TL_IMAGE, 8);
static XML_ImageParser ListImageParser(LIST_TL_IMAGE, 8);
static XML_ImageParser ThumbImageParser(THUMB_T_IMAGE, 5);
static XML_ImageParser SliderImageParser(SLIDER_L_IMAGE, 4);
static XML_ImageParser ButtonImageParser(BUTTON_L_IMAGE, 3);

class XML_DColorParser : public XML_ElementParser {
public:
	XML_DColorParser(int base, int num = 1) : XML_ElementParser("color"), base_index(base), max_index(num - 1) {}

	bool Start()
	{
		have_red = have_green = have_blue = false;
		idx = 0;
		return true;
	}

	bool HandleAttribute(const char *tag, const char *value)
	{
		float v;
		if (StringsEqual(tag, "index")) {
			return ReadBoundedNumericalValue(value, "%d", idx, 0, max_index);
		} else if (StringsEqual(tag, "red")) {
			if (ReadNumericalValue(value, "%f", v)) {
				have_red = true;
				color.r = uint8(PIN(255 * v + 0.5, 0, 255));
			} else
				return false;
		} else if (StringsEqual(tag, "green")) {
			if (ReadNumericalValue(value, "%f", v)) {
				have_green = true;
				color.g = uint8(PIN(255 * v + 0.5, 0, 255));
			} else
				return false;
		} else if (StringsEqual(tag, "blue")) {
			if (ReadNumericalValue(value, "%f", v)) {
				have_blue = true;
				color.b = uint8(PIN(255 * v + 0.5, 0, 255));
			} else
				return false;
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}

	bool AttributesDone()
	{
		if (!have_red || !have_green || !have_blue) {
			AttribsMissing();
			return false;
		}
		dialog_color[base_index + idx] = color;
		return true;
	}

private:
	int base_index, max_index;

	bool have_red, have_green, have_blue;

	SDL_Color color;
protected:
	int idx;
};

static bool foundLabelOutlineColor = false;

class XML_DLabelColorParser : public XML_DColorParser
{
public:
	XML_DLabelColorParser(int base, int num = 1) : XML_DColorParser(base, num) { }

	bool AttributesDone()
	{
		if (idx == 2)
		{
			foundLabelOutlineColor = true;
		}

		return XML_DColorParser::AttributesDone();
	}
};

static XML_DColorParser BackgroundColorParser(BACKGROUND_COLOR);
static XML_DColorParser TitleColorParser(TITLE_COLOR);
static XML_DColorParser ButtonColorParser(BUTTON_COLOR, 2);
static XML_DLabelColorParser LabelColorParser(LABEL_COLOR, 3);
static XML_DColorParser ItemColorParser(ITEM_COLOR, 2);
static XML_DColorParser MessageColorParser(MESSAGE_COLOR);
static XML_DColorParser TextEntryColorParser(TEXT_ENTRY_COLOR, 3);


class XML_DFontParser : public XML_ElementParser {
public:
	XML_DFontParser(int i) : XML_ElementParser("font"), idx(i) {}
	
	bool Start()
	{
		have_id = have_size = false;
		style = 0;
		return true;
	}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "id")) {
			if (ReadNumericalValue(value, "%d", id))
				have_id = true;
			else
				return false;
		} else if (StringsEqual(tag, "size")) {
			if (ReadNumericalValue(value, "%d", size))
				have_size = true;
			else
				return false;
		} else if (StringsEqual(tag, "style")) {
			return ReadNumericalValue(value, "%d", style);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}

	bool AttributesDone()
	{
		if (!have_id || !have_size) {
			AttribsMissing();
			return false;
		}
		dialog_font_spec[idx].font = id;
		dialog_font_spec[idx].style = style;
		dialog_font_spec[idx].size = size;
		return true;
	}

private:
	bool have_id, have_size;

	int idx;
	int id, size, style;
};

static XML_DFontParser TitleFontParser(TITLE_FONT);
static XML_DFontParser ButtonFontParser(BUTTON_FONT);
static XML_DFontParser LabelFontParser(LABEL_FONT);
static XML_DFontParser ItemFontParser(ITEM_FONT);
static XML_DFontParser MessageFontParser(MESSAGE_FONT);
static XML_DFontParser TextEntryFontParser(TEXT_ENTRY_FONT);
static XML_DFontParser TextBoxFontParser(TEXT_BOX_FONT);

class XML_DFrameParser : public XML_ElementParser {
public:
	XML_DFrameParser() : XML_ElementParser("frame") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "top")) {
			return ReadNumericalValue(value, "%hu", dialog_space[FRAME_T_SPACE]);
		} else if (StringsEqual(tag, "bottom")) {
			return ReadNumericalValue(value, "%hu", dialog_space[FRAME_B_SPACE]);
		} else if (StringsEqual(tag, "left")) {
			return ReadNumericalValue(value, "%hu", dialog_space[FRAME_L_SPACE]);
		} else if (StringsEqual(tag, "right")) {
			return ReadNumericalValue(value, "%hu", dialog_space[FRAME_R_SPACE]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_DFrameParser FrameParser;

struct XML_BackgroundParser : public XML_ElementParser {XML_BackgroundParser() : XML_ElementParser("background") {}};
static XML_BackgroundParser BackgroundParser;

struct XML_TitleParser : public XML_ElementParser {XML_TitleParser() : XML_ElementParser("title") {}};
static XML_TitleParser TitleParser;

class XML_SpacerParser : public XML_ElementParser {
public:
	XML_SpacerParser() : XML_ElementParser("spacer") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "height")) {
			return ReadNumericalValue(value, "%hu", dialog_space[SPACER_HEIGHT]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_SpacerParser SpacerParser;

class XML_ButtonParser : public XML_ElementParser {
public:
	XML_ButtonParser() : XML_ElementParser("button") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "top")) {
			return ReadNumericalValue(value, "%hu", dialog_space[BUTTON_T_SPACE]);
		} else if (StringsEqual(tag, "left")) {
			return ReadNumericalValue(value, "%hu", dialog_space[BUTTON_L_SPACE]);
		} else if (StringsEqual(tag, "right")) {
			return ReadNumericalValue(value, "%hu", dialog_space[BUTTON_R_SPACE]);
		} else if (StringsEqual(tag, "height")) {
			return ReadNumericalValue(value, "%hu", dialog_space[BUTTON_HEIGHT]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_ButtonParser ButtonParser;

struct XML_LabelParser : public XML_ElementParser {
	XML_LabelParser() : XML_ElementParser("label") {}
public:
	bool Start()
	{
		foundLabelOutlineColor = false;
		return true;
	}

	bool End()
	{
		if (!foundLabelOutlineColor)
		{
			dialog_color[LABEL_OUTLINE_COLOR] = dialog_color[BACKGROUND_COLOR];
		}
		return true;
	}
};
static XML_LabelParser LabelParser;

class XML_DItemParser : public XML_ElementParser {
public:
	XML_DItemParser() : XML_ElementParser("item") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "space")) {
			return ReadNumericalValue(value, "%hu", dialog_space[LABEL_ITEM_SPACE]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_DItemParser ItemParser;

struct XML_MessageParser : public XML_ElementParser {XML_MessageParser() : XML_ElementParser("message") {}};
static XML_MessageParser MessageParser;

struct XML_TextEntryParser : public XML_ElementParser {XML_TextEntryParser() : XML_ElementParser("text_entry") {}};
static XML_TextEntryParser TextEntryParser;

struct XML_TextBoxParser : public XML_ElementParser { XML_TextBoxParser() : XML_ElementParser("text_box") {}};
static XML_TextBoxParser TextBoxParser;

class XML_TroughParser : public XML_ElementParser {
public:
	XML_TroughParser() : XML_ElementParser("trough") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "top")) {
			return ReadNumericalValue(value, "%hu", dialog_space[TROUGH_T_SPACE]);
		} else if (StringsEqual(tag, "bottom")) {
			return ReadNumericalValue(value, "%hu", dialog_space[TROUGH_B_SPACE]);
		} else if (StringsEqual(tag, "right")) {
			return ReadNumericalValue(value, "%hu", dialog_space[TROUGH_R_SPACE]);
		} else if (StringsEqual(tag, "width")) {
			return ReadNumericalValue(value, "%hu", dialog_space[TROUGH_WIDTH]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_TroughParser TroughParser;

struct XML_ThumbParser : public XML_ElementParser {XML_ThumbParser() : XML_ElementParser("thumb") {}};
static XML_ThumbParser ThumbParser;

class XML_ListParser : public XML_ElementParser {
public:
	XML_ListParser() : XML_ElementParser("list") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "top")) {
			return ReadNumericalValue(value, "%hu", dialog_space[LIST_T_SPACE]);
		} else if (StringsEqual(tag, "bottom")) {
			return ReadNumericalValue(value, "%hu", dialog_space[LIST_B_SPACE]);
		} else if (StringsEqual(tag, "left")) {
			return ReadNumericalValue(value, "%hu", dialog_space[LIST_L_SPACE]);
		} else if (StringsEqual(tag, "right")) {
			return ReadNumericalValue(value, "%hu", dialog_space[LIST_R_SPACE]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_ListParser ListParser;

class XML_SliderParser : public XML_ElementParser {
public:
	XML_SliderParser() : XML_ElementParser("slider") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (StringsEqual(tag, "top")) {
			return ReadNumericalValue(value, "%hu", dialog_space[SLIDER_T_SPACE]);
		} else if (StringsEqual(tag, "left")) {
			return ReadNumericalValue(value, "%hu", dialog_space[SLIDER_L_SPACE]);
		} else if (StringsEqual(tag, "right")) {
			return ReadNumericalValue(value, "%hu", dialog_space[SLIDER_R_SPACE]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_SliderParser SliderParser;

class XML_ThemeParser : public XML_ElementParser {
public:
	XML_ThemeParser() : XML_ElementParser("theme") {}
};

static XML_ThemeParser ThemeParser;

XML_ElementParser *Theme_GetParser()
{
	FrameParser.AddChild(&FrameImageParser);
	ThemeParser.AddChild(&FrameParser);

	BackgroundParser.AddChild(&BackgroundColorParser);
	ThemeParser.AddChild(&BackgroundParser);

	TitleParser.AddChild(&TitleFontParser);
	TitleParser.AddChild(&TitleColorParser);
	ThemeParser.AddChild(&TitleParser);

	ThemeParser.AddChild(&SpacerParser);

	ButtonParser.AddChild(&ButtonFontParser);
	ButtonParser.AddChild(&ButtonColorParser);
	ButtonParser.AddChild(&ButtonImageParser);
	ThemeParser.AddChild(&ButtonParser);

	LabelParser.AddChild(&LabelFontParser);
	LabelParser.AddChild(&LabelColorParser);
	ThemeParser.AddChild(&LabelParser);

	ItemParser.AddChild(&ItemFontParser);
	ItemParser.AddChild(&ItemColorParser);
	ThemeParser.AddChild(&ItemParser);

	MessageParser.AddChild(&MessageFontParser);
	MessageParser.AddChild(&MessageColorParser);
	ThemeParser.AddChild(&MessageParser);

	TextEntryParser.AddChild(&TextEntryFontParser);
	TextEntryParser.AddChild(&TextEntryColorParser);
	ThemeParser.AddChild(&TextEntryParser);

	TextBoxParser.AddChild(&TextBoxFontParser);
	ThemeParser.AddChild(&TextBoxParser);

	ListParser.AddChild(&ListImageParser);
	ListParser.AddChild(&TroughParser);
	ThumbParser.AddChild(&ThumbImageParser);
	ListParser.AddChild(&ThumbParser);
	ThemeParser.AddChild(&ListParser);

	SliderParser.AddChild(&SliderImageParser);
	ThemeParser.AddChild(&SliderParser);

	return &ThemeParser;
}


/*
 *  Load theme
 */

void load_theme(FileSpecifier &theme)
{
	// Unload previous theme
	unload_theme();

	// Set defaults, the theme overrides these
	set_theme_defaults();

	// Parse theme MML script
	FileSpecifier theme_mml = theme + "theme.mml";
	XML_Loader_SDL loader;
	loader.CurrentElement = &RootParser;
	loader.ParseFile(theme_mml);

	// Open resource file
	FileSpecifier theme_rsrc = theme + "resources";
	theme_rsrc.Open(theme_resources);
	clear_game_error();

	// Load fonts
	for (int i=0; i<NUM_DIALOG_FONTS; i++)
		dialog_font[i] = load_font(dialog_font_spec[i]);

	// Load images
	for (int i=0; i<NUM_DIALOG_IMAGES; i++) {
		FileSpecifier file = theme + dialog_image_spec[i].name;
		SDL_Surface *s =SDL_LoadBMP(file.GetPath());
		if (s)
			SDL_SetColorKey(s, SDL_SRCCOLORKEY, SDL_MapRGB(s->format, 0x00, 0xff, 0xff));
		dialog_image[i] = s;
	}
}


/*
 *  Set theme default values
 */

static const SDL_Color default_dialog_color[NUM_DIALOG_COLORS] = {
	{0x00, 0x00, 0x00}, // BACKGROUND COLOR
	{0xc0, 0xc0, 0xc0}, // TITLE_COLOR
	{0xc0, 0x00, 0x00}, // BUTTON_COLOR
	{0xff, 0xff, 0xff}, // BUTTON_ACTIVE_COLOR
	{0x20, 0x20, 0xff}, // LABEL_COLOR
	{0x80, 0x80, 0xff}, // LABEL_ACTIVE_COLOR
	{0x40, 0xff, 0x40}, // ITEM_COLOR
	{0xff, 0xff, 0xff}, // ITEM_ACTIVE_COLOR
	{0xff, 0xff, 0xff}, // MESSAGE_COLOR
	{0x40, 0xff, 0x40}, // TEXT_ENTRY_COLOR
	{0xff, 0xff, 0xff}, // TEXT_ENTRY_ACTIVE_COLOR
	{0xff, 0xff, 0xff}, // TEXT_ENTRY_CURSOR_COLOR
	{0x60, 0x60, 0x60},  // KEY_BINDING_COLOR
	{0x00, 0x00, 0x00}, // LABEL_OUTLINE_COLOR
};

static const int default_dialog_space[NUM_DIALOG_SPACES] = {
	6,	// FRAME_T_SPACE
	6,	// FRAME_L_SPACE
	6,	// FRAME_R_SPACE
	6,	// FRAME_B_SPACE
	8,	// SPACER_HEIGHT
	16,	// LABEL_ITEM_SPACE
	2,	// LIST_T_SPACE
	2,	// LIST_L_SPACE
	18,	// LIST_R_SPACE
	2,	// LIST_B_SPACE
	0,	// TROUGH_T_SPACE
	16,	// TROUGH_R_SPACE
	0,	// TROUGH_B_SPACE
	16,	// TROUGH_WIDTH
	0,	// SLIDER_T_SPACE
	0,	// SLIDER_L_SPACE
	0,	// SLIDER_R_SPACE
	0,	// BUTTON_T_SPACE
	0,	// BUTTON_L_SPACE
	0,	// BUTTON_R_SPACE
	14	// BUTTON_HEIGHT
};

static void set_theme_defaults(void)
{
	for (int i=0; i<NUM_DIALOG_FONTS; i++)
		dialog_font[i] = NULL;

	for (int i=0; i<NUM_DIALOG_COLORS; i++)
		dialog_color[i] = default_dialog_color[i];

	for (int i=0; i<NUM_DIALOG_IMAGES; i++) {
		dialog_image_spec[i].name = "";
		dialog_image_spec[i].scale = false;
		dialog_image[i] = NULL;
	}

	for (int i=0; i<NUM_DIALOG_SPACES; i++)
		dialog_space[i] = default_dialog_space[i];
}


/*
 *  Unload theme
 */

static void unload_theme(void)
{
	// Unload fonts
	for (int i=0; i<NUM_DIALOG_FONTS; i++)
		if (dialog_font[i]) {
			unload_font(dialog_font[i]);
			dialog_font[i] = NULL;
		}

	// Free surfaces
	for (int i=0; i<NUM_DIALOG_IMAGES; i++)
		if (dialog_image[i]) {
			SDL_FreeSurface(dialog_image[i]);
			dialog_image[i] = NULL;
		}

	// Close resource file
	theme_resources.Close();
}


/*
 *  Get dialog font/color/image/space from theme
 */

const sdl_font_info *get_dialog_font(int which, uint16 &style)
{
	assert(which >= 0 && which < NUM_DIALOG_FONTS);
	const sdl_font_info *font = dialog_font[which];
	if (font) {
		style = dialog_font_spec[which].style;
		return font;
	} else {
		style = styleNormal;
		return default_font;
	}
}

uint32 get_dialog_color(int which)
{
	assert(which >= 0 && which < NUM_DIALOG_COLORS);
	return SDL_MapRGB(dialog_surface->format, dialog_color[which].r, dialog_color[which].g, dialog_color[which].b);
}

// ZZZ: added this for convenience; taken from w_player_color::draw().
// Obviously, this color does not come from the theme.
uint32 get_dialog_player_color(size_t colorIndex) {
        SDL_Color c;
        _get_interface_color(PLAYER_COLOR_BASE_INDEX + colorIndex, &c);
        return SDL_MapRGB(dialog_surface->format, c.r, c.g, c.b);
}

SDL_Surface *get_dialog_image(int which, int width, int height)
{
	assert(which >= 0 && which < NUM_DIALOG_IMAGES);
	SDL_Surface *s = dialog_image[which];
	if (s == NULL)
		s = default_image;

	// If no width and height is given, the surface is returned as-is and must
	// not be freed by the caller
	if (width == 0 && height == 0)
		return s;

	// Otherwise, a new tiled/rescaled surface is created which must be freed
	// by the caller
	int req_width = width ? width : s->w;
	int req_height = height ? height : s->h;
	SDL_Surface *s2 = dialog_image_spec[which].scale ? rescale_surface(s, req_width, req_height) : tile_surface(s, req_width, req_height);
	SDL_SetColorKey(s2, SDL_SRCCOLORKEY, SDL_MapRGB(s2->format, 0x00, 0xff, 0xff));
	return s2;
}

uint16 get_dialog_space(size_t which)
{
	assert(which < NUM_DIALOG_SPACES);
	return dialog_space[which];
}


/*
 *  Play dialog sound
 */

void play_dialog_sound(int which)
{
	play_sound(which, NULL, NONE);
}


/*
 *  Dialog constructor
 */

dialog::dialog() : active_widget(NULL), active_widget_num(UNONE), done(false),
            cursor_was_visible(false), parent_dialog(NULL),
            processing_function(NULL), active_tab(0)
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
}


/*
 *  Add widget
 */

void dialog::add(widget *w)
{
	widgets.push_back(w);
	tabs.push_back(NONE);
        w->set_owning_dialog(this);
}

void dialog::add_to_tab(widget *w, int tab)
{
	widgets.push_back(w);
	tabs.push_back(tab);
        w->set_owning_dialog(this);
}


/*
 *  Layout dialog
 */

void dialog::layout()
{
	// Layout all widgets, calculate total width and height
	int y = get_dialog_space(FRAME_T_SPACE);
	int left = 0;
	int right = 0;
	
	// Initialise tab-specific y's
	map<int, int> y_per_tab;
	for (vector<int>::const_iterator tab = tabs.begin(); tab != tabs.end(); ++tab)
		if (*tab != NONE)
			y_per_tab[*tab] = y;
	
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	vector<int>::const_iterator j = tabs.begin();
	while (i != end) {
		// Are we interested in the global y or a tab-specific?
		int& the_y = (*j == NONE) ? y : y_per_tab[*j];
		
		widget *w = *i;
		w->rect.y = the_y;
		the_y += w->layout();
		if (w->rect.x < left)
			left = w->rect.x;
		if (w->rect.x + w->rect.w > right)
			right = w->rect.x + w->rect.w;
		
		if (*j == NONE)
			// Later tabbed widgets must appear below this widget
			for (map<int, int>::iterator tab = y_per_tab.begin(); tab != y_per_tab.end(); ++tab)
				(*tab).second = y;
		else
			// The next untabbed widget must appear below this widget
			if (y < y_per_tab[*j])
				y = y_per_tab[*j];
		
		i++;
		j++;
	}
	left = abs(left);
	rect.w = (left > right ? left : right) * 2 + get_dialog_space(FRAME_L_SPACE) + get_dialog_space(FRAME_R_SPACE);
	rect.h = y + get_dialog_space(FRAME_B_SPACE);

	// Center dialog on video surface
	SDL_Surface *video = SDL_GetVideoSurface();
	rect.x = (video->w - rect.w) / 2;
	rect.y = (video->h - rect.h) / 2;

	// Transform positions of all widgets to be relative to the dialog surface
	i = widgets.begin();
	int offset_x = (rect.w - get_dialog_space(FRAME_L_SPACE) - get_dialog_space(FRAME_R_SPACE)) / 2 + get_dialog_space(FRAME_L_SPACE);
        // ZZZ: to provide more detailed layout info to widgets
        int leftmost_x = get_dialog_space(FRAME_L_SPACE);
        int usable_width = rect.w - leftmost_x - get_dialog_space(FRAME_R_SPACE);
        
	while (i != end) {
		(*i)->rect.x += offset_x;
                (*i)->capture_layout_information(leftmost_x, usable_width);
		i++;
	}
}


/*
 *  Update part of dialog on screen
 */

void dialog::update(SDL_Rect r) const
{
	SDL_Surface *video = SDL_GetVideoSurface();
// ZZZ: this less efficient way (copies more stuff) is an attempt to fix dialog updating
// in the face of double-buffered page-flipping.
/*
	SDL_Rect dst_rect = {r.x + rect.x, r.y + rect.y, r.w, r.h};
	SDL_BlitSurface(dialog_surface, &r, video, &dst_rect);
	SDL_UpdateRects(video, 1, &dst_rect);
*/
        // this is needed because 'rect' is const here and SDL_* don't seem to use const.
#ifdef HAVE_OPENGL
	if (OGL_IsActive()) {
		SDL_Rect ScreenRect = {0, 0, video->w, video->h};
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		
 		// Disable everything but alpha blending
 		glDisable(GL_CULL_FACE);
 		glDisable(GL_DEPTH_TEST);
 		glDisable(GL_ALPHA_TEST);
 		glEnable(GL_BLEND);
 		glDisable(GL_FOG);
 		glDisable(GL_SCISSOR_TEST);
 		glDisable(GL_STENCIL_TEST);

		glViewport(0, 0, ScreenRect.w, ScreenRect.h);
		
 		// Direct projection
 		glMatrixMode(GL_PROJECTION);
 		glPushMatrix();
 		glLoadIdentity();

		gluOrtho2D(0.0, ScreenRect.w, 0.0, ScreenRect.h);
 		glMatrixMode(GL_MODELVIEW);
 		glPushMatrix();
 		glLoadIdentity();
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glRasterPos2f(rect.x, ScreenRect.h - rect.y);
		glPixelZoom(1.0, -1.0);

#ifdef ALEPHONE_LITTLE_ENDIAN
		SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
		SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
		SDL_BlitSurface(dialog_surface, NULL, s, NULL);
		glDrawPixels(s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
		SDL_FreeSurface(s);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();

		SDL_GL_SwapBuffers();
	} else 
#endif
	{
		SDL_Rect dst_rect = rect;
		SDL_Rect src_rect = { 0, 0, rect.w, rect.h };
		SDL_BlitSurface(dialog_surface, &src_rect, video, &dst_rect);
		SDL_UpdateRects(video, 1, &dst_rect);

	}
}


/*
 *  Draw dialog
 */

void dialog::draw_widget(widget *w, bool do_update) const
{
	// Clear and redraw widget
	SDL_FillRect(dialog_surface, &w->rect, get_dialog_color(BACKGROUND_COLOR));
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

void dialog::draw(void) const
{
	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(BACKGROUND_COLOR));

	// Draw frame
	draw_frame_image(frame_tl, 0, 0);
	draw_frame_image(frame_t, frame_tl->w, 0);
	draw_frame_image(frame_tr, frame_tl->w + frame_t->w, 0);
	draw_frame_image(frame_l, 0, frame_tl->h);
	draw_frame_image(frame_r, rect.w - frame_r->w, frame_tr->h);
	draw_frame_image(frame_bl, 0, frame_tl->h + frame_l->h);
	draw_frame_image(frame_b, frame_bl->w, rect.h - frame_b->h);
	draw_frame_image(frame_br, frame_bl->w + frame_b->w, frame_tr->h + frame_r->h);

	// Draw all widgets, except those with inactive tabs
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	vector<int>::const_iterator j = tabs.begin();
	while (i != end) {
		if (*j == NONE || *j == active_tab)
			draw_widget(*i, false);
		i++;
		j++;
	}

	// Blit to screen
	SDL_Rect r = {0, 0, rect.w, rect.h};
	update(r);
}

void
dialog::draw_dirty_widgets() const
{
        for (unsigned i=0; i<widgets.size(); i++)
		if (widgets[i]->dirty)
			if (tabs[i] == NONE || tabs[i] == active_tab)
				draw_widget(widgets[i]);
}       

/*
 *  Deactivate currently active widget
 */

void dialog::deactivate_currently_active_widget(bool draw)
{
	if (active_widget) {
		active_widget->active = false;
		if (draw)
			draw_widget(active_widget);

        active_widget = NULL;
        active_widget_num = UNONE;
	}
}


/*
 *  Activate widget
 */

void dialog::activate_widget(size_t num, bool draw)
{
	if (num == active_widget_num)
		return;
	// BUG: may crash if num==UNONE or NONE
	if (!widgets[num]->is_selectable())
		return;

	// Deactivate previously active widget
    deactivate_currently_active_widget(draw);

	// Activate new widget
	active_widget = widgets[num];
	active_widget_num = num;
	active_widget->active = true;
	if (draw) {
		draw_widget(active_widget);
//		play_dialog_sound(DIALOG_SELECT_SOUND);
	}
}


/*
 *  Activate first selectable widget (don't draw)
 */

void dialog::activate_first_widget(void)
{
	for (size_t i=0; i<widgets.size(); i++) {
		if (widgets[i]->is_selectable() && (tabs[i] == NONE || tabs[i] == active_tab)) {
			activate_widget(i, false);
			break;
		}
	}
}


/*
 *  Activate next/previous selectable widget
 */

void dialog::activate_next_widget(void)
{
	size_t i = active_widget_num;
	// BUG: infinate loop if active_widget_num == UNONE or NONE
	do {
		i++;
		if (i >= int(widgets.size()))
			i = 0;
	} while (!(widgets[i]->is_selectable() && (tabs[i] == NONE || tabs[i] == active_tab)) && i != active_widget_num);

    // Either widgets[i] is selectable, or i == active_widget_num (in which case we wrapped all the way around)
	if (widgets[i]->is_selectable())
		activate_widget(i);
	else
		deactivate_currently_active_widget();
}

void dialog::activate_prev_widget(void)
{
	size_t i = active_widget_num;
	// BUG: infinate loop if active_widget_num == UNONE or NONE
	do {
		if (i == 0)
			i = widgets.size() - 1;
		else
			i--;
	} while (!(widgets[i]->is_selectable() && (tabs[i] == NONE || tabs[i] == active_tab)) && i != active_widget_num);

    // Either widgets[i] is selectable, or i == active_widget_num (in which case we wrapped all the way around)
	if (widgets[i]->is_selectable())
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
	vector<int>::const_iterator j = tabs.begin();
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	int num = 0;
	while (i != end) {
		widget *w = *i;
		if (*j == NONE || *j == active_tab) // Don't find widgets in inactive tabs
			if (x >= w->rect.x && y >= w->rect.y && x < w->rect.x + w->rect.w && y < w->rect.y + w->rect.h)
				return num;
		i++; j++; num++;
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
	&& ((e.key.keysym.mod & KMOD_ALT) || (e.key.keysym.mod & KMOD_META))) {
      toggle_fullscreen();
      if (!get_screen_mode()->fill_the_screen)
	      layout();
      draw();
      handled = true;
    }
    break;
  case SDL_ACTIVEEVENT:
    if (e.active.state & SDL_APPACTIVE) {
      if (e.active.gain) {
	draw();
	handled = true;
      }
    }
    break;
  }
  
  if (!handled) {
  // First pass event to active widget (which may modify it)
  if (active_widget)
    active_widget->event(e);
	
	// Remaining events handled by dialog
	switch (e.type) {

		// Key pressed
		case SDL_KEYDOWN:
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
			break;

		// Mouse moved
		case SDL_MOUSEMOTION: {
			int x = e.motion.x, y = e.motion.y;
			int num = find_widget(x, y);
			if (num >= 0) {
				assert(num == (size_t)num);
				activate_widget(num);
				widget *w = widgets[num];
				w->mouse_move(x - rect.x - w->rect.x, y - rect.y - w->rect.y);
			}
			break;
		}

		// Mouse button pressed
		case SDL_MOUSEBUTTONDOWN: {
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
			case SDL_BUTTON_RIGHT: {
				int x = e.button.x, y = e.button.y;
				int num = find_widget(x, y);
				if (num >= 0) {
					widget *w = widgets[num];
					w->click(x - rect.x - w->rect.x, y - rect.y - w->rect.y);
				}
				break;
			}
			}
			break;
		}

		// Quit requested
		case SDL_QUIT:
			exit(0);
			break;
	}
  }
	// Redraw dirty widgets
        draw_dirty_widgets();
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
        
		// Run custom processing function
		if (processing_function)
			processing_function(this);

		// Give time to system
		global_idle_proc();
		SDL_Delay(10);
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

	// Set new active dialog
	parent_dialog = top_dialog;
	top_dialog = this;

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(BACKGROUND_COLOR));

	// Activate first widget
	activate_first_widget();

	// Layout dialog
	layout();

	// Get frame images
	frame_tl = get_dialog_image(FRAME_TL_IMAGE);
	frame_tr = get_dialog_image(FRAME_TR_IMAGE);
	frame_bl = get_dialog_image(FRAME_BL_IMAGE);
	frame_br = get_dialog_image(FRAME_BR_IMAGE);
	frame_t = get_dialog_image(FRAME_T_IMAGE, rect.w - frame_tl->w - frame_tr->w, 0);
	frame_l = get_dialog_image(FRAME_L_IMAGE, 0, rect.h - frame_tl->h - frame_bl->h);
	frame_r = get_dialog_image(FRAME_R_IMAGE, 0, rect.h - frame_tr->h - frame_br->h);
	frame_b = get_dialog_image(FRAME_B_IMAGE, rect.w - frame_bl->w - frame_br->w, 0);

#if defined(HAVE_OPENGL)
	if (OGL_IsActive()) {
#if defined(OPENGL_DOESNT_COPY_ON_SWAP)
		{
			// the screen will flicker if we don't clear or copy,
			// and copying takes way too long
 			clear_screen();
 			glClearColor(0,0,0,0);
 			glClear(GL_COLOR_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		}
#endif
	}
#endif

	// Draw dialog
	draw();

	// Show cursor
	cursor_was_visible = (SDL_ShowCursor(SDL_ENABLE) == SDL_ENABLE);

	// Welcome sound
	if (play_sound)
		play_dialog_sound(DIALOG_INTRO_SOUND);

	// Enable unicode key translation
	saved_unicode_state = SDL_EnableUNICODE(true);
	saved_keyrepeat_state = sKeyRepeatActive;
	sKeyRepeatActive = true;
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

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

		// Get next event
		SDL_Event e;
		e.type = SDL_NOEVENT;
		SDL_PollEvent(&e);

		if (e.type == SDL_NOEVENT)
			break;

		// Handle event
		event(e);
	}

	return done;
}


/*
 *  Remove dialog from screen
 */

int dialog::finish(bool play_sound)
{
	// Disable unicode key translation
	SDL_EnableUNICODE(saved_unicode_state);
	sKeyRepeatActive = saved_keyrepeat_state;
	if (!saved_keyrepeat_state)
	{
		SDL_EnableKeyRepeat(0, 0);
	}

	// Farewell sound
	if (play_sound)
		play_dialog_sound(result == 0 ? DIALOG_OK_SOUND : DIALOG_CANCEL_SOUND);

	// Hide cursor
	if (!cursor_was_visible)
		SDL_ShowCursor(false);

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(BACKGROUND_COLOR));

#ifdef HAVE_OPENGL
	if (OGL_IsActive()) {
#ifdef OPENGL_DOESNT_COPY_ON_SWAP
		clear_screen();
#endif
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else
#endif 
	{

		// Erase dialog from screen
		SDL_Surface *video = SDL_GetVideoSurface();
		SDL_FillRect(video, &rect, SDL_MapRGB(video->format, 0, 0, 0));
		SDL_UpdateRects(video, 1, &rect);
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

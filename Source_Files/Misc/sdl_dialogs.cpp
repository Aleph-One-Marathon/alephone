/*
 *  sdl_dialogs.cpp - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
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

#include "XML_Loader_SDL.h"
#include "XML_ParseTreeRoot.h"


// Global variables
static SDL_Surface *dialog_surface = NULL;

static sdl_font_info *default_font = NULL;
static SDL_Surface *default_image = NULL;

static OpenedResourceFile theme_resources;

static TextSpec dialog_font_spec[NUM_DIALOG_FONTS];
static sdl_font_info *dialog_font[NUM_DIALOG_FONTS];
static SDL_Color dialog_color[NUM_DIALOG_COLORS];
static int dialog_space[NUM_DIALOG_SPACES];

static struct {
	string name;
	bool scale;
} dialog_image_spec[NUM_DIALOG_IMAGES];

static SDL_Surface *dialog_image[NUM_DIALOG_IMAGES];

// From shell_sdl.cpp
extern FileSpecifier global_themes_dir, local_themes_dir;

// Prototypes
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
		if (strcmp(tag, "index") == 0) {
			if (ReadBoundedNumericalValue(value, "%d", index, 0, max_index))
				have_index = true;
			else
				return false;
		} else if (strcmp(tag, "file") == 0) {
			name = value;
			have_name = true;
		} else if (strcmp(tag, "scale") == 0) {
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
		if (strcmp(tag, "index") == 0) {
			return ReadBoundedNumericalValue(value, "%d", idx, 0, max_index);
		} else if (strcmp(tag, "red") == 0) {
			if (ReadNumericalValue(value, "%f", v)) {
				have_red = true;
				color.r = uint8(PIN(255 * v + 0.5, 0, 255));
			} else
				return false;
		} else if (strcmp(tag, "green") == 0) {
			if (ReadNumericalValue(value, "%f", v)) {
				have_green = true;
				color.g = uint8(PIN(255 * v + 0.5, 0, 255));
			} else
				return false;
		} else if (strcmp(tag, "blue") == 0) {
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

	int idx;
	SDL_Color color;
};

static XML_DColorParser BackgroundColorParser(BACKGROUND_COLOR);
static XML_DColorParser TitleColorParser(TITLE_COLOR);
static XML_DColorParser ButtonColorParser(BUTTON_COLOR, 2);
static XML_DColorParser LabelColorParser(LABEL_COLOR, 2);
static XML_DColorParser ItemColorParser(ITEM_COLOR, 2);
static XML_DColorParser MessageColorParser(MESSAGE_COLOR);
static XML_DColorParser TextEntryColorParser(TEXT_ENTRY_COLOR, 3);

class XML_FontParser : public XML_ElementParser {
public:
	XML_FontParser(int i) : XML_ElementParser("font"), idx(i) {}

	bool Start()
	{
		have_id = have_size = false;
		style = 0;
		return true;
	}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (strcmp(tag, "id") == 0) {
			if (ReadNumericalValue(value, "%d", id))
				have_id = true;
			else
				return false;
		} else if (strcmp(tag, "size") == 0) {
			if (ReadNumericalValue(value, "%d", size))
				have_size = true;
			else
				return false;
		} else if (strcmp(tag, "style") == 0) {
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

static XML_FontParser TitleFontParser(TITLE_FONT);
static XML_FontParser ButtonFontParser(BUTTON_FONT);
static XML_FontParser LabelFontParser(LABEL_FONT);
static XML_FontParser ItemFontParser(ITEM_FONT);
static XML_FontParser MessageFontParser(MESSAGE_FONT);
static XML_FontParser TextEntryFontParser(TEXT_ENTRY_FONT);

class XML_FrameParser : public XML_ElementParser {
public:
	XML_FrameParser() : XML_ElementParser("frame") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (strcmp(tag, "top") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[FRAME_T_SPACE]);
		} else if (strcmp(tag, "bottom") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[FRAME_B_SPACE]);
		} else if (strcmp(tag, "left") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[FRAME_L_SPACE]);
		} else if (strcmp(tag, "right") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[FRAME_R_SPACE]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_FrameParser FrameParser;

struct XML_BackgroundParser : public XML_ElementParser {XML_BackgroundParser() : XML_ElementParser("background") {}};
static XML_BackgroundParser BackgroundParser;

struct XML_TitleParser : public XML_ElementParser {XML_TitleParser() : XML_ElementParser("title") {}};
static XML_TitleParser TitleParser;

class XML_SpacerParser : public XML_ElementParser {
public:
	XML_SpacerParser() : XML_ElementParser("spacer") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (strcmp(tag, "height") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[SPACER_HEIGHT]);
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
		if (strcmp(tag, "top") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[BUTTON_T_SPACE]);
		} else if (strcmp(tag, "left") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[BUTTON_L_SPACE]);
		} else if (strcmp(tag, "right") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[BUTTON_R_SPACE]);
		} else if (strcmp(tag, "height") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[BUTTON_HEIGHT]);
		} else {
			UnrecognizedTag();
			return false;
		}
		return true;
	}
};

static XML_ButtonParser ButtonParser;

struct XML_LabelParser : public XML_ElementParser {XML_LabelParser() : XML_ElementParser("label") {}};
static XML_LabelParser LabelParser;

class XML_DItemParser : public XML_ElementParser {
public:
	XML_DItemParser() : XML_ElementParser("item") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (strcmp(tag, "space") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[LABEL_ITEM_SPACE]);
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

class XML_TroughParser : public XML_ElementParser {
public:
	XML_TroughParser() : XML_ElementParser("trough") {}

	bool HandleAttribute(const char *tag, const char *value)
	{
		if (strcmp(tag, "top") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[TROUGH_T_SPACE]);
		} else if (strcmp(tag, "bottom") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[TROUGH_B_SPACE]);
		} else if (strcmp(tag, "right") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[TROUGH_R_SPACE]);
		} else if (strcmp(tag, "width") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[TROUGH_WIDTH]);
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
		if (strcmp(tag, "top") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[LIST_T_SPACE]);
		} else if (strcmp(tag, "bottom") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[LIST_B_SPACE]);
		} else if (strcmp(tag, "left") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[LIST_L_SPACE]);
		} else if (strcmp(tag, "right") ==0 ) {
			return ReadNumericalValue(value, "%d", dialog_space[LIST_R_SPACE]);
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
		if (strcmp(tag, "top") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[SLIDER_T_SPACE]);
		} else if (strcmp(tag, "left") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[SLIDER_L_SPACE]);
		} else if (strcmp(tag, "right") == 0) {
			return ReadNumericalValue(value, "%d", dialog_space[SLIDER_R_SPACE]);
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
	FileSpecifier theme_mml = theme;
	theme_mml.AddPart("theme.mml");
	XML_Loader_SDL loader;
	loader.CurrentElement = &RootParser;
	loader.ParseFile(theme_mml);

	// Open resource file
	FileSpecifier theme_rsrc = theme;
	theme_rsrc.AddPart("resources");
	theme_rsrc.Open(theme_resources);
	clear_game_error();

	// Load fonts
	for (int i=0; i<NUM_DIALOG_FONTS; i++)
		dialog_font[i] = load_font(dialog_font_spec[i]);

	// Load images
	for (int i=0; i<NUM_DIALOG_IMAGES; i++) {
		FileSpecifier file = theme;
		file.AddPart(dialog_image_spec[i].name);
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
	{0x60, 0x60, 0x60}  // KEY_BINDING_COLOR
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

int get_dialog_space(int which)
{
	assert(which >= 0 && which < NUM_DIALOG_SPACES);
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
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		widget *w = *i;
		w->rect.y = y;
		y += w->layout();
		if (w->rect.x < left)
			left = w->rect.x;
		if (w->rect.x + w->rect.w > right)
			right = w->rect.x + w->rect.w;
		i++;
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
	while (i != end) {
		(*i)->rect.x += offset_x;
		i++;
	}
}


/*
 *  Update part of dialog on screen
 */

void dialog::update(SDL_Rect r) const
{
	SDL_Surface *video = SDL_GetVideoSurface();
	SDL_Rect dst_rect = {r.x + rect.x, r.y + rect.y, r.w, r.h};
	SDL_BlitSurface(dialog_surface, &r, video, &dst_rect);
	SDL_UpdateRects(video, 1, &dst_rect);
#ifdef HAVE_OPENGL
	if (video->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif
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
	// Draw frame
	draw_frame_image(frame_tl, 0, 0);
	draw_frame_image(frame_t, frame_tl->w, 0);
	draw_frame_image(frame_tr, frame_tl->w + frame_t->w, 0);
	draw_frame_image(frame_l, 0, frame_tl->h);
	draw_frame_image(frame_r, rect.w - frame_r->w, frame_tr->h);
	draw_frame_image(frame_bl, 0, frame_tl->h + frame_l->h);
	draw_frame_image(frame_b, frame_bl->w, rect.h - frame_b->h);
	draw_frame_image(frame_br, frame_bl->w + frame_b->w, frame_tr->h + frame_r->h);

	// Draw all widgets
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		draw_widget(*i, false);
		i++;
	}

	// Blit to screen
	SDL_Rect r = {0, 0, rect.w, rect.h};
	update(r);
}


/*
 *  Activate widget
 */

void dialog::activate_widget(int num, bool draw)
{
	if (num == active_widget_num)
		return;
	if (!widgets[num]->is_selectable())
		return;

	// Deactivate previously active widget
	if (active_widget) {
		active_widget->active = false;
		if (draw)
			draw_widget(active_widget);
	}

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
	for (int i=0; i<widgets.size(); i++) {
		if (widgets[i]->is_selectable()) {
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
	int i = active_widget_num;
	do {
		i++;
		if (i >= widgets.size())
			i = 0;
	} while (!widgets[i]->is_selectable());
	activate_widget(i);
}

void dialog::activate_prev_widget(void)
{
	int i = active_widget_num;
	do {
		i--;
		if (i < 0)
			i = widgets.size() - 1;
	} while (!widgets[i]->is_selectable());
	activate_widget(i);
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
		if (x >= w->rect.x && y >= w->rect.y && x < w->rect.x + w->rect.w && y < w->rect.y + w->rect.h)
			return num;
		i++; num++;
	}
	return -1;
}


/*
 *  Handle event
 */

void dialog::event(SDL_Event &e)
{
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
				case SDLK_RETURN:		// Return = Action on widget
					active_widget->click(0, 0);
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
				activate_widget(num);
				widget *w = widgets[num];
				w->mouse_move(x - rect.x - w->rect.x, y - rect.y - w->rect.y);
			}
			break;
		}

		// Mouse button pressed
		case SDL_MOUSEBUTTONDOWN: {
			int x = e.button.x, y = e.button.y;
			int num = find_widget(x, y);
			if (num >= 0) {
				widget *w = widgets[num];
				w->click(x - rect.x - w->rect.x, y - rect.y - w->rect.y);
			}
			break;
		}

		// Quit requested
		case SDL_QUIT:
			exit(0);
			break;
	}

	// Redraw dirty widgets
	for (int i=0; i<widgets.size(); i++)
		if (widgets[i]->dirty)
			draw_widget(widgets[i]);
}


/*
 *  Run dialog modally, returns result code (0 = ok, -1 = cancel)
 */

int dialog::run(bool intro_exit_sounds)
{
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

	// Draw dialog
	draw();

	// Show cursor
	bool cursor_was_visible = SDL_ShowCursor(true);

	// Welcome sound
	if (intro_exit_sounds)
		play_dialog_sound(DIALOG_INTRO_SOUND);

	// Enable unicode key translation
	SDL_EnableUNICODE(true);

	// Dialog event loop
	result = 0;
	done = false;
	while (!done) {

		// Get next event
		SDL_Event e;
		e.type = SDL_NOEVENT;
		SDL_PollEvent(&e);

		// Handle event
		event(e);

		// Give time to system
		if (e.type == SDL_NOEVENT) {
			global_idle_proc();
			SDL_Delay(10);
		}
	}

	// Disable unicode key translation
	SDL_EnableUNICODE(false);

	// Farewell sound
	if (intro_exit_sounds)
		play_dialog_sound(result == 0 ? DIALOG_OK_SOUND : DIALOG_CANCEL_SOUND);

	// Hide cursor
	if (!cursor_was_visible)
		SDL_ShowCursor(false);

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(BACKGROUND_COLOR));

	// Erase dialog from screen
	SDL_Surface *video = SDL_GetVideoSurface();
	SDL_FillRect(video, &rect, SDL_MapRGB(video->format, 0, 0, 0));
	SDL_UpdateRects(video, 1, &rect);
#ifdef HAVE_OPENGL
	if (video->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif

	// Free frame images
	if (frame_t) SDL_FreeSurface(frame_t);
	if (frame_l) SDL_FreeSurface(frame_l);
	if (frame_r) SDL_FreeSurface(frame_r);
	if (frame_b) SDL_FreeSurface(frame_b);

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

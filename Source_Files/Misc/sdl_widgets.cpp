/*
 *  sdl_widgets.cpp - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_dialogs.h"
#include "sdl_widgets.h"
#include "resource_manager.h"

#include "shape_descriptors.h"
#include "screen_drawing.h"
#include "images.h"
#include "shell.h"
#include "world.h"
#include "mysound.h"
#include "interface.h"


/*
 *  Widget base class
 */

widget::widget(int f) : active(false), font(get_dialog_font(f, style)) {}

int widget::layout(void)
{
	// Default layout behaviour: center horizontally
	rect.x = -rect.w / 2;
	return rect.h;
}


/*
 *  Static text
 */

w_static_text::w_static_text(const char *t, int f, int c) : widget(f), text(t), color(c)
{
	rect.w = text_width(text, font, style);
	rect.h = font_line_height(font);
}

void w_static_text::draw(SDL_Surface *s) const
{
	draw_text(s, text, rect.x, rect.y + font_ascent(font), get_dialog_color(s, color), font, style);
}


/*
 *  Picture (PICT resource)
 */

w_pict::w_pict(int id)
{
	LoadedResource rsrc;
	get_resource(FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), id, rsrc);
	picture = picture_to_surface(rsrc);
	if (picture) {
		rect.w = picture->w;
		rect.h = picture->h;
		SDL_SetColorKey(picture, SDL_SRCCOLORKEY, SDL_MapRGB(picture->format, 0xff, 0xff, 0xff));
	} else
		rect.w = rect.h = 0;
}

w_pict::~w_pict()
{
	if (picture)
		SDL_FreeSurface(picture);
}

void w_pict::draw(SDL_Surface *s) const
{
	if (picture)
		SDL_BlitSurface(picture, NULL, s, const_cast<SDL_Rect *>(&rect));
}


/*
 *  Button
 */

w_button::w_button(const char *t, action_proc p, void *a) : widget(BUTTON_FONT), text(t), proc(p), arg(a)
{
	rect.w = text_width(text, font, style);
	rect.h = font_line_height(font);
}

void w_button::draw(SDL_Surface *s) const
{
	draw_text(s, text, rect.x, rect.y + font_ascent(font), active ? get_dialog_color(s, BUTTON_ACTIVE_COLOR) : get_dialog_color(s, BUTTON_COLOR), font, style);
}

void w_button::click(int x, int y)
{
	proc(arg);
}


/*
 *  Button on left/right side of dialog box
 */

const int LR_BUTTON_OFFSET = 100;

int w_left_button::layout(void)
{
	rect.x = -(LR_BUTTON_OFFSET + rect.w) / 2;
	return 0;	// This will place the right button on the same y position
}

int w_right_button::layout(void)
{
	rect.x = (LR_BUTTON_OFFSET - rect.w) / 2;
	return rect.h;
}


/*
 *  Selection button
 */

const int SPACING = 8;
const int VISIBLE_CHARS = 32;

w_select_button::w_select_button(const char *n, const char *s, action_proc p, void *a) : widget(NORMAL_FONT), name(n), selection(s), proc(p), arg(a) {}

int w_select_button::layout(void)
{
	int name_width = text_width(name, font, style);
	int max_selection_width = font_width(font) * VISIBLE_CHARS;

	rect.x = -(SPACING + name_width);
	rect.w = name_width + 2 * SPACING + max_selection_width;
	rect.h = font_line_height(font);
	selection_x = name_width + 2 * SPACING;

	return rect.h;
}

void w_select_button::draw(SDL_Surface *s) const
{
	int y = rect.y + font_ascent(font);

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(s, LABEL_ACTIVE_COLOR) : get_dialog_color(s, LABEL_COLOR), font, style);

	// Selection
	set_drawing_clip_rectangle(0, rect.x + selection_x, s->h, rect.x + rect.w);
	draw_text(s, selection, rect.x + selection_x, y, active ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), font, style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);

	// Cursor
	if (active) {
		//!!
	}
}

void w_select_button::click(int x, int y)
{
	proc(arg);
}

void w_select_button::set_selection(const char *s)
{
	selection = s;
	dirty = true;
}


/*
 *  Selection widget (base class)
 */

w_select::w_select(const char *n, int s, const char **l) : widget(NORMAL_FONT), name(n), labels(l), selection(s)
{
	num_labels = 0;
	while (labels[num_labels])
		num_labels++;
	if (selection >= num_labels)
		selection = -1;
}

int w_select::layout(void)
{
	int name_width = text_width(name, font, style);

	int max_label_width = 0;
	for (int i=0; i<num_labels; i++) {
		int width = text_width(labels[i], font, style);
		if (width > max_label_width)
			max_label_width = width;
	}

	rect.x = -(SPACING + name_width);
	rect.w = name_width + 2 * SPACING + max_label_width;
	rect.h = font_line_height(font);
	label_x = name_width + 2 * SPACING;

	return rect.h;
}

void w_select::draw(SDL_Surface *s) const
{
	int y = rect.y + font_ascent(font);

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(s, LABEL_ACTIVE_COLOR) : get_dialog_color(s, LABEL_COLOR), font, style);

	// Selection
	const char *str = (selection >= 0 ? labels[selection] : "<unknown>");
	draw_text(s, str, rect.x + label_x, y, active ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), font, style);

	// Cursor
	if (active) {
		//!!
	}
}

void w_select::click(int x, int y)
{
	selection++;
	if (selection >= num_labels)
		selection = 0;
	selection_changed();
}

void w_select::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_LEFT) {
			selection--;
			if (selection < 0)
				selection = num_labels - 1;
			selection_changed();
			e.type = SDL_NOEVENT;	// Swallow event
		} else if (e.key.keysym.sym == SDLK_RIGHT) {
			selection++;
			if (selection >= num_labels)
				selection = 0;
			selection_changed();
			e.type = SDL_NOEVENT;	// Swallow event
		}
	}
}

void w_select::set_selection(int s)
{
	if (s >= num_labels)
		s = -1;
	else
		selection = s;
	dirty = true;
}

void w_select::selection_changed(void)
{
	play_sound(DIALOG_CLICK_SOUND, NULL, NONE);
	dirty = true;
}


/*
 *  On-off toggle
 */

const char *w_toggle::onoff_labels[] = {"Off", "On", NULL};

w_toggle::w_toggle(const char *name, bool selection, const char **labels) : w_select(name, selection, labels) {}


/*
 *  Player color selection
 */

const char *w_player_color::color_labels[9] = {
	"Slate", "Red", "Violet", "Yellow", "White", "Orange", "Blue", "Green", NULL
};

void w_player_color::draw(SDL_Surface *s) const
{
	int y = rect.y + font_ascent(font);

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(s, LABEL_ACTIVE_COLOR) : get_dialog_color(s, LABEL_COLOR), font, style);

	// Selection
	if (selection >= 0) {
		SDL_Color c;
		_get_interface_color(PLAYER_COLOR_BASE_INDEX + selection, &c);
		uint32 pixel = SDL_MapRGB(s->format, c.r, c.g, c.b);
		SDL_Rect r = {rect.x + label_x, rect.y + 1, 48, rect.h - 2};
		SDL_FillRect(s, &r, pixel);
	} else
		draw_text(s, "<unknown>", rect.x + label_x, y, active ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), font, style);

	// Cursor
	if (active)	{
		//!!
	}
}


/*
 *  Text entry widget
 */

const int TE_VISIBLE_CHARS = 16;

w_text_entry::w_text_entry(const char *n, int max, const char *initial_text) : widget(NORMAL_FONT), name(n), max_chars(max)
{
	// Initialize buffer
	buf = new char[max_chars + 1];
	set_text(initial_text);

	// Get font
	text_font = get_dialog_font(TEXT_ENTRY_FONT, text_style);
}

w_text_entry::~w_text_entry()
{
	delete[] buf;
}

int w_text_entry::layout(void)
{
	int name_width = text_width(name, font, style);
	max_text_width = font_width(text_font) * TE_VISIBLE_CHARS;

	rect.x = -(SPACING + name_width);
	rect.w = name_width + 2 * SPACING + max_text_width;
	rect.h = font_line_height(font);
	text_x = name_width + 2 * SPACING;

	return rect.h;
}

void w_text_entry::draw(SDL_Surface *s) const
{
	int y = rect.y + font_ascent(font);

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(s, LABEL_ACTIVE_COLOR) : get_dialog_color(s, LABEL_COLOR), font, style);

	// Text
	int x = rect.x + text_x;
	int width = text_width(buf, text_font, text_style);
	if (width > max_text_width)
		x -= width - max_text_width;
	set_drawing_clip_rectangle(0, rect.x + text_x, s->h, rect.x + rect.w);
	draw_text(s, buf, x, y, active ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), text_font, text_style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);

	// Cursor
	if (active) {
		SDL_Rect r = {x + width - 1, rect.y, 1, rect.h};
		SDL_FillRect(s, &r, get_dialog_color(s, ITEM_ACTIVE_COLOR));
	}
}

void w_text_entry::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_LEFT:			// Left/right does nothing (for now)
			case SDLK_RIGHT:
				e.type = SDL_NOEVENT;	// Swallow event
				break;

			case SDLK_BACKSPACE:	// Backspace deletes last character
backspace:		if (num_chars) {
					buf[--num_chars] = 0;
					modified_text();
					play_sound(DIALOG_DELETE_SOUND, NULL, NONE);
				}
				break;

			default: {				// Printable characters are entered into the buffer
				uint16 uc = e.key.keysym.unicode;
				if (uc >= ' ' && uc < 0x80 && (num_chars + 1) < max_chars) {
					buf[num_chars++] = uc;
					buf[num_chars] = 0;
					modified_text();
					play_sound(DIALOG_TYPE_SOUND, NULL, NONE);
				} else if (uc == 21) {			// Ctrl-U: erase text
					buf[0] = 0;
					num_chars = 0;
					modified_text();
					play_sound(DIALOG_ERASE_SOUND, NULL, NONE);
				} else if (uc == 4 || uc == 8)	// Ctrl-D/H: backspace
					goto backspace;
				break;
			}
		}
	}
}

void w_text_entry::set_text(const char *text)
{
	memset(buf, 0, max_chars + 1);
	if (text)
		strncpy(buf, text, max_chars);
	num_chars = strlen(buf);
}

void w_text_entry::modified_text(void)
{
	dirty = true;
}


/*
 *  Number entry widget
 */

w_number_entry::w_number_entry(const char *name, int initial_number) : w_text_entry(name, 16, NULL)
{
	set_number(initial_number);
}

void w_number_entry::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		uint16 uc = e.key.keysym.unicode;
		if (uc >= ' ' && uc < 0x80) {
			if (uc < '0' || uc > '9') {
				// Swallow all non-numbers
				e.type = SDL_NOEVENT;
				return;
			}
		}
	}
	w_text_entry::event(e);
}

void w_number_entry::set_number(int number)
{
	char str[16];
	sprintf(str, "%d", number);
	set_text(str);
}


/*
 *  Key name widget
 */

static const char *WAITING_TEXT = "waiting for new key";

w_key::w_key(const char *n, SDLKey key) : widget(NORMAL_FONT), name(n), binding(false)
{
	set_key(key);
}

int w_key::layout(void)
{
	int name_width = text_width(name, font, style);

	rect.x = -(SPACING + name_width);
	rect.w = name_width + 2 * SPACING + text_width(WAITING_TEXT, font, style);
	rect.h = font_line_height(font);
	key_x = name_width + 2 * SPACING;

	return rect.h;
}

void w_key::draw(SDL_Surface *s) const
{
	int y = rect.y + font_ascent(font);

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(s, LABEL_ACTIVE_COLOR) : get_dialog_color(s, LABEL_COLOR), font, style);

	// Key
	int x = rect.x + key_x;
	if (binding) {
		SDL_Rect r = {x, rect.y, text_width(WAITING_TEXT, font, style), rect.h};
		SDL_FillRect(s, &r, get_dialog_color(s, DISABLED_COLOR));
		draw_text(s, WAITING_TEXT, x, y, get_dialog_color(s, ITEM_ACTIVE_COLOR), font, style);
	} else {
		draw_text(s, SDL_GetKeyName(key), x, y, active ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), font, style);
	}
}

void w_key::click(int x, int y)
{
	if (!binding) {
		binding = true;
		dirty = true;
	}
}

void w_key::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		if (binding) {
			if (e.key.keysym.sym != SDLK_ESCAPE)
				set_key(e.key.keysym.sym);
			dirty = true;
			binding = false;
			e.key.keysym.sym = SDLK_DOWN;	// Activate next widget
		}
	}
}

void w_key::set_key(SDLKey k)
{
	key = k;
}

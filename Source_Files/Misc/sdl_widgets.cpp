/*
 *  sdl_widgets.cpp - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_dialogs.h"
#include "sdl_fonts.h"
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
	rect.h = font->get_line_height();
}

void w_static_text::draw(SDL_Surface *s) const
{
	draw_text(s, text, rect.x, rect.y + font->get_ascent(), get_dialog_color(color), font, style);
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

#ifdef __MVCPP__
w_button::w_button(const char *t, action_proc p, void *a) : widget(BUTTON_FONT), text(t), proc(*p), arg(a)
#else
w_button::w_button(const char *t, action_proc p, void *a) : widget(BUTTON_FONT), text(t), proc(p), arg(a)
#endif
{
	rect.w = text_width(text, font, style) + get_dialog_space(BUTTON_L_SPACE) + get_dialog_space(BUTTON_R_SPACE);
	button_l = get_dialog_image(BUTTON_L_IMAGE);
	button_r = get_dialog_image(BUTTON_R_IMAGE);
	button_c = get_dialog_image(BUTTON_C_IMAGE, rect.w - button_l->w - button_r->w);
	rect.h = get_dialog_space(BUTTON_HEIGHT);
}

w_button::~w_button()
{
	if (button_c) SDL_FreeSurface(button_c);
}

void w_button::draw(SDL_Surface *s) const
{
	// Button image
	SDL_Rect r = {rect.x, rect.y, button_l->w, button_l->h};
	SDL_BlitSurface(button_l, NULL, s, &r);
	r.x += button_l->w;
	r.w = button_c->w; r.h = button_c->h;
	SDL_BlitSurface(button_c, NULL, s, &r);
	r.x += button_c->w;
	r.w = button_r->w; r.h = button_r->h;
	SDL_BlitSurface(button_r, NULL, s, &r);

	// Label
	draw_text(s, text, rect.x + get_dialog_space(BUTTON_L_SPACE), rect.y + get_dialog_space(BUTTON_T_SPACE) + font->get_ascent(), active ? get_dialog_color(BUTTON_ACTIVE_COLOR) : get_dialog_color(BUTTON_COLOR), font, style);
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

const int MAX_TEXT_WIDTH = 200;

w_select_button::w_select_button(const char *n, const char *s, action_proc p, void *a) : widget(LABEL_FONT), name(n), selection(s), proc(p), arg(a) {}

int w_select_button::layout(void)
{
	int name_width = text_width(name, font, style);
	int max_selection_width = MAX_TEXT_WIDTH;
	int spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -(spacing / 2 + name_width);
	rect.w = name_width + spacing + max_selection_width;
	rect.h = font->get_line_height();
	selection_x = name_width + spacing;

	return rect.h;
}

void w_select_button::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Selection
	set_drawing_clip_rectangle(0, rect.x + selection_x, s->h, rect.x + rect.w);
	draw_text(s, selection, rect.x + selection_x, y, active ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
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

w_select::w_select(const char *n, int s, const char **l) : widget(LABEL_FONT), name(n), labels(l), selection(s)
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
	max_label_width += 6;

	int spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -(spacing / 2 + name_width);
	rect.w = name_width + spacing + max_label_width;
	rect.h = font->get_line_height();
	label_x = name_width + spacing;

	return rect.h;
}

void w_select::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Selection
	const char *str = (selection >= 0 ? labels[selection] : "<unknown>");
	draw_text(s, str, rect.x + label_x, y, active ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);

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
	selection = s;
	dirty = true;
}

void w_select::selection_changed(void)
{
	play_dialog_sound(DIALOG_CLICK_SOUND);
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
	int y = rect.y + font->get_ascent();

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Selection
	if (selection >= 0) {
		SDL_Color c;
		_get_interface_color(PLAYER_COLOR_BASE_INDEX + selection, &c);
		uint32 pixel = SDL_MapRGB(s->format, c.r, c.g, c.b);
		SDL_Rect r = {rect.x + label_x, rect.y + 1, 48, rect.h - 2};
		SDL_FillRect(s, &r, pixel);
	} else
		draw_text(s, "<unknown>", rect.x + label_x, y, active ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);

	// Cursor
	if (active)	{
		//!!
	}
}


/*
 *  Text entry widget
 */

w_text_entry::w_text_entry(const char *n, int max, const char *initial_text) : widget(LABEL_FONT), name(n), max_chars(max)
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
	max_text_width = MAX_TEXT_WIDTH;
	int spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -(spacing / 2 + name_width);
	rect.w = name_width + spacing + max_text_width;
	rect.h = font->get_line_height();
	text_x = name_width + spacing;

	return rect.h;
}

void w_text_entry::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Text
	int x = rect.x + text_x;
	int width = text_width(buf, text_font, text_style);
	if (width > max_text_width)
		x -= width - max_text_width;
	set_drawing_clip_rectangle(0, rect.x + text_x, s->h, rect.x + rect.w);
	draw_text(s, buf, x, y, active ? get_dialog_color(TEXT_ENTRY_ACTIVE_COLOR) : get_dialog_color(TEXT_ENTRY_COLOR), text_font, text_style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);

	// Cursor
	if (active) {
		SDL_Rect r = {x + width - 1, rect.y, 1, rect.h};
		SDL_FillRect(s, &r, get_dialog_color(TEXT_ENTRY_CURSOR_COLOR));
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
					play_dialog_sound(DIALOG_DELETE_SOUND);
				}
				break;

			default: {				// Printable characters are entered into the buffer
				uint16 uc = e.key.keysym.unicode;
				if (uc >= ' ' && uc < 0x80 && (num_chars + 1) < max_chars) {
					buf[num_chars++] = uc;
					buf[num_chars] = 0;
					modified_text();
					play_dialog_sound(DIALOG_TYPE_SOUND);
				} else if (uc == 21) {			// Ctrl-U: erase text
					buf[0] = 0;
					num_chars = 0;
					modified_text();
					play_dialog_sound(DIALOG_ERASE_SOUND);
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
	modified_text();
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

w_key::w_key(const char *n, SDLKey key) : widget(LABEL_FONT), name(n), binding(false)
{
	set_key(key);
}

int w_key::layout(void)
{
	int name_width = text_width(name, font, style);
	int spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -(spacing / 2 + name_width);
	rect.w = name_width + spacing + text_width(WAITING_TEXT, font, style);
	rect.h = font->get_line_height();
	key_x = name_width + spacing;

	return rect.h;
}

void w_key::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Key
	int x = rect.x + key_x;
	if (binding) {
		SDL_Rect r = {x, rect.y, text_width(WAITING_TEXT, font, style), rect.h};
		SDL_FillRect(s, &r, get_dialog_color(KEY_BINDING_COLOR));
		draw_text(s, WAITING_TEXT, x, y, get_dialog_color(ITEM_ACTIVE_COLOR), font, style);
	} else {
		draw_text(s, SDL_GetKeyName(key), x, y, active ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
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


/*
 *  Slider
 */

const int SLIDER_WIDTH = 160;

w_slider::w_slider(const char *n, int num, int s) : widget(LABEL_FONT), name(n), selection(s), num_items(num), thumb_dragging(false)
{
	slider_l = get_dialog_image(SLIDER_L_IMAGE);
	slider_r = get_dialog_image(SLIDER_R_IMAGE);
	slider_c = get_dialog_image(SLIDER_C_IMAGE, SLIDER_WIDTH - slider_l->w - slider_r->w);
	thumb = get_dialog_image(SLIDER_IMAGE);
	trough_width = SLIDER_WIDTH - get_dialog_space(SLIDER_L_SPACE) - get_dialog_space(SLIDER_R_SPACE);
}

w_slider::~w_slider()
{
	if (slider_c) SDL_FreeSurface(slider_c);
}

int w_slider::layout(void)
{
	int name_width = text_width(name, font, style);
	int spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -(spacing / 2 + name_width);
	rect.w = name_width + spacing + SLIDER_WIDTH;
	rect.h = MAX(font->get_line_height(), slider_c->h);
	slider_x = name_width + spacing;
	set_selection(selection);

	return rect.h;
}

void w_slider::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent() + (rect.h - font->get_line_height()) / 2;

	// Name
	draw_text(s, name, rect.x, y, active ? get_dialog_color(LABEL_ACTIVE_COLOR) : get_dialog_color(LABEL_COLOR), font, style);

	// Slider trough
	SDL_Rect r = {rect.x + slider_x, rect.y, slider_l->w, slider_l->h};
	SDL_BlitSurface(slider_l, NULL, s, &r);
	r.x += slider_l->w;
	r.w = slider_c->w; r.h = slider_c->h;
	SDL_BlitSurface(slider_c, NULL, s, &r);
	r.x += slider_c->w;
	r.w = slider_r->w; r.h = slider_r->h;
	SDL_BlitSurface(slider_r, NULL, s, &r);

	// Slider thumb
	r.x = rect.x + thumb_x;
	r.y = rect.y + get_dialog_space(SLIDER_T_SPACE);
	r.w = thumb->w; r.h = thumb->h;
	SDL_BlitSurface(thumb, NULL, s, &r);
}

void w_slider::mouse_move(int x, int y)
{
	if (thumb_dragging) {
		int delta_x = (x - slider_x - get_dialog_space(SLIDER_L_SPACE)) - thumb_drag_x;
		set_selection(delta_x * num_items / trough_width);
	}
}

void w_slider::click(int x, int y)
{
	if (x >= slider_x && x < slider_x + SLIDER_WIDTH) {
		thumb_dragging = dirty = true;
		thumb_drag_x = x - thumb_x;
	}
}

void w_slider::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_LEFT) {
			set_selection(selection - 1);
			item_selected();
			e.type = SDL_NOEVENT;	// Swallow event
		} else if (e.key.keysym.sym == SDLK_RIGHT) {
			set_selection(selection + 1);
			item_selected();
			e.type = SDL_NOEVENT;	// Swallow event
		}
	} else if (e.type == SDL_MOUSEBUTTONUP) {
		if (thumb_dragging) {
			thumb_dragging = false;
			dirty = true;
			item_selected();
		}
	}
}

void w_slider::set_selection(int s)
{
	if (s >= num_items)
		s = num_items - 1;
	else if (s < 0)
		s = 0;
	selection = s;
	thumb_x = int(float(selection * (trough_width - thumb->w)) / (num_items - 1) + 0.5);
	thumb_x += get_dialog_space(SLIDER_L_SPACE) + slider_x;
	dirty = true;
}


/*
 *  List selection
 */

w_list_base::w_list_base(int width, int lines, int sel) : widget(ITEM_FONT), shown_items(lines), thumb_dragging(false)
{
	font_height = font->get_line_height();
	rect.w = width;
	rect.h = font_height * shown_items + get_dialog_space(LIST_T_SPACE) + get_dialog_space(LIST_B_SPACE);

	frame_tl = get_dialog_image(LIST_TL_IMAGE);
	frame_tr = get_dialog_image(LIST_TR_IMAGE);
	frame_bl = get_dialog_image(LIST_BL_IMAGE);
	frame_br = get_dialog_image(LIST_BR_IMAGE);
	frame_t = get_dialog_image(LIST_T_IMAGE, rect.w - frame_tl->w - frame_tr->w, 0);
	frame_l = get_dialog_image(LIST_L_IMAGE, 0, rect.h - frame_tl->h - frame_bl->h);
	frame_r = get_dialog_image(LIST_R_IMAGE, 0, rect.h - frame_tr->h - frame_br->h);
	frame_b = get_dialog_image(LIST_B_IMAGE, rect.w - frame_bl->w - frame_br->w, 0);

	thumb_t = get_dialog_image(THUMB_T_IMAGE);
	thumb_tc = NULL;
	SDL_Surface *thumb_tc_unscaled = get_dialog_image(THUMB_TC_IMAGE);
	thumb_c = get_dialog_image(THUMB_C_IMAGE);
	SDL_Surface *thumb_bc_unscaled = get_dialog_image(THUMB_BC_IMAGE);
	thumb_bc = NULL;
	thumb_b = get_dialog_image(THUMB_B_IMAGE);

	min_thumb_height = thumb_t->h + thumb_tc_unscaled->h + thumb_c->h + thumb_bc_unscaled->h + thumb_b->h;

	trough_rect.x = rect.w - get_dialog_space(TROUGH_R_SPACE);
	trough_rect.y = get_dialog_space(TROUGH_T_SPACE);
	trough_rect.w = get_dialog_space(TROUGH_WIDTH);
	trough_rect.h = rect.h - get_dialog_space(TROUGH_T_SPACE) - get_dialog_space(TROUGH_B_SPACE);
}

w_list_base::~w_list_base()
{
	if (frame_t) SDL_FreeSurface(frame_t);
	if (frame_l) SDL_FreeSurface(frame_l);
	if (frame_r) SDL_FreeSurface(frame_r);
	if (frame_b) SDL_FreeSurface(frame_b);
	if (thumb_tc) SDL_FreeSurface(thumb_tc);
	if (thumb_bc) SDL_FreeSurface(thumb_bc);
}

int w_list_base::layout(void)
{
	rect.x = -rect.w / 2;
	return rect.h;
}

void w_list_base::draw_image(SDL_Surface *dst, SDL_Surface *s, int x, int y) const
{
	SDL_Rect r = {x, y, s->w, s->h};
	SDL_BlitSurface(s, NULL, dst, &r);
}

void w_list_base::draw(SDL_Surface *s) const
{
	// Draw frame
	int x = rect.x;
	int y = rect.y;
	draw_image(s, frame_tl, x, y);
	draw_image(s, frame_t, x + frame_tl->w, y);
	draw_image(s, frame_tr, x + frame_tl->w + frame_t->w, y);
	draw_image(s, frame_l, x, y + frame_tl->h);
	draw_image(s, frame_r, x + rect.w - frame_r->w, y + frame_tr->h);
	draw_image(s, frame_bl, x, y + frame_tl->h + frame_l->h);
	draw_image(s, frame_b, x + frame_bl->w, y + rect.h - frame_b->h);
	draw_image(s, frame_br, x + frame_bl->w + frame_b->w, y + frame_tr->h + frame_r->h);

	// Draw thumb
	x = rect.x + trough_rect.x;
	y = rect.y + thumb_y;
	draw_image(s, thumb_t, x, y);
	draw_image(s, thumb_tc, x, y += thumb_t->h);
	draw_image(s, thumb_c, x, y += thumb_tc->h);
	draw_image(s, thumb_bc, x, y += thumb_c->h);
	draw_image(s, thumb_b, x, y += thumb_bc->h);

	// Draw items
	draw_items(s);
}

void w_list_base::mouse_move(int x, int y)
{
	if (thumb_dragging) {
		int delta_y = y - thumb_drag_y;
		set_top_item(delta_y * num_items / trough_rect.h);
	} else {
		if (x < get_dialog_space(LIST_L_SPACE) || x >= rect.w - get_dialog_space(LIST_R_SPACE)
		 || y < get_dialog_space(LIST_T_SPACE) || y >= rect.h - get_dialog_space(LIST_B_SPACE))
			return;
		set_selection((y - get_dialog_space(LIST_T_SPACE)) / font_height + top_item);
	}
}

void w_list_base::click(int x, int y)
{
	if (x >= trough_rect.x && x < trough_rect.x + trough_rect.w
	 && y >= trough_rect.y && y < trough_rect.y + trough_rect.h) {
		thumb_dragging = dirty = true;
		thumb_drag_y = y - thumb_y;
	} else if (num_items > 0 && is_item_selectable(selection))
		item_selected();
}

void w_list_base::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_UP:
				set_selection(selection - 1);
				e.type = SDL_NOEVENT;	// Prevent selection of previous widget
				break;
			case SDLK_DOWN:
				set_selection(selection + 1);
				e.type = SDL_NOEVENT;	// Prevent selection of next widget
				break;
			case SDLK_PAGEUP:
				set_selection(selection - shown_items);
				break;
			case SDLK_PAGEDOWN:
				set_selection(selection + shown_items);
				break;
			case SDLK_HOME:
				set_selection(0);
				break;
			case SDLK_END:
				set_selection(num_items - 1);
				break;
			default:
				break;
		}
	} else if (e.type == SDL_MOUSEBUTTONUP) {
		if (thumb_dragging) {
			thumb_dragging = false;
			dirty = true;
		}
	}
}

void w_list_base::set_selection(int s)
{
	// Set selection, check for bounds
	if (s < 0)
		s = 0;
	else if (s >= num_items)
		s = num_items - 1;
	if (s != selection)
		dirty = true;
	selection = s;

	// Make selection visible
	if (s < top_item)
		set_top_item(s);
	else if (s >= top_item + shown_items)
		set_top_item(s - shown_items + 1);
}

void w_list_base::new_items(void)
{
	// Reset top item and selection
	top_item = selection = 0;
	dirty = true;

	// Calculate thumb height
	if (num_items <= shown_items)
		thumb_height = trough_rect.h;
	else if (num_items == 0)
		thumb_height = shown_items * trough_rect.h;
	else
		thumb_height = int(float(shown_items * trough_rect.h) / num_items + 0.5);
	if (thumb_height < min_thumb_height)
		thumb_height = min_thumb_height;
	else if (thumb_height > trough_rect.h)
		thumb_height = trough_rect.h;

	// Create dynamic thumb images
	if (thumb_tc)
		SDL_FreeSurface(thumb_tc);
	if (thumb_bc)
		SDL_FreeSurface(thumb_bc);
	int rem_height = thumb_height - thumb_t->h - thumb_c->h - thumb_b->h;
	int dyn_height = rem_height / 2;
	thumb_tc = get_dialog_image(THUMB_TC_IMAGE, 0, dyn_height);
	thumb_bc = get_dialog_image(THUMB_BC_IMAGE, 0, (rem_height & 1) ? dyn_height + 1 : dyn_height);
}

void w_list_base::center_item(int i)
{
	set_top_item(selection - shown_items / 2);
}

void w_list_base::set_top_item(int i)
{
	// Set top item (check for bounds)
	if (i > num_items - shown_items)
		i = num_items - shown_items;
	if (i < 0)
		i = 0;
	if (i != top_item)
		dirty = true;
	top_item = i;

	// Calculate thumb y position
	if (num_items == 0)
		thumb_y = 0;
	else
		thumb_y = int(float(top_item * trough_rect.h) / num_items + 0.5);
	if (thumb_y > trough_rect.h - thumb_height)
		thumb_y = trough_rect.h - thumb_height;
	thumb_y += trough_rect.y;
}

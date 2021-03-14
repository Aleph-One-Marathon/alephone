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
 *  sdl_widgets.cpp - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Sept-Nov. 2001 (Woody Zenfell):
 *      Significant extensions to support more dynamic behavior and more useful layouts
 *
 *  Mar 1, 2002 (Woody Zenfell):
 *      Moved w_levels here from shell_sdl; am using it in Setup Network Game box.
 *
 *  May 16, 2002 (Woody Zenfell):
 *      changes to w_key to support assignment of mouse buttons as well as keys;
 *      also fixed mouse-movement-while-binding behavior.
 *
 *  August 27, 2003 (Woody Zenfell):
 *	new w_enabling_toggle can enable/disable a bank of other widgets according to its state
 *	new w_file_chooser displays filename; allows selection of file (via FileSpecifier::ReadDialog())
 */

#include "cseries.h"
#include "sdl_dialogs.h"
#include "network_dialog_widgets_sdl.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"
#include "resource_manager.h"

#include "shape_descriptors.h"
#include "screen_drawing.h"
#include "images.h"
#include "shell.h"
#include "world.h"
#include "SoundManager.h"
#include "interface.h"
#include "player.h"

#include "screen.h"

// ZZZ: for stringset business for modified w_select
#include	"TextStrings.h"

#include    "mouse.h"   // (ZZZ) NUM_SDL_MOUSE_BUTTONS, SDLK_BASE_MOUSE_BUTTON
#include "joystick.h"

#include <sstream>

/*
 *  Widget base class
 */

// ZZZ: initialize my additional storage elements
// (thought: I guess "widget" could be simplified, and have a subclass "useful_widget" to handle most things
// other than spacers.  Spacers are common and I guess we're starting to eat a fair amount of storage for a
// widget that does nothing and draws nothing... oh well, at least RAM is cheap.  ;) )
widget::widget() : active(false), dirty(false), enabled(true), font(NULL), identifier(NONE), owning_dialog(NULL), saved_min_width(0), saved_min_height(0), associated_label(0)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
}

widget::widget(int theme_widget) : active(false), dirty(false), enabled(true), font(get_theme_font(theme_widget, style)), identifier(NONE), owning_dialog(NULL), saved_min_width(0), saved_min_height(0), associated_label(0)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
}

void widget::associate_label(w_label *label)
{
	associated_label = label;
}

w_label *widget::label(const char *text)
{
	if (!associated_label)
	{
		associated_label = new w_label(text);
		associated_label->associate_widget(this);
	}

	return associated_label;
}

// ZZZ: enable/disable
void widget::set_enabled(bool inEnabled)
{
	if(enabled != inEnabled) {
		enabled = inEnabled;
		
		// If we had the focus when we were disabled, we should not have the focus afterward.
		if(active && !enabled)
			owning_dialog->activate_next_widget();
		
		// Assume we need a redraw to reflect new state
		dirty = true;
		
		if (associated_label)
			associated_label->set_enabled(inEnabled);
	}
}

void widget::set_active(bool new_active)
{
	// Assume we need a redraw to reflect new state
	if (enabled && (active != new_active))
		dirty = true;
	active = new_active;
}

void widget::place(const SDL_Rect &r, placement_flags flags)
{
	rect.h = r.h;
	rect.y = r.y;

	if (flags & placeable::kFill)
	{
		rect.x = r.x;
		rect.w = r.w;
	}
	else
	{
		rect.w = saved_min_width;
		if (flags & placeable::kAlignLeft)
		{
			rect.x = r.x;
		}
		else if (flags & placeable::kAlignRight)
		{
			rect.x = r.x + r.w - saved_min_width;
		}
		else
		{
			rect.x = r.x + (r.w - saved_min_width) / 2;
		}
	}
}


/*
 *  Static text
 */

// ZZZ change: copy the given string instead of just pointing to it.  Much easier for messages that change.
w_static_text::w_static_text(const char *t, int _theme_type) : widget(_theme_type), theme_type(_theme_type)
{
        text = strdup(t);
	rect.w = text_width(text, font, style);
	rect.h = font->get_line_height();
	saved_min_height = rect.h;
	saved_min_width = rect.w;
}

void w_static_text::draw(SDL_Surface *s) const
{
	uint32 pixel;
	pixel = get_theme_color(theme_type, DEFAULT_STATE, 0);

	draw_text(s, text, rect.x, rect.y + font->get_ascent(), pixel, font, style);
}

void w_label::click(int x, int y)
{
	if (associated_widget)
		associated_widget->click(0, 0);

}

void w_label::draw(SDL_Surface *s) const
{
	int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;
	uint16 style = 0;
	draw_text(s, text, rect.x, rect.y + font->get_ascent() + (rect.h - font->get_line_height()) / 2, get_theme_color(LABEL_WIDGET, state, FOREGROUND_COLOR), font, style);
}

// ZZZ addition: change text.
void
w_static_text::set_text(const char* t) {
    free(text);
    text = strdup(t);
    dirty = true;
}

// ZZZ addition: free text buffer.
w_static_text::~w_static_text() {
    free(text);
}

w_styled_text::w_styled_text(const char *t, int _theme_type) : w_static_text(t, _theme_type), text_string(t) {
	rect.w = font->styled_text_width(text_string, text_string.size(), style);
	saved_min_width = rect.w;
}

void w_styled_text::set_text(const char* t)
{
	w_static_text::set_text(t);
	text_string = t;
	// maybe reset rect.width here, but parent w_static_text doesn't, so we won't either
}

void w_styled_text::draw(SDL_Surface *s) const
{
	uint32 pixel;
	pixel = get_theme_color(theme_type, DEFAULT_STATE, 0);

	font->draw_styled_text(s, text_string, text_string.size(), rect.x, rect.y + font->get_ascent(), pixel, style);
}

void w_slider_text::draw(SDL_Surface *s) const
{
	int state = associated_slider->enabled ? (associated_slider->active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;
	uint16 style = 0;
	draw_text(s, text, rect.x, rect.y + font->get_ascent() + (rect.h - font->get_line_height()) / 2, get_theme_color(LABEL_WIDGET, state, FOREGROUND_COLOR), font, style);
}


/*
 *  Button
 */

w_button_base::w_button_base(const char *t, action_proc p, void *a, int _type) : widget(_type), text(t), proc(p), arg(a), down(false), pressed(false), type(_type)
{
	rect.w = text_width(text.c_str(), font, style) + get_theme_space(_type, BUTTON_L_SPACE) + get_theme_space(_type, BUTTON_R_SPACE);
	button_c_default = get_theme_image(_type, DEFAULT_STATE, BUTTON_C_IMAGE, rect.w - get_theme_image(_type, DEFAULT_STATE, BUTTON_L_IMAGE)->w - get_theme_image(_type, DEFAULT_STATE, BUTTON_R_IMAGE)->w);
	button_c_active = get_theme_image(_type, ACTIVE_STATE, BUTTON_C_IMAGE, rect.w - get_theme_image(_type, ACTIVE_STATE, BUTTON_L_IMAGE)->w - get_theme_image(_type, ACTIVE_STATE, BUTTON_R_IMAGE)->w);
	button_c_disabled = get_theme_image(_type, DISABLED_STATE, BUTTON_C_IMAGE, rect.w - get_theme_image(_type, DISABLED_STATE, BUTTON_L_IMAGE)->w - get_theme_image(_type, DISABLED_STATE, BUTTON_R_IMAGE)->w);
	button_c_pressed = get_theme_image(_type, PRESSED_STATE, BUTTON_C_IMAGE, rect.w - get_theme_image(_type, PRESSED_STATE, BUTTON_L_IMAGE)->w - get_theme_image(_type, PRESSED_STATE, BUTTON_R_IMAGE)->w);

	rect.h = static_cast<uint16>(get_theme_space(_type, BUTTON_HEIGHT));

	saved_min_width = rect.w;
	saved_min_height = rect.h;
}

w_button_base::~w_button_base()
{
	if (button_c_default) SDL_FreeSurface(button_c_default);
	if (button_c_active) SDL_FreeSurface(button_c_active);
	if (button_c_disabled) SDL_FreeSurface(button_c_disabled);
	if (button_c_pressed) SDL_FreeSurface(button_c_pressed);
}

void w_button_base::set_callback(action_proc p, void *a)
{
	proc = p;
	arg = a;
}

void w_button_base::draw(SDL_Surface *s) const
{
	// Label (ZZZ: different color for disabled)
	int state = DEFAULT_STATE;
	if (pressed)
		state = PRESSED_STATE;
	else if (!enabled)
		state = DISABLED_STATE;
	else if (active)
		state = ACTIVE_STATE;
		
	if (use_theme_images(type))
	{
		SDL_Surface *button_l = get_theme_image(type, state, BUTTON_L_IMAGE);
		SDL_Surface *button_r = get_theme_image(type, state, BUTTON_R_IMAGE);
		SDL_Surface *button_c = button_c_default;
		if (pressed)
			button_c = button_c_pressed;
		else if (!enabled)
			button_c = button_c_disabled;
		else if (active)
			button_c = button_c_active;

		// Button image
		SDL_Rect r = {rect.x, rect.y, 
			      static_cast<Uint16>(button_l->w), 
			      static_cast<Uint16>(button_l->h)};
		SDL_BlitSurface(button_l, NULL, s, &r);
		r.x = r.x + static_cast<Sint16>(button_l->w); // MDA: MSVC throws warnings if we use +=
		r.w = static_cast<Uint16>(button_c->w);
		r.h = static_cast<Uint16>(button_c->h);
		SDL_BlitSurface(button_c, NULL, s, &r);
		r.x = r.x + static_cast<Sint16>(button_c->w);
		r.w = static_cast<Uint16>(button_r->w);
		r.h = static_cast<Uint16>(button_r->h);
		SDL_BlitSurface(button_r, NULL, s, &r);
	}
	else
	{
		uint32 pixel;
		if (use_theme_color(type, BACKGROUND_COLOR))
		{
			uint32 pixel = get_theme_color(type, state, BACKGROUND_COLOR);
			SDL_Rect r = {rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2};
			SDL_FillRect(s, &r, pixel);
		}

		pixel = get_theme_color(type, state, FRAME_COLOR);
		draw_rectangle(s, &rect, pixel);
	}

	draw_text(s, text.c_str(), rect.x + get_theme_space(type, BUTTON_L_SPACE),
		  rect.y + get_theme_space(type, BUTTON_T_SPACE) + font->get_ascent(),
		  get_theme_color(type, state), font, style);
}

void w_button_base::mouse_move(int x, int y)
{
	if (down)
	{
		if (x >= 0 && x <= rect.w && y >= 0 && y <= rect.h)
		{
			if (!pressed)
				dirty = true;
			pressed = true;
		}
		else
		{
			if (pressed)
				dirty = true;
			pressed = false;
		}
		get_owning_dialog()->draw_dirty_widgets();
	}
}

void w_button_base::mouse_down(int, int)
{
	if (!enabled) return;
	down = true;
	pressed = true;
	dirty = true;
	get_owning_dialog()->draw_dirty_widgets();
}

void w_button_base::mouse_up(int x, int y)
{
	if (!enabled) return;

	down = false;
	pressed = false;
	dirty = true;
	get_owning_dialog()->draw_dirty_widgets();
	
	if (proc && x >= 0 && x <= rect.w && y >= 0 && y <= rect.h)
		proc(arg);
}

void w_button_base::click(int /*x*/, int /*y*/)
{
	// simulate a mouse press
	mouse_down(0, 0);
	sleep_for_machine_ticks(MACHINE_TICKS_PER_SECOND / 12);
	mouse_up(0, 0);
}

/*
 * Clickable link
 */

void w_hyperlink::prochandler(void *arg)
{
	toggle_fullscreen(false);
	launch_url_in_browser(static_cast<const char *>(arg));
	get_owning_dialog()->draw();
}

w_hyperlink::w_hyperlink(const char *url, const char *txt) : w_button_base((txt ? txt : url), boost::bind(&w_hyperlink::prochandler, this, _1), const_cast<char *>(url), HYPERLINK_WIDGET)
{
	rect.w = text_width(text.c_str(), font, style);
	rect.h = font->get_line_height();
	saved_min_height = rect.h;
	saved_min_width = rect.w;
}

void w_hyperlink::draw(SDL_Surface *s) const
{
	int state = DEFAULT_STATE;
	if (pressed)
		state = PRESSED_STATE;
	else if (!enabled)
		state = DISABLED_STATE;
	else if (active)
		state = ACTIVE_STATE;
	
	uint32 pixel = get_theme_color(HYPERLINK_WIDGET, state, 0);
	
	draw_text(s, text.c_str(), rect.x, rect.y + font->get_ascent(), pixel, font, style);
	
	// draw_text doesn't support underline, so draw one manually
	if (style & styleUnderline)
	{
		SDL_Rect r = {rect.x, rect.y + rect.h - 1, rect.w, 1};
		SDL_FillRect(s, &r, pixel);
	}
}


/* 
 * Tabs
 */

w_tab::w_tab(const vector<string>& _labels, tab_placer *_placer) : widget(TAB_WIDGET), labels(_labels), placer(_placer), active_tab(1), pressed_tab(0)
{
	saved_min_height = get_theme_space(TAB_WIDGET, BUTTON_HEIGHT);
	for (vector<string>::iterator it = labels.begin(); it != labels.end(); ++it)
	{
		int l_space = (it == labels.begin()) ? get_theme_space(TAB_WIDGET, BUTTON_L_SPACE) : get_theme_space(TAB_WIDGET, TAB_LC_SPACE);
		int r_space = (it == labels.end() - 1) ? get_theme_space(TAB_WIDGET, BUTTON_R_SPACE) : get_theme_space(TAB_WIDGET, TAB_RC_SPACE);
		int width = l_space + r_space + font->text_width(it->c_str(), style);
		widths.push_back(width);
		saved_min_width += width;

		// load center images
		const int states[] = { DEFAULT_STATE, PRESSED_STATE, ACTIVE_STATE, DISABLED_STATE };
		images.resize(PRESSED_STATE + 1);
		for (int i = 0; i < 4; ++i)
		{
			int li_space = (it == labels.begin()) ? get_theme_image(TAB_WIDGET, states[i], TAB_L_IMAGE)->w : get_theme_image(TAB_WIDGET, states[i], TAB_LC_IMAGE)->w;
			int ri_space = (it == labels.end() - 1) ? get_theme_image(TAB_WIDGET, states[i], TAB_R_IMAGE)->w : get_theme_image(TAB_WIDGET, states[i], TAB_RC_IMAGE)->w;
			int c_space = width - li_space - ri_space;
			images[states[i]].push_back(get_theme_image(TAB_WIDGET, states[i], TAB_C_IMAGE, c_space));
		}
	}

}

w_tab::~w_tab()
{
	for (std::vector<std::vector<SDL_Surface *> >::iterator it = images.begin(); it != images.end(); ++it)
	{
		for (std::vector<SDL_Surface *>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
		{
			SDL_FreeSurface(*it2);
		}
	}
}

void w_tab::draw(SDL_Surface *s) const 
{
	int x = rect.x;
	for (int i = 0; i < labels.size(); ++i)
	{
		int state;
		if (!enabled)
			state = DISABLED_STATE;
		else if (i == pressed_tab)
			state = PRESSED_STATE;
		else if (active && i == active_tab)
			state = ACTIVE_STATE;
		else
			state = DEFAULT_STATE;

		int l_space;
		int l_offset = 0;

		SDL_Surface *l_image;
		if (i == 0)
		{
			l_space = get_theme_space(TAB_WIDGET, BUTTON_L_SPACE);
			l_offset = 1;
			l_image = get_theme_image(TAB_WIDGET, state, TAB_L_IMAGE);
		}
		else
		{
			l_space = get_theme_space(TAB_WIDGET, TAB_LC_SPACE);
			l_image = get_theme_image(TAB_WIDGET, state, TAB_LC_IMAGE);
		}

		int r_space;
		int r_offset = 0;
		SDL_Surface *r_image;
		if (i == labels.size() - 1)
		{
			r_space = get_theme_space(TAB_WIDGET, BUTTON_R_SPACE);
			r_offset = 1;
			r_image = get_theme_image(TAB_WIDGET, state, TAB_R_IMAGE);
		}
		else
		{
			r_space = get_theme_space(TAB_WIDGET, TAB_RC_SPACE);
			r_image = get_theme_image(TAB_WIDGET, state, TAB_RC_IMAGE);
		}

		int c_space;
		SDL_Surface *c_image = images[state][i];
		c_space = font->text_width(labels[i].c_str(), style);

		if (use_theme_images(TAB_WIDGET))
		{
			SDL_Rect r = { x, rect.y, static_cast<Uint16>(l_image->w), static_cast<Uint16>(l_image->h) };
			SDL_BlitSurface(l_image, NULL, s, &r);
			r.x = r.x + static_cast<Sint16>(l_image->w);
			r.w = static_cast<Uint16>(c_image->w);
			r.h = static_cast<Uint16>(c_image->h);
			SDL_BlitSurface(c_image, NULL, s, &r);
			r.x = r.x + static_cast<Sint16>(c_image->w);
			r.w = static_cast<Uint16>(r_image->w);
			r.h = static_cast<Uint16>(r_image->h);
			SDL_BlitSurface(r_image, NULL, s, &r);
		}
		else
		{
			if (use_theme_color(TAB_WIDGET, BACKGROUND_COLOR))
			{
				SDL_Rect r = { x + l_offset, rect.y + 1, l_space + c_space + r_space - l_offset - r_offset, get_theme_space(TAB_WIDGET, BUTTON_HEIGHT) - 2 };
				uint32 pixel = get_theme_color(TAB_WIDGET, state, BACKGROUND_COLOR);
				SDL_FillRect(s, &r, pixel);
			}
		}

		font->draw_text(s, labels[i].c_str(), labels[i].size(), x + l_space, rect.y + get_theme_space(TAB_WIDGET, BUTTON_T_SPACE) + font->get_ascent(), get_theme_color(TAB_WIDGET, state, FOREGROUND_COLOR), style);

		x += l_space + c_space + r_space;
	}

	if (!use_theme_images(TAB_WIDGET))
	{
		uint32 pixel = get_theme_color(TAB_WIDGET, DEFAULT_STATE, FRAME_COLOR);
		// draw the frame
		SDL_Rect frame = { rect.x, rect.y, x - rect.x, get_theme_space(TAB_WIDGET, BUTTON_HEIGHT) };
		draw_rectangle(s, &frame, pixel);
	}
}

void w_tab::choose_tab(int i)
{
	pressed_tab = i;
	active_tab = (i + 1) % labels.size();
	placer->choose_tab(i);
	get_owning_dialog()->draw();
}

void w_tab::click(int x, int y)
{
	if (enabled)
	{
		if (!x && !y)
		{
			choose_tab(active_tab);
		}
		else
		{
			int width = 0;
			for (int i = 0; i < labels.size(); ++i)
			{
				if (x > width && x < width + widths[i])
				{
					choose_tab(i);
					return;
				}

				width += widths[i];
			}
		}
	}
}

void w_tab::event(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN)
	{
		switch (e.key.keysym.sym) {
		case SDLK_LEFT:
			if (active_tab > 0)
			{
				if (active_tab - 1== pressed_tab)
				{
					if (pressed_tab > 0)
						active_tab -= 2;
				}
				else
					active_tab--;

			}
			dirty = true;
			e.type = SDL_LASTEVENT;
			break;

		case SDLK_RIGHT:
			if (active_tab < labels.size() - 1)
			{
				if (active_tab + 1 == pressed_tab)
				{
					if (pressed_tab < labels.size() - 1)
						active_tab += 2;
				}
				else
					active_tab++;
			}
			dirty = true;
			e.type = SDL_LASTEVENT;
			break;
		
		default:
			break;

		}
	}
	else if (e.type == SDL_CONTROLLERBUTTONDOWN)
	{
		switch (e.cbutton.button) {
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				if (active_tab > 0)
				{
					if (active_tab - 1== pressed_tab)
					{
						if (pressed_tab > 0)
							active_tab -= 2;
					}
					else
						active_tab--;
					
				}
				dirty = true;
				e.type = SDL_LASTEVENT;
				break;
				
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				if (active_tab < labels.size() - 1)
				{
					if (active_tab + 1 == pressed_tab)
					{
						if (pressed_tab < labels.size() - 1)
							active_tab += 2;
					}
					else
						active_tab++;
				}
				dirty = true;
				e.type = SDL_LASTEVENT;
				break;
				
			default:
				break;
				
		}
	}
}

/*
 *  Selection button
 */

const uint16 MAX_TEXT_WIDTH = 200;

// ZZZ: how come we have to do this?  because of that "&" in the typedef for action_proc?
// Anyway, this fixes the "crash when clicking in the Environment menu" bug we've seen
// in the Windows version all this time.
w_select_button::w_select_button(const char *s, action_proc p, void *a, bool u)
	: widget(LABEL_WIDGET), selection(s), proc(p), arg(a), utf8(u), p_flags(placeable::kDefault)
{
	uint16 max_selection_width = MAX_TEXT_WIDTH;

	saved_min_width = max_selection_width;
	saved_min_height = font->get_line_height();
}



void w_select_button::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();
	
	// Selection (ZZZ: different color for disabled)
	set_drawing_clip_rectangle(0, rect.x + selection_x, static_cast<uint16>(s->h), rect.x + rect.w);
	
	int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;
	
	draw_text(s, selection, rect.x + selection_x, y, get_theme_color(ITEM_WIDGET, state), font, style, utf8);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	
	// Cursor
	if (active) {
		//!!
	}
}

void w_select_button::click(int /*x*/, int /*y*/)
{
    if(enabled)
	    proc(arg);
}

void w_select_button::set_selection(const char *s)
{
	selection = s;
	if (p_flags & placeable::kAlignRight)
	{
		selection_x = rect.w - text_width(selection, font, style);
	}
	dirty = true;
}

void w_select_button::place(const SDL_Rect &r, placement_flags flags)
{
	rect.h = r.h;
	rect.y = r.y;

	rect.x = r.x;
	rect.w = r.w;
	p_flags = flags;

	if (flags & placeable::kAlignRight)
	{
		selection_x = rect.w - text_width(selection, font, style);
	}
	else
	{
		selection_x = 0;
	}

}

/*
 *  Selection widget (base class)
 */

// ZZZ: change of behavior/semantics:
// if passed an invalid selection, reset to a VALID one (i.e. 0)
// if no valid labels, returns -1 when asked for selection
// draw(), get_selection() check num_labels directly instead of trying to keep selection set at -1

static const char* sNoValidOptionsString = "(no valid options)"; // XXX should be moved outside compiled code e.g. to MML

w_select::w_select(size_t s, const char **l) : widget(LABEL_WIDGET), labels(l), we_own_labels(false), selection(s), selection_changed_callback(NULL), utf8(false)
{
	num_labels = 0;
        if(labels) {
            while (labels[num_labels])
                    num_labels++;
            if (selection >= num_labels || selection < 0)
                    selection = 0;
        }

	saved_min_height = font->get_line_height();
}


w_select::~w_select() {
    if(we_own_labels && labels)
        free(labels);
}

void w_select::place(const SDL_Rect& r, placement_flags flags)
{
	rect.h = r.h;
	rect.y = r.y;
	rect.x = r.x;
	rect.w = r.w;

}

int w_select::min_width()
{
	return get_largest_label_width();
}

void w_select::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent() + (rect.h - font->get_line_height()) / 2;

	// Selection (ZZZ: different color for disabled)
	const char *str = (num_labels > 0 ? labels[selection] : sNoValidOptionsString);

    int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;

    draw_text(s, str, rect.x, y, get_theme_color(ITEM_WIDGET, state), font, style, utf8);

	// Cursor
	if (active) {
		//!!
	}
}

void w_select::click(int /*x*/, int /*y*/)
{
    if(enabled) {
	    selection++;
	    if (selection >= num_labels)
		    selection = 0;

	    selection_changed();
    }
}

void w_select::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_LEFT) {
			if (num_labels == 0)
				selection = 0;
			else if (selection == 0)
				selection = num_labels - 1;
			else
				selection--;
			selection_changed();
			e.type = SDL_LASTEVENT;	// Swallow event
		} else if (e.key.keysym.sym == SDLK_RIGHT) {
			if (selection >= num_labels - 1)
				selection = 0;
			else
				selection++;
			selection_changed();
			e.type = SDL_LASTEVENT;	// Swallow event
		}
	} else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
		if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
			if (num_labels == 0)
				selection = 0;
			else if (selection == 0)
				selection = num_labels - 1;
			else
				selection--;
			selection_changed();
			e.type = SDL_LASTEVENT;	// Swallow event
		} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
			if (selection >= num_labels - 1)
				selection = 0;
			else
				selection++;
			selection_changed();
			e.type = SDL_LASTEVENT;	// Swallow event
		}
	}
}

void w_select::set_selection(size_t s, bool simulate_user_input /* default: false */)
{
	//if (s >= num_labels || s < 0)
	//	s = 0;
	assert(s == PIN(s, 0, num_labels - 1));

    if(selection != s) {
        selection = s;

        if(simulate_user_input)
            selection_changed();
    }

	dirty = true;
}

// ZZZ: change labels after creation
void w_select::set_labels(const char** inLabels) {
    if(we_own_labels && labels != NULL)
        free(labels);

    labels = inLabels;
    num_labels = 0;
    if(labels) {
        while(inLabels[num_labels])
            num_labels++;
    }
    we_own_labels = false;
	if (selection >= num_labels)
		set_selection(0);
	else
		set_selection(selection);

    // Hope new labels have same max width as old, or that user called set_full_width() on us.
}


// ZZZ: set labels according to stringset
void w_select::set_labels_stringset(short inStringSetID) {
    // free old label pointers, if appropriate
    if(we_own_labels && labels != NULL)
        free(labels);

    // see if we need space for label pointers.  if so, allocate and fill them in.
    if(TS_IsPresent(inStringSetID) && TS_CountStrings(inStringSetID) > 0) {
        num_labels = TS_CountStrings(inStringSetID);
        
        // Allocate one extra pointer slot to fill with NULL
        // Note: this works right if the string count is 0... but we check anyway, can save a malloc/free.
        labels = (const char**)malloc(sizeof(const char*) * (num_labels + 1));
        labels[num_labels] = NULL;
        
		for(size_t i = 0; i < num_labels; i++) {
            // shared references should be OK, stringsets ought to be pretty stable.  No need to copy...
            labels[i] = TS_GetCString(inStringSetID, i);
        }
        
        // we allocated; we free.
        we_own_labels = true;
    }
    else {	// no stringset or no strings in stringset
        labels = NULL;
        num_labels = 0;
        we_own_labels = false;
    }
    
	if (selection >= num_labels)
		set_selection(0);
	else
		set_selection(selection);
    
    // Hope new labels have same max width as old, or that user called set_full_width() on us.
}

void w_select::selection_changed(void)
{
	play_dialog_sound(DIALOG_CLICK_SOUND);
	dirty = true;

        // ZZZ: call user-specified callback
        if(selection_changed_callback != NULL)
            selection_changed_callback(this);
}


// ZZZ addition
uint16 w_select::get_largest_label_width() {
    uint16 max_label_width = 0;
    for (size_t i=0; i<num_labels; i++) {
            uint16 width = text_width(labels[i], font, style, utf8);
            if (width > max_label_width)
                    max_label_width = width;
    }

    // ZZZ: account for "no valid options" string
    if(num_labels <= 0)
	    max_label_width = text_width(sNoValidOptionsString, font, style, utf8);
    
    return max_label_width;
}


/*
 *  On-off toggle
 */

const char *w_toggle::onoff_labels[] = {"\342\230\220", "\342\230\221", NULL };

w_toggle::w_toggle(bool selection, const char **labels) : w_select(selection, labels) {
	if (labels == onoff_labels && use_theme_images(CHECKBOX))
	{
		saved_min_height = get_theme_space(CHECKBOX, BUTTON_HEIGHT);
	}
	else if (labels == onoff_labels)
	{
		labels_are_utf8(true);
		font = get_theme_font(CHECKBOX, style);
		saved_min_height = get_theme_space(CHECKBOX, BUTTON_HEIGHT);
	}
}

void w_toggle::draw(SDL_Surface *s) const
{
	// Selection (ZZZ: different color for disabled)
	const char *str = (num_labels > 0 ? labels[selection] : sNoValidOptionsString);

	int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;

	if (labels == onoff_labels && use_theme_images(CHECKBOX))
	{
		SDL_Surface *image = get_theme_image(CHECKBOX, state, selection);
		SDL_Rect r = { rect.x, rect.y + (rect.h - saved_min_height) / 2 + get_theme_space(CHECKBOX, BUTTON_T_SPACE), image->w, image->h };
		SDL_BlitSurface(image, 0, s, &r); 
	}
	else if (labels == onoff_labels)
	{
		draw_text(s, str, rect.x, rect.y + (rect.h - saved_min_height) / 2 + get_theme_space(CHECKBOX, BUTTON_T_SPACE), get_theme_color(LABEL_WIDGET, state, FOREGROUND_COLOR), font, style, utf8);
	}
	else
	{
		draw_text(s, str, rect.x, rect.y + font->get_ascent(), get_theme_color(ITEM_WIDGET, state), font, style, utf8);
	}
	
	// Cursor
	if (active) {
		//!!
	}
}

int w_toggle::min_width()
{
	if (labels == onoff_labels && use_theme_images(CHECKBOX))
	{
		return get_theme_image(CHECKBOX, DEFAULT_STATE, 0)->w;
	}
	else
	{
		return w_select::min_width();
	}
}


/*
 *	Enabling toggle (ZZZ)
 *
 *	Can enable/disable a bank of other widgets according to its state
 */


w_enabling_toggle::w_enabling_toggle(bool inSelection, bool inEnablesWhenOn, const char** inLabels)
	: w_toggle(inSelection, inLabels), enables_when_on(inEnablesWhenOn)
{
}


void
w_enabling_toggle::selection_changed()
{
	w_toggle::selection_changed();

	for(DependentCollection::iterator i = dependents.begin(); i != dependents.end(); i++)
		update_widget_enabled(*i);
}

	
/*
 *  Player color selection
 */

w_player_color::w_player_color(int selection) : w_select(selection, NULL)
{
	set_labels_stringset(kTeamColorsStringSetID);
}

void w_player_color::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Selection
	if (selection >= 0) {
		uint32 pixel = get_dialog_player_color(selection);
		SDL_Rect r = {rect.x, rect.y + 1, 48, rect.h - 2};
		SDL_FillRect(s, &r, pixel);
	} else {
		int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;

		draw_text(s, "<unknown>", rect.x, y, get_theme_color(ITEM_WIDGET, state), font, style);
	}

	// Cursor
	if (active)	{
		//!!
	}
}

class w_color_block : public widget {
public:
	w_color_block(const rgb_color *color) : m_color(color) { 
		saved_min_height = 64;
		saved_min_width = 64;
	}

	void draw(SDL_Surface *s) const {
		uint32 pixel = SDL_MapRGB(s->format, m_color->red >> 8, m_color->green >> 8, m_color->blue >> 8);
		SDL_Rect r = { rect.x, rect.y, 64, 64 };
		SDL_FillRect(s, &r, pixel);
	}

	bool is_dirty() { return true; }
private:
	const rgb_color *m_color;
};


void w_color_picker::click(int, int)
{
	if (!enabled) return;
	dialog d;
	
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("CHOOSE A COLOR"), d);
	placer->add(new w_spacer(), true);

	w_color_block *color_block = new w_color_block(&m_color);
	placer->dual_add(color_block, d);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET));
	table->col_flags(0, placeable::kAlignRight);

	w_percentage_slider *red_w = new w_percentage_slider(16, m_color.red >> 12);
	table->dual_add(red_w->label("Red"), d);
	table->dual_add(red_w, d);

	w_percentage_slider *green_w = new w_percentage_slider(16, m_color.green >> 12);
	table->dual_add(green_w->label("Green"), d);
	table->dual_add(green_w, d);

	w_percentage_slider *blue_w = new w_percentage_slider(16, m_color.blue >> 12);
	table->dual_add(blue_w->label("Blue"), d);
	table->dual_add(blue_w, d);

	placer->add(table, true);
	placer->add(new w_spacer(), true);
	
	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);
	button_placer->dual_add(new w_button("OK", dialog_ok, &d), d);
	
	placer->add(button_placer, true);

	rgb_color old_color = m_color;

	d.set_widget_placer(placer);
	d.set_processing_function(w_color_picker::update_color(red_w, green_w, blue_w, &m_color.red, &m_color.green, &m_color.blue));

	if (d.run() == 0)
	{
		m_color.red = red_w->get_selection() << 12;
		m_color.green = green_w->get_selection() << 12;
		m_color.blue = blue_w->get_selection() << 12;

		dirty = true;
		get_owning_dialog()->draw_dirty_widgets();
	}
	else
	{
		m_color = old_color;
	}
}

void w_color_picker::draw(SDL_Surface *s) const
{
	uint32 pixel = SDL_MapRGB(s->format, m_color.red >> 8, m_color.green >> 8, m_color.blue >> 8);
	SDL_Rect r = {rect.x, rect.y + 1, 48, rect.h - 2 };
	SDL_FillRect(s, &r, pixel);
}

/*
 *  Text entry widget
 */

w_text_entry::w_text_entry(size_t max_c, const char *initial_text)
	: widget(TEXT_ENTRY_WIDGET), enter_pressed_callback(NULL), value_changed_callback(NULL), max_chars(max_c), enable_mac_roman(false)
{
	// Initialize buffer
	buf = new char[max_chars + 1];
	set_text(initial_text);

	saved_min_width = MAX_TEXT_WIDTH;

	saved_min_height =  (int16) font->get_ascent() + font->get_descent() + font->get_leading();
}

w_text_entry::~w_text_entry()
{
	delete[] buf;
}

void w_text_entry::place(const SDL_Rect& r, placement_flags flags)
{
	rect.h = (int16) font->get_ascent() + (int16) font->get_descent() + font->get_leading();

	rect.y = r.y + (r.h - rect.h) / 2;

	text_x = 0;
	rect.x = r.x;
	rect.w = r.w;
	max_text_width = rect.w;
}

void w_text_entry::draw(SDL_Surface *s) const
{
  int y = rect.y + font->get_ascent();
  
  int16 theRectX = rect.x;
  uint16 theRectW = rect.w;
  int16 theTextX = text_x;
  
  // Text
  int16 x = theRectX + theTextX;
  uint16 width = text_width(buf, font, style);
  if (width > max_text_width)
    x -= width - max_text_width;
  set_drawing_clip_rectangle(0, theRectX + theTextX, static_cast<uint16>(s->h), theRectX + theRectW);
  
  int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;
  
  draw_text(s, buf, x, y, get_theme_color(TEXT_ENTRY_WIDGET, state), font, style);
  set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
  
  // Cursor
  if (active) {
	  width = text_width(buf, cursor_position, font, style);
	  SDL_Rect r = {x + width - (width ? 1 : 0), rect.y, 1, rect.h};
	  SDL_FillRect(s, &r, get_theme_color(TEXT_ENTRY_WIDGET, CURSOR_STATE));
  }
}

void w_text_entry::set_active(bool new_active) {
	if (new_active && !active) {
		cursor_position = num_chars;
		SDL_StartTextInput();
	} else if (!new_active && active) {
		SDL_StopTextInput();
	}
	widget::set_active(new_active);
}

void w_text_entry::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_LEFT:
left:			if (cursor_position > 0) {
					cursor_position--;
					dirty = true;
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_RIGHT:
right:			if (cursor_position < num_chars) {
					cursor_position++;
					dirty = true;
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				if (enter_pressed_callback)
					enter_pressed_callback(this);
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_BACKSPACE:
backspace:		if (num_chars && cursor_position) {
					memmove(&buf[cursor_position - 1], &buf[cursor_position], num_chars - cursor_position);
					buf[--num_chars] = 0;
					--cursor_position;
					modified_text();
					play_dialog_sound(DIALOG_DELETE_SOUND);
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_DELETE:
del:			if (cursor_position < num_chars) {
					memmove(&buf[cursor_position], &buf[cursor_position + 1], num_chars - cursor_position - 1);
					buf[--num_chars] = 0;
					modified_text();
					play_dialog_sound(DIALOG_DELETE_SOUND);
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_HOME:
home:			if (cursor_position > 0) {
					cursor_position = 0;
					dirty = true;
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_END:
end:			if (cursor_position < num_chars) {
					cursor_position = num_chars;
					dirty = true;
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_UP:
			case SDLK_DOWN:
				break;
			case SDLK_a:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto home;
				break;
			case SDLK_b:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto left;
				break;
			case SDLK_d:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto del;
				break;
			case SDLK_e:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto end;
				break;
			case SDLK_f:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto right;
				break;
			case SDLK_h:
				if (e.key.keysym.mod & KMOD_CTRL)
					goto backspace;
				break;
			case SDLK_k:
				if (e.key.keysym.mod & KMOD_CTRL) {
					if (cursor_position < num_chars) {
						num_chars = cursor_position;
						buf[num_chars] = 0;
						modified_text();
						play_dialog_sound(DIALOG_ERASE_SOUND);
					}
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_t:
				if (e.key.keysym.mod & KMOD_CTRL) {
					if (cursor_position) {
						if (cursor_position == num_chars)
							--cursor_position;
						char tmp = buf[cursor_position - 1];
						buf[cursor_position - 1] = buf[cursor_position];
						buf[cursor_position] = tmp;
						++cursor_position;
						modified_text();
						play_dialog_sound(DIALOG_TYPE_SOUND);
					}
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_u:
				if (e.key.keysym.mod & KMOD_CTRL) {
					if (num_chars && cursor_position) {
						memmove(&buf[0], &buf[cursor_position], num_chars - cursor_position);
						num_chars -= cursor_position;
						buf[num_chars] = 0;
						cursor_position = 0;
						modified_text();
						play_dialog_sound(DIALOG_ERASE_SOUND);
					}
				}
				e.type = SDL_LASTEVENT;
				break;
			case SDLK_w:
				if (e.key.keysym.mod & KMOD_CTRL) {
					size_t erase_position = cursor_position;
					while (erase_position && buf[erase_position - 1] == ' ')
						--erase_position;
					while (erase_position && buf[erase_position - 1] != ' ')
						--erase_position;
					if (erase_position < cursor_position) {
						if (cursor_position < num_chars)
						memmove(&buf[erase_position], &buf[cursor_position], num_chars - cursor_position);
						num_chars -= cursor_position - erase_position;
						cursor_position = erase_position;
						modified_text();
						play_dialog_sound(DIALOG_ERASE_SOUND);
					}
				}
				e.type = SDL_LASTEVENT;
				break;
			default:
				break;
		}
	} else if (e.type == SDL_TEXTINPUT) {
		std::string input_utf8 = e.text.text;
		std::string input_roman = utf8_to_mac_roman(input_utf8);
		for (std::string::iterator it = input_roman.begin(); it != input_roman.end(); ++it)
		{
			uint16 uc = *it;
			if (uc >= ' ' && (uc < 0x80 || enable_mac_roman) && (num_chars + 1) < max_chars) {
				memmove(&buf[cursor_position + 1], &buf[cursor_position], num_chars - cursor_position);
				buf[cursor_position++] = static_cast<char>(uc);
				buf[++num_chars] = 0;
				modified_text();
				play_dialog_sound(DIALOG_TYPE_SOUND);
			}
		}
		e.type = SDL_LASTEVENT;
	}
}

void w_text_entry::click(int x, int y)
{
	bool was_active = active;
	get_owning_dialog()->activate_widget(this);
	
	// Don't reposition cursor if:
	// - we were inactive before this click
	// - our text field is empty
	// - the click was simulated (0, 0) or out of bounds
	if (!was_active || !num_chars ||
		(x == 0 && y == 0) ||
	    x < 0 || y < 0 || x >= rect.w || y >= rect.h) {
		return;
	}
	
	// Find closest character boundary to click
	int width_remaining = x - text_x;
	size_t pos = 0;
	while (pos < num_chars && width_remaining > 0) {
		int cw = char_width(buf[pos], font, style);
		if (width_remaining > cw/2) {
			pos++; // right side is closer to target than left
		}
		width_remaining -= cw;
	}
	cursor_position = pos;
	dirty = true;
}

void w_text_entry::set_text(const char *text)
{
	memset(buf, 0, max_chars + 1);
	if (text)
		strncpy(buf, text, max_chars);
	num_chars = strlen(buf);
	cursor_position = num_chars;
	modified_text();
}

void w_text_entry::modified_text(void)
{
	dirty = true;
                                        
        // ZZZ: callback if desired
        if(value_changed_callback)
            value_changed_callback(this);
}

/*
 *  Number entry widget
 */

w_number_entry::w_number_entry(int initial_number) : w_text_entry(/*16*/4, NULL)
{
	set_number(initial_number);
	saved_min_width = MAX_TEXT_WIDTH / 2;
}

void w_number_entry::event(SDL_Event &e)
{
	if (e.type == SDL_TEXTINPUT) {
		std::string input_utf8 = e.text.text;
		std::string input_roman = utf8_to_mac_roman(input_utf8);
		for (std::string::iterator it = input_roman.begin(); it != input_roman.end(); ++it)
		{
			uint16 uc = *it;
			if (uc >= '0' && uc <= '9' && (num_chars + 1) < max_chars) {
				memmove(&buf[cursor_position + 1], &buf[cursor_position], num_chars - cursor_position);
				buf[cursor_position++] = static_cast<char>(uc);
				buf[++num_chars] = 0;
				modified_text();
				play_dialog_sound(DIALOG_TYPE_SOUND);
			}
		}
	}
	else
	{
		w_text_entry::event(e);
	}
}

void w_number_entry::set_number(int number)
{
	char str[16];
	sprintf(str, "%d", number);
	set_text(str);
}

w_password_entry::w_password_entry(size_t max_chars, const char *initial_text) : w_text_entry(max_chars, initial_text)
{

}

void w_password_entry::draw(SDL_Surface *s) const
{
	string real_text = buf;
	memset(buf, '*', strlen(buf));
	w_text_entry::draw(s);
	strcpy(buf, real_text.c_str());
}

/*
 *  Key name widget
 */

static const std::vector<std::string> WAITING_TEXT = { "waiting for key", "waiting for button", "waiting for button" };
static const std::vector<std::string> UNBOUND_TEXT = { "none", "none", "none" };

w_key::w_key(SDL_Scancode key, w_key::Type event_type) : widget(LABEL_WIDGET), binding(false), event_type(event_type)
{
	set_key(key);

	saved_min_width = text_width(WAITING_TEXT[event_type].c_str(), font, style);
	saved_min_height = font->get_line_height();
}

void w_key::place(const SDL_Rect& r, placement_flags flags)
{
	rect.h = r.h;
	rect.y = r.y;

	key_x = 0;
	rect.x = r.x;
	rect.w = r.w;
}
		
// ZZZ: we provide phony key names for the phony keys used for mouse buttons.
static const char* sMouseButtonKeyName[NUM_SDL_MOUSE_BUTTONS] = {
        "Mouse Left",
        "Mouse Middle",
        "Mouse Right",
        "Mouse X1",
        "Mouse X2",
        "Mouse Scroll Up",
        "Mouse Scroll Down"
};

static const char* get_joystick_button_key_name(int offset)
{
	static_assert(SDL_CONTROLLER_BUTTON_MAX <= 21 &&
				  SDL_CONTROLLER_AXIS_MAX <= 12,
				  "SDL changed the number of buttons/axes again!");

	static const char* buttons[] = {
		"A", "B", "X", "Y", "Back", "Guide", "Start",
		"LS", "RS", "LB", "RB", "Up", "Down", "Left", "Right",
		// new in SDL 2.0.14
		"Misc", "Paddle 1", "Paddle 2", "Paddle 3", "Paddle 4", "TP Button",
	};

	static const char* axes[] = {
		"LS Right", "LS Down", "RS Right", "RS Down", "LT", "RT",
		"LS Left", "LS Up", "RS Left", "RS Up", "LT Neg", "RT Neg"
	};

	if (offset < SDL_CONTROLLER_BUTTON_MAX)
	{
		return buttons[offset];
	}
	else
	{
		return axes[offset - SDL_CONTROLLER_BUTTON_MAX];
	}
}

// ZZZ: this injects our phony key names but passes along the rest.
const char*
GetSDLKeyName(SDL_Scancode inKey) {
	if (w_key::event_type_for_key(inKey) == w_key::MouseButton)
        return sMouseButtonKeyName[inKey - AO_SCANCODE_BASE_MOUSE_BUTTON];
	else if (w_key::event_type_for_key(inKey) == w_key::JoystickButton)
	    return get_joystick_button_key_name(inKey - AO_SCANCODE_BASE_JOYSTICK_BUTTON);
    else
        return SDL_GetScancodeName(inKey);
}

void w_key::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Key
	int16 x = rect.x + key_x;
	if (binding) {
		draw_text(s, WAITING_TEXT[event_type].c_str(), x, y, get_theme_color(ITEM_WIDGET, ACTIVE_STATE), font, style);
	} else if (key == SDL_SCANCODE_UNKNOWN) {
		int state = enabled ? (active ? ACTIVE_STATE : DISABLED_STATE) : DISABLED_STATE;
		draw_text(s, UNBOUND_TEXT[event_type].c_str(), x, y, get_theme_color(ITEM_WIDGET, state), font, style);
	} else {
        int state = enabled ? (active ? ACTIVE_STATE : DEFAULT_STATE) : DISABLED_STATE;
		draw_text(s, GetSDLKeyName(key), x, y, get_theme_color(ITEM_WIDGET, state), font, style);
	}
}

void w_key::click(int /*x*/, int /*y*/)
{
	get_owning_dialog()->activate_widget(this);
    if(enabled) {
	    if (!binding) {
		    binding = true;
		    dirty = true;
	    }
    }
}

void w_key::set_active(bool new_active) {
	if (!new_active && binding) {
		binding = false;
		dirty = true;
	}
	widget::set_active(new_active);
}

w_key::Type w_key::event_type_for_key(SDL_Scancode key) {
	if (key >= AO_SCANCODE_BASE_MOUSE_BUTTON && key < (AO_SCANCODE_BASE_MOUSE_BUTTON + NUM_SDL_MOUSE_BUTTONS))
		return MouseButton;
	else if (key >= AO_SCANCODE_BASE_JOYSTICK_BUTTON && key < (AO_SCANCODE_BASE_JOYSTICK_BUTTON + NUM_SDL_JOYSTICK_BUTTONS))
		return JoystickButton;
	return KeyboardKey;
}

void w_key::event(SDL_Event &e)
{
    if(binding) {
		bool handled = false;
		bool up = false;
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN:
				if (event_type == MouseButton) {
					if (e.button.button < NUM_SDL_REAL_MOUSE_BUTTONS + 1) {
						set_key(static_cast<SDL_Scancode>(AO_SCANCODE_BASE_MOUSE_BUTTON + e.button.button - 1));
						handled = true;
					}
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				if ((e.cbutton.button + AO_SCANCODE_BASE_JOYSTICK_BUTTON) == AO_SCANCODE_JOYSTICK_ESCAPE) {
					set_key(SDL_SCANCODE_UNKNOWN);
					handled = true;
				} else if (event_type == JoystickButton) {
					if (e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX) {
						set_key(static_cast<SDL_Scancode>(AO_SCANCODE_BASE_JOYSTICK_BUTTON + e.cbutton.button));
						handled = true;
					}
				}
				break;
			case SDL_CONTROLLERAXISMOTION:
				if (event_type == JoystickButton) {
					if (e.caxis.value >= 16384) {
						set_key(static_cast<SDL_Scancode>(AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE + e.caxis.axis));
						handled = true;
					} else if (e.caxis.value <= -16384) {
						set_key(static_cast<SDL_Scancode>(AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE + e.caxis.axis));
						handled = true;
					}
				}
				break;
			case SDL_MOUSEWHEEL:
				if (event_type == MouseButton) {
					up = (e.wheel.y > 0);
#if SDL_VERSION_ATLEAST(2,0,4)
					if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
						up = !up;
#endif
					set_key(static_cast<SDL_Scancode>(up ? AO_SCANCODE_MOUSESCROLL_UP : AO_SCANCODE_MOUSESCROLL_DOWN));
					handled = true;
				}
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					set_key(SDL_SCANCODE_UNKNOWN);
					handled = true;
				} else if (event_type == KeyboardKey) {
					set_key(e.key.keysym.scancode);
					handled = true;
				}
				break;
			case SDL_MOUSEMOTION:
				e.type = SDL_LASTEVENT; // suppress motion while assigning
				break;
			default:
				break;
		}
		
		if (handled) {
			dirty = true;
			binding = false;
			// activate next widget by faking a key press
			e.type = SDL_KEYDOWN;
			e.key.keysym.sym = SDLK_DOWN;
			e.key.keysym.scancode = SDL_SCANCODE_DOWN;
		}
    }
}


void w_key::set_key(SDL_Scancode k)
{
	key = k;
}

/*
 * Progress
 */
void w_progress_bar::draw(SDL_Surface* s) const
{
	int filled_width = (rect.w - 2) * value / max_value;
	SDL_Rect dst_rect = rect;
	dst_rect.h -= 2;
	dst_rect.y += 2;
	SDL_FillRect(s, &dst_rect, get_theme_color(MESSAGE_WIDGET, DEFAULT_STATE, FOREGROUND_COLOR));
	dst_rect.x += filled_width + 1;
	dst_rect.y++;
	dst_rect.h -= 2;
	dst_rect.w = dst_rect.w - filled_width - 2;
	if (use_theme_color(DIALOG_FRAME, BACKGROUND_COLOR))
		SDL_FillRect(s, &dst_rect, get_theme_color(DIALOG_FRAME, DEFAULT_STATE, BACKGROUND_COLOR));
}

void w_progress_bar::set_progress(int inValue, int inMaxValue)
{
	value = inValue;
	max_value = inMaxValue;
	dirty = true;
}
 


/*
 *  Slider
 */

const int SLIDER_WIDTH = 160;
const int SLIDER_THUMB_HEIGHT = 14;
const int SLIDER_THUMB_WIDTH = 8;
const int SLIDER_TROUGH_HEIGHT = 8;
const int SLIDER_LABEL_SPACE = 5;

w_slider::w_slider(int num, int s) : widget(LABEL_WIDGET), selection(s), num_items(num), thumb_dragging(false), readout(NULL), slider_changed_callback(NULL)
{
	slider_l = get_theme_image(SLIDER_WIDGET, DEFAULT_STATE, SLIDER_L_IMAGE);
	slider_r = get_theme_image(SLIDER_WIDGET, DEFAULT_STATE, SLIDER_R_IMAGE);
	slider_c = get_theme_image(SLIDER_WIDGET, DEFAULT_STATE, SLIDER_C_IMAGE, SLIDER_WIDTH - slider_l->w - slider_r->w);
	thumb = get_theme_image(SLIDER_THUMB, DEFAULT_STATE, 0);
	
	trough_width = SLIDER_WIDTH - get_theme_space(SLIDER_WIDGET, SLIDER_L_SPACE) - get_theme_space(SLIDER_WIDGET, SLIDER_R_SPACE);

	saved_min_width = SLIDER_WIDTH;
	if (use_theme_images(SLIDER_WIDGET))
		saved_min_height = std::max(static_cast<uint16>(slider_c->h), static_cast<uint16>(thumb->h));
	else
		saved_min_height = SLIDER_THUMB_HEIGHT + 2;
	
	readout_x = saved_min_width + SLIDER_LABEL_SPACE;
	init_formatted_value();
}

w_slider::~w_slider()
{
	if (slider_c) SDL_FreeSurface(slider_c);
}

void w_slider::place(const SDL_Rect& r, placement_flags flags)
{
	rect.h = r.h;
	rect.y = r.y + (r.h - saved_min_height) / 2;
	rect.x = r.x;
	slider_x = 0;
	rect.w = r.w;
	
	SDL_Rect r2;
	r2.h = r.h;
	r2.y = r.y;
	r2.x = r.x + readout_x;
	r2.w = readout->min_width();
	readout->place(r2, placeable::kDefault);

	set_selection(selection);
}

void w_slider::draw(SDL_Surface *s) const
{
	if (use_theme_images(SLIDER_WIDGET))
	{
		// Slider trough
		SDL_Rect r = {rect.x + slider_x, rect.y + (saved_min_height - slider_l->h) / 2,
			      static_cast<Uint16>(slider_l->w),
			      static_cast<Uint16>(slider_l->h)};
		SDL_BlitSurface(slider_l, NULL, s, &r);
		r.x = r.x + static_cast<Sint16>(slider_l->w);
		r.w = static_cast<Uint16>(slider_c->w);
		r.h = static_cast<Uint16>(slider_c->h);
		SDL_BlitSurface(slider_c, NULL, s, &r);
		r.x = r.x + static_cast<Sint16>(slider_c->w);
		r.w = static_cast<Uint16>(slider_r->w);
		r.h = static_cast<Uint16>(slider_r->h);
		SDL_BlitSurface(slider_r, NULL, s, &r);

		// Slider thumb
		r.x = rect.x + static_cast<Sint16>(thumb_x);
		r.y = rect.y + (saved_min_height - thumb->h) / 2 + get_theme_space(SLIDER_WIDGET, SLIDER_T_SPACE);
		r.w = static_cast<Uint16>(thumb->w);
		r.h = static_cast<Uint16>(thumb->h);
		SDL_BlitSurface(thumb, NULL, s, &r);
	} 
	else
	{
		SDL_Rect r = {rect.x, rect.y + (saved_min_height - SLIDER_TROUGH_HEIGHT) / 2 + get_theme_space(SLIDER_WIDGET, SLIDER_T_SPACE), SLIDER_WIDTH, SLIDER_TROUGH_HEIGHT};
		uint32 pixel = get_theme_color(SLIDER_WIDGET, DEFAULT_STATE, FRAME_COLOR);
		draw_rectangle(s, &r, pixel);

		pixel = get_theme_color(SLIDER_WIDGET, DEFAULT_STATE, FOREGROUND_COLOR);
		r.x = r.x + 1;
		r.y = r.y + 1;
		r.w = r.w - 2;
		r.h = r.h - 2;
		SDL_FillRect(s, &r, pixel);

		pixel = get_theme_color(SLIDER_THUMB, DEFAULT_STATE, FRAME_COLOR);
		r.x = rect.x + static_cast<Sint16>(thumb_x);
		r.y = rect.y + (saved_min_height - SLIDER_THUMB_HEIGHT) / 2;
		r.w = thumb_width();
		r.h = SLIDER_THUMB_HEIGHT;
		draw_rectangle(s, &r, pixel);

		pixel = get_theme_color(SLIDER_THUMB, DEFAULT_STATE, FOREGROUND_COLOR);
		r.x = r.x + 1;
		r.y = r.y + 1;
		r.w = r.w - 2;
		r.h = r.h - 2;
		SDL_FillRect(s, &r, pixel);
		
	}
	readout->draw(s);
}

void w_slider::mouse_move(int x, int /*y*/)
{
	if (thumb_dragging) {
		int delta_x = (x - slider_x - get_theme_space(SLIDER_WIDGET, SLIDER_L_SPACE)) - thumb_drag_x;
		set_selection(delta_x * num_items / (trough_width - thumb_width()));
	}
}

void w_slider::click(int x, int /*y*/)
{
    if(enabled) {
	    if (x >= thumb_x && x < thumb_x + thumb_width()) {
		    thumb_dragging = dirty = true;
		    thumb_drag_x = x - thumb_x;
	    }
    }
}

void w_slider::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_LEFT) {
			set_selection(selection - 1);
			item_selected();
			e.type = SDL_LASTEVENT;	// Swallow event
		} else if (e.key.keysym.sym == SDLK_RIGHT) {
			set_selection(selection + 1);
			item_selected();
			e.type = SDL_LASTEVENT;	// Swallow event
		}
	} else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
		if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
			set_selection(selection - 1);
			item_selected();
			e.type = SDL_LASTEVENT;	// Swallow event
		} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
			set_selection(selection + 1);
			item_selected();
			e.type = SDL_LASTEVENT;	// Swallow event
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
	thumb_x = int(float(selection * (trough_width - thumb_width())) / (num_items - 1) + 0.5);
	thumb_x += get_theme_space(SLIDER_WIDGET, SLIDER_L_SPACE) + slider_x;
	dirty = true;
	selection_changed();
}

int w_slider::thumb_width() const
{
	if (use_theme_images(SLIDER_WIDGET))
		return thumb->w;
	else
		return SLIDER_THUMB_WIDTH;
}

std::string w_slider::formatted_value()
{
	std::ostringstream ss;
	ss << selection;
	return ss.str();
}

void w_slider::selection_changed()
{
	readout->set_text(formatted_value().c_str());
	
	if (slider_changed_callback != NULL)
		slider_changed_callback(this);
}

void w_slider::init_formatted_value()
{
	int temp = selection;
	selection = num_items;
	std::string tempstr = formatted_value();
	selection = temp;
	delete readout;
	readout = new w_slider_text(tempstr.c_str());
	readout->associated_slider = this;
	saved_min_height = std::max(saved_min_height, readout->min_height());
	saved_min_width = readout_x + readout->min_width();
}

std::string w_percentage_slider::formatted_value()
{
	std::ostringstream ss;
	ss << (selection * 100 / (num_items - 1)) << "%";
	return ss.str();
}


/*
 *  List selection
 */

w_list_base::w_list_base(uint16 width, size_t lines, size_t /*sel*/) : widget(ITEM_WIDGET), num_items(0), shown_items(lines), thumb_dragging(false), top_item(0)
{
	rect.w = width;
	rect.h = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE);

	frame_tl = get_theme_image(LIST_WIDGET, DEFAULT_STATE, TL_IMAGE);
	frame_tr = get_theme_image(LIST_WIDGET, DEFAULT_STATE, TR_IMAGE);
	frame_bl = get_theme_image(LIST_WIDGET, DEFAULT_STATE, BL_IMAGE);
	frame_br = get_theme_image(LIST_WIDGET, DEFAULT_STATE, BR_IMAGE);
	frame_t = get_theme_image(LIST_WIDGET, DEFAULT_STATE, T_IMAGE, rect.w - frame_tl->w - frame_tr->w, 0);
	frame_l = get_theme_image(LIST_WIDGET, DEFAULT_STATE, L_IMAGE, 0, rect.h - frame_tl->h - frame_bl->h);
	frame_r = get_theme_image(LIST_WIDGET, DEFAULT_STATE, R_IMAGE, 0, rect.h - frame_tr->h - frame_br->h);
	frame_b = get_theme_image(LIST_WIDGET, DEFAULT_STATE, B_IMAGE, rect.w - frame_bl->w - frame_br->w, 0);

	thumb_t = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_T_IMAGE);
	thumb_tc = NULL;
	SDL_Surface *thumb_tc_unscaled = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_TC_IMAGE);
	thumb_c = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_C_IMAGE);
	SDL_Surface *thumb_bc_unscaled = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_BC_IMAGE);
	thumb_bc = NULL;
	thumb_b = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_B_IMAGE);

	min_thumb_height = static_cast<uint16>(thumb_t->h + thumb_tc_unscaled->h + thumb_c->h + thumb_bc_unscaled->h + thumb_b->h);

	trough_rect.x = rect.w - get_theme_space(LIST_WIDGET, TROUGH_R_SPACE);
	trough_rect.y = get_theme_space(LIST_WIDGET, TROUGH_T_SPACE);
	trough_rect.w = get_theme_space(LIST_WIDGET, TROUGH_WIDTH);
	trough_rect.h = rect.h - get_theme_space(LIST_WIDGET, TROUGH_T_SPACE) - get_theme_space(LIST_WIDGET, TROUGH_B_SPACE);

	saved_min_width = rect.w;
	saved_min_height = rect.h;
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

void w_list_base::draw_image(SDL_Surface *dst, SDL_Surface *s, int16 x, int16 y) const
{
	SDL_Rect r = {x, y, static_cast<Uint16>(s->w), static_cast<Uint16>(s->h)};
	SDL_BlitSurface(s, NULL, dst, &r);
}

void w_list_base::draw(SDL_Surface *s) const
{
	if (use_theme_images(LIST_WIDGET))
	{
		// Draw frame
		int16 x = rect.x;
		int16 y = rect.y;
		draw_image(s, frame_tl, x, y);
		draw_image(s, frame_t, x + static_cast<int16>(frame_tl->w), y);
		draw_image(s, frame_tr, x + static_cast<int16>(frame_tl->w) + static_cast<int16>(frame_t->w), y);
		draw_image(s, frame_l, x, y + static_cast<int16>(frame_tl->h));
		draw_image(s, frame_r, x + rect.w - static_cast<int16>(frame_r->w), y + static_cast<int16>(frame_tr->h));
		draw_image(s, frame_bl, x, y + static_cast<int16>(frame_tl->h) + static_cast<int16>(frame_l->h));
		draw_image(s, frame_b, x + static_cast<int16>(frame_bl->w), y + rect.h - static_cast<int16>(frame_b->h));
		draw_image(s, frame_br, x + static_cast<int16>(frame_bl->w) + static_cast<int16>(frame_b->w), y + static_cast<int16>(frame_tr->h) + static_cast<int16>(frame_r->h));

		// Draw thumb
		x = rect.x + trough_rect.x;
		y = rect.y + thumb_y;
		draw_image(s, thumb_t, x, y);
		draw_image(s, thumb_tc, x, y = y + static_cast<int16>(thumb_t->h));
		draw_image(s, thumb_c, x, y = y + static_cast<int16>(thumb_tc->h));
		draw_image(s, thumb_bc, x, y = y + static_cast<int16>(thumb_c->h));
		draw_image(s, thumb_b, x, y = y + static_cast<int16>(thumb_bc->h));
		
	}
	else
	{
		uint32 pixel = get_theme_color(LIST_WIDGET, DEFAULT_STATE, FRAME_COLOR);
		draw_rectangle(s, &rect, pixel);

		SDL_Rect real_trough = { rect.x + trough_rect.x, rect.y + trough_rect.y, trough_rect.w, trough_rect.h };
		draw_rectangle(s, &real_trough, pixel);
		real_trough.x = real_trough.x + 1;
		real_trough.y = real_trough.y + 1;
		real_trough.w = real_trough.w - 2;
		real_trough.h = real_trough.h - 2;
		if (use_theme_color(LIST_THUMB, BACKGROUND_COLOR))
		{
			pixel = get_theme_color(LIST_THUMB, DEFAULT_STATE, BACKGROUND_COLOR);
			SDL_FillRect(s, &real_trough, pixel);
		}

		pixel = get_theme_color(LIST_THUMB, DEFAULT_STATE, FRAME_COLOR);
		SDL_Rect thumb_rect = { rect.x + trough_rect.x, rect.y + thumb_y, trough_rect.w, thumb_t->h + thumb_tc->h + thumb_c->h + thumb_bc->h + thumb_b->h};
		draw_rectangle(s, &thumb_rect, pixel);
		
		pixel = get_theme_color(LIST_THUMB, DEFAULT_STATE, FOREGROUND_COLOR);
		thumb_rect.x = thumb_rect.x + 1;
		thumb_rect.y = thumb_rect.y + 1;
		thumb_rect.w = thumb_rect.w - 2;
		thumb_rect.h = thumb_rect.h - 2;
		SDL_FillRect(s, &thumb_rect, pixel);
		
	}
		
	// Draw items
	draw_items(s);
}

void w_list_base::mouse_move(int x, int y)
{
	if (thumb_dragging) {
		int delta_y = y - thumb_drag_y;
		if (delta_y > 0 && num_items > shown_items && trough_rect.h > thumb_height) {
			set_top_item(delta_y * (num_items - shown_items) / (trough_rect.h - thumb_height));
		} else {
		  set_top_item(0);
		}
	} else {
		if (x < get_theme_space(LIST_WIDGET, L_SPACE) || x >= rect.w - get_theme_space(LIST_WIDGET, R_SPACE)
		    || y < get_theme_space(LIST_WIDGET, T_SPACE) || y >= rect.h - get_theme_space(LIST_WIDGET, B_SPACE))
			return;

		if ((y - get_theme_space(LIST_WIDGET, T_SPACE)) / item_height() + top_item < std::min(num_items, top_item + shown_items))
		{	set_selection((y - get_theme_space(LIST_WIDGET, T_SPACE)) / item_height() + top_item); }
//		else
//		{	set_selection(num_items - 1); }
	}
}

void w_list_base::place(const SDL_Rect& r, placement_flags flags)
{
	widget::place(r, flags);
	
	trough_rect.x = rect.w - get_theme_space(LIST_WIDGET, TROUGH_R_SPACE);
	trough_rect.y = get_theme_space(LIST_WIDGET, TROUGH_T_SPACE);
	trough_rect.w = get_theme_space(LIST_WIDGET, TROUGH_WIDTH);
	trough_rect.h = rect.h - get_theme_space(LIST_WIDGET, TROUGH_T_SPACE) - get_theme_space(LIST_WIDGET, TROUGH_B_SPACE);

	frame_t = get_theme_image(LIST_WIDGET, DEFAULT_STATE, T_IMAGE, rect.w - frame_tl->w - frame_tr->w, 0);
	frame_l = get_theme_image(LIST_WIDGET, DEFAULT_STATE, L_IMAGE, 0, rect.h - frame_tl->h - frame_bl->h);
	frame_r = get_theme_image(LIST_WIDGET, DEFAULT_STATE, R_IMAGE, 0, rect.h - frame_tr->h - frame_br->h);
	frame_b = get_theme_image(LIST_WIDGET, DEFAULT_STATE, B_IMAGE, rect.w - frame_bl->w - frame_br->w, 0);
}

void w_list_base::click(int x, int y)
{
	if (x >= trough_rect.x && x < trough_rect.x + trough_rect.w
//	 && y >= trough_rect.y && y < trough_rect.y + trough_rect.h) {
	    && y >= thumb_y && y <= thumb_y + thumb_height) {
		thumb_dragging = dirty = true;
		thumb_drag_y = y - thumb_y;
	} else {
		if (num_items > 0 && is_item_selectable(selection))
			item_selected();
	}
}

void w_list_base::event(SDL_Event &e)
{
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_UP:
				if (selection != 0)
				{	set_selection(selection - 1); }
				e.type = SDL_LASTEVENT;	// Prevent selection of previous widget
				break;
			case SDLK_DOWN:
				if (selection < num_items - 1)
				{	set_selection(selection + 1); }
				e.type = SDL_LASTEVENT;	// Prevent selection of next widget
				break;
			case SDLK_PAGEUP:
				if (selection > shown_items)
				{	set_selection(selection - shown_items); }
				else
				{	set_selection(0); }
				break;
			case SDLK_PAGEDOWN:
				if (selection + shown_items < num_items - 1)
				{	set_selection(selection + shown_items); }
				else
				{	set_selection(num_items - 1); }
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
	} else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
		switch (e.cbutton.button) {
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				if (selection != 0)
				{	set_selection(selection - 1); }
				e.type = SDL_LASTEVENT;	// Prevent selection of previous widget
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				if (selection < num_items - 1)
				{	set_selection(selection + 1); }
				e.type = SDL_LASTEVENT;	// Prevent selection of next widget
				break;
		}
	} else if (e.type == SDL_MOUSEBUTTONUP) {
		if (thumb_dragging) {
			thumb_dragging = false;
			dirty = true;
		}
	} else if (e.type == SDL_MOUSEWHEEL) {
		int amt = e.wheel.y * -1 * kListScrollSpeed;
		if (amt < 0) {
			amt = amt * -1;
			if (top_item > amt)
				set_top_item(top_item - amt);
			else 
				set_top_item(0); 
		} else if (amt > 0) {
			if (top_item + amt < num_items - shown_items)
				set_top_item(top_item + amt);
			else
				set_top_item(num_items - shown_items);
		}
	}
}

void w_list_base::set_selection(size_t s)
{
	// Set selection, check for bounds
	assert(s == PIN(s, 0, num_items - 1));
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
	
	// ghs: actually, remember top item and selection
	size_t saved_top_item = top_item;
	size_t saved_selection = get_selection();
	top_item = selection = 0;
	dirty = true;

	// Calculate thumb height
	if (num_items <= shown_items)
		thumb_height = trough_rect.h;
	else if (num_items == 0)
		thumb_height = static_cast<uint16>(shown_items) * trough_rect.h;
	else
		thumb_height = uint16(float(shown_items * trough_rect.h) / num_items + 0.5);
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
	thumb_tc = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_TC_IMAGE, 0, dyn_height);
	thumb_bc = get_theme_image(LIST_THUMB, DEFAULT_STATE, THUMB_BC_IMAGE, 0, (rem_height & 1) ? dyn_height + 1 : dyn_height);

	thumb_y = 0;
	if (thumb_y > trough_rect.h - thumb_height)
		thumb_y = trough_rect.h - thumb_height;
	thumb_y = thumb_y + trough_rect.y;

	if (saved_selection < num_items) 
		set_selection(saved_selection);
	if (saved_top_item)
		set_top_item(saved_top_item);
}

void w_list_base::center_item(size_t i)
{
  set_top_item((i > shown_items / 2) ? i - shown_items / 2 : 0);
}

void w_list_base::set_top_item(size_t i)
{
	// Set top item (check for bounds)
	if (num_items > shown_items)
		i = PIN(i, 0, num_items - shown_items);
	else
		i = 0;
	if (i != top_item)
		dirty = true;
	top_item = i;

	// Calculate thumb y position
	if (num_items <= shown_items)
		thumb_y = 0;
	else
		thumb_y = int16(float(top_item * (trough_rect.h - thumb_height)) / (num_items - shown_items) + 0.5);
	if (thumb_y > trough_rect.h - thumb_height)
		thumb_y = trough_rect.h - thumb_height;
	thumb_y = thumb_y + trough_rect.y;
}



// ZZZ: maybe this belongs in a new file or something - it's definitely A1-related
// whereas most (but not all) of the other widgets here are sort of 'general-purpose'.
// Anyway, moved here from shell_sdl.h and enhanced ever so slightly, will now be
// using it in the setup network game dialog as well.

/*
 *  Level number dialog
 */

w_levels::w_levels(const vector<entry_point> &items, dialog *d)
	  : w_list<entry_point>(items, 400, 8, 0), parent(d), show_level_numbers(true) {}

// ZZZ: new constructor gives more control over widget's appearance.
w_levels::w_levels(const vector<entry_point>& items, dialog* d, uint16 inWidth,
        size_t inNumLines, size_t inSelectedItem, bool in_show_level_numbers)
	  : w_list<entry_point>(items, inWidth, inNumLines, inSelectedItem), parent(d), show_level_numbers(in_show_level_numbers) {}

void
w_levels::item_selected(void)
{
	parent->quit(0);
}

void
w_levels::draw_item(vector<entry_point>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
{
	y = y + font->get_ascent();
	char str[256];

    if(show_level_numbers)
    	sprintf(str, "%d - %s", i->level_number + 1, i->level_name);
    else
        sprintf(str, "%s", i->level_name);

	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	draw_text(s, str, x, y, get_theme_color(ITEM_WIDGET, selected ? ACTIVE_STATE : DEFAULT_STATE), font, style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


/*
 *  String List
 */

w_string_list::w_string_list(const vector<string> &items, dialog *d, int sel)
	: w_list<string>(items, 400, 8, sel), parent(d) {}

void w_string_list::item_selected(void)
{
	parent->quit(0);
}

void w_string_list::draw_item(vector<string>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
{
	y = y + font->get_ascent();
	char str[256];

	sprintf(str, "%s", i->c_str ());

	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	draw_text(s, str, x, y, get_theme_color(ITEM_WIDGET, selected ? ACTIVE_STATE : DEFAULT_STATE), font, style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


/*
 *  Selection Popup
 */

w_select_popup::w_select_popup (action_proc p, void *a) : w_select_button ("", gotSelectedCallback, NULL)
{
	set_arg(this);
	selection = -1;
	
	action = p;
	arg = a;
}

void w_select_popup::set_labels(const vector<string>& inLabels)
{
	labels = inLabels;

	// recalculate min width
	saved_min_width = 0;
	for (vector<string>::iterator it = labels.begin(); it != labels.end(); ++it)
	{
		uint16 width = text_width(it->c_str(), font, style);
		if (width > saved_min_width)
			saved_min_width = width;
	}
}

void w_select_popup::set_selection (int value)
{
	if (value < labels.size () && value >= 0)
		selection = value;
	else
		selection = -1;
	
	if (selection == -1)
		w_select_button::set_selection ("");
	else
		w_select_button::set_selection (labels[selection].c_str ());
}

void w_select_popup::gotSelected ()
{
	if (labels.size () > 1) {
		dialog theDialog;
		vertical_placer *placer = new vertical_placer;
		
		w_string_list* string_list_w = new w_string_list (labels, &theDialog, selection >= 0 ? selection : 0);
		placer->dual_add (string_list_w, theDialog);
		theDialog.activate_widget(string_list_w);
		
		theDialog.set_widget_placer(placer);
		if (theDialog.run () == 0)
			set_selection (string_list_w->get_selection ());
	}
	
	if (action)
		action (arg);
}


static const char* const sFileChooserInvalidFileString = "(no valid selection)";

w_file_chooser::w_file_chooser(const char* inDialogPrompt, Typecode inTypecode)
	: w_select_button("", NULL, NULL, true), typecode(inTypecode)
{
	strncpy(dialog_prompt, inDialogPrompt, sizeof(dialog_prompt));
	set_selection(sFileChooserInvalidFileString);
}



void
w_file_chooser::set_file(const FileSpecifier& inFile)
{
	file = inFile;
	update_filename();
}



void
w_file_chooser::click(int, int)
{
	if(enabled)
	{
		if(file.ReadDialog(typecode, dialog_prompt))
		{
			update_filename();
			if (m_callback)
				m_callback ();
		}
	}	
}



void
w_file_chooser::update_filename()
{
	if(file.Exists())
	{
		file.GetName(filename);
		std::string filename_copy = filename;
		strcpy(filename, FileSpecifier::HideExtension(filename_copy).c_str());
		set_selection(filename);
	}
	else
		set_selection(sFileChooserInvalidFileString);
}


const string w_items_in_room_get_name_of_item (GameListMessage::GameListEntry item)
{
	return item.name ();
}

const string w_items_in_room_get_name_of_item (prospective_joiner_info item)
{
	return std::string(item.name);
}

const string w_items_in_room_get_name_of_item (MetaserverPlayerInfo item)
{
	return item.name ();
}

void w_games_in_room::draw_item(const GameListMessage::GameListEntry& item, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const
{
	int game_style = style;

	int state;
	if (!item.compatible())
	{
		state = item.target() ? SELECTED_INCOMPATIBLE_GAME : INCOMPATIBLE_GAME;
	}
	else if (item.running())
	{
		state = item.target() ? SELECTED_RUNNING_GAME : RUNNING_GAME;
	}
	else 
	{
		state = item.target() ? SELECTED_GAME : GAME;
	}

	uint32 fg;
	if (selected)
	{
		fg = get_theme_color(ITEM_WIDGET, ACTIVE_STATE);
	}
	else 
	{
		fg = get_theme_color(METASERVER_GAMES, state, FOREGROUND_COLOR);
	}

	uint32 bg = get_theme_color(METASERVER_GAMES, state, BACKGROUND_COLOR);
	SDL_Rect r = { x, y, width, 3 * font->get_line_height() + 2};
	SDL_FillRect(s, &r, bg);

	if (use_theme_color(METASERVER_GAMES, FRAME_COLOR))
	{
		uint32 frame = get_theme_color(METASERVER_GAMES, state, FRAME_COLOR);
		draw_rectangle(s, &r, frame);
	}
	

	x += 1;
	width -= 2;
	y += font->get_ascent() + 1;

	std::ostringstream time_or_ping;
	int right_text_width = 0;

	// first line, game name, ping or time remaining
	if (item.running())
	{
		if (item.m_description.m_timeLimit && !(item.m_description.m_timeLimit == INT32_MAX || item.m_description.m_timeLimit == -1))
		{
			if (item.minutes_remaining() == 1)
			{
				time_or_ping << "~1 Minute";
			}
			else
			{
				time_or_ping << item.minutes_remaining() << " Minutes";
			}
		}
		else
		{
			time_or_ping << "Untimed";
		}
	}
	else
	{
		// draw ping
	}

	right_text_width = text_width(time_or_ping.str().c_str(), font, game_style);

	// draw game name
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width - right_text_width);
	font->draw_styled_text(s, item.name(), item.name().size(), x, y, fg, game_style);
	
	// draw remaining or ping
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	draw_text(s, time_or_ping.str().c_str(), x + width - right_text_width, y, fg, font, game_style);

	y += font->get_line_height();

	std::ostringstream game_and_map;

	if (!item.compatible())
	{
		game_and_map << "|i" << item.m_description.m_scenarioName;
		if (item.m_description.m_scenarioVersion != "")
		{
			game_and_map << ", Version " << item.m_description.m_scenarioVersion;
		}
	} 
	else
	{
		game_and_map << item.game_string()
			     << " on |i"
			     << item.m_description.m_mapName;
	}
	
	font->draw_styled_text(s, game_and_map.str().c_str(), game_and_map.str().size(), x, y, fg, game_style);

	y += font->get_line_height();

	right_text_width = font->styled_text_width(item.m_hostPlayerName, item.m_hostPlayerName.size(), game_style);
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width - right_text_width);

	std::ostringstream game_settings;
	if (item.running())
	{
		if (item.m_description.m_numPlayers == 1)
		{
			game_settings << "1 Player";
		}
		else
		{
			game_settings << static_cast<uint16>(item.m_description.m_numPlayers) << " Players";
		}
	}
	else
	{
		game_settings << static_cast<uint16>(item.m_description.m_numPlayers)
			      << "/"
			      << item.m_description.m_maxPlayers
			      << " Players";
	}

	if (item.m_description.m_timeLimit && !(item.m_description.m_timeLimit == INT32_MAX || item.m_description.m_timeLimit == -1))
	{
		game_settings << ", " 
			      << item.m_description.m_timeLimit / 60 / TICKS_PER_SECOND 
			      << " Minutes";
	}

	if (item.m_description.m_teamsAllowed)
	{
		game_settings << ", Teams";
	}

	draw_text(s, game_settings.str().c_str(), x, y, fg, font, game_style);

	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
	font->draw_styled_text(s, item.m_hostPlayerName, item.m_hostPlayerName.size(), x + width - right_text_width, y, fg, game_style);

	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


static inline uint8 darken(uint8 component, uint8 amount)
{
	return PIN((uint16) component * (255 - amount) / 255, 0, 255);
}

static inline uint8 lighten(uint8 component, uint8 amount)
{
	return PIN((uint16) component + (255 - component) * amount / 255, 0, 255);
}

void w_players_in_room::draw_item(const MetaserverPlayerInfo& item, SDL_Surface* s,
					int16 x, int16 y, uint16 width, bool selected) const
{
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);

	SDL_Rect r = {x, y, width, font->get_line_height() + 4};

	if (item.target())
	{
		SDL_FillRect(s, &r, SDL_MapRGB(s->format, 0xff, 0xff, 0xff));
	}

	// background is player color
	uint32 pixel;
	if (item.target())
	{
		int amount = 0x7f;
		pixel = SDL_MapRGB(s->format, 
				   lighten(item.color()[0] >> 8, amount),
				   lighten(item.color()[1] >> 8, amount),
				   lighten(item.color()[2] >> 8, amount));
	}
	else
	{
		int amount;
		if (item.away()) 
			amount = 0xbf;
		else
			amount = 0;

		pixel = SDL_MapRGB(s->format,
				   darken(item.color()[0] >> 8, amount),
				   darken(item.color()[1] >> 8, amount),
				   darken(item.color()[2] >> 8, amount));
	}

	r.x = x + kPlayerColorSwatchWidth + kSwatchGutter + 1;
	r.y = y + 1;
	r.w = width - kPlayerColorSwatchWidth - kSwatchGutter - 2;
	r.h = font->get_line_height() + 2;
	SDL_FillRect(s, &r, pixel);

	// team swatch
	r.x = x + 1;
	r.y = y + 1;
	r.w = kPlayerColorSwatchWidth;
	r.h = font->get_line_height() + 2;

	if (item.target())
	{
		int amount = 0x7f;
		pixel = SDL_MapRGB(s->format, 
				   lighten(item.team_color()[0] >> 8, amount),
				   lighten(item.team_color()[1] >> 8, amount),
				   lighten(item.team_color()[2] >> 8, amount));
	}
	else
	{
		int amount;
		if (item.away()) 
			amount = 0x7f;
		else
			amount = 0;

		pixel = SDL_MapRGB(s->format,
				   darken(item.team_color()[0] >> 8, amount),
				   darken(item.team_color()[1] >> 8, amount),
				   darken(item.team_color()[2] >> 8, amount));
	}

	SDL_FillRect(s, &r, pixel);

	y += font->get_ascent();
	uint32 color;
	if (selected)
	{
		color = get_theme_color(ITEM_WIDGET, ACTIVE_STATE);
	}
	else if (item.away())
	{
		color = SDL_MapRGB(s->format, 0x7f, 0x7f, 0x7f);
	}
	else
	{
		color = SDL_MapRGB(s->format, 0xff, 0xff, 0xff);
	}

	font->draw_styled_text(s, item.name(), item.name().size(), x + kPlayerColorSwatchWidth + kSwatchGutter + 2, y + 1, color, item.away() ? style : style | styleShadow);

	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


void w_colorful_chat::append_entry(const ColoredChatEntry& e)
{
	if (e.message.empty())
	{
		get_owning_dialog()->draw_dirty_widgets();
		return;
	}

	string name;
	if (font->styled_text_width(e.sender, e.sender.size(), style | styleShadow) > kNameWidth)
		name = string(e.sender, 0, font->trunc_styled_text(e.sender, kNameWidth, style | styleShadow));
	else
		name = e.sender;
	
	int message_style = style;
	int available_width = rect.w - get_theme_space(LIST_WIDGET, L_SPACE) - get_theme_space(LIST_WIDGET, R_SPACE);
	if (e.type == ColoredChatEntry::ChatMessage)
	{
		available_width -= kNameWidth + taper_width() + 2;
	}
	else if (e.type == ColoredChatEntry::PrivateMessage)
	{
		message_style |= styleShadow;
		available_width -= kNameWidth + taper_width() + 4;
	}
	else
	{
		message_style |= styleShadow;
		available_width -= 2;
	}

	size_t usable_characters = font->trunc_styled_text(e.message, available_width, message_style);
	string::const_iterator middle;
	string::const_iterator rest;
	if (usable_characters != e.message.size()) {
		size_t last_space = e.message.find_last_of(' ', usable_characters);
		if (last_space != 0 && last_space <= usable_characters)
		{
			middle = e.message.begin() + last_space;
			rest = middle + 1;
		}
		else
		{
			middle = e.message.begin() + usable_characters;
			rest = middle;
		}
	} else {
		middle = e.message.begin() + usable_characters;
		rest = middle;
	}

	ColoredChatEntry e_begin = e;
	e_begin.message = string(e.message.begin(), middle);
	e_begin.sender = name;
	
	ColoredChatEntry e_rest = e;
	if (rest != e.message.end())
	{
		e_rest.message = font->style_at(e.message, middle, message_style) + string(rest, e.message.end());
	}
	else
		e_rest.message = string(rest, e.message.end());
	e_rest.sender = name;
	
	bool save_top_item = top_item < num_items - shown_items;
	size_t saved_top_item = top_item;
	entries.push_back(e_begin);
	
	num_items = entries.size();
	new_items();
	if (save_top_item) {
		set_top_item(saved_top_item);
	} else if (num_items > shown_items) {
		set_top_item(num_items - shown_items);
	}
	
	append_entry(e_rest);
}

void w_colorful_chat::draw_item(vector<ColoredChatEntry>::const_iterator it, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
{
	int computed_y = y + font->get_ascent();

	uint16 message_x = x;
	uint16 message_width = width;

	if ((*it).type == ColoredChatEntry::ChatMessage || (*it).type == ColoredChatEntry::PrivateMessage)
	{
		// draw the name
		SDL_Rect r = { x, y, kNameWidth, font->get_line_height() + 1};
		uint32 pixel = SDL_MapRGB(s->format,
					  (*it).color.red >> 8,
					  (*it).color.green >> 8,
					  (*it).color.blue >> 8);
		SDL_FillRect(s, &r, pixel);

		// draw taper
		r.x += kNameWidth ;
		if (it->type == ColoredChatEntry::PrivateMessage)
		{
			// red bar under taper
			r.w = taper_width() + 2;
			SDL_FillRect(s, &r, SDL_MapRGB(s->format, 0x7f, 0x0, 0x0));
		}
			
		r.w = 1;
		for (int i = 0; i < taper_width(); ++i)
		{
			r.y++;
			r.h -= 2;
			SDL_FillRect(s, &r, pixel);
			r.x++;
		}
		
		set_drawing_clip_rectangle(0, x, static_cast<uint16>(s->h), x + kNameWidth);
		font->draw_styled_text(s, it->sender, it->sender.size(), x + 1, computed_y, SDL_MapRGB(s->format, 0xff, 0xff, 0xff), style | styleShadow);

		message_x += kNameWidth + taper_width() + 2;
		message_width -= kNameWidth + taper_width() + 2;
	}

	uint32 message_color = SDL_MapRGB(s->format, 0xff, 0xff, 0xff);
	if (it->type == ColoredChatEntry::ChatMessage)
	{
		message_color = get_theme_color(CHAT_ENTRY, DEFAULT_STATE, FOREGROUND_COLOR);
	}

	int message_style = style;
	if ((*it).type != ColoredChatEntry::ChatMessage) message_style |= styleShadow;

	if ((*it).type == ColoredChatEntry::ServerMessage)
	{
		// draw the blue bar
		uint32 pixel = SDL_MapRGB(s->format, 0x0, 0x0, 0x7f);
		SDL_Rect r = { message_x, y, message_width, font->get_line_height() + 1 };
		SDL_FillRect(s, &r, pixel);
		message_x += 1;
		message_width -= 2;
	}
	else if ((*it).type == ColoredChatEntry::PrivateMessage)
	{
		// draw a red bar
		uint32 pixel = SDL_MapRGB(s->format, 0x7f, 0x0, 0x0);
		SDL_Rect r = { message_x, y, message_width, font->get_line_height() + 1 };
		SDL_FillRect(s, &r, pixel);
		message_x += 1;
		message_width -= 2;
	}
	else if ((*it).type == ColoredChatEntry::LocalMessage)
	{
		// draw a gray bar
		uint32 pixel = SDL_MapRGB(s->format, 0x3f, 0x3f, 0x3f);
		SDL_Rect r = { message_x, y, message_width, font->get_line_height() + 1 };
		SDL_FillRect(s, &r, pixel);
		message_x += 1;
		message_width -= 2;
	}
	
	set_drawing_clip_rectangle(0, message_x, static_cast<uint16>(s->h), message_x + message_width);
	font->draw_styled_text(s, it->message, it->message.size(), message_x, computed_y, message_color, message_style);

	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}

void SDLWidgetWidget::hide ()
{
	hidden = true;
	m_widget->set_enabled (false);
}

void SDLWidgetWidget::show ()
{
	hidden = false;
	m_widget->set_enabled (!inactive);
}

void SDLWidgetWidget::deactivate ()
{
	inactive = true;
	m_widget->set_enabled (false);
}

void SDLWidgetWidget::activate ()
{
	inactive = false;
	m_widget->set_enabled (!hidden);
}


PlayersInGameWidget::PlayersInGameWidget (w_players_in_game2* pig)
	: SDLWidgetWidget (pig)
	, m_pig (pig)
	{}
	
void PlayersInGameWidget::redraw ()
{
	m_pig->start_displaying_actual_information ();
	m_pig->update_display ();
	m_pig->get_owning_dialog ()->draw_dirty_widgets ();
}




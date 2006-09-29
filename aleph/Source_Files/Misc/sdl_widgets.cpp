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
#include "mysound.h"
#include "interface.h"
#include "player.h"

#include "screen.h"

// ZZZ: for stringset business for modified w_select
#include	"TextStrings.h"

#include    "mouse.h"   // (ZZZ) NUM_SDL_MOUSE_BUTTONS, SDLK_BASE_MOUSE_BUTTON


/*
 *  Widget base class
 */

// ZZZ: initialize my additional storage elements
// (thought: I guess "widget" could be simplified, and have a subclass "useful_widget" to handle most things
// other than spacers.  Spacers are common and I guess we're starting to eat a fair amount of storage for a
// widget that does nothing and draws nothing... oh well, at least RAM is cheap.  ;) )
widget::widget() : active(false), dirty(false), enabled(true), font(NULL), identifier(NONE), owning_dialog(NULL),
                    widget_alignment(kAlignNatural), full_width(false), width_reduction(0), align_bottom_peer(NULL)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
}

widget::widget(int f) : active(false), dirty(false), enabled(true), font(get_dialog_font(f, style)),
                        identifier(NONE), owning_dialog(NULL), widget_alignment(kAlignNatural), full_width(false),
                        width_reduction(0), align_bottom_peer(NULL)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
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
    }
}

// ZZZ: several new stupid layout tricks(tm) methods here
void widget::set_alignment(widget::alignment inAlignment) {
    widget_alignment = inAlignment;
}

void widget::align_bottom_with_bottom_of(widget* inWidget) {
    align_bottom_peer = inWidget;
}

void widget::set_full_width() {
    full_width = true;
}

void widget::reduce_width_by_width_of(widget* inEncroachingWidget) {
    width_reduction = inEncroachingWidget->rect.w;
}

int widget::layout(void)
{
	// Default layout behaviour: center horizontally
	rect.x = -rect.w / 2;
	
    // ZZZ: stupid layout tricks(tm)
    if(align_bottom_peer != NULL) {
        rect.y = align_bottom_peer->rect.y + align_bottom_peer->rect.h - rect.h;
        return 0;
    }
    else
        return rect.h;
}

// ZZZ: more stupid layout tricks(tm)
void widget::capture_layout_information(int16 leftmost_x, int16 available_width) {
    switch(widget_alignment) {
    case kAlignNatural:
        // (no changes)
        break;

    case kAlignLeft:
        rect.x = leftmost_x;
        break;

    case kAlignCenter:
        rect.x = leftmost_x + (available_width - rect.w) / 2;
        break;

    case kAlignRight:
        rect.x = leftmost_x + available_width - rect.w;
        break;

    default:
        assert(false);
        break;
    }

	assert(static_cast<uint16>(available_width - (rect.x - leftmost_x) - width_reduction)
		== available_width - (rect.x - leftmost_x) - width_reduction);
    if(full_width)
        rect.w = static_cast<uint16>(available_width - (rect.x - leftmost_x) - width_reduction);
}


/*
 *  Static text
 */

// ZZZ change: copy the given string instead of just pointing to it.  Much easier for messages that change.
w_static_text::w_static_text(const char *t, int f, int c) : widget(f), color(c)
{
        text = strdup(t);
	rect.w = text_width(text, font, style);
	rect.h = font->get_line_height();
}

void w_static_text::draw(SDL_Surface *s) const
{
	draw_text(s, text, rect.x, rect.y + font->get_ascent(), get_dialog_color(color), font, style);
}

/*
// ZZZ addition: in support of left-justification
void w_static_text::capture_layout_information(int leftmost_x, int usable_width) {
    if(left_justified) {
        rect.x = leftmost_x;
        rect.w = usable_width - width_reduction;
    }
}

// ZZZ addition: left-justification
void w_static_text::set_left_justified() {
    left_justified = true;
}

// ZZZ addition: reduce width of left-justified text (usually it would take the whole dialog width)
void w_static_text::reduce_left_justified_width_by_width_of(widget* otherWidget) {
    width_reduction = otherWidget->rect.w;
}
*/

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

/*
 *  Picture (PICT resource)
 */

w_pict::w_pict(int id)
{
	LoadedResource rsrc;
	get_resource(FOUR_CHARS_TO_INT('P', 'I', 'C', 'T'), id, rsrc);
	picture = picture_to_surface(rsrc);
	if (picture) {
		rect.w = static_cast<uint16>(picture->w);
		rect.h = static_cast<uint16>(picture->h);
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

w_button::w_button(const char *t, action_proc p, void *a) : widget(BUTTON_FONT), text(t),
                    proc(p),
                    arg(a)
{
	rect.w = text_width(text.c_str(), font, style) + get_dialog_space(BUTTON_L_SPACE) + get_dialog_space(BUTTON_R_SPACE);
	button_l = get_dialog_image(BUTTON_L_IMAGE);
	button_r = get_dialog_image(BUTTON_R_IMAGE);
	button_c = get_dialog_image(BUTTON_C_IMAGE, rect.w - button_l->w - button_r->w);
	rect.h = static_cast<uint16>(get_dialog_space(BUTTON_HEIGHT));
}

w_button::~w_button()
{
	if (button_c) SDL_FreeSurface(button_c);
}

void w_button::set_callback(action_proc p, void *a)
{
	proc = p;
	arg = a;
}

void w_button::draw(SDL_Surface *s) const
{
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

	// Label (ZZZ: different color for disabled)
    int theColorToUse = enabled ? (active ? BUTTON_ACTIVE_COLOR : BUTTON_COLOR) : BUTTON_DISABLED_COLOR;

	draw_text(s, text.c_str(), rect.x + get_dialog_space(BUTTON_L_SPACE),
        rect.y + get_dialog_space(BUTTON_T_SPACE) + font->get_ascent(),
        get_dialog_color(theColorToUse), font, style);
}

void w_button::click(int /*x*/, int /*y*/)
{
    if(enabled && proc)
	    proc(arg);
}


/*
 *  Button on left/right side of dialog box
 */

const uint16 LR_BUTTON_OFFSET = 100;

int w_left_button::layout(void)
{
	rect.x = (LR_BUTTON_OFFSET + rect.w) / 2;
	rect.x = -rect.x; // MVCPP Warnings require this funny way of doing things.  Yuck!
	return 0;	// This will place the right button on the same y position
}

int w_right_button::layout(void)
{
	rect.x = (LR_BUTTON_OFFSET - rect.w) / 2;
	return rect.h;
}


/*
 *  Tab-changing buttons
 */

w_tab_button::w_tab_button (const char *name)
	: w_button (name, &w_tab_button::click_callback, this)
	{}

void w_tab_button::click_callback (void* me)
{
	w_tab_button* w = reinterpret_cast<w_tab_button*>(me);
	w->get_owning_dialog ()->set_active_tab (w->get_identifier ());
	w->get_owning_dialog ()->draw ();
}

int w_tab_button::layout(void)
{
	rect.x = xPos;
	if (last)
		return rect.h;
	else
		return 0;
}

void make_tab_buttons_for_dialog (dialog &theDialog, vector<string> &names, int tabBase)
{
	vector<w_tab_button*> tab_buttons;
	int fullWidth = 0;
	int tabNum = 1;
	
	for (vector<string>::iterator it = names.begin (); it != names.end (); ++it) {
		w_tab_button* tab_button_w = new w_tab_button (it->c_str ());
		tab_buttons.push_back (tab_button_w);
		fullWidth += tab_button_w->rect.w;
		tab_button_w->set_identifier (tabBase+tabNum);
		++tabNum;
		theDialog.add (tab_button_w);
	}
	
	int pos = 0;
	
	for (vector<w_tab_button*>::iterator it = tab_buttons.begin (); it != tab_buttons.end (); ++it) {
		if (it == tab_buttons.begin ()) {
			pos = (*it)->rect.x - fullWidth/2;
		}
		(*it)->xPos = pos;
		pos += (*it)->rect.w;
		(*it)->last = false;
	}
	
	tab_buttons[tab_buttons.size() - 1]->last = true;
}


/*
 *  Selection button
 */

const uint16 MAX_TEXT_WIDTH = 200;

// ZZZ: how come we have to do this?  because of that "&" in the typedef for action_proc?
// Anyway, this fixes the "crash when clicking in the Environment menu" bug we've seen
// in the Windows version all this time.
w_select_button::w_select_button(const char *n, const char *s, action_proc p, void *a)
    : widget(LABEL_FONT), name(n), selection(s), proc(p), arg(a) {}


int w_select_button::layout(void)
{
	uint16 name_width = text_width(name, font, style);
	uint16 max_selection_width = MAX_TEXT_WIDTH;
	uint16 spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -spacing / 2 - name_width;
	rect.w = name_width + spacing + max_selection_width;
	rect.h = font->get_line_height();
	selection_x = name_width + spacing;

	return rect.h;
}

void w_select_button::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name (ZZZ: different color for disabled)
    int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;

	draw_text(s, name, rect.x, y, get_dialog_color(theColorToUse), font, style);

	// Selection (ZZZ: different color for disabled)
	set_drawing_clip_rectangle(0, rect.x + selection_x, static_cast<uint16>(s->h), rect.x + rect.w);

    theColorToUse = enabled ? (active ? ITEM_ACTIVE_COLOR : ITEM_COLOR) : ITEM_DISABLED_COLOR;

	draw_text(s, selection, rect.x + selection_x, y, get_dialog_color(theColorToUse), font, style);
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
	dirty = true;
}


/*
 *  Selection widget (base class)
 */

// ZZZ: change of behavior/semantics:
// if passed an invalid selection, reset to a VALID one (i.e. 0)
// if no valid labels, returns -1 when asked for selection
// draw(), get_selection() check num_labels directly instead of trying to keep selection set at -1

static const char* sNoValidOptionsString = "(no valid options)"; // XXX should be moved outside compiled code e.g. to MML

w_select::w_select(const char *n, size_t s, const char **l) : widget(LABEL_FONT), name(n), labels(l), we_own_labels(false), selection(s), selection_changed_callback(NULL)//, center_entire_widget(false)
{
	num_labels = 0;
        if(labels) {
            while (labels[num_labels])
                    num_labels++;
            if (selection >= num_labels || selection < 0)
                    selection = 0;
        }
}


w_select::~w_select() {
    if(we_own_labels && labels)
        free(labels);
}


int w_select::layout(void)
{
    rect.h = font->get_line_height();

    int theResult = widget::layout();
    
    uint16 name_width = text_width(name, font, style);

	uint16 max_label_width = get_largest_label_width();
        
	max_label_width += 6;

	uint16 spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.w = name_width + spacing + max_label_width;
/*
        if(center_entire_widget)
            rect.x = -rect.w / 2;
        else
*/
	label_x = name_width + spacing;

        // We do this so widget contributes minimally to dialog layout unless we're forcing natural alignment.
	if(widget_alignment == kAlignNatural)
            rect.x = -spacing / 2 - name_width;
        else
            rect.x = -rect.w / 2;
            
    //	return rect.h;
    return theResult;
}

void w_select::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name (ZZZ: different color for disabled)
    int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;

	draw_text(s, name, rect.x, y, get_dialog_color(theColorToUse), font, style);

	// Selection (ZZZ: different color for disabled)
	const char *str = (num_labels > 0 ? labels[selection] : sNoValidOptionsString);

    theColorToUse = enabled ? (active ? ITEM_ACTIVE_COLOR : ITEM_COLOR) : ITEM_DISABLED_COLOR;

    draw_text(s, str, rect.x + label_x, y, get_dialog_color(theColorToUse), font, style);

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
			e.type = SDL_NOEVENT;	// Swallow event
		} else if (e.key.keysym.sym == SDLK_RIGHT) {
			if (selection >= num_labels - 1)
				selection = 0;
			else
				selection++;
			selection_changed();
			e.type = SDL_NOEVENT;	// Swallow event
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
            uint16 width = text_width(labels[i], font, style);
            if (width > max_label_width)
                    max_label_width = width;
    }

    // ZZZ: account for "no valid options" string
    if(num_labels <= 0)
        max_label_width = text_width(sNoValidOptionsString, font, style);
    
    return max_label_width;
}


/*
 *  On-off toggle
 */

const char *w_toggle::onoff_labels[] = {"Off", "On", NULL};

w_toggle::w_toggle(const char *name, bool selection, const char **labels) : w_select(name, selection, labels) {}


/*
 *	Enabling toggle (ZZZ)
 *
 *	Can enable/disable a bank of other widgets according to its state
 */


w_enabling_toggle::w_enabling_toggle(const char* inName, bool inSelection, bool inEnablesWhenOn, const char** inLabels)
	: w_toggle(inName, inSelection, inLabels), enables_when_on(inEnablesWhenOn)
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

w_player_color::w_player_color(const char *name, int selection) : w_select(name, selection, NULL)
{
	set_labels_stringset(kTeamColorsStringSetID);
}

void w_player_color::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name (ZZZ: different color for disabled)
	int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;

	draw_text(s, name, rect.x, y, get_dialog_color(theColorToUse), font, style);

	// Selection
	if (selection >= 0) {
		uint32 pixel = get_dialog_player_color(selection);
		SDL_Rect r = {rect.x + label_x, rect.y + 1, 48, rect.h - 2};
		SDL_FillRect(s, &r, pixel);
	} else {
		theColorToUse = enabled ? (active ? ITEM_ACTIVE_COLOR : ITEM_COLOR) : ITEM_DISABLED_COLOR;

		draw_text(s, "<unknown>", rect.x + label_x, y, get_dialog_color(theColorToUse), font, style);
	}

	// Cursor
	if (active)	{
		//!!
	}
}


/*
 *  Text entry widget
 */

w_text_entry::w_text_entry(const char *n, size_t max, const char *initial_text)
    : widget(LABEL_FONT), enter_pressed_callback(NULL), value_changed_callback(NULL), name(n), max_chars(max), new_rect_valid(false)
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
  rect.h = max(font->get_ascent(), text_font->get_ascent()) +
    max (font->get_descent(), text_font->get_descent()) +
    max (font->get_leading(), text_font->get_leading());

    int theResult = widget::layout();
    uint16 name_width = text_width(name.c_str(), font, style);
	max_text_width = MAX_TEXT_WIDTH;
	uint16 spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -spacing / 2 - name_width;
	rect.w = name_width + spacing + max_text_width;
	text_x = name_width + spacing;

//	return rect.h;
    return theResult;
}

void w_text_entry::draw(SDL_Surface *s) const
{
  int y = rect.y + max(font->get_ascent(), text_font->get_ascent());
  
  int16 theRectX = new_rect_valid ? new_rect_x : rect.x;
  uint16 theRectW = new_rect_valid ? new_rect_w : rect.w;
  int16 theTextX = new_rect_valid ? new_text_x : text_x;
  
  // Name (ZZZ: different color for disabled)
  int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;
  
  draw_text(s, name.c_str(), theRectX, y, get_dialog_color(theColorToUse), font, style);
  
  // Text
  int16 x = theRectX + theTextX;
  uint16 width = text_width(buf, text_font, text_style);
  if (width > max_text_width)
    x -= width - max_text_width;
  set_drawing_clip_rectangle(0, theRectX + theTextX, static_cast<uint16>(s->h), theRectX + theRectW);
  
  theColorToUse = enabled ? (active ? TEXT_ENTRY_ACTIVE_COLOR : TEXT_ENTRY_COLOR) : TEXT_ENTRY_DISABLED_COLOR;
  
  draw_text(s, buf, x, y, get_dialog_color(theColorToUse), text_font, text_style);
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
                                
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                                if(enter_pressed_callback)
                                    enter_pressed_callback(this);
                        
                                e.type = SDL_NOEVENT;	// Swallow event (shouldn't typing do this also??)
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
					assert(uc == static_cast<char>(uc));
					buf[num_chars++] = static_cast<char>(uc);
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
                                        
        // ZZZ: callback if desired
        if(value_changed_callback)
            value_changed_callback(this);
}

// (ZZZ addition) this really ought to be available for more widgets.
void w_text_entry::set_name(const char* inName) 
{
    name = inName;    
    dirty = true;

    // Here we assume that if rect.w == 0, we have not been laid out yet.  So, we should
    // only try to "fix" our layout if rect.w != 0...
    if(rect.w != 0) {
        //    printf("set_name: old=%s, new=%s\n", name, inName);
        
        // I used to compare inName to name, but because of the way I am using this, (and
        // because for some reason I am not doing the right thing and making my own copy),
        // the storage to which name points becomes invalid right before this call is made.
        // (At the least, it no longer contains the previous name...)
    
        // So now, I infer what the old width must have been based on the total widget size.
    
        // I can't update this information in draw() because draw() is const and I don't feel
        // like jumping through TOO many hoops...
        if(new_rect_valid) {
            rect.w = new_rect_w;
            rect.x = new_rect_x;
            text_x = new_text_x;
    
            new_rect_valid = false;
        }
    
        // This calculation based on w_text_entry::layout(), we "invert" its operation a bit.
		uint16 spacing         = get_dialog_space(LABEL_ITEM_SPACE);
        uint16 old_name_width  = rect.w - spacing - max_text_width;
        uint16 new_name_width  = text_width(inName, font, style);
        int16 name_width_diff = new_name_width - old_name_width;
    
        if(name_width_diff != 0) {
            // Note here we now keep a "new width" and "new x".  We need the rect to stay where it was
            // so that adjusting to a narrower rect still manages to erase the old name.
    
            new_rect_valid = true;
    
            // Adjust the overall widget width to accomodate new name (hope it fits in dialog!!)
            new_rect_w = rect.w + name_width_diff;
            // Always place text the same wrt name
            new_text_x = text_x + name_width_diff;
            
            // Adjust placement of rect in dialog according to alignment
            switch(widget_alignment) {
                case kAlignLeft:
                    // do nothing - widget remains left-justified
                break;
    
                case kAlignCenter:
                    // rect shifts somewhat to accomodate new name.  Hmm... possible rounding error?
                    new_rect_x = rect.x - name_width_diff / 2;
                break;
    
                case kAlignRight:
                case kAlignNatural:
                    // rect needs to shift to accomodate new name.
                    new_rect_x = rect.x - name_width_diff;
                break;
            }
    
            // If we're moving to a larger rect, make the changes now so drawing works right.
            if(new_rect_w >= rect.w) {
                rect.w = new_rect_w;
                rect.x = new_rect_x;
                text_x = new_text_x;
    
                new_rect_valid = false;
            }
    
        } // name width differs
        
    } // had already been laid out
    
} // set_name


void
w_text_entry::capture_layout_information(int16 leftmost_x, int16 usable_width) {
    widget::capture_layout_information(leftmost_x, usable_width);

    if(full_width) {
		assert(rect.w >= text_x);
        max_text_width	= rect.w - text_x;
    }
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
            // ZZZ fix: under Mac OS X, Christian's code was filtering out backspace in numeric entry fields
            // Maybe it would be better to have w_text_entry call a virtual method "typed_printable" or something
            // which w_number_entry could override to filter out non-numeric characters.
            // Anyway, here I just ignore the keysym.sym's that show up in w_text_entry::event(), to avoid
            // filtering them out incorrectly.
            switch(e.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT:
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                case SDLK_BACKSPACE:
                break;
                
                default:
                {
                    uint16 uc = e.key.keysym.unicode;
                    if (uc >= ' ' && uc < 0x80) {
                            if (uc < '0' || uc > '9') {
                                    // Swallow all non-numbers
                                    e.type = SDL_NOEVENT;
                                    return;
                            }
                    }
                } // default
                break;
            } // switch
	} // if key down
        
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
	uint16 name_width = text_width(name, font, style);
	uint16 spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -spacing / 2 - name_width;
	rect.w = name_width + spacing + text_width(WAITING_TEXT, font, style);
	rect.h = font->get_line_height();
	key_x = name_width + spacing;

	return rect.h;
}

// ZZZ: we provide phony key names for the phony keys used for mouse buttons.
static const char* sMouseButtonKeyName[NUM_SDL_MOUSE_BUTTONS] = {
        "Left Mouse",   // things like "Middle Mouse Button" are too long to draw properly
        "Middle Mouse",
        "Right Mouse",
        "Mouse Button 4",
        "Mouse Button 5",
        "Mouse Button 6",
        "Mouse Button 7",
        "Mouse Button 8"
};

// ZZZ: this injects our phony key names but passes along the rest.
static const char*
GetSDLKeyName(SDLKey inKey) {
    if(inKey >= SDLK_BASE_MOUSE_BUTTON && inKey < SDLK_BASE_MOUSE_BUTTON + NUM_SDL_MOUSE_BUTTONS)
        return sMouseButtonKeyName[inKey - SDLK_BASE_MOUSE_BUTTON];
    else
        return SDL_GetKeyName(inKey);
}

void w_key::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent();

	// Name (ZZZ: different color for disabled)
    int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;

	draw_text(s, name, rect.x, y, get_dialog_color(theColorToUse), font, style);

	// Key
	int16 x = rect.x + key_x;
	if (binding) {
		SDL_Rect r = {x, rect.y, text_width(WAITING_TEXT, font, style), rect.h};
		SDL_FillRect(s, &r, get_dialog_color(KEY_BINDING_COLOR));
		draw_text(s, WAITING_TEXT, x, y, get_dialog_color(ITEM_ACTIVE_COLOR), font, style);
	} else {
        theColorToUse = enabled ? (active ? ITEM_ACTIVE_COLOR : ITEM_COLOR) : ITEM_DISABLED_COLOR;

        // ZZZ: potential to use the phony (i.e. mouse-button) key names
		draw_text(s, GetSDLKeyName(key), x, y, get_dialog_color(theColorToUse), font, style);
	}
}

void w_key::click(int /*x*/, int /*y*/)
{
    if(enabled) {
	    if (!binding) {
		    binding = true;
		    dirty = true;
	    }
    }
}

void w_key::event(SDL_Event &e)
{
    if(binding) {
        // ZZZ: let mouse buttons assign like (unused) keys
        if(e.type == SDL_MOUSEBUTTONDOWN) {
            e.type = SDL_KEYDOWN;
            e.key.keysym.sym = (SDLKey)(SDLK_BASE_MOUSE_BUTTON + e.button.button - 1);
        }

    	if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym != SDLK_ESCAPE)
				set_key(e.key.keysym.sym);
			dirty = true;
			binding = false;
			e.key.keysym.sym = SDLK_DOWN;	// Activate next widget
	    }

        // ZZZ: suppress mouse motion while assigning
        // (it's annoying otherwise, trust me)
        if(e.type == SDL_MOUSEMOTION)
            e.type = SDL_NOEVENT;
    }
}

void w_key::set_key(SDLKey k)
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
	SDL_FillRect(s, &dst_rect, get_dialog_color(MESSAGE_COLOR));
	dst_rect.x += filled_width + 1;
	dst_rect.y++;
	dst_rect.h -= 2;
	dst_rect.w = dst_rect.w - filled_width - 2;
	SDL_FillRect(s, &dst_rect, get_dialog_color(BACKGROUND_COLOR));
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
	uint16 name_width = text_width(name, font, style);
	uint16 spacing = get_dialog_space(LABEL_ITEM_SPACE);

	rect.x = -spacing / 2 - name_width;
	rect.w = name_width + spacing + SLIDER_WIDTH;
	rect.h = MAX(font->get_line_height(), static_cast<uint16>(slider_c->h));
	slider_x = name_width + spacing;
	set_selection(selection);

	return rect.h;
}

void w_slider::draw(SDL_Surface *s) const
{
	int y = rect.y + font->get_ascent() + (rect.h - font->get_line_height()) / 2;

	// Name (ZZZ: different color for disabled)
    int theColorToUse = enabled ? (active ? LABEL_ACTIVE_COLOR : LABEL_COLOR) : LABEL_DISABLED_COLOR;

	draw_text(s, name, rect.x, y, get_dialog_color(theColorToUse), font, style);

	// Slider trough
	SDL_Rect r = {rect.x + slider_x, rect.y,
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
	r.y = rect.y + get_dialog_space(SLIDER_T_SPACE);
	r.w = static_cast<Uint16>(thumb->w);
	r.h = static_cast<Uint16>(thumb->h);
	SDL_BlitSurface(thumb, NULL, s, &r);
}

void w_slider::mouse_move(int x, int /*y*/)
{
	if (thumb_dragging) {
		int delta_x = (x - slider_x - get_dialog_space(SLIDER_L_SPACE)) - thumb_drag_x;
		set_selection(delta_x * num_items / trough_width);
	}
}

void w_slider::click(int x, int /*y*/)
{
    if(enabled) {
	    if (x >= slider_x && x < slider_x + SLIDER_WIDTH) {
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

w_list_base::w_list_base(uint16 width, size_t lines, size_t /*sel*/) : widget(ITEM_FONT), num_items(0), shown_items(lines), thumb_dragging(false)
{
	font_height = font->get_line_height();
	rect.w = width;
	rect.h = font_height * static_cast<uint16>(shown_items) + get_dialog_space(LIST_T_SPACE) + get_dialog_space(LIST_B_SPACE);

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

	min_thumb_height = static_cast<uint16>(thumb_t->h + thumb_tc_unscaled->h + thumb_c->h + thumb_bc_unscaled->h + thumb_b->h);

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

void w_list_base::draw_image(SDL_Surface *dst, SDL_Surface *s, int16 x, int16 y) const
{
	SDL_Rect r = {x, y, static_cast<Uint16>(s->w), static_cast<Uint16>(s->h)};
	SDL_BlitSurface(s, NULL, dst, &r);
}

void w_list_base::draw(SDL_Surface *s) const
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

	// Draw items
	draw_items(s);
}

void w_list_base::mouse_move(int x, int y)
{
	if (thumb_dragging) {
		int delta_y = y - thumb_drag_y;
		if (delta_y > 0) {
		  set_top_item(delta_y * num_items / trough_rect.h);
		} else {
		  set_top_item(0);
		}
	} else {
		if (x < get_dialog_space(LIST_L_SPACE) || x >= rect.w - get_dialog_space(LIST_R_SPACE)
		 || y < get_dialog_space(LIST_T_SPACE) || y >= rect.h - get_dialog_space(LIST_B_SPACE))
			return;

		if ((y - get_dialog_space(LIST_T_SPACE)) / font_height + top_item < num_items)
		{	set_selection((y - get_dialog_space(LIST_T_SPACE)) / font_height + top_item); }
		else
		{	set_selection(num_items - 1); }
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
				if (selection != 0)
				{	set_selection(selection - 1); }
				e.type = SDL_NOEVENT;	// Prevent selection of previous widget
				break;
			case SDLK_DOWN:
				if (selection < num_items - 1)
				{	set_selection(selection + 1); }
				e.type = SDL_NOEVENT;	// Prevent selection of next widget
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
	} else if (e.type == SDL_MOUSEBUTTONUP) {
		if (thumb_dragging) {
			thumb_dragging = false;
			dirty = true;
		}
	} else if (e.type == SDL_MOUSEBUTTONDOWN) {
		switch (e.button.button) {
		case SDL_BUTTON_WHEELUP:
			if (top_item > 3)
				set_top_item(top_item - 3); 
			else 
				set_top_item(0); 
			break;	
		case SDL_BUTTON_WHEELDOWN:
			if (top_item < num_items - shown_items - 3)
				set_top_item(top_item + 3);
			else 
				set_top_item(num_items - shown_items);
			break;
		default:
			break;
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
	thumb_tc = get_dialog_image(THUMB_TC_IMAGE, 0, dyn_height);
	thumb_bc = get_dialog_image(THUMB_BC_IMAGE, 0, (rem_height & 1) ? dyn_height + 1 : dyn_height);

	thumb_y = 0;
	if (thumb_y > trough_rect.h - thumb_height)
		thumb_y = trough_rect.h - thumb_height;
	thumb_y = thumb_y + trough_rect.y;
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
	if (num_items == 0)
		thumb_y = 0;
	else
		thumb_y = int16(float(top_item * trough_rect.h) / num_items + 0.5);
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
	draw_text(s, str, x, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
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
	draw_text(s, str, x, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


/*
 *  Selection Popup
 */

w_select_popup::w_select_popup (const char *name, action_proc p, void *a) : w_select_button (name, "", gotSelectedCallback, NULL)
{
	set_arg(this);
	selection = -1;
	
	action = p;
	arg = a;
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
		
		w_string_list* string_list_w = new w_string_list (labels, &theDialog, selection >= 0 ? selection : 0);
		theDialog.add (string_list_w);
		
		theDialog.run ();
		
		set_selection (string_list_w->get_selection ());
	}
	
	if (action)
		action (arg);
}


w_tab_popup::w_tab_popup (const char *name) : w_select_popup (name, gotSelectedCallback, this) {}

void w_tab_popup::gotSelectedCallback (void* me)
{
	w_tab_popup* w = reinterpret_cast<w_tab_popup*>(me);
	w->get_owning_dialog ()->set_active_tab (w->get_selection ()  + w->get_identifier () + 1);
	w->get_owning_dialog ()->draw ();
}


static const char* const sFileChooserInvalidFileString = "(no valid selection)";

w_file_chooser::w_file_chooser(const char* inLabel, const char* inDialogPrompt, Typecode inTypecode)
	: w_select_button(inLabel, "", NULL, NULL), typecode(inTypecode)
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
	return pstring_to_string (item.name);
}

const string w_items_in_room_get_name_of_item (MetaserverPlayerInfo item)
{
	return item.name ();
}

void w_players_in_room::draw_item(const MetaserverPlayerInfo& item, SDL_Surface* s,
					int16 x, int16 y, uint16 width, bool selected) const
{
	set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);

	SDL_Rect r = {x, y + 1, kPlayerColorSwatchWidth, font->get_ascent() - 2};
	uint32 pixel = SDL_MapRGB(s->format,
				  item.color()[0] >> 8, 
				  item.color()[1] >> 8, 
				  item.color()[2] >> 8);
	SDL_FillRect(s, &r, pixel);

	r.x += kPlayerColorSwatchWidth;
	r.w = kTeamColorSwatchWidth;
	pixel = SDL_MapRGB(s->format,
				  item.team_color()[0] >> 8, 
				  item.team_color()[1] >> 8, 
				  item.team_color()[2] >> 8);
	SDL_FillRect(s, &r, pixel);

	y += font->get_ascent();
	draw_text(s, item.name().c_str(), x + kTeamColorSwatchWidth + kPlayerColorSwatchWidth + kSwatchGutter, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);

	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}


void w_text_box::append_text(const string& s)
{
	if (s.empty ()) {
		get_owning_dialog()->draw_dirty_widgets();
		return;
	}
		
	int available_width = rect.w - get_dialog_space(LIST_L_SPACE) - get_dialog_space(LIST_R_SPACE);
	size_t usable_characters = trunc_text(s.c_str (), available_width, font, style);
	
	string::const_iterator middle;
	if (usable_characters != s.size ()) {
		size_t last_space = s.find_last_of(' ', usable_characters);
		if (last_space != 0 && last_space < usable_characters)
			middle = s.begin() + last_space;
		else
			middle = s.begin() + usable_characters;
	} else
		middle = s.begin() + usable_characters;

	bool save_top_item = top_item < num_items - shown_items;
	size_t saved_top_item = top_item;
	text_lines.push_back (string (s.begin(), middle));

	num_items = text_lines.size();
	new_items();
	if (save_top_item) {
		set_top_item(saved_top_item);
	} else if(num_items > shown_items) {
		set_top_item(num_items - shown_items);
	}
	
	append_text (string (middle, s.end ()));
}

void w_text_box::draw_item(vector<string>::const_iterator i, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const
{
    int computed_y = y + font->get_ascent();

    draw_text(s, (*i).c_str (), x, computed_y, get_dialog_color(MESSAGE_COLOR), font, style);
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




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
 *  sdl_widgets.h - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Sept-Nov 2001 (Woody Zenfell) - significant changes to many widgets:
 *    addition of more alignment and layout features, addition of methods to dynamically
 *    alter widget parts not previously alterable, addition of callback mechanisms to
 *    let user code respond to various events of interest, addition of widget enabling/disabling,
 *    addition of mechanisms to let user code find a widget in a dialog and get a widget's dialog
 *
 *  Mar 1, 2002 (Woody Zenfell):
 *      moved w_levels here (from shell_sdl); am using it in Setup Network Game dialog box.
 *
 *  August 27, 2003 (Woody Zenfell):
 *	new w_enabling_toggle can enable/disable a bank of other widgets according to its state
 *	new w_file_chooser displays filename; allows selection of file (via FileSpecifier::ReadDialog())
 *	miscellaneous minor code cleanup (dead code removal etc.)
 */

#ifndef SDL_WIDGETS_H
#define SDL_WIDGETS_H

#include	<SDL.h>

#include	"cseries.h"
#include	"sdl_dialogs.h"

#include    "map.h" // for entry_point, for w_levels
#include	"tags.h"	// for Typecode, for w_file_chooser
#include	"FileHandler.h"	// for FileSpecifier, for w_file_chooser

#include	<vector>
#include	<set>

struct SDL_Surface;
class sdl_font_info;


/*
 *  Widget base class
 */

class widget {
	friend class dialog;

public:
    enum alignment {
            kAlignNatural,
            kAlignLeft,
            kAlignCenter,
            kAlignRight
    };

    // ZZZ: initialize identifier, owning dialog, layout extensions
	widget();
        widget(int font);
	virtual ~widget() {}

	// Layout widget: calculate x position relative to dialog center (-> rect.x)
	// and return effective height of widget
	virtual int layout(void);

	// Draw widget
	virtual void draw(SDL_Surface *s) const = 0;

    // ZZZ: (dis)allow user interactions.  assume widget alters its drawing behavior for disabled state.
    void set_enabled(bool inEnabled);
    
	// Handle event
	virtual void mouse_move(int /*x*/, int /*y*/) {}
	virtual void click(int /*x*/, int /*y*/) {}
	virtual void event(SDL_Event & /*e*/) {}

	// Widget selectable?  (ZZZ change to use enabled by default)
	virtual bool is_selectable(void) const {return enabled; /* was "true" */ }
        
        // ZZZ: Get/set ID - see dialog::get_widget_by_id()
        short	get_identifier() const {return identifier;}
        void	set_identifier(short id) {identifier = id;}

        // ZZZ: Get dialog
        dialog*	get_owning_dialog() { return owning_dialog; }
        
        // ZZZ: Callback (from dialog code) provides additional layout information
        // This could probably be hidden from public: since dialog is a friend class, but I'm leaving
        // it exposed since layout() is exposed for some reason.
        virtual void	capture_layout_information(int16 x_offset, int16 available_width);

        // ZZZ: You must do most of these stupid layout tricks(tm) before dialog::run() or dialog::start().
        // They are basically "static" - there is no dynamic adjustment made once the dialog has been displayed.
        // They are of various quality from barely masked hack to nearly generally-applicable.  YMMV.  Most
        // combinations of layout options have NOT been tested.
        // ZZZ: Set widget alignment
        void    set_alignment(alignment inAlignment);

        // ZZZ: Set widget to full width of dialog - by default, widget uses "natural" width based on its contents
        // Important note: this generally increases only the width of the "mutable" part of the widget (e.g.,
        // the area the user types text into for w_text_entry), not the "static" part (e.g., the "prompt").
        void    set_full_width();

        // ZZZ: Reduce widget width (probably because another widget is being crammed in next to it)
        // You probably ought to do this to widgets that were set_full_width().
        void    reduce_width_by_width_of(widget* inEncroachingWidget);

        // ZZZ: Align bottom of widget with bottom of some previously-added widget
        void    align_bottom_with_bottom_of(widget* inWidget);
        
protected:
        // ZZZ: called by friend class dialog when we're added
        void	set_owning_dialog(dialog* new_owner) { owning_dialog = new_owner; }

	SDL_Rect rect;	// Position relative to dialog surface, and dimensions

	bool active;	// Flag: widget active (ZZZ note: this means it has the focus)
	bool dirty;		// Flag: widget needs redraw
    bool    enabled; // ZZZ Flag: roughly, should the user be allowed to interact with the widget?

	const sdl_font_info *font;	// Widget font
	uint16 style;				// Widget font style
        
        short	identifier;	// ZZZ: numeric ID in support of dialog::find_widget_by_id()
        dialog*	owning_dialog;	// ZZZ: which dialog currently contains us?  for get_dialog()

        // ZZZ: stupid layout tricks(tm) support
        alignment   widget_alignment;
        bool    full_width;
        int     width_reduction;
        widget* align_bottom_peer;
};


/*
 *  Vertical space
 */

class w_spacer : public widget {
public:
	w_spacer(uint16 height = get_dialog_space(SPACER_HEIGHT)) {rect.w = 0; rect.h = height;}

	void draw(SDL_Surface * /*s*/) const {}
	bool is_selectable(void) const {return false;}
};


/*
 *  Static text
 */

// ZZZ change: static_text now owns its own copy of whatever string(s) you give it, so constructor
// allocates storage, destructor frees, etc.
class w_static_text : public widget {
public:
	w_static_text(const char *text, int font = MESSAGE_FONT, int color = MESSAGE_COLOR);

	void draw(SDL_Surface *s) const;

        // ZZZ addition: change text after creation
        void set_text(const char* t);
        
	bool is_selectable(void) const {return false;}

        ~w_static_text();

private:
	char *text;
	int color;
};


/*
 *  Picture (PICT resource)
 */

class w_pict : public widget {
public:
	w_pict(int id);
	~w_pict();

	void draw(SDL_Surface *s) const;
	bool is_selectable(void) const {return false;}

private:
	SDL_Surface *picture;
};


/*
 *  Button
 */

typedef void (*action_proc)(void *);

class w_button : public widget {
public:
	w_button(const char *text, action_proc proc, void *arg);
	~w_button();

	void draw(SDL_Surface *s) const;
	void click(int x, int y);

protected:
	const char *text;
	action_proc proc;
	void *arg;
        
	SDL_Surface *button_l, *button_c, *button_r;
};


/*
 *  Button on left/right side of dialog box (always use in pairs, left button first)
 */

class w_left_button : public w_button {
public:

	w_left_button(const char *text, action_proc proc, void *arg) : w_button(text, proc, arg) {}

	int layout(void);
};

class w_right_button : public w_button {
public:

	w_right_button(const char *text, action_proc proc, void *arg) : w_button(text, proc, arg) {}

	int layout(void);
};


/*
 *  Selection button
 */

class w_select_button : public widget {
public:
	w_select_button(const char *name, const char *selection, action_proc proc, void *arg);

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);

	void set_selection(const char *selection);

protected:
	void set_arg(void *arg) { this->arg = arg; }

private:
	const char *name, *selection;
	action_proc proc;
	void *arg;

	int16 selection_x;			// X offset of selection display
};


/*
 *  Selection widget (base class)
 */

// ZZZ: type for callback function
class w_select;
typedef	void (*selection_changed_callback_t)(w_select* theWidget);

class w_select : public widget {
public:
	w_select(const char *name, size_t selection, const char **labels);
	virtual ~w_select();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	size_t get_selection(void) const {return (num_labels > 0 ? selection : UNONE);}
	void set_selection(size_t selection, bool simulate_user_input = false);

	// ZZZ: change labels after creation (used for entry-point selection when game type changes)
	// this is the "old way" - superseded (I hope) by set_labels_stringset.
	// both should still be valid, and should not have any evil interactions, so I may as well leave this in.
	// note that storage for string pointers AND storage for string data must be "kept alive" until
	// the widget is destroyed or another set_labels[_stringset] call is made.
	void	set_labels(const char** inLabels);

	// ZZZ: install stringset to use as labels
	// stringset is not copied.  altering stringset and not calling this method again -> no guarantees.
	// between widget and stringset, storage should be taken care of.  relax.
	void	set_labels_stringset(short inStringSetID);

	// ZZZ: set selection-changed callback - this will be called (if set) at the end of selection_changed (currently, anytime it "beeps")
	// removes need to subclass this class (or its subclasses!) just to add selection-changed behavior.
	void	set_selection_changed_callback(selection_changed_callback_t proc) { selection_changed_callback = proc; }

protected:
	virtual void selection_changed(void);

	const char *name;

	const char **labels;
	size_t num_labels;
	bool	we_own_labels;	// true if set up via stringset; false if not
	// Currently stringset approach allocates memory for pointers to strings, but shares string storage
	// with the stringset itself.  So we don't really own the *labels* - the strings - but we own the
	// storage for the data member "labels".

	size_t selection;			// UNONE means unknown selection
	int16 label_x;			// X offset of label display
        
	// ZZZ: storage for callback function
	selection_changed_callback_t	selection_changed_callback;

	// ZZZ: ripped this out for sharing
	uint16				get_largest_label_width();
};


/*
 *  On-off toggle
 */

class w_toggle : public w_select {
public:
	static const char *onoff_labels[3];

	w_toggle(const char *name, bool selection, const char **labels = onoff_labels);
};


/*
 *	Enabling toggle (ZZZ)
 *
 *	Can enable/disable a bank of other widgets according to its state
 */

class w_enabling_toggle : public w_toggle
{
public:
	w_enabling_toggle(const char* inName, bool inSelection, bool inEnablesWhenOn = true, const char** inLabels = onoff_labels);
	void add_dependent_widget(widget* inWidget) { dependents.insert(inWidget); update_widget_enabled(inWidget); }
	void remove_dependent_widget(widget* inWidget) { dependents.erase(inWidget); }
	
protected:
	void selection_changed();
	
private:
	void update_widget_enabled(widget* inWidget)
	{
		inWidget->set_enabled(selection == enables_when_on);
	}

	typedef std::set<widget*> DependentCollection;
	
	DependentCollection	dependents;
	bool			enables_when_on;
};


/*
 *  Player color selection
 */

class w_player_color : public w_select {
public:
	w_player_color(const char *name, int selection) : w_select(name, selection, color_labels) {}

	void draw(SDL_Surface *s) const;

	static const char *color_labels[9];
};


/*
 *  Text entry widget
 */

class w_text_entry;
typedef	void (*text_entry_event_callback_t)(w_text_entry* theWidget);

class w_text_entry : public widget {
public:
	w_text_entry(const char *name, size_t max_chars, const char *initial_text = NULL);
	~w_text_entry();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void event(SDL_Event &e);

	void set_text(const char *text);
	const char *get_text(void) {return buf;}

        // ZZZ: Change prompt for entry
        void set_name(const char *inName);
        
        // ZZZ: set callback for "enter" or "return" keypress
        void	set_enter_pressed_callback(text_entry_event_callback_t func) { enter_pressed_callback = func; }
        
        // ZZZ: set callback for value changed (will be called if value changed programmatically also)
        // (thought: this probably ought to be unified with w_select selection changed callback)
        void	set_value_changed_callback(text_entry_event_callback_t func) { value_changed_callback = func; }

        // ZZZ: capture more detailed layout information
        void	capture_layout_information(int16 leftmost_x, int16 usable_width);
       
protected:
	char *buf;		// Text entry buffer

        text_entry_event_callback_t	enter_pressed_callback;
        text_entry_event_callback_t	value_changed_callback;
        
private:
	void modified_text(void);

	const char *name;

	const sdl_font_info *text_font;	// Font for text
	uint16 text_style;

	size_t num_chars;		// Length of text in buffer
	size_t max_chars;		// Maximum number of chars in buffer
	int16 text_x;			// X offset of text display
	uint16 max_text_width;	// Maximum width of text display

    // ZZZ: these are used in conjunction with set_name to allow late updating of the real widget rect
    // so that moving from a larger rect to a smaller one correctly erases the leftover space.
    bool    new_rect_valid; // should these be used instead of the widget rect for internal drawing/computation?
    int16     new_rect_x;     // corresponds to rect.x
    uint16     new_rect_w;     // corresponds to rect.w
    int16     new_text_x;     // corresponds to text_x
};


/*
 *  Number entry widget
 */

class w_number_entry : public w_text_entry {
public:
	w_number_entry(const char *name, int initial_number = 0);

	void event(SDL_Event &e);

	void set_number(int number);
	int get_number(void) {return atoi(get_text());}
};


/*
 *  Key name widget
 */

class w_key : public widget {
public:
	w_key(const char *name, SDLKey key);

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	virtual void set_key(SDLKey key);
	SDLKey get_key(void) {return key;}

private:
	const char *name;

	int16 key_x;			// X offset of key name

	SDLKey key;
	bool binding;		// Flag: next key press will bind key
};


/*
 *  Progress Bar (ZZZ)
 */
/*	ZZZ abandoned for now - who would ever see the progress bar being drawn anyway? (in sending/receiving map etc.)
class w_progress_bar : public widget {
public:
        w_progress_bar(int inWidth) : widget(), max_value(10), value(0) {
            rect.w = inWidth;
            rect.h = 10;
        }
        
        ~w_progress_bar() {}
        
        void draw(SDL_Surface* s) const;
        
        bool is_selectable() { return false; }

        void set_progress(int inValue, int inMaxValue);

protected:
        int max_value;
        int value;
};
*/


/*
 *  Slider
 */

class w_slider : public widget {
public:
	w_slider(const char *name, int num_items, int sel);
	~w_slider();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	int get_selection(void) {return selection;}
	void set_selection(int selection);

	virtual void item_selected(void) {}

protected:
	const char *name;

	int16 slider_x;			// X offset of slider image

	int selection;			// Currently selected item
	int num_items;			// Total number of items

	bool thumb_dragging;	// Flag: currently dragging thumb
	int thumb_x;			// X position of thumb
	int trough_width;		// Width of trough

	int thumb_drag_x;		// X start position when dragging

	SDL_Surface *slider_l, *slider_c, *slider_r, *thumb;
};


/*
 *  Template for list selection widgets
 */

class w_list_base : public widget {
public:
	w_list_base(uint16 width, size_t lines, size_t sel);
	~w_list_base();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	size_t get_selection(void) {return selection;}

	virtual bool is_item_selectable(size_t /*i*/) {return true;}
	virtual void item_selected(void) = 0;

protected:
	virtual void draw_items(SDL_Surface *s) const = 0;
	void draw_image(SDL_Surface *dst, SDL_Surface *s, int16 x, int16 y) const;
	void set_selection(size_t s);
	void new_items(void);
	void center_item(size_t i);
	void set_top_item(size_t i);

	size_t selection;		// Currently selected item
	uint16 font_height;		// Height of font

	size_t num_items;		// Total number of items
	size_t shown_items;		// Number of shown items
	size_t top_item;		// Number of first visible item

	bool thumb_dragging;	// Flag: currently dragging scroll bar thumb
	SDL_Rect trough_rect;	// Dimensions of trough
	uint16 thumb_height;		// Height of thumb
	uint16 min_thumb_height;	// Minimal height of thumb
	int16 thumb_y;			// Y position of thumb

	int thumb_drag_y;		// Y start position when dragging

	SDL_Surface *frame_tl, *frame_t, *frame_tr, *frame_l, *frame_r, *frame_bl, *frame_b, *frame_br;
	SDL_Surface *thumb_t, *thumb_tc, *thumb_c, *thumb_bc, *thumb_b;
};

template <class T>
class w_list : public w_list_base {
public:
	w_list(const vector<T> &it, uint16 width, size_t lines, size_t sel) : w_list_base(width, lines, sel), items(it)
	{
		num_items = items.size();
		new_items();
		set_selection(sel);
		center_item(selection);
	}

	~w_list() {}

protected:
	void draw_items(SDL_Surface *s) const
	{
		typename vector<T>::const_iterator i = items.begin() + top_item;
		int16 x = rect.x + get_dialog_space(LIST_L_SPACE);
		int16 y = rect.y + get_dialog_space(LIST_T_SPACE);
		uint16 width = rect.w - get_dialog_space(LIST_L_SPACE) - get_dialog_space(LIST_R_SPACE);
		for (size_t n=top_item; n<top_item + MIN(shown_items, num_items); n++, i++, y=y+font_height)
			draw_item(i, s, x, y, width, n == selection && active);
	}

	const vector<T> &items;	// List of items

private:
	virtual void draw_item(typename vector<T>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const = 0;
};



/*
 *  Level number dialog
 *
 *  ZZZ: new constructor gives more control over appearance (including omission of level numbers).
 */

class w_levels : public w_list<entry_point> {
public:
	w_levels(const vector<entry_point> &items, dialog *d);
	w_levels(const vector<entry_point>& items, dialog* d, uint16 inWidth,
	size_t inNumLines, size_t inSelectedItem, bool in_show_level_numbers);

	void item_selected(void);

	void draw_item(vector<entry_point>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const;

private:
	dialog *parent;
	bool    show_level_numbers;
};



/*
 *	General file-chooser (ZZZ)
 */

class w_file_chooser : public w_select_button
{
public:
	w_file_chooser(const char* inLabel, const char* inDialogPrompt, Typecode inTypecode);

	void click(int x, int y);

	void set_file(const FileSpecifier& inFile);
	const FileSpecifier& get_file() { return file; }

private:
	void update_filename();
	
	FileSpecifier	file;
	char		filename[256];
	char		dialog_prompt[256];
	Typecode	typecode;
};

#endif

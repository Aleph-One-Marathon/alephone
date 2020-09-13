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
#include        "sdl_fonts.h"
#include        "screen_drawing.h"

#include    "map.h" // for entry_point, for w_levels
#include	"tags.h"	// for Typecode, for w_file_chooser
#include	"FileHandler.h"	// for FileSpecifier, for w_file_chooser

#include	<vector>
#include	<set>
#include	<boost/function.hpp>
#include 	<boost/bind.hpp>

#include "metaserver_messages.h" // for GameListMessage, for w_games_in_room and MetaserverPlayerInfo, for w_players_in_room
#include "network.h" // for prospective_joiner_info

#include	"binders.h"

struct SDL_Surface;
//class sdl_font_info;

typedef boost::function<void (void)> ControlHitCallback;
typedef boost::function<void (char)> GotCharacterCallback;

/*
 *  Widget base class
 */
class w_label;

class widget : public placeable{
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
        widget(int theme_widget);
	virtual ~widget() {}

	// Draw widget
	virtual void draw(SDL_Surface *s) const = 0;

    // ZZZ: (dis)allow user interactions.  assume widget alters its drawing behavior for disabled state.
    void set_enabled(bool inEnabled);
    
	// Handle event
	virtual void mouse_move(int /*x*/, int /*y*/) {}
	virtual void mouse_down(int x, int y) { click(x, y); }
	virtual void mouse_up(int /*x*/, int /*y*/) {}
	virtual void click(int /*x*/, int /*y*/) {}
	virtual void event(SDL_Event & /*e*/) {}

	// Widget selectable?  (ZZZ change to use enabled by default)
	virtual bool is_selectable(void) const {return enabled; /* was "true" */ }
        
        // ZZZ: Get/set ID - see dialog::get_widget_by_id()
        short	get_identifier() const {return identifier;}
        void	set_identifier(short id) {identifier = id;}

        // ZZZ: Get dialog
        dialog*	get_owning_dialog() { return owning_dialog; }

	// New positioning stuff
	virtual void set_rect(const SDL_Rect &r) { rect = r; }
        
	// implement placeable
	void place(const SDL_Rect &r, placement_flags flags);
	int min_height() { return saved_min_height; }
	int min_width() { return saved_min_width; }
	
	virtual bool is_dirty() { return dirty; }

	// labels are activated/enabled/disabled at the same time as this widget

	// creates and returns a new label
	w_label *label(const char *);
	void associate_label(w_label *label);
	
protected:
        // ZZZ: called by friend class dialog when we're added
        void	set_owning_dialog(dialog* new_owner) { owning_dialog = new_owner; }

	SDL_Rect rect;	// Position relative to dialog surface, and dimensions

	virtual void set_active(bool new_active);
	bool active;	// Flag: widget active (ZZZ note: this means it has the focus)
	bool dirty;		// Flag: widget needs redraw
    bool    enabled; // ZZZ Flag: roughly, should the user be allowed to interact with the widget?

	font_info *font;
	uint16 style;				// Widget font style
        
        short	identifier;	// ZZZ: numeric ID in support of dialog::find_widget_by_id()
        dialog*	owning_dialog;	// ZZZ: which dialog currently contains us?  for get_dialog()

	int saved_min_width;
	int saved_min_height;

	w_label* associated_label;
};


/*
 *  Vertical space
 */

class w_spacer : public placeable {
public:
	w_spacer(uint16 space = get_theme_space(SPACER_WIDGET)) : m_space(space) { }

	int min_height() { return m_space; }
	int min_width() { return m_space; }
	void place(const SDL_Rect&, placement_flags) { }

private:
	uint16 m_space;
};


/*
 *  Static text
 */

// ZZZ change: static_text now owns its own copy of whatever string(s) you give it, so constructor
// allocates storage, destructor frees, etc.
class w_static_text : public widget {
public:
	w_static_text(const char *text, int theme_type = MESSAGE_WIDGET);

	void draw(SDL_Surface *s) const;

        // ZZZ addition: change text after creation
        void set_text(const char* t);
        
	bool is_selectable(void) const {return false;}

        ~w_static_text();

protected:
	char *text;
	int theme_type;
};

class w_label : public w_static_text {
	friend class dialog;
	friend class w_slider;
public:
	w_label(const char *text) : w_static_text(text, LABEL_WIDGET), associated_widget(0) {}

	void associate_widget(widget *w) { associated_widget = w; }
	void draw(SDL_Surface *s) const;
	void click(int, int);
	bool is_selectable(void) const { if (associated_widget) return associated_widget->is_selectable(); else return false; }
private:
	widget *associated_widget;
};

class w_title : public w_static_text {
public:
	w_title(const char *text) : w_static_text(text, TITLE_WIDGET) {}
};

class w_styled_text : public w_static_text {
public:
	w_styled_text(const char *text, int theme_type = MESSAGE_WIDGET);

	void set_text(const char* t);
	void draw(SDL_Surface *s) const;
private:
	std::string text_string;
};

class w_slider_text : public w_static_text {
	friend class w_slider;
public:
	w_slider_text(const char *text) : w_static_text(text) {}
	void draw(SDL_Surface *s) const;
protected:
	class w_slider *associated_slider;
};


/*
 *  Button
 */

// typedef void (*action_proc)(void *);
typedef boost::function<void (void*)> action_proc;

class w_button_base : public widget {
public:
	w_button_base(const char *text, action_proc proc = NULL, void *arg = NULL, int type = BUTTON_WIDGET);
	virtual ~w_button_base();

	void set_callback (action_proc proc, void *arg);
	
	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void mouse_down(int x, int y);
	void mouse_up(int x, int y);
	void click(int x, int y);

protected:
	const std::string text;
	action_proc proc;
	void *arg;

	bool down, pressed;

	int type;
	// cache button centers since they are tiled or scaled
	SDL_Surface *button_c_default;
	SDL_Surface *button_c_active;
	SDL_Surface *button_c_disabled;
	SDL_Surface *button_c_pressed;
};
	

class w_button : public w_button_base {
public:
	w_button(const char *text, action_proc proc = NULL, void *arg = NULL) : w_button_base(text, proc, arg, BUTTON_WIDGET) {}
};

class w_tiny_button : public w_button_base {
public:
	w_tiny_button(const char *text, action_proc proc = NULL, void *arg = NULL) : w_button_base(text, proc, arg, TINY_BUTTON) {}
};

class w_hyperlink : public w_button_base {
public:
	w_hyperlink(const char *url, const char *text = NULL);
	
	void draw(SDL_Surface *s) const;
	void prochandler(void* arg);
};

/* 
 * Tabs
 */

class w_tab : public widget {
public:
	w_tab(const vector<string>& labels, tab_placer *placer);
	~w_tab();
	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event& e);

	void choose_tab(int i);
private:
	vector<string> labels;
	vector<int> widths;
	tab_placer *placer;

	vector<vector<SDL_Surface *> > images;

	int pressed_tab;
	int active_tab;
};

/*
 *  Selection button
 */

class w_select_button : public widget {
public:
	w_select_button(const char *selection, action_proc proc = NULL, void *arg = NULL, bool utf8 = false);

	void draw(SDL_Surface *s) const;
	void click(int x, int y);

	void set_selection(const char *selection);
	void set_callback(action_proc p, void* a) { proc = p; arg = a; }

	void place(const SDL_Rect& r, placement_flags flags = placeable::kDefault);
	
protected:
	void set_arg(void *arg) { this->arg = arg; }
	placement_flags p_flags;
private:
	const char *selection;
	action_proc proc;
	void *arg;

	int16 selection_x;			// X offset of selection display
	bool utf8; // render utf8 text instead of roman
};


/*
 *  Selection widget (base class)
 */

// ZZZ: type for callback function
class w_select;
// typedef void (*selection_changed_callback_t)(w_select* theWidget);
typedef boost::function<void (w_select*)> selection_changed_callback_t;

class w_select : public widget {
public:
	w_select(size_t selection, const char **labels);
	virtual ~w_select();

	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	void place(const SDL_Rect& r, placement_flags flags = placeable::kDefault);
	int min_width();

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

	void labels_are_utf8(bool _utf8) { utf8 = _utf8; }

protected:
	virtual void selection_changed(void);

	const char **labels;
	size_t num_labels;
	bool	we_own_labels;	// true if set up via stringset; false if not
	// Currently stringset approach allocates memory for pointers to strings, but shares string storage
	// with the stringset itself.  So we don't really own the *labels* - the strings - but we own the
	// storage for the data member "labels".

	size_t selection;			// UNONE means unknown selection
        
	// ZZZ: storage for callback function
	selection_changed_callback_t	selection_changed_callback;

	// ZZZ: ripped this out for sharing
	uint16				get_largest_label_width();

	bool utf8; // labels are UTF-8
};


/*
 *  On-off toggle
 */

class w_toggle : public w_select {
public:
	static const char *onoff_labels[3];

	w_toggle(bool selection, const char **labels = onoff_labels);
	int min_width();
	void draw(SDL_Surface *) const;
};


/*
 *	Enabling toggle (ZZZ)
 *
 *	Can enable/disable a bank of other widgets according to its state
 */

class w_enabling_toggle : public w_toggle
{
public:
	w_enabling_toggle(bool inSelection, bool inEnablesWhenOn = true, const char** inLabels = onoff_labels);
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
	w_player_color(int selection);

	void draw(SDL_Surface *s) const;
};

/*
 *  Text entry widget
 */

class w_text_entry : public widget {
	friend class w_number_entry;
public:
	typedef boost::function<void (w_text_entry*)> Callback;

	w_text_entry(size_t max_chars, const char *initial_text = NULL);
	~w_text_entry();

	void draw(SDL_Surface *s) const;
	void event(SDL_Event &e);
	void click(int, int);

	void set_text(const char *text);
	const char *get_text(void) {return buf;}
	
	void set_min_width(int w) { saved_min_width = w; }

        // ZZZ: set callback for "enter" or "return" keypress
        void	set_enter_pressed_callback(Callback func) { enter_pressed_callback = func; }
        
        // ZZZ: set callback for value changed (will be called if value changed programmatically also)
        // (thought: this probably ought to be unified with w_select selection changed callback)
        void	set_value_changed_callback(Callback func) { value_changed_callback = func; }

	void enable_mac_roman_input(bool enable = true) { enable_mac_roman = enable; }
	void place(const SDL_Rect& r, placement_flags flags);

       
protected:
	char *buf;		// Text entry buffer
	void set_active(bool new_active);

        Callback	enter_pressed_callback;
        Callback	value_changed_callback;
        
private:
	void modified_text(void);

	size_t num_chars;		// Length of text in buffer
	size_t max_chars;		// Maximum number of chars in buffer
	int16 text_x;			// X offset of text display
	uint16 max_text_width;	// Maximum width of text display
	size_t cursor_position;	// cursor position within buffer

	bool enable_mac_roman; // enable MacRoman input
};

class w_chat_entry : public w_text_entry {
public:
	w_chat_entry(size_t max_c) : w_text_entry(max_c, "") {
		font = get_theme_font(CHAT_ENTRY, style);
		saved_min_height =  (int16) font->get_ascent() + font->get_descent() + font->get_leading();
	}
};
/*
 *  Number entry widget
 */

class w_number_entry : public w_text_entry {
public:
	w_number_entry(int initial_number = 0);

	void event(SDL_Event &e);

	void set_number(int number);
	int get_number(void) {return atoi(get_text());}
};

/*
 * Displays asterisks instead of password
 */

class w_password_entry : public w_text_entry {
public:
	w_password_entry(size_t max_chars, const char *initial_text = 0);

	void draw(SDL_Surface *s) const;
};

/*
 *  Key name widget
 */

class w_key : public widget {
public:
	enum Type {
		KeyboardKey,
		MouseButton,
		JoystickButton
	} event_type;
	
	static Type event_type_for_key(SDL_Scancode key);
	
	w_key(SDL_Scancode key, w_key::Type event_type);

	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	virtual void set_key(SDL_Scancode key);
	SDL_Scancode get_key(void) {return key;}
	void place(const SDL_Rect& r, placement_flags flags);

protected:
	virtual void set_active(bool new_active);

private:
	const char *name;

	int16 key_x;			// X offset of key name

	SDL_Scancode key;
	bool binding;		// Flag: next key press will bind key
};


/*
 *  Progress Bar (ZZZ)
 */

class w_progress_bar : public widget {
public:
w_progress_bar(int inWidth) : widget(), max_value(10), value(0) {
		rect.w = inWidth;
		rect.h = 14;

		saved_min_width = rect.w;
		saved_min_height = rect.h;
        }
        
        ~w_progress_bar() {}
        
        void draw(SDL_Surface* s) const;
        
        bool is_selectable() { return false; }
	
        void set_progress(int inValue, int inMaxValue);

protected:
        int max_value;
        int value;
};



/*
 *  Slider
 */

class w_slider;
typedef boost::function<void (w_slider*)> slider_changed_callback_t;

class w_slider : public widget {
	friend class w_slider_text;
public:
	w_slider(int num_items, int sel);
	~w_slider();

	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	int get_selection(void) {return selection;}
	void set_selection(int selection);

	virtual void item_selected(void) {}
	virtual std::string formatted_value(void);

	void place(const SDL_Rect& r, placement_flags flags);
	
	void set_slider_changed_callback(slider_changed_callback_t proc) { slider_changed_callback = proc; }


protected:
	void init_formatted_value(void);
	
	w_slider_text *readout; // display current slider value
	int readout_x;			// relative X offset where readout is drawn
	
	int16 slider_x;			// X offset of slider image

	int selection;			// Currently selected item
	int num_items;			// Total number of items

	bool thumb_dragging;	// Flag: currently dragging thumb
	int thumb_x;			// X position of thumb
	int trough_width;		// Width of trough

	int thumb_drag_x;		// X start position when dragging

	SDL_Surface *slider_l, *slider_c, *slider_r, *thumb;
	
	slider_changed_callback_t slider_changed_callback;
	
private:
	int thumb_width() const;
	void selection_changed(void);
};

class w_percentage_slider : public w_slider {
public:
	w_percentage_slider(int num_items, int sel) : w_slider(num_items, sel) {
		init_formatted_value();
	}
	
	virtual std::string formatted_value(void);
};

class w_color_picker : public widget {
public:
	w_color_picker(rgb_color &color) : widget(MESSAGE_WIDGET), m_color(color) {
		saved_min_width = 48;
		saved_min_height = font->get_line_height();
	}
	
	const rgb_color& get_selection() { return m_color; }

	void draw(SDL_Surface *s) const;
	void click(int, int);
	
private:
	rgb_color m_color;

	struct update_color {
		update_color(w_percentage_slider *red, w_percentage_slider *green, w_percentage_slider *blue, uint16 *i_red, uint16 *i_green, uint16 *i_blue) : red_w(red), green_w(green), blue_w(blue), red(i_red), blue(i_blue), green(i_green) { }
		void operator()(dialog *) {
			*red = red_w->get_selection() << 12;
			*green = green_w->get_selection() << 12;
			*blue = blue_w->get_selection() << 12;
		}

		w_percentage_slider *red_w;
		w_percentage_slider *green_w;
		w_percentage_slider *blue_w;
		
		uint16* red;
		uint16* blue;
		uint16* green;
	};
};


/*
 *  Template for list selection widgets
 */

class w_list_base : public widget {
public:
	w_list_base(uint16 width, size_t lines, size_t sel);
	~w_list_base();

	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	size_t get_selection(void) {return selection;}

	virtual bool is_item_selectable(size_t /*i*/) {return true;}
	virtual void item_selected(void) = 0;

	void place(const SDL_Rect& r, placement_flags flags);

protected:
	virtual void draw_items(SDL_Surface *s) const = 0;
	void draw_image(SDL_Surface *dst, SDL_Surface *s, int16 x, int16 y) const;
	void set_selection(size_t s);
	void new_items(void);
	void center_item(size_t i);
	void set_top_item(size_t i);
	virtual uint16 item_height() const { return font->get_line_height(); }

	static const int kListScrollSpeed = 1;

	size_t selection;		// Currently selected item

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

	uint16 item_height() const { return font->get_line_height(); }

	~w_list() {}

protected:
	void draw_items(SDL_Surface *s) const
	{
		typename vector<T>::const_iterator i = items.begin() + top_item;
		int16 x = rect.x + get_theme_space(LIST_WIDGET, L_SPACE);
		int16 y = rect.y + get_theme_space(LIST_WIDGET, T_SPACE);
		uint16 width = rect.w - get_theme_space(LIST_WIDGET, L_SPACE) - get_theme_space(LIST_WIDGET, R_SPACE);
		for (size_t n=top_item; n<top_item + MIN(shown_items, num_items); n++, i++, y=y+item_height())
			draw_item(i, s, x, y, width, n == selection && active);
	}

	const vector<T> &items;	// List of items

private:
	virtual void draw_item(typename vector<T>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const = 0;

	w_list(const w_list<T>&);
	w_list<T>& operator =(const w_list<T>&);
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
 *  String List
 */
 
class w_string_list : public w_list<string> {
public:
	w_string_list(const vector<string> &items, dialog *d, int sel);

	void item_selected(void);

	void draw_item(vector<string>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const;

private:
	dialog *parent;
};


/*
 *  Selection Popup
 */

class w_select_popup : public w_select_button {
public:
	w_select_popup (action_proc p = NULL, void *a = NULL);
	
	void set_labels (const vector<string>& inLabels);/* {labels = inLabels;}*/
	void set_selection (int value);
	int get_selection () {return selection;}

	// Not to be confused with set_callback as inherited from w_select_button
	void set_popup_callback (action_proc p, void* a) {action = p; arg = a;}

private:
	int selection;
	vector<string> labels;

	action_proc action;
	void* arg;

	static void gotSelectedCallback (void* arg) {reinterpret_cast<w_select_popup*>(arg)->gotSelected();}
	void gotSelected ();
};


/*
 *	General file-chooser (ZZZ)
 */

class w_file_chooser : public w_select_button
{
public:
	w_file_chooser(const char* inDialogPrompt, Typecode inTypecode);

	void click(int x, int y);

	void set_file(const FileSpecifier& inFile);
	const FileSpecifier& get_file() { return file; }

	// we also have void set_callback(action_proc, void*)
	// as inherited from w_select_button
	void set_callback (ControlHitCallback callback) { m_callback = callback; }

private:
	void update_filename();
	
	FileSpecifier	file;
	char		filename[256];
	char		dialog_prompt[256];
	Typecode	typecode;
	ControlHitCallback m_callback;
};


/*
 *	Lists for metaserver dialog; moved from SdlMetaserverClientUi.cpp
 */

extern void set_drawing_clip_rectangle(short top, short left, short bottom, short right);

const string w_items_in_room_get_name_of_item (GameListMessage::GameListEntry item);
const string w_items_in_room_get_name_of_item (prospective_joiner_info item);
const string w_items_in_room_get_name_of_item (MetaserverPlayerInfo item);

template <typename tElement>
class w_items_in_room : public w_list_base
{
public:
	typedef typename boost::function<void (const tElement& item)> ItemClickedCallback;
	typedef typename std::vector<tElement> ElementVector;

	w_items_in_room(ItemClickedCallback itemClicked, int width, int numRows) :
		w_list_base(width, numRows, 0),
		m_itemClicked(itemClicked)
	{
		num_items = 0;
		new_items();
	}

	void set_collection(const std::vector<tElement>& elements) {
		m_items = elements;
		num_items = m_items.size();
		new_items();
		
		// do other crap - manage selection, force redraw, etc.
		get_owning_dialog ()->draw_dirty_widgets ();
	}
	
	void set_item_clicked_callback (ItemClickedCallback itemClicked) { m_itemClicked = itemClicked; }

	void item_selected() {
		if(m_itemClicked)
			m_itemClicked(m_items[selection]);
	}

	uint16 item_height() const { return font->get_line_height(); }

protected:
	void draw_items(SDL_Surface* s) const {
		typename ElementVector::const_iterator i = m_items.begin();
		int16 x = rect.x + get_theme_space(LIST_WIDGET, L_SPACE);
		int16 y = rect.y + get_theme_space(LIST_WIDGET, T_SPACE);
		uint16 width = rect.w - get_theme_space(LIST_WIDGET, L_SPACE) - get_theme_space(LIST_WIDGET, R_SPACE);

		for(size_t n = 0; n < top_item; n++)
			++i;

		for (size_t n=top_item; n<top_item + MIN(shown_items, num_items); n++, ++i, y=y+item_height())
			draw_item(*i, s, x, y, width, n == selection && active);
	}

private:
	ElementVector			m_items;
	ItemClickedCallback		m_itemClicked;

	// This should be factored out into a "drawer" object/Strategy
	virtual void draw_item(const tElement& item, SDL_Surface* s,
			int16 x, int16 y, uint16 width, bool selected) const {
		y += font->get_ascent();
		set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
		draw_text(s, w_items_in_room_get_name_of_item(item).c_str(), x, y, selected ? get_theme_color(ITEM_WIDGET, ACTIVE_STATE) : get_theme_color(ITEM_WIDGET, DEFAULT_STATE), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

	w_items_in_room(const w_items_in_room<tElement>&);
	w_items_in_room<tElement>& operator =(const w_items_in_room<tElement>&);
};


typedef w_items_in_room<prospective_joiner_info> w_joining_players_in_room;

class w_games_in_room : public w_items_in_room<GameListMessage::GameListEntry>
{
public:
	w_games_in_room(w_items_in_room<GameListMessage::GameListEntry>::ItemClickedCallback itemClicked, int width, int numRows)
		: w_items_in_room<GameListMessage::GameListEntry>(itemClicked, width, numRows), kGameSpacing(get_theme_space(METASERVER_GAMES, GAME_SPACING))
		{ font = get_theme_font(METASERVER_GAMES, style); saved_min_height = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE); }

	uint16 item_height() const { return 3 * font->get_line_height() + 2 + kGameSpacing; }

	void refresh() {
		dirty = true;
		get_owning_dialog()->draw_dirty_widgets();
	}

	enum {
		GAME_ENTRIES,
		GAME_SPACING,
	};

	enum {
		GAME,
		RUNNING_GAME,
		INCOMPATIBLE_GAME,
		SELECTED_GAME,
		SELECTED_RUNNING_GAME,
		SELECTED_INCOMPATIBLE_GAME,
	};
private:
	const int kGameSpacing;
	void draw_item(const GameListMessage::GameListEntry& item, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const;
};

class w_players_in_room : public w_items_in_room<MetaserverPlayerInfo>
{
public:
	w_players_in_room(w_items_in_room<MetaserverPlayerInfo>::ItemClickedCallback itemClicked, int width, int numRows)
	: w_items_in_room<MetaserverPlayerInfo>(itemClicked, width, numRows)
	{
		font = get_theme_font(METASERVER_PLAYERS, style);
		saved_min_height = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE);
	}

protected:
	uint16 item_height() const { return font->get_line_height() + 4; }
private:
	static const int kPlayerColorSwatchWidth = 8;
	static const int kTeamColorSwatchWidth = 4;
	static const int kSwatchGutter = 2;

	void draw_item(const MetaserverPlayerInfo& item, SDL_Surface* s,
		int16 x, int16 y, uint16 width, bool selected) const;
};



struct ColoredChatEntry
{
	enum Type {
		ChatMessage,
		PrivateMessage,
		ServerMessage,
		LocalMessage
	} type;
	
	// these next two are only valid for chat and private
	// otherwise they should be gray and ""
	rgb_color color;
	std::string sender;
	
	std::string message;
	
	ColoredChatEntry() : type(ChatMessage) { color.red = color.blue = color.green = 0x7fff; }
};

class w_colorful_chat : public w_list<ColoredChatEntry>
{
private:
	vector<ColoredChatEntry> entries;
public:
	w_colorful_chat(int width, int numRows) :
		w_list<ColoredChatEntry>(entries, width, numRows, 0),
		kNameWidth(get_theme_space(CHAT_ENTRY) - taper_width())
		{ num_items = 0; font = get_theme_font(CHAT_ENTRY, style); saved_min_height = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE); }

	virtual bool is_selectable(void) const { return true; }

	void item_selected() {} 

	void append_entry(const ColoredChatEntry&);

	void clear() { entries.clear(); num_items = 0; new_items(); }

	~w_colorful_chat() {} 

	uint16 item_height() const { return font->get_line_height() + 2; }

private:
	const int kNameWidth;

	uint16 taper_width() const { return (font->get_line_height() + 1) / 2 - 1; }

	void draw_item(vector<ColoredChatEntry>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const;
};

/*
 *	Wrappers to common widget interface follow
 */

class SDLWidgetWidget
{
public:
	void hide ();
	void show ();
	
	void activate ();
	void deactivate ();

protected:
	SDLWidgetWidget (widget* in_widget)
		: m_widget (in_widget)
		, hidden (false)
		, inactive (false)
		{}

	widget* m_widget;

private:
	bool hidden, inactive;
};

class ColorfulChatWidgetImpl : public SDLWidgetWidget
{
public:
	ColorfulChatWidgetImpl(w_colorful_chat* w)
		: SDLWidgetWidget(w), m_chat(w) { }
	
	void Append(const ColoredChatEntry& e) { m_chat->append_entry(e); }
	void Clear () { m_chat->clear (); }

private:
	w_colorful_chat *m_chat;
};

	
class ToggleWidget : public SDLWidgetWidget, public Bindable<bool>
{
public:
	ToggleWidget (w_toggle* toggle)
		: SDLWidgetWidget (toggle)
		, m_toggle (toggle)
		, m_callback (NULL)
		{ m_toggle->set_selection_changed_callback (boost::bind(&ToggleWidget::massage_callback, this, _1)); }

	void set_callback (ControlHitCallback callback) { m_callback = callback; }
	
	bool get_value () { return m_toggle->get_selection (); }
	void set_value (bool value) { m_toggle->set_selection (value); }
	
	bool bind_export () { return get_value (); }
	void bind_import (bool value) { set_value (value); }

private:
	void massage_callback (w_select* ignored)
		{ if (m_callback) m_callback (); }
	
	w_toggle* m_toggle;
	ControlHitCallback m_callback;
};

class SelectorWidget : public SDLWidgetWidget, public Bindable<int>
{
public:
	virtual void set_callback (ControlHitCallback callback) { m_callback = callback; }
	
	virtual void set_labels (int stringset) { set_labels (build_stringvector_from_stringset (stringset)); }
	virtual void set_labels (const std::vector<std::string>& labels) = 0;
	
	virtual int get_value () = 0;
	virtual void set_value (int value) = 0;

	int bind_export () { return get_value (); }
	void bind_import (int value) { set_value (value); }

	virtual ~SelectorWidget () {}

protected:
	SelectorWidget (widget* in_widget)
		: SDLWidgetWidget (in_widget)
		, m_callback (NULL)
		{}

	ControlHitCallback m_callback;
};

class PopupSelectorWidget : public SelectorWidget
{
public:
	PopupSelectorWidget (w_select_popup* select_popup_w)
		: SelectorWidget (select_popup_w)
		, m_select_popup (select_popup_w)
		{ select_popup_w->set_popup_callback (boost::bind(&PopupSelectorWidget::massage_callback, this, _1), NULL); }

	virtual void set_labels (const std::vector<std::string>& labels) { m_select_popup->set_labels (labels); }
	
	virtual int get_value () { return m_select_popup->get_selection (); }
	virtual void set_value (int value) { m_select_popup->set_selection (value); }
	
private:
	w_select_popup* m_select_popup;
	
	void massage_callback (void* ignored) { if (m_callback) m_callback (); }
};

class SelectSelectorWidget : public SelectorWidget
{
public:
	SelectSelectorWidget (w_select* select_w)
		: SelectorWidget (select_w)
		, m_select (select_w)
		{ m_select->set_selection_changed_callback (boost::bind(&SelectSelectorWidget::massage_callback, this, _1)); }

	// w_select only knows how to set labels from stringsets
	virtual void set_labels (int stringset) { m_select->set_labels_stringset (stringset); }
	virtual void set_labels (const std::vector<std::string>& labels) {}
	
	virtual int get_value () { return m_select->get_selection (); }
	virtual void set_value (int value) { m_select->set_selection (value); }
	
private:
	w_select* m_select;
	
	void massage_callback (w_select* ignored)
		{ if (m_callback) m_callback (); }
};

class ColourSelectorWidget : public SelectSelectorWidget
{
public:
	ColourSelectorWidget (w_player_color* player_color_w)
		: SelectSelectorWidget (player_color_w) {}

	// We ignore the labels and use swatches of colour instead
	virtual void set_labels (int stringset) {}
	virtual void set_labels (const std::vector<std::string>& labels) {}
};

class SliderSelectorWidget : public SelectorWidget
{
public:
	SliderSelectorWidget (w_slider* slider_w)
		: SelectorWidget (slider_w)
		, m_slider (slider_w)
		{}
	
	// Sliders don't get labels
	virtual void set_labels (const std::vector<std::string>& labels) {};

	virtual int get_value () { return m_slider->get_selection (); }
	virtual void set_value (int value) { m_slider->set_selection (value); }
	
	// Sliders don't get callbacks either

private:
	w_slider* m_slider;
};

class ButtonWidget : public SDLWidgetWidget
{
public:
	ButtonWidget (w_button_base* button)
		: SDLWidgetWidget (button)
		, m_button (button)
		, m_callback (NULL)
		{ m_button->set_callback (bounce_callback, this); }

	void set_callback (ControlHitCallback callback) { m_callback = callback; }

	void push () { if (m_callback) m_callback (); }

private:
	static void bounce_callback(void* arg)
		{ reinterpret_cast<ButtonWidget*>(arg)->push (); }

	w_button_base* m_button;
	ControlHitCallback m_callback;
};

class StaticTextWidget : public SDLWidgetWidget
{
public:
	StaticTextWidget (w_static_text* static_text_w)
		: SDLWidgetWidget (static_text_w)
		, m_static_text (static_text_w)
		{}
	
	void set_text (std::string s) { m_static_text->set_text (s.c_str ()); }
	
private:
	w_static_text* m_static_text;
};

class EditTextWidget : public SDLWidgetWidget, public Bindable<std::string>
{
public:
	EditTextWidget (w_text_entry* text_entry)
		: SDLWidgetWidget (text_entry)
		, m_text_entry (text_entry)
		{ m_text_entry->set_enter_pressed_callback (boost::bind(&EditTextWidget::got_submit, this, _1)); }

	void set_callback (GotCharacterCallback callback) { m_callback = callback; }

	void set_text (string s) { m_text_entry->set_text(s.c_str ()); }
	const string get_text () { return string(m_text_entry->get_text()); }
	
	std::string bind_export () { return get_text (); }
	void bind_import (std::string s) { set_text (s); }

private:
	w_text_entry* m_text_entry;
	GotCharacterCallback m_callback;
	
	// Yeah, I know.  Interface can be refined later.
	void got_submit(w_text_entry* ignored) { if (m_callback) m_callback ('\r'); }
};

class EditNumberWidget : public SDLWidgetWidget, public Bindable<int>
{
public:
	EditNumberWidget (w_number_entry* number_entry)
		: SDLWidgetWidget (number_entry)
		, m_number_entry (number_entry)
		{}

	void set_label (const std::string& s) {  }

	void set_value (int value) { m_number_entry->set_number (value); }
	int get_value () { return m_number_entry->get_number (); }
	
	int bind_export () { return get_value (); }
	void bind_import (int value) { set_value (value); }

private:
	w_number_entry* m_number_entry;
};

class FileChooserWidget : public SDLWidgetWidget, public Bindable<FileSpecifier>
{
public:
	FileChooserWidget (w_file_chooser* file_chooser)
		: SDLWidgetWidget (file_chooser)
		, m_file_chooser (file_chooser)
		{}
		
	void set_callback (ControlHitCallback callback) { m_file_chooser->set_callback (callback); }
	
	void set_file (const FileSpecifier& file) { m_file_chooser->set_file (file); }
	FileSpecifier get_file () { return m_file_chooser->get_file (); }
	
	virtual FileSpecifier bind_export () { return get_file (); }
	virtual void bind_import (FileSpecifier f) { set_file (f); }
	
private:
	w_file_chooser* m_file_chooser;
};


class GameListWidget
{
public:
	GameListWidget (w_games_in_room* games_in_room)
		: m_games_in_room (games_in_room)
		{
			m_games_in_room->set_item_clicked_callback(boost::bind(&GameListWidget::bounce_callback, this, _1));
		}

	void SetItems(const vector<GameListMessage::GameListEntry>& items) { m_games_in_room->set_collection (items); }

	void SetItemSelectedCallback(const boost::function<void (GameListMessage::GameListEntry)> itemSelected)
		{ m_callback = itemSelected; }

private:
	w_games_in_room* m_games_in_room;
	boost::function<void (GameListMessage::GameListEntry)> m_callback;
	
	void bounce_callback (GameListMessage::GameListEntry thingy)
		{ m_callback (thingy); }
};

class PlayerListWidget
{
public:
	PlayerListWidget (w_players_in_room* players_in_room)
		: m_players_in_room (players_in_room) {
		m_players_in_room->set_item_clicked_callback(boost::bind(&PlayerListWidget::bounce_callback, this, _1));
	}

	void SetItems(const vector<MetaserverPlayerInfo>& items) { m_players_in_room->set_collection (items); }
	void SetItemSelectedCallback(const boost::function<void (MetaserverPlayerInfo)> itemSelected)
	{ m_callback = itemSelected; }

	

private:
	w_players_in_room* m_players_in_room;
	boost::function<void (MetaserverPlayerInfo)> m_callback;
	
	void bounce_callback (MetaserverPlayerInfo thingy)
	{ m_callback(thingy); }
};

class JoiningPlayerListWidget
{
public:
	JoiningPlayerListWidget (w_joining_players_in_room* joining_players_in_room)
		: m_joining_players_in_room (joining_players_in_room)
		{
			m_joining_players_in_room->set_item_clicked_callback(boost::bind(&JoiningPlayerListWidget::bounce_callback, this, _1));
		}

	void SetItems(const vector<prospective_joiner_info>& items) { m_joining_players_in_room->set_collection (items); }

	void SetItemSelectedCallback(const boost::function<void (prospective_joiner_info)> itemSelected)
		{ m_callback = itemSelected; }

private:
	w_joining_players_in_room* m_joining_players_in_room;
	boost::function<void (prospective_joiner_info)> m_callback;
	
	void bounce_callback (prospective_joiner_info thingy)
		{ m_callback (thingy); }
};

class w_players_in_game2;
class PlayersInGameWidget : SDLWidgetWidget
{
public:
	PlayersInGameWidget (w_players_in_game2*);
	
	void redraw ();

private:
	w_players_in_game2* m_pig;
};

// There are no colour pickers in sdl; we never try to actually construct one of these guys
class ColourPickerWidget : public Bindable<RGBColor> {};

#endif

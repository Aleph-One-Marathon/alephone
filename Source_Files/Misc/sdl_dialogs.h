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
 *  sdl_dialogs.h - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_DIALOGS_H
#define SDL_DIALOGS_H

#include "cstypes.h"
#include <vector>
#include <memory>
#include <boost/function.hpp>
#include <SDL.h>

#ifndef NO_STD_NAMESPACE
using std::vector;
#endif

class widget;
class font_info;
class FileSpecifier;


/*
 *  Definitions
 */

class dialog;

class placeable {
public:
	enum placement_flags
	{
		kDefault = 0x0,
		kAlignLeft = 0x1,
		kAlignCenter = 0x2,
		kAlignRight = 0x4,
		kFill = 0x8,
	};
		
	placeable() : m_visible(true) { }
	virtual ~placeable() { }
	
	virtual void place(const SDL_Rect &r, placement_flags flags = kDefault) = 0;
	virtual int min_height() = 0;
	virtual int min_width() = 0;

	virtual bool visible() { return m_visible; }
	virtual void visible(bool visible) { m_visible = visible; }

private:
	bool m_visible;
};

class widget_placer : public placeable
{
public:
	widget_placer() : placeable() { }
	~widget_placer();

	void place(const SDL_Rect &r, placement_flags flags = kDefault) = 0;
	int min_height() = 0;
	int min_width() = 0;

protected:
	void assume_ownership(placeable *p) { m_owned.push_back(p); }

private:
	std::vector<placeable *> m_owned;
};

class tab_placer : public widget_placer
{
public:
	tab_placer() : widget_placer(), m_tab(0) { }
	void add(placeable *p, bool assume_ownership = false);
	void dual_add(widget *w, dialog& d);
	int min_height();
	int min_width();
	int tabs() { return m_tabs.size(); };
	void choose_tab(int new_tab);
	void place(const SDL_Rect &r, placement_flags flags = kDefault);
	bool visible() { return widget_placer::visible(); }
	void visible(bool visible);
private:
	int m_tab;
	std::vector<placeable *> m_tabs;
};

class table_placer : public widget_placer
{
public:
	enum { kSpace = 4 };
	table_placer(int columns, int space = kSpace, bool balance_widths = false) : widget_placer(), m_add(0), m_columns(columns), m_space(space), m_balance_widths(balance_widths) { m_col_flags.resize(m_columns); m_col_min_widths.resize(m_columns);}
	void add(placeable *p, bool assume_ownership = false);
	void add_row(placeable *p, bool assume_ownership = false);
	void col_flags(int col, placement_flags flags = kDefault) { m_col_flags[col] = flags; }
	void col_min_width(int col, int min_width) { m_col_min_widths[col] = min_width; }
	void dual_add(widget *w, dialog& d);
	void dual_add_row(widget *w, dialog& d);
	int min_height();
	int min_width();
	void place(const SDL_Rect &r, placement_flags flags = kDefault);
	void visible(bool visible);
	int col_width(int column);
private:
	int row_height(int row);
	int m_add; // column to add next widget to
	int m_columns; // number of columns
	int m_space;
	bool m_balance_widths;
	std::vector<std::vector<placeable *> > m_table;
	std::vector<placement_flags> m_col_flags;
	std::vector<int> m_col_min_widths;
};


class vertical_placer : public widget_placer
{
public:
	enum { kSpace = 0 };
	vertical_placer(int space = kSpace) : m_space(space), widget_placer(), m_min_width(0), m_add_flags(kDefault) { }

	void add(placeable *p, bool assume_ownership = false);
	void add_flags(placement_flags flags = kDefault) { m_add_flags = flags; }
	void dual_add(widget *w, dialog& d);
	int min_height();
	int min_width();
	void min_width(int w) { m_min_width = w; }

	void place(const SDL_Rect &r, placement_flags flags = kDefault);
	void visible(bool visible);

private:
	std::vector<placeable *> m_widgets;
	std::vector<int> m_widget_heights;
	std::vector<placement_flags> m_placement_flags;
	int m_min_width;
	placement_flags m_add_flags;
	int m_space;
};

class horizontal_placer : public widget_placer
{
public:
	enum { kSpace = 4 };
	horizontal_placer(int space = kSpace, bool balance_widths = false) : widget_placer(), m_space(space), m_add_flags(kDefault), m_balance_widths(balance_widths) { }
	
	void add(placeable *p, bool assume_ownership = false);
	void add_flags(placement_flags flags = kDefault) { m_add_flags = flags; }
	// adds w to this placer, as well as dialog d
	void dual_add(widget *w, dialog& d);
	int min_height();
	int min_width();

	void place(const SDL_Rect &r, placement_flags flags = kDefault);
	void visible(bool visible);

private:
	std::vector<placeable *> m_widgets;
	std::vector<int> m_widget_widths;
	std::vector<placement_flags> m_placement_flags;
	int m_space;
	placement_flags m_add_flags;
	bool m_balance_widths;
};
	

// Dialog structure
class dialog {
public:
	typedef boost::function<void (dialog* d)> Callback;

	dialog();
	~dialog();

	// Add widget to dialog
	void add(widget *w);

	// Run dialog modally
	int run(bool intro_exit_sounds = true);

	// Put dialog on screen
	void start(bool play_sound = true);

	// Process pending events, returns "done" flag
	bool process_events();

	// Remove dialog from screen, returns dialog result
	int finish(bool play_sound = true);

	// Quit dialog, return result
	void quit(int result);

	// Draw dialog
	void draw(void);
        
        // ZZZ: Draw those widgets that are marked as needing redraw
        // This is used automatically by the dialog code in response to user events --
        // other code doesn't need to call this unless it's altering dialog widgets
        // on its own (e.g. on a timer, or according to network activity, etc.)
        void draw_dirty_widgets() const;
        
	// Find the first widget with matching numerical ID
	widget *get_widget_by_id(short inID) const;

	// Set custom processing function, called while dialog is idle
	void set_processing_function(Callback func) { processing_function = func; }

	// Activate next selectable widget
	void activate_next_widget(void);

	// exposed to allow toggling fill_the_screen
	// don't call this unless you know what you're doing
	void layout(void); 

	// takes ownership
	void set_widget_placer(widget_placer *w) { placer = w; }

	void activate_widget(widget *w);

private:
	SDL_Surface *get_surface(void) const;
	void update(SDL_Rect r) const;
	void draw_widget(widget *w, bool do_update = true) const;
	void deactivate_currently_active_widget();
	void activate_first_widget(void);
	void activate_widget(size_t num);
	void activate_prev_widget(void);
	int find_widget(int x, int y);
	void event(SDL_Event &e);

	void new_layout(void);

	SDL_Rect rect;				// Position relative to video surface, and dimensions

	vector<widget *> widgets;	// List of widgets

	widget *active_widget;		// Pointer to active widget
	widget *mouse_widget;           // Pointer to mouse event widget
	size_t active_widget_num;		// Number of active widget

	int result;					// Dialog result code
	bool done;					// Flag: dialog done
	bool cursor_was_visible;	// Previous cursor state

	dialog *parent_dialog;		// Pointer to parent dialog

	Callback processing_function; // Custom idle procedure

	// Frame images (frame_t, frame_l, frame_r and frame_b must be freed)
	SDL_Surface *frame_tl, *frame_t, *frame_tr, *frame_l, *frame_r, *frame_bl, *frame_b, *frame_br;

	widget_placer *placer;
	bool layout_for_fullscreen; // is the current layout for fullscreen?

	Uint32 last_redraw;
	bool initial_text_input;
};

// Pointer to top-level dialog, NULL = no dialog active
extern dialog *top_dialog;

// Sounds
enum {
	DIALOG_INTRO_SOUND,
	DIALOG_OK_SOUND,
	DIALOG_CANCEL_SOUND,
	DIALOG_ERROR_SOUND,
	DIALOG_SELECT_SOUND,
	DIALOG_CLICK_SOUND,
	DIALOG_TYPE_SOUND,
	DIALOG_DELETE_SOUND,
	DIALOG_ERASE_SOUND,
	NUMBER_OF_DIALOG_SOUNDS
};

/*
 *  Functions
 */

extern void initialize_dialogs();

extern bool load_dialog_theme(bool force_reload = false);

extern uint32 get_dialog_player_color(size_t colorIndex); // ZZZ: added
extern void play_dialog_sound(int which);

// new theme stuff

// States
enum {
	DEFAULT_STATE,
	DISABLED_STATE,
	ACTIVE_STATE,
	CURSOR_STATE,
	PRESSED_STATE
};

// Widget types
enum {
	DEFAULT_WIDGET,
	DIALOG_FRAME,
	TITLE_WIDGET,
	LABEL_WIDGET,
	MESSAGE_WIDGET,
	ITEM_WIDGET,
	TEXT_ENTRY_WIDGET,
	SPACER_WIDGET,
	BUTTON_WIDGET,
	SLIDER_WIDGET,
	SLIDER_THUMB,
	LIST_WIDGET,
	LIST_THUMB,
	CHAT_ENTRY,
	TINY_BUTTON,
	CHECKBOX,
	TAB_WIDGET,
	METASERVER_WIDGETS,
	METASERVER_PLAYERS,
	METASERVER_GAMES,
	HYPERLINK_WIDGET,
};

enum {
	FOREGROUND_COLOR,
	BACKGROUND_COLOR,
	FRAME_COLOR
};

enum {
	TL_IMAGE,
	T_IMAGE,
	TR_IMAGE,
	L_IMAGE,
	R_IMAGE,
	BL_IMAGE,
	B_IMAGE,
	BR_IMAGE
};

enum {
	BUTTON_L_IMAGE,
	BUTTON_C_IMAGE,
	BUTTON_R_IMAGE
};

enum {
	SLIDER_L_IMAGE,
	SLIDER_C_IMAGE,
	SLIDER_R_IMAGE,
};

enum {
	SLIDER_T_SPACE,
	SLIDER_L_SPACE,
	SLIDER_R_SPACE
};

enum {
	T_SPACE,
	L_SPACE,
	R_SPACE,
	B_SPACE
};

enum {
	BUTTON_T_SPACE,
	BUTTON_L_SPACE,
	BUTTON_R_SPACE,
	BUTTON_HEIGHT,
};

enum {
	TAB_LC_SPACE = BUTTON_HEIGHT + 1,
	TAB_RC_SPACE
};

enum {
	TAB_L_IMAGE,
	TAB_LC_IMAGE,
	TAB_C_IMAGE,
	TAB_RC_IMAGE,
	TAB_R_IMAGE
};

enum {
	TROUGH_T_SPACE = B_SPACE + 1,
	TROUGH_R_SPACE,
	TROUGH_B_SPACE,
	TROUGH_WIDTH
};

enum {
	THUMB_T_IMAGE,
	THUMB_TC_IMAGE,
	THUMB_C_IMAGE,
	THUMB_BC_IMAGE,
	THUMB_B_IMAGE
};

extern font_info *get_theme_font(int widget_type, uint16 &style);

extern uint32 get_theme_color(int widget_type, int state, int which = 0);
extern SDL_Surface* get_theme_image(int widget_type, int state, int which, int width = 0, int height = 0);
extern bool use_theme_images(int widget_type);
extern bool use_theme_color(int widget_type, int which);
extern int get_theme_space(int widget_type, int which = 0);

extern void dialog_ok(void *arg);
extern void dialog_cancel(void *arg);

// ZZZ: some more handy callbacks
class w_text_entry;
extern void dialog_try_ok(w_text_entry* text_entry);
extern void dialog_disable_ok_if_empty(w_text_entry* inTextEntry);

#endif

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
 *  sdl_dialogs.h - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_DIALOGS_H
#define SDL_DIALOGS_H

#include <vector>
#include <memory>
#include <boost/function.hpp>

#ifndef NO_STD_NAMESPACE
using std::vector;
#endif

class widget;
struct SDL_Surface;
class sdl_font_info;
class FileSpecifier;


#ifdef __MVCPP__
#include "sdl_cseries.h"
#include "sdl_video.h"
#include "sdl_events.h"
//#include "sdl_widgets.h"
#endif

/*
 *  Definitions
 */

// Dialog structure
class dialog {
public:
	typedef boost::function<void (dialog* d)> Callback;

	dialog();
	~dialog();

	// Add widget to dialog
	void add(widget *w);
	void add_to_tab(widget *w, int tab);

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
	void draw(void) const;
        
        // ZZZ: Draw those widgets that are marked as needing redraw
        // This is used automatically by the dialog code in response to user events --
        // other code doesn't need to call this unless it's altering dialog widgets
        // on its own (e.g. on a timer, or according to network activity, etc.)
        void draw_dirty_widgets() const;
        
	// Switch between tabs, making on-tab widgets appear and off-tab widgets disappear
	void set_active_tab(int tab) { active_tab = tab; }
	
	// Find the first widget with matching numerical ID
	widget *get_widget_by_id(short inID) const;

	// Set custom processing function, called while dialog is idle
	void set_processing_function(Callback func) { processing_function = func; }

	// Activate next selectable widget
	void activate_next_widget(void);

private:
	SDL_Surface *get_surface(void) const;
	void update(SDL_Rect r) const;
	void draw_widget(widget *w, bool do_update = true) const;
	void deactivate_currently_active_widget(bool draw = true);
	void activate_first_widget(void);
	void activate_widget(size_t num, bool draw = true);
	void activate_prev_widget(void);
	int find_widget(int x, int y);
	void event(SDL_Event &e);
	void layout(void);

	SDL_Rect rect;				// Position relative to video surface, and dimensions

	vector<widget *> widgets;	// List of widgets
	vector<int> tabs;		// The tab (or lack thereof) associated with each widget

	int active_tab;			// The current tab

	widget *active_widget;		// Pointer to active widget
	size_t active_widget_num;		// Number of active widget

	int result;					// Dialog result code
	bool done;					// Flag: dialog done
	bool cursor_was_visible;	// Previous cursor state
	int saved_unicode_state;        // unicode state entering this dialog

	dialog *parent_dialog;		// Pointer to parent dialog

	Callback processing_function; // Custom idle procedure

	// Frame images (frame_t, frame_l, frame_r and frame_b must be freed)
	SDL_Surface *frame_tl, *frame_t, *frame_tr, *frame_l, *frame_r, *frame_bl, *frame_b, *frame_br;
};

// Pointer to top-level dialog, NULL = no dialog active
extern dialog *top_dialog;

// Fonts
enum {
	TITLE_FONT,
	BUTTON_FONT,
	LABEL_FONT,
	ITEM_FONT,
	MESSAGE_FONT,
	TEXT_ENTRY_FONT,
	TEXT_BOX_FONT,
	NUM_DIALOG_FONTS
};

// Colors
enum {
	BACKGROUND_COLOR,
	TITLE_COLOR,
	BUTTON_COLOR,
	BUTTON_ACTIVE_COLOR,
	LABEL_COLOR,
	LABEL_ACTIVE_COLOR,
	ITEM_COLOR,
	ITEM_ACTIVE_COLOR,
	MESSAGE_COLOR,
	TEXT_ENTRY_COLOR,
	TEXT_ENTRY_ACTIVE_COLOR,
	TEXT_ENTRY_CURSOR_COLOR,
	KEY_BINDING_COLOR,
	NUM_DIALOG_COLORS,
    
    // ZZZ: these should eventually be "real" colors gotten from MML theme, but I don't want to
    // get into that at this point.  It should be straightforward, so have at it ;)
    // Actually while I'm talking about this, really a better way would probably be to have the
    // MML do something like (I know little about XML/MML, so you'll have to interpret this a bit)
    // <widget>
    // <state default> <property> <property> etc. </state>
    // <state enabled> <property> etc. </state>
    // <state active> <property> etc. </state>
    // <state disabled> <property> etc. </state>
    // </widget>
    // Anything not specified in other substates would be gotten from the default state.  This way
    // other states like "pressed" or whatever could be easily added, and most widgets would get
    // reasonable default values.  Also, widgets with graphics (like buttons and lists) could easily
    // use different graphics for different states.
    // Maybe the default state need not be explicitly defined - it could get whatever properties are
    // listed for the widget (that are not claimed inside a different <state></state> pair).
    BUTTON_DISABLED_COLOR   = BUTTON_COLOR,
    LABEL_DISABLED_COLOR    = BUTTON_COLOR,
    ITEM_DISABLED_COLOR     = BUTTON_COLOR,
    TEXT_ENTRY_DISABLED_COLOR = BUTTON_COLOR
};

// Images
enum {
	FRAME_TL_IMAGE,
	FRAME_T_IMAGE,
	FRAME_TR_IMAGE,
	FRAME_L_IMAGE,
	FRAME_R_IMAGE,
	FRAME_BL_IMAGE,
	FRAME_B_IMAGE,
	FRAME_BR_IMAGE,
	LIST_TL_IMAGE,
	LIST_T_IMAGE,
	LIST_TR_IMAGE,
	LIST_L_IMAGE,
	LIST_R_IMAGE,
	LIST_BL_IMAGE,
	LIST_B_IMAGE,
	LIST_BR_IMAGE,
	THUMB_T_IMAGE,
	THUMB_TC_IMAGE,
	THUMB_C_IMAGE,
	THUMB_BC_IMAGE,
	THUMB_B_IMAGE,
	SLIDER_L_IMAGE,
	SLIDER_C_IMAGE,
	SLIDER_R_IMAGE,
	SLIDER_IMAGE,
	BUTTON_L_IMAGE,
	BUTTON_C_IMAGE,
	BUTTON_R_IMAGE,
	NUM_DIALOG_IMAGES
};

// Spaces
enum {
	FRAME_T_SPACE,
	FRAME_L_SPACE,
	FRAME_R_SPACE,
	FRAME_B_SPACE,
	SPACER_HEIGHT,
	LABEL_ITEM_SPACE,
	LIST_T_SPACE,
	LIST_L_SPACE,
	LIST_R_SPACE,
	LIST_B_SPACE,
	TROUGH_T_SPACE,
	TROUGH_R_SPACE,
	TROUGH_B_SPACE,
	TROUGH_WIDTH,
	SLIDER_T_SPACE,
	SLIDER_L_SPACE,
	SLIDER_R_SPACE,
	BUTTON_T_SPACE,
	BUTTON_L_SPACE,
	BUTTON_R_SPACE,
	BUTTON_HEIGHT,
	NUM_DIALOG_SPACES
};

// Sounds
#define DIALOG_INTRO_SOUND _snd_pattern_buffer
#define DIALOG_OK_SOUND _snd_pattern_buffer
#define DIALOG_CANCEL_SOUND _snd_defender_hit
#define DIALOG_ERROR_SOUND _snd_spht_door_obstructed
#define DIALOG_SELECT_SOUND _snd_major_fusion_charged
#define DIALOG_CLICK_SOUND _snd_computer_interface_page
#define DIALOG_TYPE_SOUND _snd_computer_interface_page
#define DIALOG_DELETE_SOUND _snd_hummer_attack
#define DIALOG_ERASE_SOUND _snd_compiler_death


/*
 *  Functions
 */

extern void initialize_dialogs(FileSpecifier &theme);

extern void load_theme(FileSpecifier &theme);

extern const sdl_font_info *get_dialog_font(int which, uint16 &style);
extern uint32 get_dialog_color(int which);
extern uint32 get_dialog_player_color(size_t colorIndex); // ZZZ: added
extern SDL_Surface *get_dialog_image(int which, int width = 0, int height = 0);
extern uint16 get_dialog_space(size_t which);
extern void play_dialog_sound(int which);

extern void dialog_ok(void *arg);
extern void dialog_cancel(void *arg);

// ZZZ: some more handy callbacks
class w_text_entry;
extern void dialog_try_ok(w_text_entry* text_entry);
extern void dialog_disable_ok_if_empty(w_text_entry* inTextEntry);

// Get the parser for themes (name "theme")
class XML_ElementParser;
XML_ElementParser *Theme_GetParser();

#endif

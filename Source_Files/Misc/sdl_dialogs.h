/*
 *  sdl_dialogs.h - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_DIALOGS_H
#define SDL_DIALOGS_H

#include <vector>

class widget;
struct SDL_Surface;
struct sdl_font_info;


/*
 *  Definitions
 */

// Dialog structure
class dialog {
public:
	dialog() : active_widget(NULL), active_widget_num(-1) {}
	~dialog();

	// Add widget to dialog
	void add(widget *w);

	// Run dialog modally
	int run(bool intro_exit_sounds = true);

	// Quit dialog, return result
	void quit(int result);

	// Draw dialog
	void draw(void) const;

private:
	SDL_Surface *get_surface(void) const;
	void layout(void);
	void update(SDL_Rect r) const;
	void draw_widget(widget *w, bool do_update = true) const;
	void activate_first_widget(void);
	void activate_widget(int num, bool draw = true);
	void activate_widget(widget *w, bool draw = true);
	void activate_next_widget(void);
	void activate_prev_widget(void);
	int find_widget(int x, int y);
	void event(SDL_Event &e);

	SDL_Rect rect;				// Position relative to video surface, and dimensions

	vector<widget *> widgets;	// List of widgets

	widget *active_widget;		// Pointer to active widget
	int active_widget_num;		// Number of active widget

	int result;					// Dialog result code
	bool done;					// Flag: dialog done
};

// Fonts
enum {
	TITLE_FONT,
	BUTTON_FONT,
	NORMAL_FONT,
	MESSAGE_FONT,
	TEXT_ENTRY_FONT,
	NUM_DIALOG_FONTS
};

// Colors
enum {
	BLACK_COLOR,
	WHITE_COLOR,
	TITLE_COLOR,
	BUTTON_COLOR,
	BUTTON_ACTIVE_COLOR,
	LABEL_COLOR,
	LABEL_ACTIVE_COLOR,
	ITEM_COLOR,
	ITEM_ACTIVE_COLOR,
	DISABLED_COLOR,
	MESSAGE_COLOR,
	BORDER_COLOR,
	THUMB_COLOR,
	THUMB_ACTIVE_COLOR,
	NUM_DIALOG_COLORS
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

extern void initialize_dialogs(void);
extern const sdl_font_info *get_dialog_font(int which, uint16 &style);
extern uint32 get_dialog_color(SDL_Surface *s, int which);

extern void dialog_ok(void *arg);
extern void dialog_cancel(void *arg);

#endif

/*
 *  sdl_dialogs.h - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_DIALOGS_H
#define SDL_DIALOGS_H

#include <vector>

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
	NUM_DIALOG_COLORS
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
extern SDL_Surface *get_dialog_image(int which, int width = 0, int height = 0);
extern int get_dialog_space(int which);
extern void play_dialog_sound(int which);

extern void dialog_ok(void *arg);
extern void dialog_cancel(void *arg);

// Get the parser for themes (name "theme")
class XML_ElementParser;
XML_ElementParser *Theme_GetParser();

#endif

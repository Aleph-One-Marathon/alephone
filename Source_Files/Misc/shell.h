#ifndef __SHELL_H
#define __SHELL_H

/*
SHELL.H
Saturday, August 22, 1992 2:18:48 PM

Saturday, January 2, 1993 10:22:46 PM
	thank god c doesn’t choke on incomplete structure references.

Jul 5, 2000 (Loren Petrich):
	Added XML support for controlling the cheats

Jul 7, 2000 (Loren Petrich):
	Added Ben Thompson's change: an Input-Sprocket-only input mode

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
*/

class FileSpecifier;

/* ---------- constants */

#include "XML_ElementParser.h"

#define MAXIMUM_COLORS ((short)256)

enum /* window reference numbers */
{
	refSCREEN_WINDOW= 1000
};

enum /* dialog reference numbers */
{
	refPREFERENCES_DIALOG= 8000,
	refCONFIGURE_KEYBOARD_DIALOG,
	refNETWORK_SETUP_DIALOG,
	refNETWORK_GATHER_DIALOG,
	refNETWORK_JOIN_DIALOG,
	refNETWORK_CARNAGE_DIALOG,
	
	LAST_DIALOG_REFCON= refNETWORK_CARNAGE_DIALOG,
	FIRST_DIALOG_REFCON= refPREFERENCES_DIALOG
};

#define sndCHANGED_VOLUME_SOUND 2000

/* ---------- resources */

enum {
	strPROMPTS= 131,
	_save_game_prompt= 0,
	_save_replay_prompt,
	_select_replay_prompt,
	_default_prompt
};

/* ---------- structures */

struct screen_mode_data
{
	short size;
	short acceleration;
	
	boolean high_resolution;
	boolean texture_floor, texture_ceiling;
	boolean draw_every_other_line;
	
	short bit_depth;  // currently 8 or 16
	short gamma_level;
	
	short unused[8];
};

#define NUMBER_OF_KEYS 21
#define NUMBER_UNUSED_KEYS 10

#define PREFERENCES_VERSION 17
#define PREFERENCES_CREATOR '52.4'
#define PREFERENCES_TYPE 'pref'
#define PREFERENCES_NAME_LENGTH 32

enum // input devices
{
	_keyboard_or_game_pad,
	_mouse_yaw_pitch,
	_mouse_yaw_velocity,
	_cybermaxx_input,  // only put "_input" here because it was defined elsewhere.
	_input_sprocket_only
};

struct system_information_data
{
	boolean has_seven;
	boolean has_apple_events;
	boolean appletalk_is_available;
	boolean machine_is_68k;
	boolean machine_is_68040;
	boolean machine_is_ppc;
	boolean machine_has_network_memory;
};

/* ---------- globals */

extern struct system_information_data *system_information;

/* ---------- prototypes/SHELL.C */

void global_idle_proc(void);
void handle_game_key(EventRecord *event, short key);

// LP addition for handling XML stuff:
XML_ElementParser *Cheats_GetParser();

/* ---------- prototypes/SHAPES.C */

void initialize_shape_handler(void);
#if defined(mac)
PixMapHandle get_shape_pixmap(short shape, boolean force_copy);
#elif defined(SDL)
SDL_Surface *get_shape_surface(int shape);
#endif

void open_shapes_file(FileSpecifier& File);

/* ---------- prototypes/SCREEN_DRAWING.C */

void _get_player_color(short color_index, RGBColor *color);
#if defined(mac)
void _get_interface_color(short color_index, RGBColor *color);
#elif defined(SDL)
void _get_interface_color(int color_index, SDL_Color *color);
#endif

/* ---------- protoypes/INTERFACE_MACINTOSH.C */
boolean try_for_event(boolean *use_waitnext);
void process_game_key(EventRecord *event, short key);
void update_game_window(WindowPtr window, EventRecord *event);
boolean has_cheat_modifiers(EventRecord *event);

/* ---------- prototypes/PREFERENCES.C */
void load_environment_from_preferences(void);

#endif

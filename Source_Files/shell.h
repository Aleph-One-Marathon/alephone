#ifndef __SHELL_H
#define __SHELL_H

/*
SHELL.H

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

Saturday, August 22, 1992 2:18:48 PM

Saturday, January 2, 1993 10:22:46 PM
	thank god c doesnÕt choke on incomplete structure references.

Jul 5, 2000 (Loren Petrich):
	Added XML support for controlling the cheats

Jul 7, 2000 (Loren Petrich):
	Added Ben Thompson's change: an Input-Sprocket-only input mode

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Dec 29, 2000 (Loren Petrich):
	Added function for showing text messages on the screen
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
	
	bool high_resolution;
	bool fullscreen, unused1;	// CB: formerly texture_floor/texture_ceiling, which are no longer supported by the renderer
	bool draw_every_other_line;
	
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
	bool has_seven;
	bool has_ten;
	bool has_apple_events;
	bool appletalk_is_available;
	bool machine_is_68k;
	bool machine_is_68040;
	bool machine_is_ppc;
	bool machine_has_network_memory;
	bool machine_is_bluebox;
	bool sdl_networking_is_available;
};

/* ---------- globals */

extern struct system_information_data *system_information;

#ifdef TARGET_API_MAC_CARBON
#ifdef USES_NIBS

extern CFBundleRef MainBundle;
extern IBNibRef GUI_Nib;

#endif
#endif

/* ---------- prototypes/SHELL.C [now shell_misc.cpp, shell_macintosh.cpp, shell_sdl.cpp] */

void global_idle_proc(void);

#ifdef mac
// LP: added Navigation-Services detection
void handle_game_key(EventRecord *event, short key);
bool machine_has_quicktime();
bool machine_has_nav_services();

// For loading MML from shapes and sounds resource forks
// when one of those files is opened
void XML_LoadFromResourceFork(FileSpecifier& File);

#ifdef TARGET_API_MAC_CARBON

// Gets a function pointer for a MacOS-X function
// that may not be explicitly available for CFM Carbon (Carbon/Classic).
// Returns NULL if such a function pointer could not be found
void *GetSystemFunctionPointer(const CFStringRef FunctionName);

#endif
#endif

// LP addition for handling XML stuff:
XML_ElementParser *Cheats_GetParser();

// Load the base MML scripts:
void LoadBaseMMLScripts();

/* ---------- prototypes/SHAPES.C */

void initialize_shape_handler(void);

#if defined(mac)
PixMapHandle get_shape_pixmap(short shape, bool force_copy);
#elif defined(SDL)
// ZZZ: this now works with RLE'd shapes, but needs extra storage.  Caller should
// be prepared to take a byte* if using an RLE shape (it will be set to NULL if
// shape is straight-coded); caller will need to free() that storage after freeing
// the SDL_Surface.
// If inIllumination is >= 0, it'd better be <= 1.  Shading tables are then used instead of the collection's CLUT.
// Among other effects (like being able to get darkened shapes), this lets player shapes be colorized according to
// team or player color.
// OK, yet another change... we now (optionally) take shape and collection separately, since there are too many
// low-level shapes in some collections to fit in the number of bits allotted.  If collection != NONE, it's taken
// as a collection and CLUT reference together; shape is (then) taken directly as a low-level shape index.
// If collection == NONE, shape is expected to convey information about all three elements (CLUT, collection,
// low-level shape index).
// Sigh, the extensions keep piling up... now we can also provide a quarter-sized surface from a shape.  It's hacky -
// the shape is shrunk by nearest-neighbor-style scaling (no smoothing), even at 16-bit and above, and it only works for RLE shapes.
SDL_Surface *get_shape_surface(int shape, int collection = NONE, byte** outPointerToPixelData = NULL, float inIllumination = -1.0f, bool inShrinkImage = false);
#endif

void open_shapes_file(FileSpecifier& File);

/* ---------- prototypes/SCREEN_DRAWING.C */

void _get_player_color(size_t color_index, RGBColor *color);
void _get_interface_color(size_t color_index, RGBColor *color);
#if defined(SDL)
void _get_player_color(size_t color_index, SDL_Color *color);
void _get_interface_color(size_t color_index, SDL_Color *color);
#endif

/* ---------- protoypes/INTERFACE_MACINTOSH.C */
#ifdef mac
bool try_for_event(bool *use_waitnext);
void process_game_key(EventRecord *event, short key);
void update_game_window(WindowPtr window, EventRecord *event);
bool has_cheat_modifiers(EventRecord *event);
#elif defined(SDL)
void update_game_window(void);
#endif

/* ---------- prototypes/PREFERENCES.C */
void load_environment_from_preferences(void);

// LP: displays a text message on the screen in "printf" fashion
// Implemented in the "screen" routines
void screen_printf(const char *format, ...);


#endif

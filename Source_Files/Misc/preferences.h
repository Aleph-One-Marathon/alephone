#ifndef __PREFERENCES_H
#define __PREFERENCES_H

/*
	preferences.h

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

	Tuesday, June 13, 1995 10:07:04 AM- rdm created.

Feb 10, 2000 (Loren Petrich):
	Added stuff for input modifiers: run/walk and swim/sink

Feb 25, 2000 (Loren Petrich):
	Set up persistent stuff for the chase cam and crosshairs

Mar 2, 2000 (Loren Petrich):
	Added chase-cam and crosshairs interfaces

Mar 14, 2000 (Loren Petrich):
	Added OpenGL stuff

Apr 27, 2000 (Loren Petrich):
	Added Josh Elsasser's "don't switch weapons" patch

Oct 22, 2001 (Woody Zenfell):
    Changed the player name in player_preferences_data back to a Pstring (was Cstring in SDL version)
*/

#include "interface.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"


/* New preferences junk */
const _fixed DEFAULT_MONITOR_REFRESH_FREQUENCY = 60 << 16;	// 60 Hz
struct graphics_preferences_data
{
	struct screen_mode_data screen_mode;
#ifdef mac
	GDSpec device_spec;
	// LP: this is the frequency value multiplied by 0x10000,
	// so as to take care of fractional values
	_fixed refresh_frequency;
#endif
	// LP change: added OpenGL support
	OGL_ConfigureData OGL_Configure;
};

struct serial_number_data
{
	bool network_only;
	byte long_serial_number[10];
	Str255 user_name;
	Str255 tokenized_serial_number;
};

struct network_preferences_data
{
	bool allow_microphone;
	bool  game_is_untimed;
	int16 type; // look in network_dialogs.c for _ethernet, etc...
	int16 game_type;
	int16 difficulty_level;
	uint16 game_options; // Penalize suicide, etc... see map.h for constants
	int32 time_limit;
	int16 kill_limit;
	int16 entry_point;
};

struct player_preferences_data
{
// ZZZ: We will always use a Pstring here so we can share routines more effectively
#if 1 //was ifdef mac
	unsigned char name[PREFERENCES_NAME_LENGTH+1];
#else
	char name[PREFERENCES_NAME_LENGTH+1];
#endif
	int16 color;
	int16 team;
	uint32 last_time_ran;
	int16 difficulty_level;
	bool background_music_on;
	struct ChaseCamData ChaseCam;
	struct CrosshairData Crosshairs;
};

// LP addition: input-modifier flags
// run/walk and swim/sink
// LP addition: Josh Elsasser's dont-switch-weapons patch
enum {
	_inputmod_interchange_run_walk = 0x0001,
	_inputmod_interchange_swim_sink = 0x0002,
	_inputmod_dont_switch_to_new_weapon = 0x0004,
	_inputmod_invert_mouse = 0x0008,
	_inputmod_use_button_sounds = 0x0010
};

struct input_preferences_data
{
	int16 input_device;
	int16 keycodes[NUMBER_OF_KEYS];
	// LP addition: input modifiers
	uint16 modifiers;
};

#define MAXIMUM_PATCHES_PER_ENVIRONMENT (32)

struct environment_preferences_data
{
#ifdef mac
	FSSpec map_file;
	FSSpec physics_file;
	FSSpec shapes_file;
	FSSpec sounds_file;
#else
	char map_file[256];
	char physics_file[256];
	char shapes_file[256];
	char sounds_file[256];
#endif
	uint32 map_checksum;
	uint32 physics_checksum;
	TimeType shapes_mod_date;
	TimeType sounds_mod_date;
	uint32 patches[MAXIMUM_PATCHES_PER_ENVIRONMENT];
#ifdef SDL
	char theme_dir[256];
#endif
};

/* New preferences.. (this sorta defeats the purpose of this system, but not really) */
extern struct graphics_preferences_data *graphics_preferences;
extern struct serial_number_data *serial_preferences;
extern struct network_preferences_data *network_preferences;
extern struct player_preferences_data *player_preferences;
extern struct input_preferences_data *input_preferences;
extern struct sound_manager_parameters *sound_preferences;
extern struct environment_preferences_data *environment_preferences;

/* --------- functions */
void initialize_preferences(void);
void handle_preferences(void);
void write_preferences(void);

#endif

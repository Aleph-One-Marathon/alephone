#ifndef __PREFERENCES_H
#define __PREFERENCES_H

/*
	preferences.h

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

May 16, 2002 (Woody Zenfell):
	New control option "don't auto-recenter view"

Apr 10, 2003 (Woody Zenfell):
	Join hinting and autogathering have Preferences entries now

May 22, 2003 (Woody Zenfell):
	Support for preferences for multiple network game protocols; configurable local game port.
 */

#include "interface.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
#include "shell.h"
#include "SoundManager.h"

#include <map>
#include <set>


/* New preferences junk */
const float DEFAULT_MONITOR_REFRESH_FREQUENCY = 60;	// 60 Hz

enum {
	_sw_alpha_off,
	_sw_alpha_fast,
	_sw_alpha_nice,
};
enum {
	_sw_driver_default,
	_sw_driver_none,
	_sw_driver_direct3d,
	_sw_driver_opengl,
};

enum {
	_ephemera_off,
	_ephemera_low,
	_ephemera_medium,
	_ephemera_high,
	_ephemera_ultra
};

struct graphics_preferences_data
{
	struct screen_mode_data screen_mode;
	// LP change: added OpenGL support
	OGL_ConfigureData OGL_Configure;

	bool double_corpse_limit;

	int16 software_alpha_blending;
	int16 software_sdl_driver;
	int16 fps_target; // should be a multiple of 30; 0 = unlimited

	int16 movie_export_video_quality;
	int32 movie_export_video_bitrate; // 0 is automatic
    int16 movie_export_audio_quality;

	int16 ephemera_quality;
};

enum {
	_network_game_protocol_ring,
	_network_game_protocol_star,
	NUMBER_OF_NETWORK_GAME_PROTOCOLS,

	_network_game_protocol_default = _network_game_protocol_star
};

struct network_preferences_data
{
	bool allow_microphone;
	bool game_is_untimed;
	int16 type; // look in network_dialogs.c for _ethernet, etc...
	int16 game_type;
	int16 difficulty_level;
	uint16 game_options; // Penalize suicide, etc... see map.h for constants
	int32 time_limit;
	int16 kill_limit;
	int16 entry_point;
	bool autogather;
	bool join_by_address;
	char join_address[256];
	uint16 game_port;	// TCP and UDP port number used for game traffic (not player-location traffic)
	uint16 game_protocol; // _network_game_protocol_star, etc.
	bool use_speex_encoder;
	bool use_netscript;
	char netscript_file[256];
	uint16 cheat_flags;
	bool advertise_on_metaserver;
	bool attempt_upnp;
	bool check_for_updates;
	bool verify_https;

	enum {
		kMetaserverLoginLength = 16
	};

	char metaserver_login[kMetaserverLoginLength];
	char metaserver_password[kMetaserverLoginLength];
	bool use_custom_metaserver_colors;
	rgb_color metaserver_colors[2];
	bool mute_metaserver_guests;
	bool join_metaserver_by_default;
	bool allow_stats;
};

struct player_preferences_data
{
	char name[PREFERENCES_NAME_LENGTH+1];
	int16 color;
	int16 team;
	uint32 last_time_ran;
	int16 difficulty_level;
	bool background_music_on;
	bool crosshairs_active;
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
	_inputmod_use_button_sounds = 0x0010,
	_inputmod_dont_auto_recenter = 0x0020   // ZZZ addition
};

// shell keys
enum {
	_key_inventory_left,
	_key_inventory_right,
	_key_switch_view,
	_key_volume_up,
	_key_volume_down,
	_key_zoom_in,
	_key_zoom_out,
	_key_toggle_fps,
	_key_activate_console,
	_key_show_scores,
	NUMBER_OF_SHELL_KEYS
};

enum {
	_mouse_accel_none,
	_mouse_accel_classic,
	_mouse_accel_symmetric,
	NUMBER_OF_MOUSE_ACCEL_TYPES
};

static constexpr int NUMBER_OF_HOTKEYS = 12;

typedef std::map<int, std::set<SDL_Scancode> > key_binding_map;

struct input_preferences_data
{
	int16 input_device;
	// LP addition: input modifiers
	uint16 modifiers;
	// Mouse-sensitivity parameters (LP: originally ZZZ)
	_fixed sens_horizontal;
	_fixed sens_vertical;
	int16 mouse_accel_type;
	float mouse_accel_scale;
	bool raw_mouse_input;
	bool extra_mouse_precision;
	bool classic_vertical_aim;
	
	// Limit absolute-mode {yaw, pitch} deltas per tick to +/- {32, 8} instead of {63, 15}
	bool classic_aim_speed_limits;
	
	bool controller_analog;
	_fixed controller_sensitivity;

	// if an axis reading is taken below this number in absolute
	// value, then we clip it to 0.  this lets people use
	// inaccurate zero points.
	int16 controller_deadzone;
	
	key_binding_map key_bindings;
	key_binding_map shell_key_bindings;
	key_binding_map hotkey_bindings;
};

#define MAXIMUM_PATCHES_PER_ENVIRONMENT (32)

struct environment_preferences_data
{
	char map_file[256];
	char physics_file[256];
	char shapes_file[256];
	char sounds_file[256];

	uint32 map_checksum;
	uint32 physics_checksum;
	TimeType shapes_mod_date;
	TimeType sounds_mod_date;
	uint32 patches[MAXIMUM_PATCHES_PER_ENVIRONMENT];

	// ZZZ: these aren't really environment preferences, but
	// preferences that affect the environment preferences dialog
	bool group_by_directory;	// if not, display popup as one giant flat list
	bool reduce_singletons;		// make groups of a single element part of a larger parent group

	// ghs: are themes part of the environment? they are now
	bool smooth_text;

	char solo_lua_file[256];
	bool use_solo_lua;
	bool use_replay_net_lua;
	bool hide_extensions;

	FilmProfileType film_profile;

	// Marathon 1 resources from the application itself
	char resources_file[256];

	// how many auto-named save files to keep around (0 is unlimited)
	uint32 maximum_quick_saves;
};

/* New preferences.. (this sorta defeats the purpose of this system, but not really) */
extern struct graphics_preferences_data *graphics_preferences;
extern struct network_preferences_data *network_preferences;
extern struct player_preferences_data *player_preferences;
extern struct input_preferences_data *input_preferences;
//extern struct sound_manager_parameters *sound_preferences;
extern SoundManager::Parameters *sound_preferences;
extern struct environment_preferences_data *environment_preferences;

/* --------- functions */
void initialize_preferences(void);
void read_preferences();
void handle_preferences(void);
void write_preferences(void);

static inline int16 get_fps_target() {
	return graphics_preferences->fps_target;
}

void transition_preferences(const DirectorySpecifier& legacy_prefs_dir);

#endif

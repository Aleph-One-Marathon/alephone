#ifndef __PREFERENCES_H
#define __PREFERENCES_H

/*

	preferences.h
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
*/

#include "interface.h"
#include "ChaseCam.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"

/* New preferences junk */
struct graphics_preferences_data
{
	struct screen_mode_data screen_mode;
#ifdef mac
	GDSpec device_spec;
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
	short type; // look in network_dialogs.c for _ethernet, etc...
	short game_type;
	short difficulty_level;
	short game_options; // Penalize suicide, etc... see map.h for constants
	long time_limit;
	short kill_limit;
	short entry_point;
};

struct player_preferences_data
{
#ifdef mac
	unsigned char name[PREFERENCES_NAME_LENGTH+1];
#else
	char name[PREFERENCES_NAME_LENGTH+1];
#endif
	short color;
	short team;
	unsigned long last_time_ran;
	short difficulty_level;
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
};

struct input_preferences_data
{
	short input_device;
	short keycodes[NUMBER_OF_KEYS];
	// LP addition: input modifiers
	short modifiers;
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
	unsigned long patches[MAXIMUM_PATCHES_PER_ENVIRONMENT];
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

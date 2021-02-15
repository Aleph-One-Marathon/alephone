#ifndef __INTERFACE_H
#define __INTERFACE_H

/*
INTERFACE.H

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

Monday, August 23, 1993 4:35:53 PM

Feb 24, 2000 (Loren Petrich):
	Added get_number_of_collection_frames(), so as to assist in wall-texture error checking

Feb 25, 2000 (Loren Petrich):
	Added chase-cam and crosshair data structures and dialogs

Mar 2, 2000 (Loren Petrich):
	Moved the chase-cam and crosshair stuff out to ChaseCam.h and Crosshairs.h

Mar 22, 2000 (Loren Petrich):
	Added ResetFieldOfView(), a function that sets the field of view to the player's current state
	(if extravision is active, then extravision, otherwise normal). It's defined in screen.c  

Apr 27, 2000 (Loren Petrich):
	Added Josh Elsasser's "don't switch weapons" patch

Apr 30, 2000 (Loren Petrich):
	Added reloading of view context when reverting, so that OpenGL won't look funny when one
	changes a level.
	
May 1, 2000 (Loren Petrich): Added XML parser object for the infravision stuff.

May 16, 2000 (Loren Petrich): Added XML parser for the control panels

May 16, 2002 (Woody Zenfell):
    Interfaces to dont_auto_recenter and to routines to help make such modifications safer
    for films and netplay.
*/

#include "cseries.h"

class FileSpecifier;
class OpenedResourceFile;

/* ---------- constants */

#define strFILENAMES 129
enum /* filenames in strFILENAMES */
{
	filenameSHAPES8,
	filenameSHAPES16,
	filenameSOUNDS8,
	filenameSOUNDS16,
	filenamePREFERENCES,
	filenameDEFAULT_MAP,
	filenameDEFAULT_SAVE_GAME,
	filenameMARATHON_NAME,
	filenameMARATHON_RECORDING,
	filenamePHYSICS_MODEL,
	filenameMUSIC,
	filenameIMAGES,
	filenameMOVIE,
	filenameDEFAULT_THEME,
	filenameEXTERNAL_RESOURCES,
};

#define strPATHS 138

#define strERRORS 128
enum /* errors in strERRORS */
{
	badProcessor= 0,
	badQuickDraw,
	badSystem,
	badMemory,
	badMonitor,
	badExtraFileLocations,
	badSoundChannels,
	fileError,
	copyHasBeenModified, // bad serial number
	copyHasExpired,
	keyIsUsedForSound,
	keyIsUsedForMapZooming,
	keyIsUsedForScrolling,
	keyIsUsedAlready,
	outOfMemory,
	warningExternalPhysicsModel,
	warningExternalMapsFile,
	badReadMapGameError,
	badReadMapSystemError,
	badWriteMap,
	badSerialNumber,
	duplicateSerialNumbers,
	networkOnlySerialNumber,
	corruptedMap,
	checkpointNotFound,
	pictureNotFound,
	networkNotSupportedForDemo,
	serverQuitInCooperativeNetGame,
	unableToGracefullyChangeLevelsNet,
	cantFindMap,	// called when the save game can't find the map.  Reverts to default map.
	cantFindReplayMap, // called when you can't find the map that the replay references..
	notEnoughNetworkMemory,
	luascriptconflict,
	replayVersionTooNew,
	keyScrollWheelDoesntWork
};

enum /* animation types */
{
	_animated1= 1,
	_animated2to8= 2, /* ?? */
	_animated3to4= 3,
	_animated4= 4,
	_animated5to8= 5,
	_animated8= 8,
	_animated3to5= 9,
	_unanimated= 10,
	_animated5= 11
};

enum /* shading tables */
{
	_darkening_table
};

enum /* shape types (this is for the editor) */
{
	_wall_shape, /* things designated as walls */
	_floor_or_ceiling_shape, /* walls in raw format */
	_object_shape, /* things designated as objects */
	_other_shape /* anything not falling into the above categories (guns, interface elements, etc) */
};

#define TOTAL_SHAPE_COLLECTIONS 128

enum /* The various default key setups a user can select. for vbl.c and it's callers */
{
	_standard_keyboard_setup,
	_left_handed_keyboard_setup,
	_powerbook_keyboard_setup,
	NUMBER_OF_KEY_SETUPS,
	
	_custom_keyboard_setup = NONE
};

#define INDEFINATE_TIME_DELAY (INT32_MAX)

/* ---------- shape descriptors */

#include "shape_descriptors.h"

/* ---------- structures */

#define _X_MIRRORED_BIT 0x8000
#define _Y_MIRRORED_BIT 0x4000
#define _KEYPOINT_OBSCURED_BIT 0x2000

struct shape_information_data
{
	uint16 flags; /* [x-mirror.1] [y-mirror.1] [keypoint_obscured.1] [unused.13] */

	_fixed minimum_light_intensity; /* in [0,FIXED_ONE] */

	short unused[5];

	short world_left, world_right, world_top, world_bottom;
	short world_x0, world_y0;
};

struct shape_animation_data // Also used in high_level_shape_definition
{
	int16 number_of_views; /* must be 1, 2, 5 or 8 */
	
	int16 frames_per_view, ticks_per_frame;
	int16 key_frame;
	
	int16 transfer_mode;
	int16 transfer_mode_period; /* in ticks */
	
	int16 first_frame_sound, key_frame_sound, last_frame_sound;

	int16 pixels_to_world;
	
	int16 loop_frame;

	int16 unused[14];

	/* N*frames_per_view indexes of low-level shapes follow, where
	   N = 1 if number_of_views = _unanimated/_animated1,
	   N = 4 if number_of_views = _animated3to4/_animated4,
	   N = 5 if number_of_views = _animated3to5/_animated5,
	   N = 8 if number_of_views = _animated2to8/_animated5to8/_animated8 */
	int16 low_level_shape_indexes[1];
};

/* ---------- prototypes/SHELL.C */

enum { /* controllers */
	_single_player,
	_network_player,
	_demo,
	_replay,
	_replay_from_file,
	NUMBER_OF_PSEUDO_PLAYERS
};

enum { /* states. */
	_display_intro_screens,
	_display_main_menu,
	_display_chapter_heading,
	_display_prologue,
	_display_epilogue,
	_display_credits,
	_display_intro_screens_for_demo,
	_display_quit_screens,
	NUMBER_OF_SCREENS,
	_game_in_progress= NUMBER_OF_SCREENS,
	_quit_game,
	_close_game,
	_switch_demo,
	_revert_game,
	_change_level,
	_begin_display_of_epilogue,
	_displaying_network_game_dialogs,
	NUMBER_OF_GAME_STATES
};

bool game_window_is_full_screen(void);
void set_change_level_destination(short level_number);
bool networking_available(void);
void free_and_unlock_memory(void);

/* ---------- prototypes/INTERFACE.C */

void initialize_game_state(void);
void force_game_state_change(void);
bool player_controlling_game(void);

void toggle_suppression_of_background_tasks(void);
bool suppress_background_events(void);

void set_game_state(short new_state);
short get_game_state(void);
short get_game_controller(void);
bool current_netgame_allows_microphone();
void set_change_level_destination(short level_number);
bool check_level_change(void);
void pause_game(void);
void resume_game(void);
void portable_process_screen_click(short x, short y, bool cheatkeys_down);
void process_main_menu_highlight_advance(bool reverse);
void process_main_menu_highlight_select(bool cheatkeys_down);
void draw_menu_button_for_command(short index);
void update_interface_display(void);
bool idle_game_state(uint32 ticks);
void display_main_menu(void);
void do_menu_item_command(short menu_id, short menu_item, bool cheat);
bool interface_fade_finished(void);
void stop_interface_fade(void);
bool enabled_item(short item);
void paint_window_black(void);

/* ---------- prototypes/INTERFACE_MACINTOSH.C */
void do_preferences(void);
short get_level_number_from_user(void);
void toggle_menus(bool game_started);
void install_network_microphone(void);
void remove_network_microphone(void);

// Should return NONE if user cancels, 0 for single player, or 1 for multiplayer.
// Game has been loaded from file before this is called so elements like
// dynamic_world->player_count are available.  Cursor has been hidden when called.
size_t should_restore_game_networked(FileSpecifier& file);

void show_movie(short index);

void exit_networking(void);

void load_main_menu_buffers(short base_id);
bool main_menu_buffers_loaded(void);
void main_menu_bit_depth_changed(short base_id);
void free_main_menu_buffers(void);
void draw_main_menu(void);
void draw_menu_button(short index, bool pressed);

/* ---------- prototypes/INTERFACE_MACINTOSH.C- couldn't think of a better place... */
void hide_cursor(void);
void show_cursor(void);
void set_drawing_clip_rectangle(short top, short left, short bottom, short right);

/* ---------- prototypes/SHAPES.C */
void *get_global_shading_table(void);

short get_shape_descriptors(short shape_type, shape_descriptor *buffer);

#define get_shape_bitmap_and_shading_table(shape, bitmap, shading_table, shading_mode) extended_get_shape_bitmap_and_shading_table(GET_DESCRIPTOR_COLLECTION(shape), \
	GET_DESCRIPTOR_SHAPE(shape), (bitmap), (shading_table), (shading_mode))
void extended_get_shape_bitmap_and_shading_table(short collection_code, short low_level_shape_index,
	struct bitmap_definition **bitmap, void **shading_tables, short shading_mode);

#define get_shape_information(shape) extended_get_shape_information(GET_DESCRIPTOR_COLLECTION(shape), GET_DESCRIPTOR_SHAPE(shape))
struct shape_information_data *extended_get_shape_information(short collection_code, short low_level_shape_index);

void get_shape_hotpoint(shape_descriptor texture, short *x0, short *y0);
struct shape_animation_data *get_shape_animation_data(shape_descriptor texture);
void process_collection_sounds(short colleciton_code, void (*process_sound)(short sound_index));

#define mark_collection_for_loading(c) mark_collection((c), true)
#define mark_collection_for_unloading(c) mark_collection((c), false)
void mark_collection(short collection_code, bool loading);
void strip_collection(short collection_code);
void load_collections(bool with_progress_bar, bool is_opengl);
int count_replacement_collections();
void load_replacement_collections();
void unload_all_collections(void);

void set_shapes_patch_data(uint8 *data, size_t length);
uint8* get_shapes_patch_data(size_t &length);

// LP additions:
// Whether or not collection is present
bool is_collection_present(short collection_index);
// Number of texture frames in a collection (good for wall-texture error checking)
short get_number_of_collection_frames(short collection_index);
// Number of bitmaps in a collection (good for allocating texture information for OpenGL)
short get_number_of_collection_bitmaps(short collection_index);
// Which bitmap index for a frame (good for OpenGL texture rendering)
short get_bitmap_index(short collection_index, short low_level_shape_index);
// Get CLUT for collection
struct rgb_color_value *get_collection_colors(short collection_index, short clut_number, int &num_colors);

// ZZZ: made these visible
struct low_level_shape_definition *get_low_level_shape_definition(short collection_index, short low_level_shape_index);


/* ---------- prototypes/PREPROCESS_MAP_MAC.C */
void setup_revert_game_info(struct game_data *game_info, struct player_start_data *start, struct entry_point *entry);
bool revert_game(void);
bool load_game(bool use_last_load);
bool save_game(void);
bool save_game_full_auto(bool inOverwriteRecent);
void restart_game(void);

/* ---------- prototypes/GAME_WAD.C */
/* Map transferring fuctions */
int32 get_net_map_data_length(void *data);
bool process_net_map_data(void *data); /* Note that this frees it as well */
void *get_map_for_net_transfer(struct entry_point *entry);

/* ---------- prototypes/VBL.C */

void set_keyboard_controller_status(bool active);
bool get_keyboard_controller_status(void);
void pause_keyboard_controller(bool active);
int32 get_heartbeat_count(void);
float get_heartbeat_fraction(void);
void wait_until_next_frame(void);
void sync_heartbeat_count(void);
void process_action_flags(short player_identifier, const uint32 *action_flags, short count);
void rewind_recording(void);
void stop_recording(void);
void stop_replay(void);
void move_replay(void);
void check_recording_replaying(void);
bool has_recording_file(void);
void increment_replay_speed(void);
void decrement_replay_speed(void);
void reset_recording_and_playback_queues(void);
uint32 parse_keymap(void);

/* ---------- prototypes/GAME_DIALOGS.C */

bool handle_preferences_dialog(void);
void handle_load_game(void);
void handle_save_game(void);
bool handle_start_game(void);
bool quit_without_saving(void);

/* ---------- prototypes/GAME_WINDOW.C */
void scroll_inventory(short dy);

/* ---------- prototypes/NETWORK.C */

enum {	// Results for network_join
	kNetworkJoinFailedUnjoined,
        kNetworkJoinFailedJoined,
        kNetworkJoinedNewGame,
        kNetworkJoinedResumeGame
};

bool network_gather(bool inResumingGame);
int network_join(void);

/* ---------- prototypes/NETWORK_MICROPHONE.C */

void handle_microphone(bool triggered);

/* ---------- prototypes/PHYSICS.C */

void reset_absolute_positioning_device(_fixed yaw, _fixed pitch, _fixed velocity);

/* ---------- prototypes/IMPORT_DEFINITIONS.C */

void init_physics_wad_data();
void import_definition_structures(void);

/* ---------- prototypes/KEYBOARD_DIALOG.C */
bool configure_key_setup(short *keycodes);

/* --------- from PREPROCESS_MAP_MAC.C */
bool have_default_files(void);
void get_default_external_resources_spec(FileSpecifier& File);
void get_default_map_spec(FileSpecifier& File);
void get_default_physics_spec(FileSpecifier& File);
void get_default_sounds_spec(FileSpecifier& File);
void get_default_shapes_spec(FileSpecifier& File);
bool get_default_theme_spec(FileSpecifier& File);
// ZZZ addition: since Mac versions now search for any candidate files instead of picking
// by name, new interface to search for all simultaneously instead of duplicating effort.
void get_default_file_specs(FileSpecifier* outMapSpec, FileSpecifier* outShapesSpec, FileSpecifier* outSoundsSpec, FileSpecifier* outPhysicsSpec);

// external resources: terminals for Marathon 1
void set_external_resources_file(FileSpecifier&);
void close_external_resources();
extern OpenedResourceFile ExternalResources;

// LP change: resets field of view to whatever the player had had when reviving
void ResetFieldOfView();

// LP change: modification of Josh Elsasser's dont-switch-weapons patch
bool dont_switch_to_new_weapon();

bool dont_auto_recenter();

// ZZZ: let code disable (standardize)/enable behavior modifiers like
// dont_switch
void standardize_player_behavior_modifiers();
void restore_custom_player_behavior_modifiers();

// ZZZ: return whether the user's behavior matches standard behavior
// (either by being forced so or by chosen that way)
bool is_player_behavior_standard();

// LP change: force reload of view context
void ReloadViewContext();

class InfoTree;
void parse_mml_infravision(const InfoTree& root);
void reset_mml_infravision();
void parse_mml_control_panels(const InfoTree& root);
void reset_mml_control_panels();

#endif

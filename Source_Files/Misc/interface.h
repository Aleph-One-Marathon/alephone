#ifndef __INTERFACE_H
#define __INTERFACE_H

/*
INTERFACE.H
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
*/

#include "XML_ElementParser.h"

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
	filenameMOVIE
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
	notEnoughNetworkMemory
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

#define INDEFINATE_TIME_DELAY (LONG_MAX)

/* ---------- shape descriptors */

#include "shape_descriptors.h"

/* ---------- structures */

#define _X_MIRRORED_BIT 0x8000
#define _Y_MIRRORED_BIT 0x4000
#define _KEYPOINT_OBSCURED_BIT 0x2000

struct shape_information_data
{
	word flags; /* [x-mirror.1] [y-mirror.1] [keypoint_obscured.1] [unused.13] */

	fixed minimum_light_intensity; /* in [0,FIXED_ONE] */

	short unused[5];

	short world_left, world_right, world_top, world_bottom;
	short world_x0, world_y0;
};

struct shape_animation_data
{
	short number_of_views; /* must be 1, 2, 5 or 8 */
	
	short frames_per_view, ticks_per_frame;
	short key_frame;
	
	short transfer_mode;
	short transfer_mode_period; /* in ticks */
	
	short first_frame_sound, key_frame_sound, last_frame_sound;

	short pixels_to_world;
	
	short loop_frame;

	short unused[14];

	/* number_of_views*frames_per_view indexes of low-level shapes follow */
	short low_level_shape_indexes[1];
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

boolean game_window_is_full_screen(void);
void set_change_level_destination(short level_number);
boolean networking_available(void);
void free_and_unlock_memory(void);

/* ---------- prototypes/INTERFACE.C */

void initialize_game_state(void);
void force_game_state_change(void);
boolean player_controlling_game(void);

void toggle_suppression_of_background_tasks(void);
boolean suppress_background_events(void);

void set_game_state(short new_state);
short get_game_state(void);
short get_game_controller(void);
void set_change_level_destination(short level_number);
boolean check_level_change(void);
void pause_game(void);
void resume_game(void);
void portable_process_screen_click(short x, short y, boolean cheatkeys_down);
void draw_menu_button_for_command(short index);
void update_interface_display(void);
void idle_game_state(void);
void display_main_menu(void);
void do_menu_item_command(short menu_id, short menu_item, boolean cheat);
boolean interface_fade_finished(void);
void stop_interface_fade(void);
boolean enabled_item(short item);
void paint_window_black(void);

/* ---------- prototypes/INTERFACE_MACINTOSH.C */
void do_preferences(void);
short get_level_number_from_user(void);
void toggle_menus(boolean game_started);
short get_difficulty_level(void);
void install_network_microphone(void);
void remove_network_microphone(void);

void show_movie(short index);

void exit_networking(void);

void load_main_menu_buffers(short base_id);
boolean main_menu_buffers_loaded(void);
void main_menu_bit_depth_changed(short base_id);
void free_main_menu_buffers(void);
void draw_main_menu(void);
void draw_menu_button(short index, boolean pressed);

/* ---------- prototypes/INTERFACE_MACINTOSH.C- couldn't think of a better place... */
void hide_cursor(void);
void show_cursor(void);
boolean mouse_still_down(void);
void get_mouse_position(short *x, short *y);
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

#define mark_collection_for_loading(c) mark_collection((c), TRUE)
#define mark_collection_for_unloading(c) mark_collection((c), FALSE)
void mark_collection(short collection_code, boolean loading);
void strip_collection(short collection_code);
void load_collections(void);
void unload_all_collections(void);

// LP additions:
// Whether or not collection is present
bool is_collection_present(short collection_index);
// Number of texture frames in a collection (good for wall-texture error checking)
short get_number_of_collection_frames(short collection_index);
// Number of bitmaps in a collection (good for allocating texture information for OpenGL)
short get_number_of_collection_bitmaps(short collection_index);
// Which bitmap index for a frame (good for OpenGL texture rendering)
short get_bitmap_index(short collection_index, short low_level_shape_index);


/* ---------- prototypes/PREPROCESS_MAP_MAC.C */
void setup_revert_game_info(struct game_data *game_info, struct player_start_data *start, struct entry_point *entry);
boolean revert_game(void);
boolean load_game(boolean use_last_load);
boolean save_game(void);
void restart_game(void);

/* ---------- prototypes/GAME_WAD.C */
/* Map transferring fuctions */
long get_net_map_data_length(void *data);
boolean process_net_map_data(void *data); /* Note that this frees it as well */
void *get_map_for_net_transfer(struct entry_point *entry);

/* ---------- prototypes/VBL.C */

void set_keyboard_controller_status(boolean active);
boolean get_keyboard_controller_status(void);
void pause_keyboard_controller(boolean active);
long get_heartbeat_count(void);
void sync_heartbeat_count(void);
void process_action_flags(short player_identifier, long *action_flags, short count);
void rewind_recording(void);
void stop_recording(void);
void stop_replay(void);
void move_replay(void);
void check_recording_replaying(void);
short find_key_setup(short *keycodes);
void set_default_keys(short *keycodes, short which_default);
void set_keys(short *keycodes);
boolean has_recording_file(void);
void increment_replay_speed(void);
void decrement_replay_speed(void);
void reset_recording_and_playback_queues(void);
long parse_keymap(void);

#define MAXIMUM_DEQUEUED_KEYMAP_COUNT 16
short dequeue_keymaps(short count, long *buffer);

/* ---------- prototypes/GAME_DIALOGS.C */

boolean handle_preferences_dialog(void);
void handle_load_game(void);
void handle_save_game(void);
boolean handle_start_game(void);
boolean quit_without_saving(void);

/* ---------- prototypes/GAME_WINDOW.C */
void scroll_inventory(short dy);

/* ---------- prototypes/NETWORK.C */

boolean network_gather(void);
boolean network_join(void);

/* ---------- prototypes/NETWORK_MICROPHONE.C */

void handle_microphone(boolean triggered);

/* ---------- prototypes/PHYSICS.C */

void reset_absolute_positioning_device(fixed yaw, fixed pitch, fixed velocity);

/* ---------- prototypes/IMPORT_DEFINITIONS.C */

void import_definition_structures(void);

/* ---------- prototypes/KEYBOARD_DIALOG.C */
boolean configure_key_setup(short *keycodes);

// LP change: resets field of view to whatever the player had had when reviving
void ResetFieldOfView();

// LP change: modification of Josh Elsasser's dont-switch-weapons patch
bool dont_switch_to_new_weapon();

// LP change: force reload of view context
void ReloadViewContext();

// LP change: added infravision-parser export
XML_ElementParser *Infravision_GetParser();

// LP change: added control-panel-parser export
XML_ElementParser *ControlPanels_GetParser();

#endif

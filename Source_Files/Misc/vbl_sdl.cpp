/*
 *  vbl_sdl.cpp - Input handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL_thread.h>

#include "cseries.h"
#include "FileHandler.h"
#include "resource_manager.h"

#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "interface.h"
#include "player.h"
#include "mouse.h"
#include "key_definitions.h"
#include "tags.h" // for filetypes.
#include "computer_interface.h"

#include "vbl.h"

#include "vbl_definitions.h"


// Constants
#define MAXIMUM_FLAG_PERSISTENCE    15
#define DOUBLE_CLICK_PERSISTENCE    10
#define FILM_RESOURCE_TYPE          FOUR_CHARS_TO_INT('f', 'i', 'l', 'm')

#define NUMBER_OF_SPECIAL_FLAGS (sizeof(special_flags)/sizeof(struct special_flag_data))
static struct special_flag_data special_flags[]=
{
	{_double_flag, _look_dont_turn, _looking_center},
	{_double_flag, _run_dont_walk, _action_trigger_state},
	{_latched_flag, _action_trigger_state},
	{_latched_flag, _cycle_weapons_forward},
	{_latched_flag, _cycle_weapons_backward},
	{_latched_flag, _toggle_map}
};


/*
 *  Get FileDesc for replay, ask user if desired
 */

bool find_replay_to_use(bool ask_user, FileSpecifier &file)
{
	if (ask_user) {
		return file.ReadDialog(_typecode_film);
	} else
		return get_recording_filedesc(file);
}


/*
 *  Get FileDesc for default recording file
 */

bool get_recording_filedesc(FileSpecifier &File)
{
	File.SetToLocalDataDir();
	File.AddPart(getcstr(temporary, strFILENAMES, filenameMARATHON_RECORDING));
	return File.Exists();
}


/*
 *  Save film buffer to user-selected file
 */

void move_replay(void)
{
	// Get source file specification
	FileSpecifier src_file, dst_file;
	if (!get_recording_filedesc(src_file))
		return;

	// Ask user for destination file
	char prompt[256], default_name[256];
	if (!dst_file.WriteDialog(_typecode_film, getcstr(prompt, strPROMPTS, _save_replay_prompt), getcstr(default_name, strFILENAMES, filenameMARATHON_RECORDING)))
		return;

	// Copy file
	dst_file.CopyContents(src_file);
	int error = dst_file.GetError();
	if (error)
		alert_user(infoError, strERRORS, fileError, error);
}


/*
 *  Poll keyboard and return action flags
 */

uint32 parse_keymap(void)
{
	Uint8 *key_map = SDL_GetKeyState(NULL);
	uint32 flags = 0;

	// Parse the keymap
	key_definition *key = current_key_definitions;
	for (int i=0; i<NUMBER_OF_STANDARD_KEY_DEFINITIONS; i++, key++)
		if (key_map[key->offset])
			flags |= key->action_flag;

	// Post-process the keymap
	struct special_flag_data *special = special_flags;
	for (int i=0; i<NUMBER_OF_SPECIAL_FLAGS; i++, special++) {
		if (flags & special->flag) {
			switch (special->type) {
				case _double_flag:
					// If this flag has a double-click flag and has been hit within
					// DOUBLE_CLICK_PERSISTENCE (but not at MAXIMUM_FLAG_PERSISTENCE),
					// mask on the double-click flag */
					if (special->persistence < MAXIMUM_FLAG_PERSISTENCE
					 &&	special->persistence > MAXIMUM_FLAG_PERSISTENCE - DOUBLE_CLICK_PERSISTENCE)
						flags |= special->alternate_flag;
					break;
				
				case _latched_flag:
					// If this flag is latched and still being held down, mask it out
					if (special->persistence == MAXIMUM_FLAG_PERSISTENCE)
						flags &= ~special->flag;
					break;
				
				default:
					assert(false);
					break;
			}
			
			special->persistence = MAXIMUM_FLAG_PERSISTENCE;
		} else
			special->persistence = FLOOR(special->persistence-1, 0);
	}

	// Handle the selected input controller
	if (input_preferences->input_device != _keyboard_or_game_pad) {
		fixed delta_yaw, delta_pitch, delta_velocity;
		mouse_idle(input_preferences->input_device);
		test_mouse(input_preferences->input_device, &flags, &delta_yaw, &delta_pitch, &delta_velocity);
		flags = mask_in_absolute_positioning_information(flags, delta_yaw, delta_pitch, delta_velocity);
	}

	// Modify flags with run/walk and swim/sink
	bool do_interchange =
		(local_player->variables.flags & _HEAD_BELOW_MEDIA_BIT) ?
			(input_preferences->modifiers & _inputmod_interchange_swim_sink) != 0:
			(input_preferences->modifiers & _inputmod_interchange_run_walk) != 0;
	if (do_interchange)
		flags ^= _run_dont_walk;

	if (player_in_terminal_mode(local_player_index))
		flags = build_terminal_action_flags((char *)key_map);

	return flags;
}


/*
 *  Get random demo replay from map
 */

bool setup_replay_from_random_resource(uint32 map_checksum)
{
	// not supported in SDL version
	return false;
}


/*
 *  Set keys to match preferences
 */

void set_keys_to_match_preferences(void)
{
	set_keys(input_preferences->keycodes);
}


/*
 *  Periodic task management
 */

typedef bool (*timer_func)(void);

static uint32 tm_period;		// Ticks between two calls of the timer task
static SDL_Thread *tm_thread;
static volatile bool tm_quit;

static int timer_thread(void *param)
{
	// On Linux, this is more precise than using an SDL timer
	uint32 next = SDL_GetTicks();
	while (!tm_quit) {
		((timer_func)param)();
		next += tm_period;
		int32 delay = next - SDL_GetTicks();
		if (delay > 0)
			SDL_Delay(delay);
		else if (delay < -tm_period)
			next = SDL_GetTicks();
	}
	return 0;
}

timer_task_proc install_timer_task(short tasks_per_second, bool (*func)(void))
{
	// We only handle one task, which is enough
	tm_quit = false;
	tm_period = 1000 / tasks_per_second;
	tm_thread = SDL_CreateThread(timer_thread, (void *)func);
	return (timer_task_proc)tm_thread;
}

void remove_timer_task(timer_task_proc proc)
{
	tm_quit = true;
	SDL_WaitThread(tm_thread, NULL);
}

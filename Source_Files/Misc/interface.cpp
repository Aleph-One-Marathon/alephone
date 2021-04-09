/*
	INTERFACE.C

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

	Thursday, December 30, 1993 6:56:22 PM
	Mac specific code.....

	Friday, July 8, 1994 2:32:44 PM (alain)
		All old code in here is obsolete. This now has interface for the top-level
		interface (Begin Game, etc…)
	Saturday, September 10, 1994 12:45:48 AM  (alain)
		the interface gutted again. just the stuff that handles the menu though, the rest stayed
		the same.
	Thursday, June 8, 1995 2:56:16 PM (ryan)
		Pillaged, raped, & burned. (in that order)

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"
	Surrounded choose_saved_game_to_load with "extern "C""

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb. 9, 2000 (Loren Petrich):
	Changed NUMBER_OF_INTRO_SCREENS to 3
	Changed NUMBER_OF_CREDIT_SCREENS to Hamish Sinclair's favorite number
	
	Fixed multiple-clicks-necessary problem for too few screens.
	Was in next_game_state(); set game_state.phase (countdown value) to zero.
	
Feb 19, 2000 (Loren Petrich):
	Set the single-player color to the player color set in the preferences,
	for the benefit of chase-cam users.

Mar 5, 2000 (Loren Petrich):
	Added reset_screen() when starting a game, so that extravision
	will not be persistent.

May 13, 2000 (Loren Petrich):
	Added Rhys Hill's fix for problems with quitting OpenGL

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 24, 2000 (Loren Petrich):
	Added source selector to calculate_picture_clut(), in order to better deal with
	object-oriented file handlers

Nov 25, 2000 (Loren Petrich):
	Added support for movies that start at any level, including at the end of a game.
	Also added end-screen control.

Jan 31, 2001 (Loren Petrich):
	In pause_game(), will stop the liquid faders that are active
	
Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Disabled network and network microphone calls under Carbon

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Renabled network calls, but not microphone calls under Carbon

May 16, 2002 (Woody Zenfell):
    Enforcing standard player behavior with regard to films and netplay
    
Jun 5, 2002 (Loren Petrich):
	Added do-nothing "case _revert_game:" in portable_process_screen_click()
	at the request of Michael Adams.
        
Feb 1, 2003 (Woody Zenfell):
        Reenabling network microphone support on all platforms, trying to share code
        and present consistent interfaces to the greatest degree practical.
        
Feb 8-12, 2003 (Woody Zenfell):
        Introducing support for generalized game startup (will enable resumption of saved-games
        as netgames, among other things).

Feb 13, 2003 (Woody Zenfell):
        We can now resume games as network games.
*/

// NEED VISIBLE FEEDBACK WHEN APPLETALK IS NOT AVAILABLE!!!

/* ZZZ: more on enforcing standard behavior...
    + Standard behavior forced when playing a network game.
    + Standard behavior forced when replaying a film.
    + Custom behavior allowed when starting or restoring a single-player game.
    + No film recorded in single-player if custom behavior != standard behavior.

    Once films and netplay properly record each player's behavior prefs,
    and the relevant code uses per-player settings, this won't be necessary.
    Try a mass-search for "player_behavior" to find the areas affected.
*/

#include "cseries.h" // sorry ryan, nov. 4
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <algorithm>
#include <sstream>

#ifdef PERFORMANCE
#include <perf.h>

extern TP2PerfGlobals perf_globals;
#endif

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "player.h"
#include "network.h"
#include "screen_drawing.h"
#include "SoundManager.h"
#include "fades.h"
#include "game_window.h"
#include "game_errors.h"
#include "Mixer.h"
#include "Music.h"
#include "images.h"
#include "screen.h"
#include "network.h"
#include "vbl.h"
#include "shell.h"
#include "preferences.h"
#include "FileHandler.h"
#include "lua_script.h" // PostIdle
#include "interface_menus.h"
#include "XML_LevelScript.h"
#include "Music.h"
#include "Movie.h"
#include "QuickSave.h"
#include "Plugins.h"
#include "Statistics.h"
#include "shell_options.h"

#ifdef HAVE_SMPEG
#include <smpeg/smpeg.h>
#endif
#ifdef HAVE_FFMPEG
#include "SDL_ffmpeg.h"
#endif

#include "sdl_dialogs.h"
#include "sdl_widgets.h"
#include "network_dialog_widgets_sdl.h"

/* Change this when marathon changes & replays are no longer valid */
enum recording_version {
	RECORDING_VERSION_UNKNOWN = 0,
	RECORDING_VERSION_MARATHON = 1,
	RECORDING_VERSION_MARATHON_2 = 2,
	RECORDING_VERSION_MARATHON_INFINITY = 3,
	RECORDING_VERSION_ALEPH_ONE_EARLY = 4,
	RECORDING_VERSION_ALEPH_ONE_PRE_NET = 5,
	RECORDING_VERSION_ALEPH_ONE_PRE_PIN = 6,
	RECORDING_VERSION_ALEPH_ONE_1_0 = 7,
	RECORDING_VERSION_ALEPH_ONE_1_1 = 8,
	RECORDING_VERSION_ALEPH_ONE_1_2 = 9,
	RECORDING_VERSION_ALEPH_ONE_1_3 = 10,
	RECORDING_VERSION_ALEPH_ONE_1_4 = 11
};
const short default_recording_version = RECORDING_VERSION_ALEPH_ONE_1_4;
const short max_handled_recording= RECORDING_VERSION_ALEPH_ONE_1_4;

#include "screen_definitions.h"
#include "interface_menus.h"

// LP addition: getting OpenGL rendering stuff
#include "render.h"
#include "OGL_Render.h"
#include "OGL_Blitter.h"
#include "alephversion.h"

// To tell it to stop playing,
// and also to run the end-game script
#include "XML_LevelScript.h"

// Network microphone/speaker
#include "network_sound.h"
#include "network_distribution_types.h"

// ZZZ: should the function that uses these (join_networked_resume_game()) go elsewhere?
#include "wad.h"
#include "game_wad.h"

#include "motion_sensor.h" // for reset_motion_sensor()

#include "lua_hud_script.h"

using alephone::Screen;

/* ------------- enums */

/* ------------- constants */
#define DISPLAY_PICT_RESOURCE_TYPE 'PICT'
#define CLOSE_WITHOUT_WARNING_DELAY (5*TICKS_PER_SECOND)

#define NUMBER_OF_INTRO_SCREENS (3)
#define INTRO_SCREEN_DURATION (215 * MACHINE_TICKS_PER_SECOND / TICKS_PER_SECOND) // fudge to align with sound

#ifdef DEMO
#define INTRO_SCREEN_TO_START_SONG_ON (1)
#else
#define INTRO_SCREEN_TO_START_SONG_ON (0)
#endif

#define INTRO_SCREEN_BETWEEN_DEMO_BASE (INTRO_SCREEN_BASE+1) /* +1 to get past the powercomputing */
#define NUMBER_OF_INTRO_SCREENS_BETWEEN_DEMOS (1)
#define DEMO_INTRO_SCREEN_DURATION (10 * MACHINE_TICKS_PER_SECOND)

#define TICKS_UNTIL_DEMO_STARTS (30 * MACHINE_TICKS_PER_SECOND)

#define NUMBER_OF_PROLOGUE_SCREENS 0
#define PROLOGUE_DURATION (10 * MACHINE_TICKS_PER_SECOND)

#define NUMBER_OF_EPILOGUE_SCREENS 1
#define EPILOGUE_DURATION (INDEFINATE_TIME_DELAY)

#define NUMBER_OF_CREDIT_SCREENS 7
#define CREDIT_SCREEN_DURATION (15 * 60 * MACHINE_TICKS_PER_SECOND)

#define NUMBER_OF_CHAPTER_HEADINGS 0
#define CHAPTER_HEADING_DURATION (7*MACHINE_TICKS_PER_SECOND)

// For exiting the Marathon app
// #if defined(DEBUG) || !defined(DEMO)
#define NUMBER_OF_FINAL_SCREENS 0
// #else
// #define NUMBER_OF_FINAL_SCREENS 1
// #endif
#define FINAL_SCREEN_DURATION (INDEFINATE_TIME_DELAY)

/* For teleportation, end movie, etc. */
#define EPILOGUE_LEVEL_NUMBER 256

/* ------------- structures */
struct game_state {
	short state;
	short flags;
	short user;
	int32 phase;
	int32 last_ticks_on_idle;
	short current_screen;
	bool suppress_background_tasks;
	bool current_netgame_allows_microphone;
	short main_menu_display_count; // how many times have we shown the main menu?
	short highlighted_main_menu_item;
};

struct screen_data {
	short screen_base;
	short screen_count;
	int32 duration;
};

/* -------------- constants */
struct screen_data display_screens[]= {
	{ INTRO_SCREEN_BASE, NUMBER_OF_INTRO_SCREENS, INTRO_SCREEN_DURATION },
	{ MAIN_MENU_BASE, 1, 0 },
	{ CHAPTER_SCREEN_BASE, NUMBER_OF_CHAPTER_HEADINGS, CHAPTER_HEADING_DURATION },
	{ PROLOGUE_SCREEN_BASE, NUMBER_OF_PROLOGUE_SCREENS, PROLOGUE_DURATION },
	{ EPILOGUE_SCREEN_BASE, NUMBER_OF_EPILOGUE_SCREENS, EPILOGUE_DURATION },
	{ CREDIT_SCREEN_BASE, NUMBER_OF_CREDIT_SCREENS, CREDIT_SCREEN_DURATION},
	{ INTRO_SCREEN_BETWEEN_DEMO_BASE, NUMBER_OF_INTRO_SCREENS_BETWEEN_DEMOS, DEMO_INTRO_SCREEN_DURATION },
	{ FINAL_SCREEN_BASE, NUMBER_OF_FINAL_SCREENS, FINAL_SCREEN_DURATION }
};

struct screen_data m1_display_screens[]= {
	{ 1111, 4, INTRO_SCREEN_DURATION },
	{ MAIN_MENU_BASE, 1, 0 },
	{ 10000, NUMBER_OF_CHAPTER_HEADINGS, CHAPTER_HEADING_DURATION },
	{ PROLOGUE_SCREEN_BASE, NUMBER_OF_PROLOGUE_SCREENS, PROLOGUE_DURATION },
	{ EPILOGUE_SCREEN_BASE, NUMBER_OF_EPILOGUE_SCREENS, EPILOGUE_DURATION },
	{ 1000, 1, CREDIT_SCREEN_DURATION},
	{ INTRO_SCREEN_BETWEEN_DEMO_BASE, NUMBER_OF_INTRO_SCREENS_BETWEEN_DEMOS, DEMO_INTRO_SCREEN_DURATION },
	{ FINAL_SCREEN_BASE, NUMBER_OF_FINAL_SCREENS, FINAL_SCREEN_DURATION }
};



/* -------------- local globals */
static struct game_state game_state;
static FileSpecifier DraggedReplayFile;
static bool interface_fade_in_progress= false;
static short interface_fade_type;
static short current_picture_clut_depth;
static struct color_table *animated_color_table= NULL;
static struct color_table *current_picture_clut= NULL;

/* -------------- externs */
extern short interface_bit_depth;
extern short bit_depth;
extern bool shapes_file_is_m1();

/* ----------- prototypes/PREPROCESS_MAP_MAC.C */
extern bool load_game_from_file(FileSpecifier& File, bool run_scripts);
extern bool choose_saved_game_to_load(FileSpecifier& File);

/* ---------------------- prototypes */
static void display_credits(void);
static void draw_button(short index, bool pressed);
static void draw_powered_by_aleph_one();
static void handle_replay(bool last_replay);
static bool begin_game(short user, bool cheat);
static void start_game(short user, bool changing_level);
// LP: "static" removed
void handle_load_game(void);
static void handle_save_film(void);
static void finish_game(bool return_to_main_menu);
static void clean_up_after_failed_game(bool inNetgame, bool inRecording, bool inFullCleanup);
static void handle_network_game(bool gatherer);
static void next_game_screen(void);
static void handle_interface_menu_screen_click(short x, short y, bool cheatkeys_down);

static void display_introduction(void);
static void display_loading_map_error(void);
static void display_quit_screens(void);
static void	display_screen(short base_pict_id);
static void display_introduction_screen_for_demo(void);
static void display_epilogue(void);
static void display_about_dialog();

static void force_system_colors(void);
static bool point_in_rectangle(short x, short y, screen_rectangle *rect);

static void start_interface_fade(short type, struct color_table *original_color_table);
static void update_interface_fades(void);
static void interface_fade_out(short pict_resource_number, bool fade_music);
static bool can_interface_fade_out(void);
static void transfer_to_new_level(short level_number);
static void try_and_display_chapter_screen(short level, bool interface_table_is_valid, bool text_block);

static screen_data *get_screen_data(
	short index);

/* ---------------------- code begins */

screen_data *get_screen_data(
	short index)
{
	assert(index>=0 && index<NUMBER_OF_SCREENS);
	if (shapes_file_is_m1())
		return m1_display_screens+index;
	return display_screens+index;
}

void initialize_game_state(
	void)
{
	game_state.state= _display_intro_screens;
	game_state.user= _single_player;
	game_state.flags= 0;
	game_state.current_screen= 0;
	game_state.suppress_background_tasks= true;
	game_state.main_menu_display_count= 0;

	toggle_menus(false);

	if(shell_options.insecure_lua) {
	  alert_user(expand_app_variables("Insecure Lua has been manually enabled. Malicious Lua scripts can use Insecure Lua to take over your computer. Unless you specifically trust every single Lua script that will be running, you should quit $appName$ IMMEDIATELY.").c_str());
	}

	if (!shell_options.editor)
	{
		if (shell_options.skip_intro)
		{
			display_main_menu();
		}
		else
		{
			display_introduction();
		}
	}
}

void force_game_state_change(
	void)
{
	game_state.phase= 0;
}

bool player_controlling_game(
	void)
{
	bool player_in_control= false;

	if( (game_state.user==_single_player || game_state.user==_network_player) && (game_state.state==_game_in_progress || game_state.state==_switch_demo) )
	{
		player_in_control= true;
	}
	
	return player_in_control;
}

void toggle_suppression_of_background_tasks(
	void)
{
	game_state.suppress_background_tasks= !game_state.suppress_background_tasks;
}

void set_game_state(
	short new_state)
{
	short old_state= game_state.state;

	switch(old_state)
	{
		case _game_in_progress:
			switch(new_state)
			{
				case _display_epilogue:
					game_state.state= _begin_display_of_epilogue;
					game_state.phase= 0;
					break;
					
				case _close_game:
					finish_game(true);
					break;
					
				case _quit_game:
					finish_game(false);
					display_quit_screens();
					break;
					
				case _switch_demo:
					/* Because Alain's code calls us at interrupt level 1, */
					/*  we must defer processing of this message until idle */
					game_state.state= _switch_demo;
					game_state.phase= 0;
					break;
					
				case _revert_game:
					/* Because reverting a game in the middle of the update_world loop sounds */
					/*  sketchy, this is not done until idle time.. */
					game_state.state= new_state;
					game_state.phase= 0;
					break;

				case _change_level:
					game_state.state= new_state;
					game_state.phase= 0;
					break;
					
				default: 
					assert(false);
					break;
			}
			break;

		default:
			game_state.state= new_state;
			break;
	}
}

short get_game_state(
	void)
{
	return game_state.state;
}

bool current_netgame_allows_microphone()
{
	return game_state.current_netgame_allows_microphone;
}

bool suppress_background_events(
	void)
{
	return game_state.suppress_background_tasks;
}

short get_game_controller(
	void)
{
	return game_state.user;
}

void set_change_level_destination(
	short level_number)
{
	assert(game_state.state== _change_level);
	game_state.current_screen= level_number;
}

static short get_difficulty_level(void)
{
	return player_preferences->difficulty_level;
}


// ----- ZZZ start support for generalized game startup -----
// (should this be split out (with some other game startup code) to a new file?)

// In this scheme, a "start" corresponds to a player available at the moment, who will
// participate in the game we're starting up.  In general when this code talks about a
// 'player', it is referring to an already-existing player in the game world (which should
// already be restored or initialized fairly well before these routines are used).
// Generalized game startup will match starts to existing players, set leftover players
// as "zombies" so they exist but just stand there, and create new players to correspond
// with any remaining starts.  As such it can handle both resume-game (some players already
// exist) and new-game (dynamic_world->player_count == 0) operations.

static void construct_single_player_start(player_start_data* outStartArray, short* outStartCount)
{
        if(outStartCount != NULL)
        {
                *outStartCount = 1;
        }

        outStartArray[0].team = player_preferences->color;
        outStartArray[0].color = player_preferences->color;
        outStartArray[0].identifier = 0;
        strncpy(outStartArray[0].name, player_preferences->name, MAXIMUM_PLAYER_START_NAME_LENGTH+1);
				
        set_player_start_doesnt_auto_switch_weapons_status(&outStartArray[0], dont_switch_to_new_weapon());
}

#if !defined(DISABLE_NETWORKING)
static void construct_multiplayer_starts(player_start_data* outStartArray, short* outStartCount)
{
        int number_of_players = NetGetNumberOfPlayers();
        
        if(outStartCount != NULL)
        {
                *outStartCount = number_of_players;
        }
    
        for(int player_index= 0; player_index<number_of_players; ++player_index)
        {
                player_info *player_information = (player_info *)NetGetPlayerData(player_index);
                outStartArray[player_index].team = player_information->team;
                outStartArray[player_index].color= player_information->color;
                outStartArray[player_index].identifier = NetGetPlayerIdentifier(player_index);
                strncpy(outStartArray[player_index].name, player_information->name, MAXIMUM_PLAYER_START_NAME_LENGTH+1);
        }
}
#endif // !defined(DISABLE_NETWORKING)

// This should be safe to use whether starting or resuming and whether single-player or multiplayer.
void match_starts_with_existing_players(player_start_data* ioStartArray, short* ioStartCount)
{
        // This code could be smarter, but it doesn't run very often, doesn't get big data sets, etc.
        // so I'm not going to worry about it.
        
        bool startAssigned[MAXIMUM_NUMBER_OF_PLAYERS];
        int8 startAssignedToPlayer[MAXIMUM_NUMBER_OF_PLAYERS];
        for(int i = 0; i < MAXIMUM_NUMBER_OF_PLAYERS; i++)
        {
                startAssigned[i] = false;
                startAssignedToPlayer[i] = NONE;
        }
        
        // First, match starts to players by name.
        for(int s = 0; s < *ioStartCount; s++)
        {
                for(int p = 0; p < dynamic_world->player_count; p++)
                {
                        if(startAssignedToPlayer[p] == NONE)
                        {
                                if(strcmp(ioStartArray[s].name, get_player_data(p)->name) == 0)
                                {
                                        startAssignedToPlayer[p] = s;
                                        startAssigned[s] = true;
                                        break;
                                }
                        }
                }
        }
        
        // Match remaining starts to remaining players arbitrarily.
        for(int s = 0; s < *ioStartCount; s++)
        {
                if(!startAssigned[s])
                {
                        for(int p = 0; p < dynamic_world->player_count; p++)
                        {
                                if(startAssignedToPlayer[p] == NONE)
                                {
                                        startAssignedToPlayer[p] = s;
                                        startAssigned[s] = true;
                                        break;
                                }
                        }
                }
        }
        
        // Create new starts for any players not covered.
        int p = 0;        
        while(*ioStartCount < dynamic_world->player_count)
        {
                if(startAssignedToPlayer[p] == NONE)
                {
                        player_data* thePlayer = get_player_data(p);
                        ioStartArray[*ioStartCount].team = thePlayer->team;
                        ioStartArray[*ioStartCount].color = thePlayer->color;
                        ioStartArray[*ioStartCount].identifier = NONE;
                        strncpy(ioStartArray[*ioStartCount].name, thePlayer->name, MAXIMUM_PLAYER_START_NAME_LENGTH+1);
                        startAssignedToPlayer[p] = *ioStartCount;
                        startAssigned[*ioStartCount] = true;
                        (*ioStartCount)++;
                }
                
                p++;
        }
        
        // Assign remaining starts to players that don't exist yet
        p = dynamic_world->player_count;
        for(int s = 0; s < *ioStartCount; s++)
        {
                if(!startAssigned[s])
                {
                        startAssignedToPlayer[p] = s;
                        startAssigned[s] = true;
                        p++;
                }
        }
        
        // Reorder starts to match players - this is particularly unclever
        player_start_data theOriginalStarts[MAXIMUM_NUMBER_OF_PLAYERS];
        memcpy(theOriginalStarts, ioStartArray, sizeof(theOriginalStarts));
        for(p = 0; p < *ioStartCount; p++)
        {
                ioStartArray[p] = theOriginalStarts[startAssignedToPlayer[p]];
        }
}

// This should be safe to use whether starting or resuming, and whether single- or multiplayer.
static void synchronize_players_with_starts(const player_start_data* inStartArray, short inStartCount, short inLocalPlayerIndex)
{
        assert(inLocalPlayerIndex >= 0 && inLocalPlayerIndex < inStartCount);
        
        // s will walk through all the starts
        int s = 0;
        
        // First we process existing players
        for( ; s < dynamic_world->player_count; s++)
        {
                player_data* thePlayer = get_player_data(s);

                if(inStartArray[s].identifier == NONE)
                {
                        // No start (live player) to go with this player (stored player)
                        SET_PLAYER_ZOMBIE_STATUS(thePlayer, true);
                }
                else
                {
                        // Update player's appearance to match the start
                        thePlayer->team = inStartArray[s].team;
                        thePlayer->color = inStartArray[s].color;
                        thePlayer->identifier = player_identifier_value(inStartArray[s].identifier);
                        strncpy(thePlayer->name, inStartArray[s].name, MAXIMUM_PLAYER_NAME_LENGTH+1);

                        SET_PLAYER_DOESNT_AUTO_SWITCH_WEAPONS_STATUS(thePlayer,
                            player_identifier_doesnt_auto_switch_weapons(inStartArray[s].identifier));

                        // Make sure if player was saved as zombie, they're not now.
                        SET_PLAYER_ZOMBIE_STATUS(thePlayer, false);
                }
        }
        
        // Designate the local player if they already exist
        if (inLocalPlayerIndex < s)
        {
            set_local_player_index(inLocalPlayerIndex);
            set_current_player_index(inLocalPlayerIndex);
        }
        
        // If there are any starts left, we need new players for them
        for( ; s < inStartCount; s++)
        {
                new_player_flags flags = (s == inLocalPlayerIndex ? new_player_make_local_and_current : 0);
                int theIndex = new_player(inStartArray[s].team, inStartArray[s].color, inStartArray[s].identifier, flags);
                assert(theIndex == s);
                player_data* thePlayer = get_player_data(theIndex);
                strncpy(thePlayer->name, inStartArray[s].name, MAXIMUM_PLAYER_NAME_LENGTH+1);
        }
}

static short find_start_for_identifier(const player_start_data* inStartArray, short inStartCount, short _inIdentifier)
{
        short inIdentifier= player_identifier_value(_inIdentifier);
        short s;
        for(s = 0; s < inStartCount; s++)
        {
                if(player_identifier_value(inStartArray[s].identifier) == inIdentifier)
                {
                        break;
                }
        }
        
        return (s == inStartCount) ? NONE : s;
}

// The single-player machine, gatherer, and joiners all will use this routine.  It should take most of its
// cues from the "extras" that load_game_from_file() does.
static bool make_restored_game_relevant(bool inNetgame, const player_start_data* inStartArray, short inStartCount)
{
        game_is_networked = inNetgame;
        
        // set_random_seed() needs to happen before synchronize_players_with_starts()
        // since the latter calls new_player() which almost certainly uses global_random().
        // Note we always take the random seed directly from the dynamic_world, no need to screw around
        // with copying it from game_information or the like.
        set_random_seed(dynamic_world->random_seed);
        
        short theLocalPlayerIndex;
        
#if !defined(DISABLE_NETWORKING)
        // Much of the code in this if()...else is very similar to code in begin_game(), should probably try to share.
        if(inNetgame)
        {
                game_info *network_game_info= (game_info *)NetGetGameData();
                
                dynamic_world->game_information.game_time_remaining= network_game_info->time_limit;
                dynamic_world->game_information.kill_limit= network_game_info->kill_limit;
                dynamic_world->game_information.game_type= network_game_info->net_game_type;
                dynamic_world->game_information.game_options= network_game_info->game_options;
                dynamic_world->game_information.initial_random_seed= network_game_info->initial_random_seed;
                dynamic_world->game_information.difficulty_level= network_game_info->difficulty_level;
                dynamic_world->game_information.cheat_flags= network_game_info->cheat_flags;

                if (network_game_info->allow_mic)
                {
                        install_network_microphone();
                        game_state.current_netgame_allows_microphone= true;
                } else {
                        game_state.current_netgame_allows_microphone= false;
                }

                // ZZZ: until players specify their behavior modifiers over the network,
                // to avoid out-of-sync we must force them all the same.
                standardize_player_behavior_modifiers();
                
                theLocalPlayerIndex = NetGetLocalPlayerIndex();
        }
        else
#endif // !defined(DISABLE_NETWORKING)
        {
                dynamic_world->game_information.difficulty_level= get_difficulty_level();
                restore_custom_player_behavior_modifiers();
                theLocalPlayerIndex = find_start_for_identifier(inStartArray, inStartCount, 0);
        }
        
        assert(theLocalPlayerIndex != NONE);

        synchronize_players_with_starts(inStartArray, inStartCount, theLocalPlayerIndex);
        
        bool success = entering_map(true /*restoring game*/);

        reset_motion_sensor(theLocalPlayerIndex);

        if(!success) clean_up_after_failed_game(inNetgame, false /*recording*/, true /*full cleanup*/);

        return success;
}

// ZZZ end generalized game startup support -----

#if !defined(DISABLE_NETWORKING)
// ZZZ: this will get called (eventually) shortly after NetUpdateJoinState() returns netStartingResumeGame
bool join_networked_resume_game()
{
        bool success = true;
        
        // Get the saved-game data
        byte* theSavedGameFlatData = NetReceiveGameData(false /* do_physics */);
        if(theSavedGameFlatData == NULL)
        {
                success = false;
        }
        
        if(success)
        {
                // Use the saved-game data
                wad_header theWadHeader;
                wad_data* theWad;
                
                theWad = inflate_flat_data(theSavedGameFlatData, &theWadHeader);
                if(theWad == NULL)
                {
                        success = false;
                        free(theSavedGameFlatData);
                }
                
				bool found_map = false;
                if(success)
                {
					ResetLevelScript();
					uint32 theParentChecksum = theWadHeader.parent_checksum;
					found_map = use_map_file(theParentChecksum);

					if (found_map) {
						dynamic_data dynamic_data_wad;
						get_dynamic_data_from_wad(theWad, &dynamic_data_wad);
						RunLevelScript(dynamic_data_wad.current_level_number);
					}

                    success = process_map_wad(theWad, true /* resuming */, theWadHeader.data_version);
                    free_wad(theWad); /* Note that the flat data points into the wad. */
                    // ZZZ: maybe this is what the Bungie comment meant, but apparently
                    // free_wad() somehow (voodoo) frees theSavedGameFlatData as well.
                }
                
                if(success)
                {
                        Plugins::instance()->set_mode(Plugins::kMode_Net);
                        Crosshairs_SetActive(player_preferences->crosshairs_active);
                        LoadHUDLua();
                        RunLuaHUDScript();

                        // try to locate the Map file for the saved-game, so that (1) we have a crack
                        // at continuing the game if the original gatherer disappears, and (2) we can
                        // save the game on our own machine and continue it properly (as part of a bigger scenario) later.
                        
                        if(found_map)
                        {
                                // LP: getting the level scripting off of the map file
                                // Being careful to carry over errors so that Pfhortran errors can be ignored
                                short SavedType, SavedError = get_game_error(&SavedType);
								LoadStatsLua();
                                set_game_error(SavedType,SavedError);
                        }
                        else
                        {
                                /* Tell the user they’re screwed when they try to leave this level. */
                                // ZZZ: should really issue a different warning since the ramifications are different
                                alert_user(infoError, strERRORS, cantFindMap, 0);
        
                                // LP addition: makes the game look normal
                                hide_cursor();
                        
                                /* Set to the default map. */
                                set_to_default_map();
				
				LoadStatsLua();
                        }
                        
                        // set the revert-game info to defaults (for full-auto saving on the local machine)
                        set_saved_game_name_to_default();
                        
                        player_start_data theStarts[MAXIMUM_NUMBER_OF_PLAYERS];
                        short theStartCount;
                        construct_multiplayer_starts(theStarts, &theStartCount);
                        
			RunLuaScript();
                        success = make_restored_game_relevant(true /* multiplayer */, theStarts, theStartCount);
                }
        }
        
        if(success)
	{
		Music::instance()->PreloadLevelMusic();
		start_game(_network_player, false /*changing level?*/);
	}
        
        return success;
}
#endif // !defined(DISABLE_NETWORKING)

extern bool load_and_start_game(FileSpecifier& File);

// ZZZ: changes to use generalized game startup support
// This will be used only on the machine that picked "Continue Saved Game".
bool load_and_start_game(FileSpecifier& File)
{
	bool success;

	hide_cursor();
	if (can_interface_fade_out()) 
	{
		interface_fade_out(MAIN_MENU_BASE, true);
	}

	success= load_game_from_file(File, false);

	if (!success)
	{
		/* Reset the system colors, since the screen clut is all black.. */
		force_system_colors();
		show_cursor(); // JTP: Was hidden by force system colors
		display_loading_map_error();
	}

	bool userWantsMultiplayer;	
	size_t theResult = UNONE;

	if (success)
	{
		theResult = should_restore_game_networked(File);
	}

	if (theResult == UNONE)
	{
		// cancelled
		success = false;
	}
	else
	{
		userWantsMultiplayer = (theResult != 0);
	}
        
	if (success)
	{
#if !defined(DISABLE_NETWORKING)
		if (userWantsMultiplayer)
		{
			set_game_state(_displaying_network_game_dialogs);
			success = network_gather(true /*resuming*/);
		}
#endif // !defined(DISABLE_NETWORKING)
                
		if (success)
		{
			Plugins::instance()->set_mode(userWantsMultiplayer ? Plugins::kMode_Net : Plugins::kMode_Solo);
			Crosshairs_SetActive(player_preferences->crosshairs_active);
			LoadHUDLua();
			RunLuaHUDScript();
			
			// load the scripts we put off before
			short SavedType, SavedError = get_game_error(&SavedType);
			if(!userWantsMultiplayer)
			{
				LoadSoloLua();
			}
			LoadStatsLua();
			set_game_error(SavedType,SavedError);
			
			player_start_data theStarts[MAXIMUM_NUMBER_OF_PLAYERS];
			short theNumberOfStarts;
        
#if !defined(DISABLE_NETWORKING)
			if (userWantsMultiplayer)
			{
				construct_multiplayer_starts(theStarts, &theNumberOfStarts);
			}
			else
#endif // !defined(DISABLE_NETWORKING)
			{
				construct_single_player_start(theStarts, &theNumberOfStarts);
			}
                        
			match_starts_with_existing_players(theStarts, &theNumberOfStarts);
                        
#if !defined(DISABLE_NETWORKING)
			if (userWantsMultiplayer)
			{
				NetSetupTopologyFromStarts(theStarts, theNumberOfStarts);
				success = NetStart();
				if (success)
				{
					byte* theSavedGameFlatData = (byte*)get_flat_data(File, false /* union wad? */, 0 /* level # */);
					if (theSavedGameFlatData == NULL)
					{
						success = false;
					}

					if (success)
					{
						int32 theSavedGameFlatDataLength = get_flat_data_length(theSavedGameFlatData);
						OSErr theError = NetDistributeGameDataToAllPlayers(theSavedGameFlatData, theSavedGameFlatDataLength, false /* do_physics? */);
						if (theError != noErr)
						{
							success = false;
						}
						free(theSavedGameFlatData);
					}
				}
			}
#endif // !defined(DISABLE_NETWORKING)
                        
			if (success)
			{
				RunLuaScript();
				success = make_restored_game_relevant(userWantsMultiplayer, theStarts, theNumberOfStarts);
				if (success)
				{
					Music::instance()->PreloadLevelMusic();
					start_game(userWantsMultiplayer ? _network_player : _single_player, false);
				}
			}
		}
	}
        
	if (!success) {
		/* We failed.  Balance the cursor */
		/* Should this also force the system colors or something? */
		show_cursor();
	}

	return success;
}

extern bool handle_open_replay(FileSpecifier& File);

bool handle_open_replay(FileSpecifier& File)
{
	DraggedReplayFile = File;
	
	bool success;
	
	force_system_colors();
	success= begin_game(_replay_from_file, false);
	if(!success) display_main_menu();
	return success;
}

bool handle_edit_map()
{
	bool success;

	force_system_colors();
	success = begin_game(_single_player, false);
	if (!success) display_main_menu();
	return success;
}

// Called from within update_world..
bool check_level_change(
	void)
{
	bool level_changed= false;

	if(game_state.state==_change_level)
	{
		transfer_to_new_level(game_state.current_screen);
		level_changed= true;
	}
	
	return level_changed;
}

void pause_game(
	void)
{
	stop_fade();
	if (!OGL_IsActive() || !(TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fader)))
		set_fade_effect(NONE);
	darken_world_window();
	set_keyboard_controller_status(false);
	show_cursor();
}

void resume_game(
	void)
{
	hide_cursor();
	if (!OGL_IsActive() || !(TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fader)))
		SetFadeEffectDelay(TICKS_PER_SECOND/2);
	if (OGL_IsActive())
		OGL_Blitter::BoundScreen(true);
	validate_world_window();
	set_keyboard_controller_status(true);
}

void draw_menu_button_for_command(
	short index)
{
	short rectangle_index= index-1+START_OF_MENU_INTERFACE_RECTS;

	assert(get_game_state()==_display_main_menu);
	
	/* Draw it initially depressed.. */
	draw_button(rectangle_index, true);
	sleep_for_machine_ticks(MACHINE_TICKS_PER_SECOND / 12);
	draw_button(rectangle_index, false);
}

void update_interface_display(
	void)
{
	struct screen_data *data;

	data= get_screen_data(game_state.state);
	
	/* Use this to avoid the fade.. */
	draw_full_screen_pict_resource_from_images(data->screen_base+game_state.current_screen);

	if (game_state.state == _display_main_menu)
	{
		draw_powered_by_aleph_one();
		if (game_state.highlighted_main_menu_item >= 0)
		{
			draw_button(game_state.highlighted_main_menu_item + START_OF_MENU_INTERFACE_RECTS - 1, true);
		}
	}
}

bool idle_game_state(uint32 time)
{
	static float last_heartbeat_fraction = -1.f;
	int machine_ticks_elapsed = time - game_state.last_ticks_on_idle;

	if(machine_ticks_elapsed || game_state.phase==0)
	{
		if(game_state.phase != INDEFINATE_TIME_DELAY)
		{
			game_state.phase-= machine_ticks_elapsed;
		}
		
		/* Note that we still go through this if we have an indefinate phase.. */
		if(game_state.phase<=0)
		{
			switch(get_game_state())
			{
				case _display_quit_screens:
				case _display_intro_screens:
				case _display_prologue:
				case _display_epilogue:
				case _display_credits:
					next_game_screen();
					break;

				case _display_intro_screens_for_demo:
				case _display_main_menu:
					/* Start the demo.. */
					if(!begin_game(_demo, false))
					{
						/* This means that there was not a valid demo to play */
						game_state.phase= TICKS_UNTIL_DEMO_STARTS;
					}
					break;

				case _close_game:
					display_main_menu();
					break;

				case _switch_demo:
					/* This is deferred to the idle task because it */
					/*  occurs at interrupt time.. */
					switch(game_state.user)
					{
						case _replay:
							finish_game(true);
							break;
							
						case _demo:
							finish_game(false);
							display_introduction_screen_for_demo();
							break;
							
						default: 
							assert(false);
							break;
					}
					break;
				
				case _display_chapter_heading:
					dprintf("Chapter heading...");
					break;

				case _quit_game:
					/* About to quit, but can still hit this through order of ops.. */
					break;

				case _revert_game:
					/* Reverting while in the update loop sounds sketchy.. */
					if(revert_game())
					{
						game_state.state= _game_in_progress;
						game_state.phase = 15 * MACHINE_TICKS_PER_SECOND;
						game_state.last_ticks_on_idle= machine_tick_count();
						update_interface(NONE);
					} else {
						/* Give them the error... */
						display_loading_map_error();
						
						/* And finish their current game.. */
						finish_game(true);
					}
					break;
					
				case _begin_display_of_epilogue:
					finish_game(false);
					display_epilogue();
					break;

				case _game_in_progress:
					game_state.phase = 15 * MACHINE_TICKS_PER_SECOND;
					//game_state.last_ticks_on_idle= machine_tick_count();
					break;

				case _change_level:
				case _displaying_network_game_dialogs:
					break;
					
				default:
					assert(false);
					break;
			}
		}
		game_state.last_ticks_on_idle= machine_tick_count();
	}

	/* if we’re not paused and there’s something to draw (i.e., anything different from
		last time), render a frame */
	if(game_state.state==_game_in_progress)
	{
		// ZZZ change: update_world() whether or not get_keyboard_controller_status() is true
		// This way we won't fill up queues and stall netgames if one player switches out for a bit.
		std::pair<bool, int16> theUpdateResult= update_world();
		short ticks_elapsed= theUpdateResult.second;

		if (get_keyboard_controller_status())
		{
			// ZZZ: I don't know for sure that render_screen works best with the number of _real_
			// ticks elapsed rather than the number of (potentially predictive) ticks elapsed.
			// This is a guess.
			auto heartbeat_fraction = get_heartbeat_fraction();
			if (theUpdateResult.first || (last_heartbeat_fraction != -1 && last_heartbeat_fraction != heartbeat_fraction)) {
				last_heartbeat_fraction = heartbeat_fraction;
				render_screen(ticks_elapsed);
			}
		}
		
		return theUpdateResult.first;
	} else {
		/* Update the fade ins, etc.. */
		update_interface_fades();
		return false;
	}
}

extern SDL_Surface *draw_surface;	// from screen_drawing.cpp
//void draw_intro_screen(void);		// from screen.cpp

static SDL_Surface *powered_by_alephone_surface = 0;
#include "powered_by_alephone.h"

extern void set_about_alephone_rect(int width, int height);

static void draw_powered_by_aleph_one()
{
	if (!powered_by_alephone_surface)
	{
		SDL_RWops *rw = SDL_RWFromConstMem(powered_by_alephone_bmp, sizeof(powered_by_alephone_bmp));
		powered_by_alephone_surface = SDL_LoadBMP_RW(rw, 0);
		SDL_FreeRW(rw);

		set_about_alephone_rect(powered_by_alephone_surface->w, powered_by_alephone_surface->h);
	}

	SDL_Rect rect;
	rect.x = 640 - powered_by_alephone_surface->w;
	rect.y = 480 - powered_by_alephone_surface->h;
	rect.w = powered_by_alephone_surface->w;
	rect.h = powered_by_alephone_surface->h;
	
	_set_port_to_intro();
	SDL_BlitSurface(powered_by_alephone_surface, NULL, draw_surface, &rect);
	_restore_port();

	// have to reblit :(
	draw_intro_screen();
}

void display_main_menu(
	void)
{
	game_state.state= _display_main_menu;
	game_state.current_screen= 0;
	game_state.phase= TICKS_UNTIL_DEMO_STARTS;
	game_state.last_ticks_on_idle= machine_tick_count();
	game_state.user= _single_player;
	game_state.flags= 0;
	game_state.highlighted_main_menu_item= -1;
	
	Plugins::instance()->set_mode(Plugins::kMode_Menu);
	change_screen_mode(_screentype_menu);
	display_screen(MAIN_MENU_BASE);
	
	/* Start up the song! */
	if(!Music::instance()->Playing() && game_state.main_menu_display_count==0)
	{
		Music::instance()->RestartIntroMusic();
	}

	draw_powered_by_aleph_one();

	game_state.main_menu_display_count++;
}


// Kludge for Carbon/Classic -- when exiting a main-menu dialog box, redisplay 
static void ForceRepaintMenuDisplay()
{
}


void do_menu_item_command(
	short menu_id,
	short menu_item,
	bool cheat)
{
	switch(menu_id)
	{
            
		case mGame:
			switch(menu_item)
			{
				case iPause:
					switch(game_state.user)
					{
						case _single_player:
						case _replay:
							if (get_keyboard_controller_status())
							{
							  pause_game();
							}
							else
							{	
								resume_game();
							}
							break;
						
						case _demo:
							finish_game(true);
							break;
							
						case _network_player:
							break;
							
						default:
							assert(false);
							break;
					}
					break;

				case iSave:
					switch(game_state.user)
					{
						case _single_player:
#if 0
							save_game();
							validate_world_window();
#endif
							break;
						
						case _demo:
						case _replay:
							finish_game(true);
							break;
							
						case _network_player:
							break;
							
						default:
							assert(false);
							break;
					}
					break;
					
				case iRevert:
					/* Not implemented.. */
					break;
					
				case iCloseGame:
				case iQuitGame:
					{
						bool really_wants_to_quit= false;
					
						switch(game_state.user)
						{
							case _single_player:
								if(PLAYER_IS_DEAD(local_player) || 
								   dynamic_world->tick_count-local_player->ticks_at_last_successful_save<CLOSE_WITHOUT_WARNING_DELAY || shell_options.output.size())
								{
									really_wants_to_quit= true;
								} else {
									pause_game();
									show_cursor();
									really_wants_to_quit= quit_without_saving();
									hide_cursor();
									resume_game();
								}
								break;
							
							case _demo:
							case _replay:
							case _network_player:
								really_wants_to_quit= true;
								break;
								
							default:
								assert(false);
								break;
						}
	
						if(really_wants_to_quit)
						{
							// Rhys Hill fix for crash when quitting OpenGL
							if (!OGL_IsActive())
								render_screen(0); /* Get rid of hole.. */
							set_game_state(_close_game);
						}
					}
					break;
					
				default:
					assert(false);
					break;
			}
			break;
			
		case mInterface:
			switch(menu_item)
			{
				case iNewGame:
					begin_game(_single_player, cheat);
					ForceRepaintMenuDisplay();
					break;
				case iPlaySingletonLevel:
					begin_game(_single_player,2);
					break;

				case iJoinGame:
					handle_network_game(false);
					break;
		
				case iGatherGame:
					handle_network_game(true);
					break;
					
				case iLoadGame:
					handle_load_game();
					break;
		
				case iReplayLastFilm:
				case iReplaySavedFilm:
					handle_replay(menu_item==iReplayLastFilm);
					break;
					
				case iCredits:
					display_credits();
					break;
					
				case iPreferences:
					do_preferences();
					game_state.phase= TICKS_UNTIL_DEMO_STARTS;
					game_state.last_ticks_on_idle= machine_tick_count();
					ForceRepaintMenuDisplay();
					break;
					
				case iCenterButton:
					SoundManager::instance()->PlaySound(Sound_Center_Button(), 0, NONE);
					break;
					
				case iSaveLastFilm:
					handle_save_film();
					break;
		
				case iQuit:
					display_quit_screens();
					break;
				case iAbout:
					display_about_dialog();
					game_state.phase= TICKS_UNTIL_DEMO_STARTS;
					game_state.last_ticks_on_idle= machine_tick_count();
					break;
		
				default:
					assert(false);
					break;
			}
			break;

		default:
			assert(false);
			break;
	}
}

void portable_process_screen_click(
	short x,
	short y,
	bool cheatkeys_down)
{
	switch(get_game_state())
	{
		case _game_in_progress:
		case _begin_display_of_epilogue:
		case _change_level:
		case _displaying_network_game_dialogs:
		case _quit_game:
		case _close_game:
		case _switch_demo:
		case _revert_game:
			break;

		case _display_intro_screens_for_demo:
			/* Get out of user mode. */
			display_main_menu();
			break;

		case _display_quit_screens:
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
			/* Force the state change next time through.. */
			force_game_state_change();
			break;

		case _display_main_menu:
			handle_interface_menu_screen_click(x, y, cheatkeys_down);
			break;
		
		default:
			assert(false);
			break;
	}
}

void process_main_menu_highlight_advance(bool reverse)
{
	if (get_game_state() != _display_main_menu)
		return;
	
	int old_button = game_state.highlighted_main_menu_item;
	
	// iterate through M2/Moo order
	int item_order[] = {
		iNewGame, iLoadGame, iGatherGame, iJoinGame,
		iReplaySavedFilm, iReplayLastFilm, iSaveLastFilm,
		iPreferences, iQuit, iCredits, iAbout };
	int num_items = sizeof(item_order)/sizeof(int);
	
	if (game_state.highlighted_main_menu_item == -1) {
		game_state.highlighted_main_menu_item = reverse ? item_order[0] : item_order[num_items - 1];
	}
	do {
		int cur_idx = 0;
		for (int i = 0; i < num_items; ++i) {
			if (item_order[i] == game_state.highlighted_main_menu_item) {
				cur_idx = i;
				break;
			}
		}
		int next_idx = (cur_idx + num_items + (reverse ? -1 : 1)) % num_items;
		game_state.highlighted_main_menu_item = item_order[next_idx];
	} while (!enabled_item(game_state.highlighted_main_menu_item));
	
	if (old_button != -1)
		draw_button(old_button + START_OF_MENU_INTERFACE_RECTS - 1, false);
	draw_button(game_state.highlighted_main_menu_item + START_OF_MENU_INTERFACE_RECTS - 1, true);
}

void process_main_menu_highlight_select(bool cheatkeys_down)
{
	if (get_game_state() != _display_main_menu)
		return;
	if (game_state.highlighted_main_menu_item == -1)
		return;
	if (!enabled_item(game_state.highlighted_main_menu_item))
		return;
	do_menu_item_command(mInterface, game_state.highlighted_main_menu_item, cheatkeys_down);
	
}

bool enabled_item(
	short item)
{
	bool enabled= true;

	switch(item)
	{
		case iNewGame:
		case iLoadGame:
		case iPlaySingletonLevel:
		case iPreferences:
		case iReplaySavedFilm:
		case iCredits:
		case iQuit:
	        case iAbout:
		case iCenterButton:
			break;
			
		case iReplayLastFilm:
		case iSaveLastFilm:
			enabled= has_recording_file();
			break;

		case iGatherGame:
		case iJoinGame:
			enabled= networking_available();
			break;
			
		default:
			assert(false);
			break;
	}
	
	return enabled;
}

void paint_window_black(
	void)
{
	_set_port_to_screen_window();
	clear_screen(true);
	_restore_port();
	
	_set_port_to_intro();
	SDL_FillRect(draw_surface, NULL, SDL_MapRGB(draw_surface->format, 0, 0, 0));
	_restore_port();
}

static LoadedResource SoundRsrc;

/* --------------------- static code */

static void display_introduction(
	void)
{
	struct screen_data *screen_data= get_screen_data(_display_intro_screens);

	paint_window_black();
	game_state.state= _display_intro_screens;
	game_state.current_screen= 0;
	if (screen_data->screen_count)
	{
		if (game_state.state==_display_intro_screens && game_state.current_screen==INTRO_SCREEN_TO_START_SONG_ON)
		{
			Music::instance()->RestartIntroMusic();
		}

		game_state.phase= screen_data->duration;
		game_state.last_ticks_on_idle= machine_tick_count();
		display_screen(screen_data->screen_base);

		Mixer::instance()->StopSoundResource();
		SoundRsrc.Unload();
		if (get_sound_resource_from_images(screen_data->screen_base, SoundRsrc))
		{
			Mixer::instance()->PlaySoundResource(SoundRsrc);
		}
	}
	else
	{
		display_main_menu();
	}
}

static void display_introduction_screen_for_demo(	
	void)
{
	struct screen_data *screen_data= get_screen_data(_display_intro_screens_for_demo);

	game_state.state= _display_intro_screens_for_demo;
	game_state.current_screen= 0;
	if(screen_data->screen_count)
	{
		game_state.phase= screen_data->duration;
		game_state.last_ticks_on_idle= machine_tick_count();
		display_screen(screen_data->screen_base);
	} else {
		display_main_menu();
	}
}

static void display_epilogue(
	void)
{
	Music::instance()->RestartIntroMusic();
	
	{
		int32 ticks= machine_tick_count();
		
		do
		{
			Music::instance()->Idle();
		}
		while (machine_tick_count()-ticks<10);
	}

	game_state.state= _display_epilogue;
	game_state.phase= 0;
	game_state.current_screen= 0;
	game_state.last_ticks_on_idle= machine_tick_count();
	
	hide_cursor();
	// Setting of the end-screen parameters has been moved to XML_LevelScript.cpp
	int end_offset = EndScreenIndex;
	int end_count = NumEndScreens;
	if (shapes_file_is_m1()) // should check map, but this is easier
	{
		// ignore M2 defaults set in LoadLevelScripts()
		end_offset = 100;
		end_count = 2;
	}
	for (int i=0; i<end_count; i++)
		try_and_display_chapter_screen(end_offset+i, true, true);
	show_cursor();
}

class w_authors_list : public w_string_list
{
public:
	w_authors_list(const vector<string>& items, dialog* d) :
		w_string_list(items, d, 0) {}

	void item_selected(void) { }
};

static void display_about_dialog()
{
	force_system_colors();

	dialog d;

	tab_placer* tabs = new tab_placer();

	vertical_placer* placer = new vertical_placer;
	std::vector<std::string> labels;
	labels.push_back("ABOUT");
	labels.push_back("AUTHORS");
	w_tab *tab_w = new w_tab(labels, tabs);
	
	placer->dual_add(new w_title("ALEPH ONE"), d);
	placer->add(new w_spacer, true);

	placer->dual_add(tab_w, d);
	placer->add(new w_spacer, true);

	vertical_placer* about_placer = new vertical_placer;
	
	if (strcmp(get_application_name().c_str(), "Aleph One") != 0)
	{
		about_placer->dual_add(new w_static_text(expand_app_variables("$appName$ is powered by").c_str()), d);
	}
	about_placer->dual_add(new w_static_text(expand_app_variables("Aleph One $appVersion$ ($appDate$)").c_str()), d);

	about_placer->add(new w_spacer, true);

	about_placer->dual_add(new w_hyperlink(A1_HOMEPAGE_URL), d);

	about_placer->add(new w_spacer(2 * get_theme_space(SPACER_WIDGET)), true);
	
	about_placer->dual_add(new w_static_text(expand_app_variables("Aleph One is free software with ABSOLUTELY NO WARRANTY.").c_str()), d);
	about_placer->dual_add(new w_static_text("You are welcome to redistribute it under certain conditions."), d);
	about_placer->dual_add(new w_hyperlink("http://www.gnu.org/licenses/gpl-3.0.html"), d);

	about_placer->add(new w_spacer, true);

	about_placer->dual_add(new w_static_text("This license does not apply to game content."), d);

	about_placer->add(new w_spacer, true);

	about_placer->dual_add(new w_static_text(expand_app_variables("Scenario loaded: $scenarioName$ $scenarioVersion$").c_str()), d);

	vertical_placer *authors_placer = new vertical_placer();
	
	authors_placer->dual_add(new w_static_text("Aleph One is based on the source code for Marathon 2 and"), d);
	authors_placer->dual_add(new w_static_text("Marathon Infinity, which was developed by Bungie software."), d);
	authors_placer->add(new w_spacer, true);
	
	authors_placer->dual_add(new w_static_text("The enhancements and extensions to Marathon 2 and Marathon"), d);
	authors_placer->dual_add(new w_static_text("Infinity that constitute Aleph One have been made by:"), d);

	authors_placer->add(new w_spacer, true);

	std::vector<std::string> authors;
	authors.push_back("Joey Adams");
	authors.push_back("Michael Adams (mdmkolbe)");
	authors.push_back("Falko Axmann");
	authors.push_back("Christian Bauer");
	authors.push_back("Mike Benonis");
	authors.push_back("Steven Bytnar");
	authors.push_back("Glen Ditchfield");
	authors.push_back("Will Dyson");
	authors.push_back("Carl Gherardi");
	authors.push_back("Thomas Herzog");
	authors.push_back("Chris Hallock (LidMop)");
	authors.push_back("Benoît Hauquier (Kolfering)");
	authors.push_back("Peter Hessler");
	authors.push_back("Matthew Hielscher");
	authors.push_back("Rhys Hill");
	authors.push_back("Alan Jenkins");
	authors.push_back("Richard Jenkins (Solra Bizna)");
	authors.push_back("Jeremy, the MSVC guy");
	authors.push_back("Mark Levin");
	authors.push_back("Bo Lindbergh");
	authors.push_back("Chris Lovell");
	authors.push_back("Jesse Luehrs");
	authors.push_back("Marshall (darealshinji)");
	authors.push_back("Derek Moeller");
	authors.push_back("Jeremiah Morris");
	authors.push_back("Sam Morris");
	authors.push_back("Benoit Nadeau (Benad)");
	authors.push_back("Mihai Parparita");
	authors.push_back("Jeremy Parsons (brefin)");
	authors.push_back("Eric Peterson");
	authors.push_back("Loren Petrich");
	authors.push_back("Ian Pitcher");
	authors.push_back("Chris Pruett");
	authors.push_back("Matthew Reda");
	authors.push_back("Ian Rickard");
	authors.push_back("Etienne Samson (tiennou)");
	authors.push_back("Catherine Seppanen");
	authors.push_back("Gregory Smith (treellama)");
	authors.push_back("Scott Smith (pickle136)");
	authors.push_back("Wolfgang Sourdeau");
	authors.push_back("Peter Stirling");
	authors.push_back("Alexander Strange (mrvacbob)");
	authors.push_back("Alexei Svitkine");
	authors.push_back("Ben Thompson");
	authors.push_back("TrajansRow");
	authors.push_back("Clemens Unterkofler (hogdotmac)");
	authors.push_back("James Willson");
	authors.push_back("Woody Zenfell III");

	w_authors_list *authors_w = new w_authors_list(authors, &d);
	authors_placer->dual_add(authors_w, d);

	tabs->add(about_placer, true);
	tabs->add(authors_placer, true);

	placer->add(tabs, true);
	
	placer->add(new w_spacer, true);

	placer->dual_add(new w_button("OK", dialog_ok, &d), d);
	
	d.set_widget_placer(placer);

	clear_screen();

	d.run();

	display_main_menu();
}

static void display_credits(
	void)
{
	if (NUMBER_OF_CREDIT_SCREENS)
	{
		struct screen_data *screen_data= get_screen_data(_display_credits);
		
		game_state.state= _display_credits;
		game_state.current_screen= 0;
		game_state.user= _single_player;
		game_state.flags= 0;

		game_state.phase= screen_data->duration;
		game_state.last_ticks_on_idle= machine_tick_count();
		display_screen(screen_data->screen_base);
	}
}

static void display_quit_screens(
	void)
{
	struct screen_data *screen_data= get_screen_data(_display_quit_screens);

	if(screen_data->screen_count)
	{
		game_state.state= _display_quit_screens;
		game_state.current_screen= 0;
		game_state.user= _single_player;
		game_state.flags= 0;
		game_state.phase= screen_data->duration;
		game_state.last_ticks_on_idle= machine_tick_count();
		
		display_screen(screen_data->screen_base);
	} else {
		StatsManager::instance()->Finish();
		/* No screens. */
		game_state.state= _quit_game;
		game_state.phase= 0;
	}
}

static void transfer_to_new_level(
	short level_number)
{
	struct entry_point entry;
	bool success= true;
	
	entry.level_number= level_number;

#if !defined(DISABLE_NETWORKING)
	/* Only can transfer if NetUnSync returns true */
	if(game_is_networked) 
	{
		if(NetUnSync()) 
		{
			success= true;
		} else {
			set_game_error(gameError, errUnsyncOnLevelChange);
			success= false;
		}
	}
#endif // !defined(DISABLE_NETWORKING)

	if(success)
	{
		stop_fade();
		set_fade_effect(NONE);
		Music::instance()->StopLevelMusic();
//		if(OGL_IsActive())
		{
			exit_screen();
			// Enter_screen will be called again in start_game
		}
		set_keyboard_controller_status(false);
		FindLevelMovie(entry.level_number);
		show_movie(entry.level_number);

		// if this is the EPILOGUE_LEVEL_NUMBER, then it is time to get
		// out of here already (as we've just played the epilogue movie,
		// we can move on to the _display_epilogue game state)
		if (level_number == (shapes_file_is_m1() ? 100 : EPILOGUE_LEVEL_NUMBER)) {
			finish_game(false);
			show_cursor(); // for some reason, cursor stays hidden otherwise
			set_game_state(_begin_display_of_epilogue);
			force_game_state_change();
			return;
		}

		if (!game_is_networked) try_and_display_chapter_screen(level_number, true, false);
		success= goto_level(&entry, false, dynamic_world->player_count);
		set_keyboard_controller_status(true);
	}
	
	if(success)
	{
		start_game(game_state.user, true);
	} else {
		display_loading_map_error();
		finish_game(true);
	}
}

/* The port is set.. */
static void draw_button(
	short index, 
	bool pressed)
{
	if (index == _about_alephone_rect) return;

	screen_rectangle *screen_rect= get_interface_rectangle(index);
	short pict_resource_number= MAIN_MENU_BASE + pressed;

	set_drawing_clip_rectangle(screen_rect->top, screen_rect->left, screen_rect->bottom, screen_rect->right);
	
	/* Use this to avoid the fade.. */
	draw_full_screen_pict_resource_from_images(pict_resource_number);

	set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}
					
static void handle_replay( /* This is gross. */
	bool last_replay)
{
	bool success;
	
	if(!last_replay) force_system_colors();
	success= begin_game(_replay, !last_replay);
	if(!success) display_main_menu();
}

// ZZZ: some modifications to use generalized game-startup
static bool begin_game(
	short user,
	bool cheat)
{
	struct entry_point entry;
	struct player_start_data starts[MAXIMUM_NUMBER_OF_PLAYERS];
	struct game_data game_information;
	short number_of_players;
	bool success= true;
	bool is_networked= false;
	bool clean_up_on_failure= true;
	bool record_game= false;
	uint32 parent_checksum = 0;

	clear_game_error();
	objlist_clear(starts, MAXIMUM_NUMBER_OF_PLAYERS);
	
	switch(user)
	{
		case _network_player:
#if !defined(DISABLE_NETWORKING)
			{
				game_info *network_game_info= (game_info *)NetGetGameData();

				construct_multiplayer_starts(starts, &number_of_players);

				game_information.game_time_remaining= network_game_info->time_limit;
				game_information.kill_limit= network_game_info->kill_limit;
				game_information.game_type= network_game_info->net_game_type;
				game_information.game_options= network_game_info->game_options;
				game_information.initial_random_seed= network_game_info->initial_random_seed;
				game_information.difficulty_level= network_game_info->difficulty_level;
				parent_checksum = network_game_info->parent_checksum;
				entry.level_number = network_game_info->level_number;
				entry.level_name[0] = 0;
	
				if (network_game_info->allow_mic)
				{
					install_network_microphone();
					game_state.current_netgame_allows_microphone= true;
				} else {
					game_state.current_netgame_allows_microphone= false;
				}
				game_information.cheat_flags = network_game_info->cheat_flags;
				std::fill_n(game_information.parameters, 2, 0);

				is_networked= true;
				record_game= true;
				// ZZZ: until players specify their behavior modifiers over the network,
				// to avoid out-of-sync we must force them all the same.
				standardize_player_behavior_modifiers();
			}
#endif // !defined(DISABLE_NETWORKING)
			break;

		case _replay_from_file:
		case _replay:
		case _demo:
			switch(user)
			{
				case _replay:
					{
						FileSpecifier ReplayFile;
						show_cursor(); // JTP: Hidden one way or another :p
						
						bool prompt_to_export = false;
#ifndef MAC_APP_STORE
						SDL_Keymod m = SDL_GetModState();
						if ((m & KMOD_ALT) || (m & KMOD_GUI)) prompt_to_export = true;
#endif
						
						success= find_replay_to_use(cheat, ReplayFile);
						if(success)
						{
							if(!get_map_file().Exists())
							{
								set_game_error(systemError, ENOENT);
								display_loading_map_error();
								success= false;
							}
							else
							{
								success= setup_for_replay_from_file(ReplayFile, get_current_map_checksum(), prompt_to_export);

								hide_cursor();
							}
						}
					} 
					break;
					
				case _demo:
					// setup_replay_from_random_resource always returns false,
					// so don't bother to checksum the map
					success= false;
					// success= setup_replay_from_random_resource(get_current_map_checksum());
					break;

				case _replay_from_file:
					success= setup_for_replay_from_file(DraggedReplayFile, get_current_map_checksum());
					user= _replay;
					break;
					
				default:
					assert(false);
					break;
			}
			
			if(success)
			{
				uint32 unused1;
				short recording_version;
			
				get_recording_header_data(&number_of_players, 
					&entry.level_number, &unused1, &recording_version,
					starts, &game_information);

				if(recording_version > max_handled_recording)
				{
					stop_replay();
					alert_user(infoError, strERRORS, replayVersionTooNew, 0);
					success= false;
				}
				else
				{
					switch (recording_version)
					{
					case RECORDING_VERSION_MARATHON_2:
						load_film_profile(FILM_PROFILE_MARATHON_2);
						break;
					case RECORDING_VERSION_MARATHON_INFINITY:
						load_film_profile(FILM_PROFILE_MARATHON_INFINITY);
						break;
					case RECORDING_VERSION_ALEPH_ONE_1_0:
						load_film_profile(FILM_PROFILE_ALEPH_ONE_1_0);
						break;
					case RECORDING_VERSION_ALEPH_ONE_1_1:
						load_film_profile(FILM_PROFILE_ALEPH_ONE_1_1);
						break;
					case RECORDING_VERSION_ALEPH_ONE_1_2:
						load_film_profile(FILM_PROFILE_ALEPH_ONE_1_2);
						break;
					case RECORDING_VERSION_ALEPH_ONE_1_3:
						load_film_profile(FILM_PROFILE_ALEPH_ONE_1_3);
						break;
					case RECORDING_VERSION_ALEPH_ONE_1_4:
						load_film_profile(FILM_PROFILE_DEFAULT);
						break;
					default:
						load_film_profile(environment_preferences->film_profile);
						break;
					}

					entry.level_name[0] = 0;
					game_information.game_options |= _overhead_map_is_omniscient;
					record_game= false;
					// ZZZ: until films store behavior modifiers, we must require
					// that they record and playback only with standard modifiers.
					standardize_player_behavior_modifiers();
				}
			}
			break;
			
		case _single_player:
			if(cheat)
			{
				entry.level_number= get_level_number_from_user();
				if(entry.level_number==NONE) success= false; /* Cancelled */
			} else {
				entry.level_number= 0;
			}
	
			// ZZZ: let the user use his behavior modifiers in single-player.
			restore_custom_player_behavior_modifiers();
			
			entry.level_name[0] = 0;
			starts[0].identifier = 0;
                        //AS: make things clearer
                        memset(entry.level_name,0,66);

                        construct_single_player_start(starts, &number_of_players);

			game_information.game_time_remaining= INT32_MAX;
			game_information.kill_limit = 0;
			game_information.game_type= _game_of_kill_monsters;
			game_information.game_options= _burn_items_on_death|_ammo_replenishes|_weapons_replenish|_monsters_replenish;
			game_information.initial_random_seed= machine_tick_count();
			game_information.difficulty_level= get_difficulty_level();
			std::fill_n(game_information.parameters, 2, 0);
				
                        // ZZZ: until film files store player behavior flags, we must require
                        // that all films recorded be made with standard behavior.
			record_game= is_player_behavior_standard();
			
            break;
			
		default:
			assert(false);
			break;
	}

	if(success)
	{
		if(record_game)
		{
			if(!get_map_file().Exists())
			{
				set_game_error(systemError, ENOENT);
				display_loading_map_error();
				success= false;
			}
			else
			{
				set_recording_header_data(number_of_players, entry.level_number, (user == _network_player) ? parent_checksum : get_current_map_checksum(), 
					default_recording_version, starts, &game_information);
				start_recording();
			}
		}
	}
	if(success)
	{
		hide_cursor();
		/* This has already been done to get to gather/join */
		if(can_interface_fade_out()) 
		{
			interface_fade_out(MAIN_MENU_BASE, true);
		}

		/* Try to display the first chapter screen.. */
		if (user != _network_player && user != _demo)
		{
			FindLevelMovie(entry.level_number);
			show_movie(entry.level_number);
			try_and_display_chapter_screen(entry.level_number, false, false);
		}

		Plugins::instance()->set_mode(number_of_players > 1 ? Plugins::kMode_Net : Plugins::kMode_Solo);
		Crosshairs_SetActive(player_preferences->crosshairs_active);
		LoadHUDLua();
		RunLuaHUDScript();
		
		/* Begin the game! */
		success= new_game(number_of_players, is_networked, &game_information, starts, &entry);
		if(success)
		{
			start_game(user, false);
		} else {
			clean_up_after_failed_game(user == _network_player, record_game, clean_up_on_failure);
		}
	} else {
		/* This means that some weird replay problem happened: */
		/*  1) User cancelled */
		/*  2) Demos not present */
		/*  3) Error... */
		/* Either way, we eat them.. */
	}
	
	return success;
}

static void start_game(
	short user,
	bool changing_level)
{
	/* Change our menus.. */
	toggle_menus(true);
	
	// LP change: reset screen so that extravision will not be persistent
	reset_screen();
	
	enter_screen();
	if (!changing_level)
		L_Call_HUDInit();
	
	// LP: this is in case we are starting underneath a liquid
	if (!OGL_IsActive() || !(TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fader)))
	{
		set_fade_effect(NONE);
		SetFadeEffectDelay(TICKS_PER_SECOND/2);
	}

	// Screen should already be black! 
	validate_world_window();
	
	draw_interface();

#ifdef PERFORMANCE	
	PerfControl(perf_globals, true);
#endif

	// ZZZ: If it's a netgame, we want prediction; else no.
	set_prediction_wanted(user == _network_player);

	game_state.state= _game_in_progress;
	game_state.current_screen= 0;
	game_state.phase = MACHINE_TICKS_PER_SECOND;
	game_state.last_ticks_on_idle= machine_tick_count();
	game_state.user= user;
	game_state.flags= 0;

	assert((!changing_level&&!get_keyboard_controller_status()) || (changing_level && get_keyboard_controller_status()));
	if(!changing_level)
	{
		set_keyboard_controller_status(true);
	} 
}

// LP: "static" removed
void handle_load_game(
	void)
{
	FileSpecifier FileToLoad;
	bool success= false;

	force_system_colors();
	show_cursor(); // JTP: Was hidden by force system colors
	if(choose_saved_game_to_load(FileToLoad))
	{
		if(load_and_start_game(FileToLoad))
                {
			success= true;
		}
	}

	if(!success)
	{
		hide_cursor(); // JTP: Will be shown when fade stops
		display_main_menu();
	}
}

extern bool current_net_game_has_scores();

static void finish_game(
	bool return_to_main_menu)
{
	set_keyboard_controller_status(false);

#ifdef PERFORMANCE	
	PerfControl(perf_globals, false);
#endif
	/* Note that we have to deal with the switch demo state later because */
	/* Alain's code calls us at interrupt level 1. (so we defer it) */
	assert(game_state.state==_game_in_progress || game_state.state==_switch_demo || game_state.state==_revert_game || game_state.state==_change_level || game_state.state==_begin_display_of_epilogue);
	toggle_menus(false);

	stop_fade();
	set_fade_effect(NONE);
	L_Call_HUDCleanup();
	exit_screen();

	/* Stop the replay */
	switch(game_state.user)
	{
		case _single_player:
		case _network_player:
			stop_recording();
			break;
			
		case _demo:
		case _replay:
			stop_replay();
			break;

		default:
			vhalt(csprintf(temporary, "What is user %d?", game_state.user));
			break;
	}
	Movie::instance()->StopRecording();

	if (shell_options.editor && shell_options.output.size())
	{
		FileSpecifier file(shell_options.output);
		if (export_level(file))
		{
			exit(0);
		}
		else
		{
			exit(-1);
		}
	}

	/* Fade out! (Pray) */ // should be interface_color_table for valkyrie, but doesn't work.
	Music::instance()->ClearLevelMusic();
	Music::instance()->FadeOut(MACHINE_TICKS_PER_SECOND / 2);
	full_fade(_cinematic_fade_out, interface_color_table);
	paint_window_black();
	full_fade(_end_cinematic_fade_out, interface_color_table);

	show_cursor();

	leaving_map();
	CloseLuaHUDScript();
	
	// LP: stop playing the background music if it was present
	Music::instance()->StopLevelMusic();
	
	/* Get as much memory back as we can. */
	free_and_unlock_memory(); // this could call free_map.. 
	unload_all_collections();
	SoundManager::instance()->UnloadAllSounds();
	
#if !defined(DISABLE_NETWORKING)
	if (game_state.user==_network_player)
	{
		if(game_state.current_netgame_allows_microphone)
		{
			remove_network_microphone();
		}
		NetUnSync(); // gracefully exit from the game

		/* Don't update the screen, etc.. */
		game_state.state= _displaying_network_game_dialogs;

		change_screen_mode(_screentype_menu);
		force_system_colors();
		display_net_game_stats();
		exit_networking();
	} 
	else
#endif // !defined(DISABLE_NETWORKING)
	if (game_state.user == _replay && !(dynamic_world->game_information.game_type == _game_of_kill_monsters && dynamic_world->player_count == 1))
	{
		game_state.state = _displaying_network_game_dialogs;

		force_system_colors();
		display_net_game_stats();
	}
	
	set_local_player_index(NONE);
	set_current_player_index(NONE);
	
	load_environment_from_preferences();
	if (game_state.user == _replay || game_state.user == _demo)
	{
		Plugins::instance()->set_mode(Plugins::kMode_Menu);
		load_film_profile(FILM_PROFILE_DEFAULT);
	}
	if(return_to_main_menu) display_main_menu();
}

static void clean_up_after_failed_game(bool inNetgame, bool inRecording, bool inFullCleanup)
{
        /* Stop recording.. */
        if(inRecording)
        {
                stop_recording();
        }
        
        set_local_player_index(NONE);
        set_current_player_index(NONE);

        /* Show the cursor here on failure. */
        show_cursor();
        
        /* The only time we don't clean up is on the replays.. */
        if(inFullCleanup)
        {
                if (inNetgame)
                {
#if !defined(DISABLE_NETWORKING)
                        if(game_state.current_netgame_allows_microphone)
                        {
                                remove_network_microphone();
                        }
                        exit_networking();
#endif // !defined(DISABLE_NETWORKING)
                } else {
/* NOTE: The network code is now responsible for displaying its own errors!!!! */
                        /* Give them the error... */
                        display_loading_map_error();
                }

                /* Display the main menu on failure.... */
                display_main_menu();
        }
        set_game_error(systemError, errNone);
}

static void handle_network_game(
	bool gatherer)
{
#if !defined(DISABLE_NETWORKING)
	bool successful_gather = false;
	bool joined_resume_game = false;

	force_system_colors();

	/* Don't update the screen, etc.. */
	game_state.state= _displaying_network_game_dialogs;
	if(gatherer)
	{
		successful_gather= network_gather(false);
		if (successful_gather) successful_gather= NetStart();
	} else {
		int theNetworkJoinResult= network_join();
		if (theNetworkJoinResult == kNetworkJoinedNewGame || theNetworkJoinResult == kNetworkJoinedResumeGame) successful_gather= true;
		if (theNetworkJoinResult == kNetworkJoinedResumeGame) joined_resume_game= true;
	}
	
	if (successful_gather)
	{
		if (joined_resume_game)
		{
			if (join_networked_resume_game() == false) clean_up_after_failed_game(true /*netgame*/, false /*recording*/, true /*full cleanup*/);
		}
		else
		{
			begin_game(_network_player, false);
		}
	} else {
		/* We must restore the colors on cancel. */
		display_main_menu();
	}
#else // !defined(DISABLE_NETWORKING)
	alert_user(infoError, strERRORS, networkNotSupportedForDemo, 0);
#endif // !defined(DISABLE_NETWORKING)
}

static void handle_save_film(
	void)
{
	force_system_colors();
	show_cursor(); // JTP: Hidden by force_system_colors
	move_replay();
	hide_cursor(); // JTP: Will be shown by display_main_menu
	display_main_menu();
}

static void next_game_screen(
	void)
{
	struct screen_data *data= get_screen_data(game_state.state);

	stop_interface_fade();
	if(++game_state.current_screen>=data->screen_count)
	{
		switch(game_state.state)
		{
			case _display_main_menu:
				/* Whoops.  didn't get it. */
				alert_out_of_memory();
				break;
				
			case _display_quit_screens:
				StatsManager::instance()->Finish();

				/* Fade out.. */
				interface_fade_out(data->screen_base+game_state.current_screen, true);
				game_state.state= _quit_game;
				break;
				
			default:
				display_main_menu();
				break;
		}
	} else {
		if(game_state.state==_display_intro_screens && 
			game_state.current_screen==INTRO_SCREEN_TO_START_SONG_ON)
		{
			Music::instance()->RestartIntroMusic();
		}
		// LP addition: check to see if a picture exists before drawing it.
		// Otherwise, set the countdown value to zero.
		short pict_resource_number= data->screen_base+game_state.current_screen;
		if (images_picture_exists(pict_resource_number))
		{
			game_state.phase= data->duration;
			game_state.last_ticks_on_idle= machine_tick_count();
			display_screen(data->screen_base);
			if (game_state.state == _display_intro_screens)
			{
				Mixer::instance()->StopSoundResource();
				SoundRsrc.Unload();
				if (get_sound_resource_from_images(pict_resource_number, SoundRsrc))
				{
					_fixed pitch = (shapes_file_is_m1() && game_state.state==_display_intro_screens) ? _m1_high_frequency : _normal_frequency;
					Mixer::instance()->PlaySoundResource(SoundRsrc, pitch);
				}
			}
		}
		else
		{
			game_state.phase= 0;
			game_state.last_ticks_on_idle= machine_tick_count();
		}
	}
}

static void display_loading_map_error(	
	void)
{
	short error, type;
	
	/* Give them the error... */
	error= get_game_error(&type);
	if(type==gameError)
	{
		short string_id;
		
		switch(error)
		{
			case errServerDied:
				string_id= serverQuitInCooperativeNetGame;
				break;
			case errUnsyncOnLevelChange:
				string_id= unableToGracefullyChangeLevelsNet;
				break;
			
			case errMapFileNotSet:
			case errIndexOutOfRange:
			case errTooManyOpenFiles:
			case errUnknownWadVersion:
			case errWadIndexOutOfRange:
			default:
				string_id= badReadMapGameError;
				break;
		}
		alert_user(infoError, strERRORS, string_id, error);
	} else {
		alert_user(infoError, strERRORS, badReadMapSystemError, error);
	}
	set_game_error(systemError, errNone);
}

static void force_system_colors(
	void)
{
	if(can_interface_fade_out())
	{
		interface_fade_out(MAIN_MENU_BASE, true);
	}

	if(interface_bit_depth==8)
	{
		struct color_table *system_colors= build_8bit_system_color_table();

		assert_world_color_table(system_colors, (struct color_table *) NULL);

		delete system_colors;
	}
}

static void display_screen(
	short base_pict_id)
{
	short pict_resource_number= base_pict_id+game_state.current_screen;
	static bool picture_drawn= false;
	
	if (images_picture_exists(pict_resource_number))
	{
		stop_interface_fade();

		if(current_picture_clut)
		{
			interface_fade_out(pict_resource_number, false);
		}

		assert(!current_picture_clut);
		current_picture_clut= calculate_picture_clut(CLUTSource_Images,pict_resource_number);
		current_picture_clut_depth= interface_bit_depth;

		if(current_picture_clut)
		{
			/* slam the entire clut to black, now. */
			if (interface_bit_depth==8) 
			{
				assert_world_color_table(current_picture_clut, (struct color_table *) NULL);
			}

			full_fade(_start_cinematic_fade_in, current_picture_clut);

			draw_full_screen_pict_resource_from_images(pict_resource_number);
			picture_drawn= true;

			assert(current_picture_clut);	
			start_interface_fade(_long_cinematic_fade_in, current_picture_clut);
		}
	}
	
	if(!picture_drawn)
	{
dprintf("Didn't draw: %d;g", pict_resource_number);
		/* Go for the next one.. */
		next_game_screen();
	}
}

static bool point_in_rectangle(
	short x,
	short y,
	screen_rectangle *rect)
{
	bool in_rectangle= false;

	if(x>=rect->left && x<rect->right && y>=rect->top && y<rect->bottom)
	{
		in_rectangle= true;
	}

	return in_rectangle;
}

static void handle_interface_menu_screen_click(
	short x,
	short y,
	bool cheatkeys_down)
{
	short index;
	screen_rectangle *screen_rect;
	short xoffset = 0, yoffset = 0;

	/* find it.. */
	for(index= START_OF_MENU_INTERFACE_RECTS; index<END_OF_MENU_INTERFACE_RECTS; ++index)
	{
		screen_rect= get_interface_rectangle(index);
		if (point_in_rectangle(x - xoffset, y - yoffset, screen_rect))
			break;
	}
	
	/* we found one.. */
	if(index!=END_OF_MENU_INTERFACE_RECTS)
	{
		if(enabled_item(index-START_OF_MENU_INTERFACE_RECTS+1))
		{
			bool last_state= true;

			stop_interface_fade();

			screen_rect= get_interface_rectangle(index);

			/* Draw it initially depressed.. */
			draw_button(index, last_state);
		
			bool mouse_down = true;
			while (mouse_down)
			{
				int mx = x, my = y;
				bool mouse_changed = false;
				
				SDL_Event e;
				if (SDL_PollEvent(&e))
				{
					switch (e.type)
					{
						case SDL_MOUSEBUTTONUP:
							mx = e.button.x;
							my = e.button.y;
							mouse_changed = true;
							mouse_down = false;
							break;
						case SDL_MOUSEMOTION:
							mx = e.motion.x;
							my = e.motion.y;
							mouse_changed = true;
							break;
					}
				}
				else
				{
					SDL_Delay(10);
				}
				if (mouse_changed)
				{
					alephone::Screen::instance()->window_to_screen(mx, my);
					bool state = point_in_rectangle(mx - xoffset, my - yoffset, screen_rect);
					if (state != last_state)
					{
						draw_button(index, state);
						last_state = state;
					}
				}
			}

			/* Draw it unpressed.. */
			draw_button(index, false);
			
			if(last_state)
			{
				do_menu_item_command(mInterface, index-START_OF_MENU_INTERFACE_RECTS+1, cheatkeys_down);
			}	
		}
	}
}

/* Note that this is modal. This sucks... */
static void try_and_display_chapter_screen(
	short level,
	bool interface_table_is_valid,
	bool text_block)
{
	if (Movie::instance()->IsRecording())
		return;
	
	short pict_resource_number = get_screen_data(_display_chapter_heading)->screen_base + level;
	/* If the picture exists... */
	if (scenario_picture_exists(pict_resource_number))
	{
		short existing_state= game_state.state;
		game_state.state= _display_chapter_heading;
		free_and_unlock_memory();
		
		/* This will NOT work if the initial level entered has a chapter screen, which is why */
		/*  we perform this check. (The interface_color_table is not valid...) */
		if(interface_table_is_valid)
		{
			full_fade(_cinematic_fade_out, interface_color_table);
			paint_window_black();
		}

		change_screen_mode(_screentype_chapter);
		
		/* Fade the screen to black.. */
		assert(!current_picture_clut);
		current_picture_clut= calculate_picture_clut(CLUTSource_Scenario,pict_resource_number);
		current_picture_clut_depth= interface_bit_depth;
		
		if (current_picture_clut)
		{
			LoadedResource SoundRsrc;

			/* slam the entire clut to black, now. */
			if (interface_bit_depth==8) 
			{
				assert_world_color_table(current_picture_clut, (struct color_table *) NULL);
			}
			full_fade(_start_cinematic_fade_in, current_picture_clut);

			/* Draw the picture */
			draw_full_screen_pict_resource_from_scenario(pict_resource_number);

			if (get_sound_resource_from_scenario(pict_resource_number,SoundRsrc))
			{
				_fixed pitch = (shapes_file_is_m1() && level == 101) ? _m1_high_frequency : _normal_frequency;
				Mixer::instance()->PlaySoundResource(SoundRsrc, pitch);
			}
			
			/* Fade in.... */
			assert(current_picture_clut);	
			full_fade(_long_cinematic_fade_in, current_picture_clut);
			
			scroll_full_screen_pict_resource_from_scenario(pict_resource_number, text_block);

			wait_for_click_or_keypress(text_block ? -1 : 10*MACHINE_TICKS_PER_SECOND);
			
			/* Fade out! (Pray) */
			interface_fade_out(pict_resource_number, false);
			
			Mixer::instance()->StopSoundResource();
		}
		game_state.state= existing_state;
	}
}


#if !defined(DISABLE_NETWORKING)
/*
 *  Network microphone handling
 */

void install_network_microphone(
	void)
{
	open_network_speaker();
	NetAddDistributionFunction(kNewNetworkAudioDistributionTypeID, received_network_audio_proc, true);
	open_network_microphone();
}

void remove_network_microphone(
	void)
{
	close_network_microphone();
	NetRemoveDistributionFunction(kNewNetworkAudioDistributionTypeID);
	close_network_speaker();
}
#endif // !defined(DISABLE_NETWORKING)


/* ------------ interface fade code */
/* Be aware that we could try to change bit depths before a fade is completed. */
static void start_interface_fade(
	short type,
	struct color_table *original_color_table)
{
	hide_cursor();
	assert(!interface_fade_in_progress);
	animated_color_table= new color_table;
	obj_copy(*animated_color_table, *original_color_table);

	if(animated_color_table)
	{
		interface_fade_in_progress= true;
		interface_fade_type= type;

		explicit_start_fade(type, original_color_table, animated_color_table);
	}
}

static void update_interface_fades(
	void)
{
	bool still_fading= false;
	
	if(interface_fade_in_progress)
	{
		still_fading= update_fades();
		if(!still_fading)
		{
			stop_interface_fade();
		}
	}
}

void stop_interface_fade(
	void)
{
	if(interface_fade_in_progress)
	{
		stop_fade();
		interface_fade_in_progress= false;
		
		assert(animated_color_table);
		delete animated_color_table;

		if (interface_bit_depth==8) 
		{
			assert_world_color_table(current_picture_clut, (struct color_table *) NULL);
		}

		if(game_state.state==_display_main_menu)
		{
			/* This isn't a showcursor because of balancing problems (first time through..) */
			show_cursor();
		}
	}
}

/* Called right before we start a game.. */
void interface_fade_out(
	short pict_resource_number,
	bool fade_music)
{
	assert(current_picture_clut);
	if(current_picture_clut)
	{
		struct color_table *fadeout_animated_color_table;

		/* We have to check this because they could go into preferences and change on us, */
		/*  the evil swine. */
		if(current_picture_clut_depth != interface_bit_depth)
		{
			delete current_picture_clut;
			current_picture_clut= calculate_picture_clut(CLUTSource_Images,pict_resource_number);
			current_picture_clut_depth= interface_bit_depth;
		}
		
		hide_cursor();
			
		fadeout_animated_color_table= new color_table;
		obj_copy(*fadeout_animated_color_table, *current_picture_clut);

		if(fade_music) 
			Music::instance()->FadeOut(MACHINE_TICKS_PER_SECOND/2);
		if (fadeout_animated_color_table)
		{
			explicit_start_fade(_cinematic_fade_out, current_picture_clut, fadeout_animated_color_table);
			while (update_fades()) 
				Music::instance()->Idle();

			/* Oops.  Founda  memory leak.. */
			delete fadeout_animated_color_table;
		}
		
		if(fade_music) 
		{
			Mixer::instance()->StopSoundResource();
			while(Music::instance()->Playing()) 
				Music::instance()->Idle();

			/* and give up the memory */
			Music::instance()->Pause();
		}

		paint_window_black();
		full_fade(_end_cinematic_fade_out, current_picture_clut);

		/* Hopefully we can do this here.. */
		delete current_picture_clut;
		current_picture_clut= NULL;
	}
}

static bool can_interface_fade_out(
	void)
{
	return (current_picture_clut==NULL) ? false : true;
}

bool interface_fade_finished(
	void)
{
	return fade_finished();
}


void do_preferences(void)
{
	struct screen_mode_data mode = graphics_preferences->screen_mode;

	force_system_colors();
	handle_preferences();

	if (mode.bit_depth != graphics_preferences->screen_mode.bit_depth) {
		paint_window_black();
		Screen::instance()->Initialize(&graphics_preferences->screen_mode);

		/* Re fade in, so that we get the proper colortable loaded.. */
		display_main_menu();
	} else if (memcmp(&mode, &graphics_preferences->screen_mode, sizeof(struct screen_mode_data)))
		change_screen_mode(&graphics_preferences->screen_mode, false);
}


/*
 *  Toggle system hotkeys
 */

void toggle_menus(bool game_started)
{
	// nothing to do
}


/*
 *  Update game window
 */

void update_game_window(void)
{
	switch(get_game_state()) {
		case _game_in_progress:
			update_screen_window();
			break;
			
		case _display_quit_screens:
		case _display_intro_screens_for_demo:
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
		case _display_main_menu:
			update_interface_display();
			break;
			
		default:
			break;
	}
}


/*
 *  Exit networking
 */

void exit_networking(void)
{
#if !defined(DISABLE_NETWORKING)
	NetExit();
#endif // !defined(DISABLE_NETWORKING)
}


/*
 *  Show movie
 */

#ifdef HAVE_OPENGL
#if defined(HAVE_FFMPEG) || defined(HAVE_SMPEG)
static OGL_Blitter show_movie_blitter;
#endif
#ifdef HAVE_SMPEG
static SDL_mutex *show_movie_mutex = NULL;

// SMPEG callback for use under OpenGL
static void show_movie_frame(SDL_Surface* frame, int x, int y, unsigned int w, unsigned int h)
{
	if (show_movie_mutex && SDL_LockMutex(show_movie_mutex) != -1)
	{
		if (!show_movie_blitter.Loaded())
		{
			show_movie_blitter.Load(*frame);
			// let main thread know there's a frame ready
			SDL_Event ev;
			ev.type = SDL_USEREVENT;
			SDL_PushEvent(&ev);
		}
		SDL_UnlockMutex(show_movie_mutex);
	}
}
#endif
#endif
#ifdef HAVE_FFMPEG
static SDL_mutex *movie_audio_mutex = NULL;
static const int AUDIO_BUF_SIZE = 10;
static SDL_ffmpegAudioFrame *aframes[AUDIO_BUF_SIZE];
static uint64_t movie_sync = 0;
void movie_audio_callback(void *data, Uint8 *stream, int length)
{
	if (movie_audio_mutex && SDL_LockMutex(movie_audio_mutex) != -1)
	{
		if (aframes[0]->size == length)
		{
			movie_sync = aframes[0]->pts;
			memcpy(stream, aframes[0]->buffer, aframes[0]->size);
			aframes[0]->size = 0;
			
			SDL_ffmpegAudioFrame *f = aframes[0];
			for (int i = 1; i < AUDIO_BUF_SIZE; i++)
				aframes[i - 1] = aframes[i];
			aframes[AUDIO_BUF_SIZE - 1] = f;
		}
		else
		{
			memset(stream, 0, length);
		}
		SDL_UnlockMutex(movie_audio_mutex);
	}
}
#endif

extern bool option_nosound;

void show_movie(short index)
{
	if (Movie::instance()->IsRecording())
		return;
	
#if defined(HAVE_FFMPEG) || defined(HAVE_SMPEG)
	float PlaybackSize = 2;
	
	FileSpecifier IntroMovie;
	FileSpecifier *File = GetLevelMovie(PlaybackSize);

	if (!File && index == 0)
	{
		if (IntroMovie.SetNameWithPath(getcstr(temporary, strFILENAMES, filenameMOVIE)))
			File = &IntroMovie;
	}

	if (!File) return;

	change_screen_mode(_screentype_chapter);

	{
		SoundManager::Pause pauseSoundManager;
		SDL_Rect dst_rect = { 0, 0, 640, 480 };

#ifdef HAVE_FFMPEG
		SDL_ffmpegFile *sffile = SDL_ffmpegOpen(File->GetPath());
		if (!sffile)
			return;
		
		SDL_ffmpegSelectVideoStream(sffile, 0);
		SDL_ffmpegStream *vstream = SDL_ffmpegGetVideoStream(sffile, 0);
		
		SDL_ffmpegSelectAudioStream(sffile, 0);
		SDL_ffmpegStream *astream = SDL_ffmpegGetAudioStream(sffile, 0);
		
		SDL_ffmpegVideoFrame *vframe = vstream ? SDL_ffmpegCreateVideoFrame() : NULL;
		
		if (vframe)
		{
			vframe->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect.w, dst_rect.h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
			if (!vframe->surface)
			{
				SDL_ffmpegFreeVideoFrame(vframe);
				vframe = NULL;
			}
		}
		
		
		if (astream)
		{
			movie_audio_mutex = SDL_CreateMutex();
			SDL_AudioSpec specs = SDL_ffmpegGetAudioSpec(sffile, 512, movie_audio_callback);
			if (SDL_OpenAudio(&specs, 0) >= 0)
			{
				int frameSize = specs.channels * specs.samples * 2;
				for (int i = 0; i < AUDIO_BUF_SIZE; i++)
				{
					aframes[i] = SDL_ffmpegCreateAudioFrame(sffile, frameSize);
					SDL_ffmpegGetAudioFrame(sffile, aframes[i]);
				}
			}
		}
				
#ifdef HAVE_OPENGL
		if (OGL_IsActive())
			OGL_ClearScreen();
#endif
		
		SDL_PauseAudio(false);
		bool done = false;
		while (!done)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event) )
			{
				switch (event.type) {
				case SDL_KEYDOWN:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_CONTROLLERBUTTONDOWN:
					done = true;
					break;
				default:
					break;
				}
			}
			
			if (astream)
			{
				SDL_LockMutex(movie_audio_mutex);
				for (int i = 0; i < AUDIO_BUF_SIZE; i++)
				{
					if (!aframes[i]->size)
					{
						SDL_ffmpegGetAudioFrame(sffile, aframes[i]);
					}
				}
				if (!aframes[AUDIO_BUF_SIZE - 1]->size && aframes[AUDIO_BUF_SIZE - 1]->last)
					done = true;
				SDL_UnlockMutex(movie_audio_mutex);
			}
			
			if (vframe)
			{
				if (!vframe->ready)
				{
					SDL_ffmpegGetVideoFrame(sffile, vframe);
				}
				else if (vframe->pts <= movie_sync)
				{
#ifdef HAVE_OPENGL
					if (OGL_IsActive())
					{
						OGL_Blitter::BoundScreen();
						show_movie_blitter.Load(*(vframe->surface));
						show_movie_blitter.Draw(dst_rect);
						show_movie_blitter.Unload();
						MainScreenSwap();
					}
					else
#endif
					{
						SDL_BlitSurface(vframe->surface, 0, MainScreenSurface(), &dst_rect);
						MainScreenUpdateRects(1, &dst_rect);
					}
					vframe->ready = 0;
					if (vframe->last)
						done = true;
				}
				else 
				{
					sleep_for_machine_ticks(MIN(30, vframe->pts - movie_sync));
				}
			}
		}
		
		SDL_PauseAudio(true);
		if (astream)
		{
			for (int i = 0; i < AUDIO_BUF_SIZE; i++)
			{
				SDL_ffmpegFreeAudioFrame(aframes[i]);
			}
			SDL_DestroyMutex(movie_audio_mutex);
			movie_audio_mutex = NULL;
		}
				
		if (vframe)
			SDL_ffmpegFreeVideoFrame(vframe);
		SDL_ffmpegFree(sffile);
		SDL_CloseAudio();

#elif defined(HAVE_SMPEG) // end HAVE_FFMPEG
		
		SMPEG_Info info;
		SMPEG *movie;

		movie = SMPEG_new(File->GetPath(), &info, option_nosound ? 0 : 1);
		if (!movie) return;
		if (!info.has_video) {
			SMPEG_delete(movie);
			return;
		}
		
#ifdef HAVE_OPENGL
		SDL_Surface *gl_surface = NULL;
		if (OGL_IsActive())
		{
			if (PlatformIsLittleEndian()) {
				gl_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect.w, dst_rect.h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
			} else {
				gl_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect.w, dst_rect.h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x00000000);
			}
			SMPEG_setdisplay(movie, gl_surface, NULL, show_movie_frame);
			SMPEG_scaleXY(movie, dst_rect.w, dst_rect.h);
			show_movie_mutex = SDL_CreateMutex();
			OGL_ClearScreen();
		}
		else
#endif
		{
			SMPEG_setdisplay(movie, MainScreenSurface(), NULL, NULL);
			SMPEG_scaleXY(movie, dst_rect.w, dst_rect.h);
			SMPEG_move(movie, dst_rect.x, dst_rect.y);
		}
		
		bool done = false;
		SMPEG_play(movie);
		while (!done && SMPEG_status(movie) == SMPEG_PLAYING)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event) )
			{
				switch (event.type) {
				case SDL_KEYDOWN:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_CONTROLLERBUTTONDOWN:
					done = true;
					break;
#ifdef HAVE_OPENGL
				case SDL_USEREVENT:
					if (SDL_LockMutex(show_movie_mutex) != -1)
					{
						OGL_Blitter::BoundScreen();
						show_movie_blitter.Draw(dst_rect);
						show_movie_blitter.Unload();
						SDL_UnlockMutex(show_movie_mutex);
						MainScreenSwap();
					}
					break;
#endif
				default:
					break;
				}
			}
			
			sleep_for_machine_ticks(MACHINE_TICKS_PER_SECOND / 10);
		}
		SMPEG_delete(movie);
#ifdef HAVE_OPENGL
		SDL_FreeSurface(gl_surface);
		SDL_DestroyMutex(show_movie_mutex);
		show_movie_mutex = NULL;
		show_movie_blitter.Unload();
#endif
#endif // HAVE_SMPEG
	}
#endif // HAVE_FFMPEG || HAVE_SMPEG
}


size_t should_restore_game_networked(FileSpecifier& file)
{
	// We return -1 (NONE) for "cancel", 0 for "not networked", and 1 for "networked".
	size_t theResult = saved_game_was_networked(file);
	if (theResult != UNONE)
		return theResult;
	
        dialog d;

	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("RESUME GAME"), d);
	placer->add(new w_spacer, true);

	horizontal_placer *resume_as_placer = new horizontal_placer;
        w_toggle* theRestoreAsNetgameToggle = new w_toggle(dynamic_world->player_count > 1, 0);
        theRestoreAsNetgameToggle->set_labels_stringset(kSingleOrNetworkStringSetID);
	resume_as_placer->dual_add(theRestoreAsNetgameToggle->label("Resume as"), d);
	resume_as_placer->dual_add(theRestoreAsNetgameToggle, d);

	placer->add(resume_as_placer, true);
	
	placer->add(new w_spacer(), true);
	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("RESUME", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	placer->add(button_placer, true);


	d.set_widget_placer(placer);

        if(d.run() == 0)
        {
                theResult = theRestoreAsNetgameToggle->get_selection();
        }
        else
        {
                theResult = UNONE;
        }

        return theResult;
}

OpenedResourceFile ExternalResources;

void set_external_resources_file(FileSpecifier& f)
{
	f.Open(ExternalResources);
}

void close_external_resources()
{
	ExternalResources.Close();
}

/*

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

*/

/*
 *  shell.cpp - Main game loop and input handling
 */

#include "cseries.h"

#include "map.h"
#include "monsters.h"
#include "player.h"
#include "render.h"
#include "shell.h"
#include "interface.h"
#include "SoundManager.h"
#include "fades.h"
#include "screen.h"
#include "Music.h"
#include "images.h"
#include "vbl.h"
#include "preferences.h"
#include "tags.h" /* for scenario file type.. */
#include "network_sound.h"
#include "mouse.h"
#include "screen_drawing.h"
#include "computer_interface.h"
#include "game_wad.h" /* yuck... */
#include "game_window.h" /* for draw_interface() */
#include "extensions.h"
#include "items.h"
#include "interface_menus.h"
#include "weapons.h"
#include "lua_script.h"

#include "Crosshairs.h"
#include "OGL_Render.h"
#include "XML_ParseTreeRoot.h"
#include "FileHandler.h"

#include "mytm.h"	// mytm_initialize(), for platform-specific shell_*.h

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "XML_Loader_SDL.h"
#include "resource_manager.h"
#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"

#include "TextStrings.h"

#ifdef HAVE_CONFIG_H
#include "confpaths.h"
#endif

#include <exception>
#include <algorithm>
#include <vector>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_OPENGL
# if defined (__APPLE__) && defined (__MACH__)
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#ifdef HAVE_SDL_NET_H
#include <SDL_net.h>
#endif

#ifdef HAVE_SDL_SOUND_H
#include <SDL_sound.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#include "SDL_syswm.h"

#include "alephversion.h"

#include "Logging.h"
#include "network.h"
#include "Console.h"

// LP addition: whether or not the cheats are active
// Defined in shell_misc.cpp
extern bool CheatsActive;

// Data directories
vector <DirectorySpecifier> data_search_path; // List of directories in which data files are searched for
DirectorySpecifier local_data_dir;    // Local (per-user) data file directory
DirectorySpecifier preferences_dir;   // Directory for preferences
DirectorySpecifier saved_games_dir;   // Directory for saved games
DirectorySpecifier recordings_dir;    // Directory for recordings (except film buffer, which is stored in local_data_dir)
std::string arg_directory;

// Command-line options
bool option_nogl = false;             // Disable OpenGL
bool option_nosound = false;          // Disable sound output
bool option_nogamma = false;	      // Disable gamma table effects (menu fades)
bool option_debug = false;
static bool force_fullscreen = false; // Force fullscreen mode
static bool force_windowed = false;   // Force windowed mode

// Prototypes
static void main_event_loop(void);
extern int process_keyword_key(char key);
extern void handle_keyword(int type_of_cheat);

void PlayInterfaceButtonSound(short SoundID);

#ifdef __BEOS__
// From csfiles_beos.cpp
extern string get_application_directory(void);
extern string get_preferences_directory(void);
#endif

// From preprocess_map_sdl.cpp
extern bool get_default_music_spec(FileSpecifier &file);

// From vbl_sdl.cpp
void execute_timer_tasks(uint32 time);

// Prototypes
static void initialize_application(void);
static void shutdown_application(void);
static void initialize_marathon_music_handler(void);
static void process_event(const SDL_Event &event);

// cross-platform static variables
short vidmasterStringSetID = -1; // can be set with MML

static void usage(const char *prg_name)
{
#ifdef __WIN32__
	MessageBox(NULL, "Command line switches:\n\n"
#else
	printf("\nUsage: %s\n"
#endif
	  "\t[-h | --help]          Display this help message\n"
	  "\t[-v | --version]       Display the game version\n"
	  "\t[-d | --debug]         Allow saving of core files\n"
	  "\t                       (by disabling SDL parachute)\n"
	  "\t[-f | --fullscreen]    Run the game fullscreen\n"
	  "\t[-w | --windowed]      Run the game in a window\n"
#ifdef HAVE_OPENGL
	  "\t[-g | --nogl]          Do not use OpenGL\n"
#endif
	  "\t[-s | --nosound]       Do not access the sound card\n"
	  "\t[-m | --nogamma]       Disable gamma table effects (menu fades)\n"
#if defined(unix) || defined(__BEOS__) || defined(__WIN32__) || defined(__NetBSD__) || defined(__OpenBSD__)
	  "\nYou can use the ALEPHONE_DATA environment variable to specify\n"
	  "the data directory.\n"
#endif
#ifdef __WIN32__
	  , "Usage", MB_OK | MB_ICONINFORMATION
#else
	  , prg_name
#endif
	);
	exit(0);
}

int main(int argc, char **argv)
{
	// Print banner (don't bother if this doesn't appear when started from a GUI)
	printf ("Aleph One " A1_VERSION_STRING "\n"
	  "http://marathon.sourceforge.net/\n\n"
	  "Original code by Bungie Software <http://www.bungie.com/>\n"
	  "Additional work by Loren Petrich, Chris Pruett, Rhys Hill et al.\n"
	  "TCP/IP networking by Woody Zenfell\n"
	  "Expat XML library by James Clark\n"
	  "SDL port by Christian Bauer <Christian.Bauer@uni-mainz.de>\n"
#if defined(__MACH__) && defined(__APPLE__)
	  "Mac OS X/SDL version by Chris Lovell, Alexander Strange, and Woody Zenfell\n"
#endif
	  "\nThis is free software with ABSOLUTELY NO WARRANTY.\n"
	  "You are welcome to redistribute it under certain conditions.\n"
	  "For details, see the file COPYING.\n"
#if defined(__BEOS__) || defined(__WIN32__) 
	  // BeOS and Windows are statically linked against SDL, so we have to include this:
	  "\nSimple DirectMedia Layer (SDL) Library included under the terms of the\n"
	  "GNU Library General Public License.\n"
	  "For details, see the file COPYING.SDL.\n"
#endif
#ifdef HAVE_SDL_NET
	  "\nBuilt with network play enabled.\n"
#endif
#ifdef HAVE_LUA
	  "\nBuilt with Lua scripting enabled.\n"
#endif
    );

	// Parse arguments
	char *prg_name = argv[0];
	argc--;
	argv++;
	while (argc > 0) {
		if (strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
			usage(prg_name);
		} else if (strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0) {
			printf("Aleph One " A1_VERSION_STRING "\n");
			exit(0);
		} else if (strcmp(*argv, "-f") == 0 || strcmp(*argv, "--fullscreen") == 0) {
			force_fullscreen = true;
		} else if (strcmp(*argv, "-w") == 0 || strcmp(*argv, "--windowed") == 0) {
			force_windowed = true;
		} else if (strcmp(*argv, "-g") == 0 || strcmp(*argv, "--nogl") == 0) {
			option_nogl = true;
		} else if (strcmp(*argv, "-s") == 0 || strcmp(*argv, "--nosound") == 0) {
			option_nosound = true;
		} else if (strcmp(*argv, "-m") == 0 || strcmp(*argv, "--nogamma") == 0) {
			option_nogamma = true;
		} else if (strcmp(*argv, "-d") == 0 || strcmp(*argv, "--debug") == 0) {
		  option_debug = true;
		} else if (arg_directory == "") {
			// if it's a directory, make it the default data dir
			FileSpecifier f(*argv);
			if (f.IsDir())
			{
				arg_directory = *argv;
			}
			
			else
				printf("%s is not a directory\n", *argv);
		} else {
			printf("Unrecognized argument '%s'.\n", *argv);
			usage(prg_name);
		}
		argc--;
		argv++;
	}

	try {
		
		// Initialize everything
		initialize_application();

		// Run the main loop
		main_event_loop();

	} catch (exception &e) {
		fprintf(stderr, "Unhandled exception: %s\n", e.what());
		exit(1);
	} catch (...) {
		fprintf (stderr, "Unknown exception\n");
		exit(1);
	}

	return 0;
}

static void initialize_application(void)
{
#if defined(__WIN32__) && defined(__MINGW32__)
	if (LoadLibrary("exchndl.dll")) option_debug = true;
#endif

	// Find data directories, construct search path
	DirectorySpecifier default_data_dir;

#if defined(unix) || defined(__NetBSD__) || defined(__OpenBSD__)

	default_data_dir = PKGDATADIR;
	const char *home = getenv("HOME");
	if (home)
		local_data_dir = home;
	local_data_dir += ".alephone";

#elif defined(__APPLE__) && defined(__MACH__)
	extern char *bundle_name; // SDLMain.m
	DirectorySpecifier bundle_data_dir = bundle_name;
	bundle_data_dir += "Contents/Resources/DataFiles";

	default_data_dir = bundle_data_dir;
	const char *home = getenv("HOME");
	if (home)
	    local_data_dir = home;
	local_data_dir += "Library";
	local_data_dir += "Application Support";
	local_data_dir += "AlephOne";

#elif defined(__BEOS__)

	default_data_dir = get_application_directory();
	local_data_dir = get_preferences_directory();

#elif defined(__WIN32__)

	char file_name[MAX_PATH];
	GetModuleFileName(NULL, file_name, sizeof(file_name));
	char *sep = strrchr(file_name, '\\');
	*sep = '\0';

	default_data_dir = file_name;

	char login[17];
	DWORD len = 17;

	bool hasName = (GetUserName((LPSTR) login, &len) == TRUE);
	if (!hasName || strpbrk(login, "\\/:*?\"<>|") != NULL)
		strcpy(login, "Bob User");

	local_data_dir = file_name;
	local_data_dir += "Prefs";
	local_data_dir.CreateDirectory();
	local_data_dir += login;

#else
	default_data_dir = "";
	local_data_dir = "";
//#error Data file paths must be set for this platform.
#endif

#if defined(__WIN32__)
#define LIST_SEP ';'
#else
#define LIST_SEP ':'
#endif
	
#if !(defined(__APPLE__) && defined(__MACH__))
	if (arg_directory != "") 
		default_data_dir = arg_directory;
#endif

	const char *data_env = getenv("ALEPHONE_DATA");
	if (data_env) {
		// Read colon-separated list of directories
		string path = data_env;
		string::size_type pos;
		while ((pos = path.find(LIST_SEP)) != string::npos) {
			if (pos) {
				string element = path.substr(0, pos);
				data_search_path.push_back(element);
			}
			path.erase(0, pos + 1);
		}
		if (!path.empty())
			data_search_path.push_back(path);
	} else {
#if defined(__APPLE__) && defined(__MACH__)
		if (arg_directory != "") 
			data_search_path.push_back(arg_directory);
		else 
		{
			char* buf = getcwd (0, 0);
			data_search_path.push_back(buf);
			free (buf);
		}
#endif
#ifndef __MACOS__
		data_search_path.push_back(default_data_dir);
#endif
		data_search_path.push_back(local_data_dir);
	}

	// Subdirectories
	preferences_dir = local_data_dir;
	saved_games_dir = local_data_dir + "Saved Games";
	recordings_dir = local_data_dir + "Recordings";

	// Create local directories
	local_data_dir.CreateDirectory();
	saved_games_dir.CreateDirectory();
	recordings_dir.CreateDirectory();
#if defined(__APPLE__) && defined(__MACH__)
	DirectorySpecifier local_mml_dir = bundle_data_dir + "MML";
#else
	DirectorySpecifier local_mml_dir = local_data_dir + "MML";
#endif
	local_mml_dir.CreateDirectory();
#if defined(__APPLE__) && defined(__MACH__)
	DirectorySpecifier local_themes_dir = bundle_data_dir + "Themes";
#else
	DirectorySpecifier local_themes_dir = local_data_dir + "Themes";
#endif
	local_themes_dir.CreateDirectory();
	// Setup resource manager
	initialize_resources();

	init_physics_wad_data();

	initialize_fonts();

	// Parse MML files
	SetupParseTree();
	LoadBaseMMLScripts();

	// Check for presence of strings
	if (!TS_IsPresent(strERRORS) || !TS_IsPresent(strFILENAMES)) {
		fprintf(stderr, "Can't find required text strings (missing MML?).\n");
		exit(1);
	}

	// Load preferences
	initialize_preferences();
#ifndef HAVE_OPENGL
	graphics_preferences->screen_mode.acceleration = _no_acceleration;
#endif
	if (force_fullscreen)
		graphics_preferences->screen_mode.fullscreen = true;
	if (force_windowed)		// takes precedence over fullscreen because windowed is safer
		graphics_preferences->screen_mode.fullscreen = false;
	write_preferences();

// grr, Windows
#if defined(__WIN32__) 
	if (!SDL_getenv("SDL_VIDEODRIVER")) {
#if defined(HAVE_OPENGL)
		if (!option_nogl && 
		    graphics_preferences->screen_mode.acceleration != _no_acceleration)
		{
			SDL_putenv("SDL_VIDEODRIVER=windib");
		} else 
#endif
		{
			SDL_putenv("SDL_VIDEODRIVER=directx");
		}
	}
#endif

	SDL_putenv("SDL_VIDEO_ALLOW_SCREENSAVER=1");

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | 
		     (option_nosound ? 0 : SDL_INIT_AUDIO) |
		     (option_debug ? SDL_INIT_NOPARACHUTE : 0)
		     ) < 0) {
		fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
		exit(1);
	}
	SDL_WM_SetCaption("Aleph One", "Aleph One");
	atexit(shutdown_application);

#ifdef HAVE_SDL_NET
	// Initialize SDL_net
	if (SDLNet_Init () < 0) {
		fprintf (stderr, "Couldn't initialize SDL_net (%s)\n", SDLNet_GetError());
		exit(1);
	}
#endif

#ifdef HAVE_SDL_SOUND
	// Initialize SDL_sound
	if (Sound_Init () == 0) {
		fprintf (stderr, "Couldn't initialize SDL_sound (%s)\n", Sound_GetError());
		exit(1);
	}
#endif

#ifdef HAVE_SDL_TTF
	if (TTF_Init() < 0) {
		fprintf (stderr, "Couldn't initialize SDL_ttf (%s)\n", TTF_GetError());
		exit(1);
	}
#endif



	// Initialize everything
	mytm_initialize();
//	initialize_fonts();
	SoundManager::instance()->Initialize(*sound_preferences);
	initialize_marathon_music_handler();
	initialize_keyboard_controller();
	initialize_screen(&graphics_preferences->screen_mode, false);
	initialize_marathon();
	initialize_screen_drawing();
	FileSpecifier theme = environment_preferences->theme_dir;
	initialize_dialogs(theme);
	initialize_terminal_manager();
	initialize_shape_handler();
	initialize_fades();
	initialize_images_manager();
	load_environment_from_preferences();
	initialize_game_state();
}

static void shutdown_application(void)
{
        // ZZZ: seem to be having weird recursive shutdown problems esp. with fullscreen modes...
        static bool already_shutting_down = false;
        if(already_shutting_down)
                return;
        
        already_shutting_down = true;
        
#ifdef HAVE_SDL_NET
	SDLNet_Quit();
#endif
#ifdef HAVE_SDL_SOUND
	Sound_Quit();
#endif
#ifdef HAVE_SDL_TTF
	TTF_Quit();
#endif
	SDL_Quit();
}

bool networking_available(void)
{
#ifdef HAVE_SDL_NET
	return true;
#else
	return false;
#endif
}

static void initialize_marathon_music_handler(void)
{
	FileSpecifier file;
	if (get_default_music_spec(file))
		Music::instance()->SetupIntroMusic(file);
}

bool quit_without_saving(void)
{
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add (new w_static_text("Are you sure you wish to"), d);
	placer->dual_add (new w_static_text("cancel the game in progress?"), d);
	placer->add (new w_spacer(), true);
	
	horizontal_placer *button_placer = new horizontal_placer;
	w_button *default_button = new w_button("YES", dialog_ok, &d);
	button_placer->dual_add (default_button, d);
	button_placer->dual_add (new w_button("NO", dialog_cancel, &d), d);
	d.activate_widget(default_button);
	placer->add(button_placer, true);
	d.set_widget_placer(placer);
	return d.run() == 0;
}

// ZZZ: moved level-numbers widget into sdl_widgets for a wider audience.

const int32 AllPlayableLevels = _single_player_entry_point | _multiplayer_carnage_entry_point | _multiplayer_cooperative_entry_point | _kill_the_man_with_the_ball_entry_point | _king_of_hill_entry_point | _rugby_entry_point | _capture_the_flag_entry_point;

short get_level_number_from_user(void)
{
	// Get levels
	vector<entry_point> levels;
	if (!get_entry_points(levels, AllPlayableLevels)) {
		entry_point dummy;
		dummy.level_number = 0;
		strcpy(dummy.level_name, "Untitled Level");
		levels.push_back(dummy);
	}

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	if (vidmasterStringSetID != -1 && TS_IsPresent(vidmasterStringSetID) && TS_CountStrings(vidmasterStringSetID) > 0) {
		// if we there's a stringset present for it, load the message from there
		int num_lines = TS_CountStrings(vidmasterStringSetID);

		for (size_t i = 0; i < num_lines; i++) {
			bool message_font_title_color = true;
			char *string = TS_GetCString(vidmasterStringSetID, i);
			if (!strncmp(string, "[QUOTE]", 7)) {
				string = string + 7;
				message_font_title_color = false;
			}
			if (!strlen(string))
				placer->add(new w_spacer(), true);
			else if (message_font_title_color)
				placer->dual_add(new w_static_text(string), d);
			else
				placer->dual_add(new w_static_text(string), d);
		}

	} else {
		// no stringset or no strings in stringset - use default message
		placer->dual_add(new w_static_text("Before proceeding any further, you"), d);
		placer->dual_add(new w_static_text ("must take the oath of the vidmaster:"), d);
		placer->add(new w_spacer(), true);
		placer->dual_add(new w_static_text("\xd2I pledge to punch all switches,"), d);
		placer->dual_add(new w_static_text("to never shoot where I could use grenades,"), d);
		placer->dual_add(new w_static_text("to admit the existence of no level"), d);
		placer->dual_add(new w_static_text("except Total Carnage,"), d);
		placer->dual_add(new w_static_text("to never use Caps Lock as my \xd4run\xd5 key,"), d);
		placer->dual_add(new w_static_text("and to never, ever, leave a single Bob alive.\xd3"), d);
	}

	placer->add(new w_spacer(), true);
	placer->dual_add(new w_static_text("Start at level:"), d);

	w_levels *level_w = new w_levels(levels, &d);
	placer->dual_add(level_w, d);
	placer->add(new w_spacer(), true);
	placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	d.activate_widget(level_w);
	d.set_widget_placer(placer);

	// Run dialog
	short level;
	if (d.run() == 0)		// OK
		// Should do noncontiguous map files OK
		level = levels[level_w->get_selection()].level_number;
	else
		level = NONE;

	// Redraw main menu
	update_game_window();
	return level;
}

const uint32 TICKS_BETWEEN_EVENT_POLL = 167; // 6 Hz
static void main_event_loop(void)
{
	uint32 last_event_poll = 0;
	short game_state;

	while ((game_state = get_game_state()) != _quit_game) {
		uint32 cur_time = SDL_GetTicks();
		bool yield_time = false;
		bool poll_event = false;

		switch (game_state) {
			case _game_in_progress:
			case _change_level:
			  if (Console::instance()->input_active() || cur_time - last_event_poll >= TICKS_BETWEEN_EVENT_POLL) {
					poll_event = true;
					last_event_poll = cur_time;
			  } else {				  
					SDL_PumpEvents ();	// This ensures a responsive keyboard control
			  }
				break;

			case _display_intro_screens:
			case _display_main_menu:
			case _display_chapter_heading:
			case _display_prologue:
			case _display_epilogue:
			case _begin_display_of_epilogue:
			case _display_credits:
			case _display_intro_screens_for_demo:
			case _display_quit_screens:
			case _displaying_network_game_dialogs:
				yield_time = interface_fade_finished();
				poll_event = true;
				break;

			case _close_game:
			case _switch_demo:
			case _revert_game:
				yield_time = poll_event = true;
				break;
		}

		if (poll_event) {
			global_idle_proc();

			while (true) {
				SDL_Event event;
				event.type = SDL_NOEVENT;
				SDL_PollEvent(&event);

				if (yield_time) {
					// The game is not in a "hot" state, yield time to other
					// processes by calling SDL_Delay() but only try for a maximum
					// of 30ms
					int num_tries = 0;
					while (event.type == SDL_NOEVENT && num_tries < 3) {
						SDL_Delay(10);
						SDL_PollEvent(&event);
						num_tries++;
					}
					yield_time = false;
				} else if (event.type == SDL_NOEVENT)
					break;

				process_event(event);
			}
		}

		execute_timer_tasks(SDL_GetTicks());
		idle_game_state(SDL_GetTicks());

#ifndef __MACOS__
		if (game_state == _game_in_progress && !graphics_preferences->hog_the_cpu && (TICKS_PER_SECOND - (SDL_GetTicks() - cur_time)) > 10) 
		{
			SDL_Delay(1);
		}
#endif
	}
}

static bool has_cheat_modifiers(void)
{
	SDLMod m = SDL_GetModState();
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOS__)
	return ((m & KMOD_SHIFT) && (m & KMOD_CTRL)) || ((m & KMOD_ALT) && (m & KMOD_META));
#else
	return (m & KMOD_SHIFT) && (m & KMOD_CTRL) && !(m & KMOD_ALT) && !(m & KMOD_META);
#endif
}

static void process_screen_click(const SDL_Event &event)
{
	portable_process_screen_click(event.button.x, event.button.y, has_cheat_modifiers());
}

static void handle_game_key(const SDL_Event &event)
{
	SDLKey key = event.key.keysym.sym;
	bool changed_screen_mode = false;
	bool changed_prefs = false;

	if (!game_is_networked && (event.key.keysym.mod & KMOD_CTRL) && CheatsActive) {
		int type_of_cheat = process_keyword_key(key);
		if (type_of_cheat != NONE)
			handle_keyword(type_of_cheat);
	}
	if (Console::instance()->input_active()) {
		switch(key) {
		case SDLK_RETURN:
			Console::instance()->enter();
			break;
		case SDLK_ESCAPE:
			Console::instance()->abort();
			break;
		case SDLK_BACKSPACE:
		case SDLK_DELETE:
			Console::instance()->backspace();
			break;
		default:
			if (event.key.keysym.unicode == 8) // Crtl-H
			{
				Console::instance()->backspace();
			}
			else if (event.key.keysym.unicode == 21) // Crtl-U
			{
				Console::instance()->clear();
			}
			else if (event.key.keysym.unicode >= ' ') {
				Console::instance()->key(unicode_to_mac_roman(event.key.keysym.unicode));
			}
		}
	}
	else
	{
		if (key == SDLK_ESCAPE) // (ZZZ) Quit gesture (now safer)
		{
			if(!player_controlling_game())
				do_menu_item_command(mGame, iQuitGame, false);
			else {
				if(get_ticks_since_local_player_in_terminal() > 1 * TICKS_PER_SECOND) {
					if(!game_is_networked) {
						do_menu_item_command(mGame, iQuitGame, false);
					}
					else {
#if defined(__APPLE__) && defined(__MACH__)
						screen_printf("If you wish to quit, press Command-Q");
#else
						screen_printf("If you wish to quit, press Alt+Q.");
#endif
					}
				}
			}
		}
		else if (key == input_preferences->shell_keycodes[_key_volume_up])
		{
			changed_prefs = SoundManager::instance()->AdjustVolumeUp(_snd_adjust_volume);
		}
		else if (key == input_preferences->shell_keycodes[_key_volume_down])
		{
			changed_prefs = SoundManager::instance()->AdjustVolumeDown(_snd_adjust_volume);
		}
		else if (key == input_preferences->shell_keycodes[_key_switch_view])
		{
			walk_player_list();
			render_screen(NONE);
		}
		else if (key == input_preferences->shell_keycodes[_key_zoom_in])
		{
			if (zoom_overhead_map_in())
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
			else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == input_preferences->shell_keycodes[_key_zoom_out])
		{
			if (zoom_overhead_map_out())
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
			else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == input_preferences->shell_keycodes[_key_inventory_left])
		{
			if (player_controlling_game()) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				scroll_inventory(-1);
			} else
				decrement_replay_speed();
		}
		else if (key == input_preferences->shell_keycodes[_key_inventory_right])
		{
			if (player_controlling_game()) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				scroll_inventory(1);
			} else
				increment_replay_speed();
		}
		else if (key == input_preferences->shell_keycodes[_key_toggle_fps])
		{
			PlayInterfaceButtonSound(Sound_ButtonSuccess());
			extern bool displaying_fps;
			displaying_fps = !displaying_fps;
		}
		else if (key == input_preferences->shell_keycodes[_key_activate_console])
		{
			if (game_is_networked) {
#if !defined(DISABLE_NETWORKING)
				Console::instance()->activate_input(InGameChatCallbacks::SendChatMessage, InGameChatCallbacks::prompt());
#endif
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
			} 
			else if (Console::instance()->use_lua_console())
			{
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				Console::instance()->activate_input(ExecuteLuaString, ">");
			}
			else
			{
				PlayInterfaceButtonSound(Sound_ButtonFailure());
			}
		} 
		else if (key == SDLK_F1) // Decrease screen size
		{
			if (graphics_preferences->screen_mode.size > 0) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				graphics_preferences->screen_mode.size--;
				changed_screen_mode = changed_prefs = true;
			} else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == SDLK_F2) // Increase screen size
		{
			if (graphics_preferences->screen_mode.size < NUMBER_OF_VIEW_SIZES - 1) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				graphics_preferences->screen_mode.size++;
				changed_screen_mode = changed_prefs = true;
			} else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == SDLK_F3) // Resolution toggle
		{
			if (!OGL_IsActive()) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				graphics_preferences->screen_mode.high_resolution = !graphics_preferences->screen_mode.high_resolution;
				changed_screen_mode = changed_prefs = true;
			} else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == SDLK_F4)		// Reset OpenGL textures
		{
#ifdef HAVE_OPENGL
			if (OGL_IsActive()) {
				// Play the button sound in advance to get the full effect of the sound
				PlayInterfaceButtonSound(Sound_OGL_Reset());
				OGL_ResetTextures();
			} else
#endif
				PlayInterfaceButtonSound(Sound_ButtonInoperative());
		}
		else if (key == SDLK_F5) // Make the chase cam switch sides
		{
			if (ChaseCam_IsActive())
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
			else
				PlayInterfaceButtonSound(Sound_ButtonInoperative());
			ChaseCam_SwitchSides();
		}
		else if (key == SDLK_F6) // Toggle the chase cam
		{
			PlayInterfaceButtonSound(Sound_ButtonSuccess());
			ChaseCam_SetActive(!ChaseCam_IsActive());
		}
		else if (key == SDLK_F7) // Toggle tunnel vision
		{
			PlayInterfaceButtonSound(Sound_ButtonSuccess());
			SetTunnelVision(!GetTunnelVision());
		}
		else if (key == SDLK_F8) // Toggle the crosshairs
		{
			PlayInterfaceButtonSound(Sound_ButtonSuccess());
			Crosshairs_SetActive(!Crosshairs_IsActive());
		}
		else if (key == SDLK_F9) // Screen dump
		{
			dump_screen();
		}
		else if (key == SDLK_F10) // Toggle the position display
		{
			PlayInterfaceButtonSound(Sound_ButtonSuccess());
			{
				extern bool ShowPosition;
				ShowPosition = !ShowPosition;
			}
		}
		else if (key == SDLK_F11) // Decrease gamma level
		{
			if (graphics_preferences->screen_mode.gamma_level) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				graphics_preferences->screen_mode.gamma_level--;
				change_gamma_level(graphics_preferences->screen_mode.gamma_level);
				changed_prefs = true;
			} else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else if (key == SDLK_F12) // Increase gamma level
		{
			if (graphics_preferences->screen_mode.gamma_level < NUMBER_OF_GAMMA_LEVELS - 1) {
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				graphics_preferences->screen_mode.gamma_level++;
				change_gamma_level(graphics_preferences->screen_mode.gamma_level);
				changed_prefs = true;
			} else
				PlayInterfaceButtonSound(Sound_ButtonFailure());
		}
		else
		{
			if (get_game_controller() == _demo)
				set_game_state(_close_game);
		}
	}
	
	if (changed_screen_mode) {
		screen_mode_data temp_screen_mode = graphics_preferences->screen_mode;
		temp_screen_mode.fullscreen = get_screen_mode()->fullscreen;
		change_screen_mode(&temp_screen_mode, true);
		render_screen(0);
	}

	if (changed_prefs)
		write_preferences();
}

static void process_game_key(const SDL_Event &event)
{
	switch (get_game_state()) {
	case _game_in_progress:
#if defined(__APPLE__) && defined(__MACH__)
		if ((event.key.keysym.mod & KMOD_META)) 
#else
		if ((event.key.keysym.mod & KMOD_ALT) || (event.key.keysym.mod & KMOD_META)) 
#endif
		{
			int item = -1;
			switch (event.key.keysym.sym) {
			case SDLK_p:
				item = iPause;
				break;
			case SDLK_s:
				item = iSave;
				break;
			case SDLK_r:
				item = iRevert;
				break;
// ZZZ: Alt+F4 is also a quit gesture in Windows
#ifdef __WIN32__
			case SDLK_F4:
#endif
			case SDLK_q:
				item = iQuitGame;
				break;
			case SDLK_RETURN:
				item = 0;
				toggle_fullscreen();
				break;
			default:
				break;
			}
			if (item > 0)
				do_menu_item_command(mGame, item, has_cheat_modifiers());
			else if (item != 0)
				handle_game_key(event);
		} else
			handle_game_key(event);
		break;
	case _display_intro_screens:
	case _display_chapter_heading:
	case _display_prologue:
	case _display_epilogue:
	case _display_credits:
	case _display_quit_screens:
		if (interface_fade_finished())
			force_game_state_change();
		else
			stop_interface_fade();
		break;

	case _display_intro_screens_for_demo:
		stop_interface_fade();
		display_main_menu();
		break;

	case _quit_game:
	case _close_game:
	case _revert_game:
	case _switch_demo:
	case _change_level:
	case _begin_display_of_epilogue:
	case _displaying_network_game_dialogs:
		break;

	case _display_main_menu: 
	{
		if (!interface_fade_finished())
			stop_interface_fade();
		int item = -1;
		switch (event.key.keysym.sym) {
		case SDLK_n:
			item = iNewGame;
			break;
		case SDLK_o:
			item = iLoadGame;
			break;
		case SDLK_g:
			item = iGatherGame;
			break;
		case SDLK_j:
			item = iJoinGame;
			break;
		case SDLK_p:
			item = iPreferences;
			break;
		case SDLK_r:
			item = iReplaySavedFilm;
			break;
		case SDLK_c:
			item = iCredits;
			break;
// ZZZ: Alt+F4 is also a quit gesture in Windows
#ifdef __WIN32__
                case SDLK_F4:
#endif
		case SDLK_q:
			item = iQuit;
			break;
		case SDLK_F9:
			dump_screen();
			break;
		case SDLK_RETURN:
#if defined(__APPLE__) && defined(__MACH__)
			if ((event.key.keysym.mod & KMOD_META))
#else
			if ((event.key.keysym.mod & KMOD_META) || (event.key.keysym.mod & KMOD_ALT))
#endif
			{
				toggle_fullscreen();
			}
			break;
		default:
			break;
		}
		if (item > 0) {
			draw_menu_button_for_command(item);
			do_menu_item_command(mInterface, item, has_cheat_modifiers());
		}
		break;
	}
	}
}

static void process_system_event(const SDL_Event &event)
{
	// In order to get music in Windows, we need to process
	// system events. DirectShow notifies about events through
	// window messages. 
#ifdef WIN32
#ifndef WIN32_DISABLE_MUSIC
	switch (event.syswm.msg->msg) {
		case WM_DSHOW_GRAPH_NOTIFY:
			process_music_event_win32(event);
			break;
	}
#endif
#endif
}

static void process_event(const SDL_Event &event)
{
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (get_game_state() == _game_in_progress) 
		{
			if (event.button.button == 4 || event.button.button == 5)
			{
				mouse_scroll(event.button.button == 4);
			}
			else if (!get_keyboard_controller_status())
			{
				hide_cursor();
				set_keyboard_controller_status(true);
			}
		}
		else
			process_screen_click(event);
		break;
		
	case SDL_KEYDOWN:
		process_game_key(event);
		break;
		
	case SDL_SYSWMEVENT:
		process_system_event(event);
		break;
		
	case SDL_QUIT:
		set_game_state(_quit_game);
		break;
		
	case SDL_ACTIVEEVENT:
		if (event.active.state & SDL_APPINPUTFOCUS) {
			if (!event.active.gain && !(SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
				if (get_game_state() == _game_in_progress && get_keyboard_controller_status()) {
					darken_world_window();
					set_keyboard_controller_status(false);
					show_cursor();
				}
			}
		}
		break;
	case SDL_VIDEOEXPOSE:
#if !defined(__APPLE__) && !defined(__MACH__) // double buffering :)
#ifdef HAVE_OPENGL
		if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
			SDL_GL_SwapBuffers();
		else
#endif
			update_game_window();
#endif
		break;
	}
	
}

void dump_screen(void)
{
	// Find suitable file name
	FileSpecifier file;
	int i = 0;
	do {
		char name[256];
		sprintf(name, "Screenshot_%04d.bmp", i);
		file = local_data_dir + name;
		i++;
	} while (file.Exists());

	// Without OpenGL, dumping the screen is easy
	SDL_Surface *video = SDL_GetVideoSurface();
	if (!(video->flags & SDL_OPENGL)) {
		SDL_SaveBMP(SDL_GetVideoSurface(), file.GetPath());
		return;
	}

#ifdef HAVE_OPENGL
	// Otherwise, allocate temporary surface...
	SDL_Surface *t = SDL_CreateRGBSurface(SDL_SWSURFACE, video->w, video->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	  0x000000ff, 0x0000ff00, 0x00ff0000, 0);
#else
	  0x00ff0000, 0x0000ff00, 0x000000ff, 0);
#endif
	if (t == NULL)
		return;

	// ...and pixel buffer
	void *pixels = malloc(video->w * video->h * 3);
	if (pixels == NULL) {
		SDL_FreeSurface(t);
		return;
	}

	// Read OpenGL frame buffer
	glReadPixels(0, 0, video->w, video->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Copy pixel buffer (which is upside-down) to surface
	for (int y = 0; y < video->h; y++)
		memcpy((uint8 *)t->pixels + t->pitch * y, (uint8 *)pixels + video->w * 3 * (video->h - y - 1), video->w * 3);
	free(pixels);

	// Save surface
	SDL_SaveBMP(t, file.GetPath());
	SDL_FreeSurface(t);
#endif
}

void LoadBaseMMLScripts()
{
	XML_Loader_SDL loader;
	loader.CurrentElement = &RootParser;
	{
		vector <DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			DirectorySpecifier path = *i + "MML";
			loader.ParseDirectory(path);
			path = *i + "Scripts";
			loader.ParseDirectory(path);
			i++;
		}
	}
}

// LP: the rest of the code has been moved to Jeremy's shell_misc.file.

void PlayInterfaceButtonSound(short SoundID)
{
	if (TEST_FLAG(input_preferences->modifiers,_inputmod_use_button_sounds))
		SoundManager::instance()->PlaySound(SoundID, (world_location3d *) NULL, NONE);
}

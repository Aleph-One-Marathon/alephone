/*
 *  shell_sdl.cpp - Main game loop and input handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "XML_Loader_SDL.h"
#include "resource_manager.h"
#include "sdl_dialogs.h"
#include "sdl_widgets.h"

#ifdef HAVE_CONFIG_H
#include "confpaths.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


// Data directories
FileSpecifier global_data_dir;	// Global data file directory
FileSpecifier local_data_dir;	// Local (per-user) data file directory
FileSpecifier global_mml_dir;	// Global MML directory
FileSpecifier local_mml_dir;	// Local MML directory
FileSpecifier saved_games_dir;	// Directory for saved games
FileSpecifier recordings_dir;	// Directory for recordings (except film buffer, which is stored in local_data_dir)

// Command-line options
bool option_fullscreen = false;		// Run fullscreen
bool option_nogl = false;			// Disable OpenGL
bool option_nosound = false;		// Disable sound output


#ifdef __BEOS__
// From csfiles_beos.cpp
extern string get_application_directory(void);
extern string get_preferences_directory(void);
#endif

// From FileHandler_SDL.cpp
extern bool get_default_music_spec(FileSpecifier &file);

// Prototypes
static void initialize_application(void);
static void shutdown_application(void);
static void initialize_marathon_music_handler(void);
static void process_event(const SDL_Event &event);


/*
 *  Main function
 */

static void usage(const char *prg_name)
{
	printf(
		"Aleph One " VERSION "\n"
		"http://source.bungie.org/\n\n"
		"Original code by Bungie Software <http://www.bungie.com/>\n"
		"Additional work by Loren Petrich, Chris Pruett, Rhys Hill et al.\n"
	    "Expat XML library by James Clark\n"
		"SDL port by Christian Bauer <Christian.Bauer@uni-mainz.de>\n"
		"\nUsage: %s\n"
		"\t[-h | --help]          Display this help message\n"
		"\t[-v | --version]       Display the game version\n"
		"\t[-f | --fullscreen]    Run the game fullscreen\n"
#ifdef HAVE_OPENGL
		"\t[-g | --nogl]          Do not use OpenGL\n"
#endif
		"\t[-s | --nosound]       Do not access the sound card\n"
#if defined(__unix__) || defined(__BEOS__)
		"\nYou can use the ALEPHONE_DATA environment variable to specify\n"
		"the data directory.\n"
#endif
		, prg_name
	);
	exit(0);
}

int main(int argc, char **argv)
{
	// Parse arguments
	char *prg_name = argv[0];
	argc--; argv++;
	while (argc > 0) {
		if (strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
			usage(prg_name);
		} else if (strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0) {
			printf("Aleph One " VERSION "\n");
			exit(0);
		} else if (strcmp(*argv, "-f") == 0 || strcmp(*argv, "--fullscreen") == 0) {
			option_fullscreen = true;
		} else if (strcmp(*argv, "-g") == 0 || strcmp(*argv, "--nogl") == 0) {
			option_nogl = true;
		} else if (strcmp(*argv, "-s") == 0 || strcmp(*argv, "--nosound") == 0) {
			option_nosound = true;
		} else {
			printf("Unrecognized argument '%s'.\n", *argv);
			usage(prg_name);
		}
		argc--; argv++;	
	}

	// Initialize everything
	initialize_application();

	// Run the main loop
	main_event_loop();
	return 0;
}


/*
 *  Initialize everything
 */

static void initialize_application(void)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD | (option_nosound ? 0 : SDL_INIT_AUDIO)) < 0) {
		fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
		exit(1);
	}
	SDL_WM_SetCaption("Aleph One", "Aleph One");
	atexit(shutdown_application);

	// Get paths of local and global data directories
#if defined(__unix__)
	global_data_dir = PKGDATADIR;

	const char *data_dir = getenv("ALEPHONE_DATA");
	if (data_dir)
		global_data_dir = data_dir;

	const char *home = getenv("HOME");
	if (home)
		local_data_dir = home;
	local_data_dir.AddPart(".alephone");
#elif defined(__BEOS__)
	global_data_dir = get_application_directory();

	const char *data_dir = getenv("ALEPHONE_DATA");
	if (data_dir)
		global_data_dir = data_dir;

	local_data_dir = get_preferences_directory();
#else
#error Data file paths must be set for this platform.
#endif

	// Subdirectories
	global_mml_dir = global_data_dir;
	global_mml_dir.AddPart("MML");
	local_mml_dir = local_data_dir;
	local_mml_dir.AddPart("MML");
	saved_games_dir = local_data_dir;
	saved_games_dir.AddPart("Saved Games");
	recordings_dir = local_data_dir;
	recordings_dir.AddPart("Recordings");

	// Create local directories
	local_data_dir.CreateDirectory();
	local_mml_dir.CreateDirectory();
	saved_games_dir.CreateDirectory();
	recordings_dir.CreateDirectory();

	// Setup resource manager
	FileSpecifier resource_file = global_data_dir;
	resource_file.AddPart("Resources");
	initialize_resources(resource_file);

	// Parse MML files
	SetupParseTree();
	XML_Loader_SDL XML_Loader;
	XML_Loader.CurrentElement = &RootParser;
	XML_Loader.ParseDirectory(global_mml_dir);
	XML_Loader.ParseDirectory(local_mml_dir);

	// Load preferences
	initialize_preferences();
	graphics_preferences->screen_mode.acceleration = _no_acceleration;
#ifdef HAVE_OPENGL
	// Can't be switched later because of an SDL bug
	if (!option_nogl && graphics_preferences->screen_mode.bit_depth == 16)
		graphics_preferences->screen_mode.acceleration = _opengl_acceleration;
#endif
	write_preferences();

	// Initialize everything
	initialize_sound_manager(sound_preferences);
	initialize_marathon_music_handler();
	initialize_keyboard_controller();
	initialize_screen(&graphics_preferences->screen_mode);
	initialize_marathon();
	initialize_screen_drawing();
	initialize_dialogs();
	initialize_terminal_manager();
	initialize_shape_handler();
	initialize_fades();
	initialize_images_manager();
	load_environment_from_preferences();
	initialize_game_state();
}


/*
 *  Shutdown application
 */

static void shutdown_application(void)
{
	SDL_Quit();
}


/*
 *  Networking available?
 */

bool networking_available(void)
{
	return false;
}


/*
 *  Initialize music handling
 */

static void initialize_marathon_music_handler(void)
{
	FileSpecifier file;
	if (get_default_music_spec(file))
		initialize_music_handler(file);
}


/*
 *  Display alert message
 */

const int MAX_ALERT_WIDTH = 320;

void alert_user(short severity, short resid, short item, OSErr error)
{
	char str[256];
	getcstr(str, resid, item);
	if (SDL_GetVideoSurface() == NULL) {
		fprintf(stderr, "%s: %s (error %d)\n", severity == infoError ? "INFO" : "FATAL", str, error);
	} else {
		char message[300];
		sprintf(message, "%s (error %d)", str, error);
		dialog d;
		d.add(new w_static_text(severity == infoError ? "WARNING" : "ERROR", TITLE_FONT, TITLE_COLOR));
		d.add(new w_spacer());

		// Wrap lines
		uint16 style;
		const sdl_font_info *font = get_dialog_font(MESSAGE_FONT, style);
		char *t = message;
		while (strlen(t)) {
			int i = 0, last = 0, width = 0;
			while (i < strlen(t) && width < MAX_ALERT_WIDTH) {
				width += char_width(t[i], font, style);
				if (t[i] == ' ')
					last = i;
				i++;
			}
			if (i != strlen(t))
				t[last] = 0;
			d.add(new w_static_text(t));
			if (i != strlen(t))
				t += last + 1;
			else
				t += i;
		}

		d.add(new w_spacer());
		d.add(new w_button(severity == infoError ? "OK" : "QUIT", dialog_ok, &d));
		d.run();
		if (severity == infoError)
			update_game_window();
	}
	if (severity != infoError)
		exit(1);
}


/*
 *  Put up "Quit without saving" dialog
 */

bool quit_without_saving(void)
{
	dialog d;
	d.add(new w_static_text("Are you sure you wish to"));
	d.add(new w_static_text("cancel the game in progress?"));
	d.add(new w_spacer());
	d.add(new w_left_button("YES", dialog_ok, &d));
	d.add(new w_right_button("NO", dialog_cancel, &d));
	return d.run() == 0;
}


/*
 *  Level number dialog
 */

class w_level_number : public w_number_entry {
public:
	w_level_number(const char *name, dialog *dia) : w_number_entry(name, 1), d(dia) {}

	void event(SDL_Event &e)
	{
		// Return = close dialog
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
			d->quit(0);
		w_number_entry::event(e);
	}

private:
	dialog *d;
};

short get_level_number_from_user(void)
{
	// Find number of levels
	struct entry_point entry;
	short index = 0;
	int maximum_level_number = 0;
	while (get_indexed_entry_point(&entry, &index, _single_player_entry_point | _multiplayer_carnage_entry_point | _multiplayer_cooperative_entry_point))
		maximum_level_number++;
	if (maximum_level_number == 0)
		maximum_level_number = 1;

	// Create dialog
	dialog d;
	d.add(new w_static_text("Before proceeding any further, you", MESSAGE_FONT, TITLE_COLOR));
	d.add(new w_static_text("must take the oath of the vidmaster:", MESSAGE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	d.add(new w_static_text("\xd2I pledge to punch all switches,"));
	d.add(new w_static_text("to never shoot where I could use grenades,"));
	d.add(new w_static_text("to admit the existence of no level"));
	d.add(new w_static_text("except Total Carnage,"));
	d.add(new w_static_text("to never use Caps Lock as my \xd4run\xd5 key,"));
	d.add(new w_static_text("and to never, ever, leave a single Bob alive.\xd3"));
	d.add(new w_spacer());
	w_level_number *level_w = new w_level_number("Start at level", &d);
	d.add(level_w);
	char str[64];
	sprintf(str, "(Available levels: 1..%d)", maximum_level_number);
	d.add(new w_static_text(str, MESSAGE_FONT, LABEL_COLOR));
	d.add(new w_spacer());
	d.add(new w_left_button("OK", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Run dialog
	play_sound(DIALOG_INTRO_SOUND, NULL, NONE);
again:
	int level = NONE;
	if (d.run(false) == 0) {	// OK
		level = level_w->get_number();
		if (level <= 0 || level > maximum_level_number) {
			play_sound(DIALOG_ERROR_SOUND, NULL, NONE);
			level_w->set_number(1);
			goto again;
		}
		play_sound(DIALOG_OK_SOUND, NULL, NONE);
		level--;
	} else
		play_sound(DIALOG_CANCEL_SOUND, NULL, NONE);

	// Redraw main menu
	update_game_window();
	return level;
}


/*
 *  Main event loop
 */

const Uint32 TICKS_BETWEEN_EVENT_POLL = 167;	// 6 Hz

static void main_event_loop(void)
{
	Uint32 last_event_poll = 0;

	while (get_game_state() != _quit_game) {

		bool yield_time = false;
		bool poll_event = false;

		switch (get_game_state()) {
			case _game_in_progress:
			case _change_level:
				if (SDL_GetTicks() - last_event_poll >= TICKS_BETWEEN_EVENT_POLL) {
					poll_event = true;
					last_event_poll = SDL_GetTicks();
				} else
					SDL_PumpEvents();	// This ensures a responsive keyboard control
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
				} else
					if (event.type == SDL_NOEVENT)
						break;

				process_event(event);
			}
		}

		idle_game_state();
	}
}


/*
 *  Process SDL event
 */

static bool has_cheat_modifiers(void)
{
	SDLMod m = SDL_GetModState();
	return (m & KMOD_SHIFT) && (m & KMOD_CTRL) && !(m & KMOD_ALT) && !(m & KMOD_META);
}

static void process_screen_click(const SDL_Event &event)
{
	portable_process_screen_click(event.button.x, event.button.y, has_cheat_modifiers());
}

static bool is_keypad(SDLKey key)
{
	return key >= SDLK_KP0 && key <= SDLK_KP_EQUALS;
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

	switch (key) {
		case SDLK_PERIOD:		// sound volume up
			changed_prefs = adjust_sound_volume_up(sound_preferences, _snd_adjust_volume);
			break;
		case SDLK_COMMA:		// sound volume down
			changed_prefs = adjust_sound_volume_down(sound_preferences, _snd_adjust_volume);
			break;

		case SDLK_BACKSPACE:	// switch player view
			walk_player_list();
			render_screen(0);
			break;

		case SDLK_EQUALS:
			zoom_overhead_map_in();
			break;
		case SDLK_MINUS:
			zoom_overhead_map_out();
			break;

		case SDLK_LEFTBRACKET:
			if (player_controlling_game())
				scroll_inventory(-1);
			else
				decrement_replay_speed();
			break;
		case SDLK_RIGHTBRACKET:
			if (player_controlling_game())
				scroll_inventory(1);
			else
				increment_replay_speed();
			break;

		case SDLK_BACKSLASH: {
			extern bool displaying_fps;
			displaying_fps = !displaying_fps;
			break;
		}

		case SDLK_F1:	// Decrease screen size
			if (graphics_preferences->screen_mode.size > 0) {
				graphics_preferences->screen_mode.size--;
				changed_screen_mode = changed_prefs = true;
			}
			break;

		case SDLK_F2:	// Increase screen size
			if (graphics_preferences->screen_mode.size < NUMBER_OF_VIEW_SIZES - 1) {
				graphics_preferences->screen_mode.size++;
				changed_screen_mode = changed_prefs = true;
			}
			break;

		case SDLK_F3:	// Resolution toggle
#ifdef HAVE_OPENGL
			if (!OGL_IsActive()) {
#endif
				graphics_preferences->screen_mode.high_resolution = !graphics_preferences->screen_mode.high_resolution;
				changed_screen_mode = changed_prefs = true;
#ifdef HAVE_OPENGL
			}
#endif
			break;

		case SDLK_F4:	// Reset OpenGL textures
#ifdef HAVE_OPENGL
			OGL_ResetTextures();
#endif
			break;

		case SDLK_F5:	// Make the chase cam switch sides
			ChaseCam_SwitchSides();
			break;
					
		case SDLK_F6:	// Toggle the chase cam
			ChaseCam_SetActive(!ChaseCam_IsActive());
			break;
					
		case SDLK_F7:	// Toggle tunnel vision
			SetTunnelVision(!GetTunnelVision());
			break;
					
		case SDLK_F8:	// Toggle the crosshairs
			Crosshairs_SetActive(!Crosshairs_IsActive());
			break;

		case SDLK_F9:	// Screen dump
			dump_screen();
			break;

		case SDLK_F10:	// Toggle the position display
			{
				extern bool ShowPosition;
				ShowPosition = !ShowPosition;
			}
			break;

		case SDLK_F11:	// Decrease gamma level
			if (graphics_preferences->screen_mode.gamma_level) {
				graphics_preferences->screen_mode.gamma_level--;
				change_gamma_level(graphics_preferences->screen_mode.gamma_level);
				changed_prefs = true;
			}
			break;

		case SDLK_F12:	// Increase gamma level
			if (graphics_preferences->screen_mode.gamma_level < NUMBER_OF_GAMMA_LEVELS - 1) {
				graphics_preferences->screen_mode.gamma_level++;
				change_gamma_level(graphics_preferences->screen_mode.gamma_level);
				changed_prefs = true;
			}
			break;

		default:
			if (get_game_controller() == _demo)
				set_game_state(_close_game);
			break;
	}

	if (changed_screen_mode) {
		change_screen_mode(&graphics_preferences->screen_mode, true);
		render_screen(0);
	}

	if (changed_prefs)
		write_preferences();
}

static void process_game_key(const SDL_Event &event)
{
	switch (get_game_state()) {
		case _game_in_progress:
			if (event.key.keysym.mod & KMOD_ALT) {
				int item = -1;
				switch (event.key.keysym.sym) {
					case SDLK_p: item = iPause; break;
					case SDLK_s: item = iSave; break;
					case SDLK_r: item = iRevert; break;
					case SDLK_c: item = iCloseGame; break;
					case SDLK_q: item = iQuitGame; break;
				}
				if (item > 0)
					do_menu_item_command(mGame, item, has_cheat_modifiers());
				else
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

		case _display_main_menu: {
			if (!interface_fade_finished())
				stop_interface_fade();
			int item = -1;
			switch (event.key.keysym.sym) {
				case SDLK_n: item = iNewGame; break;
				case SDLK_o: item = iLoadGame; break;
				case SDLK_g: item = iGatherGame; break;
				case SDLK_j: item = iJoinGame; break;
				case SDLK_p: item = iPreferences; break;
				case SDLK_r: item = iReplaySavedFilm; break;
				case SDLK_c: item = iCredits; break;
				case SDLK_q: item = iQuit; break;
				case SDLK_F9: dump_screen(); break;
			}
			if (item > 0) {
				draw_menu_button_for_command(item);
				do_menu_item_command(mInterface, item, has_cheat_modifiers());
			}
			break;
		}
	}
}

static void process_event(const SDL_Event &event)
{
	switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			process_screen_click(event);
			break;

		case SDL_KEYDOWN:
			process_game_key(event);
			break;

		case SDL_QUIT:
			set_game_state(_quit_game);
			break;
	}
}


/*
 *  Save screen dump
 */

void dump_screen(void)
{
	// Find suitable file name
	FileSpecifier file;
	int i = 0;
	do {
		char name[256];
		sprintf(name, "Screenshot_%0.4d.bmp", i);
		file = local_data_dir;
		file.AddPart(name);
		i++;
	} while (file.Exists());

	// Dump screen
	SDL_SaveBMP(SDL_GetVideoSurface(), file.GetName());
}

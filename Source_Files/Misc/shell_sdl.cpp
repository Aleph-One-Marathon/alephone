/*
 *  shell_sdl.cpp - Main game loop and input handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

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
#include <GL/gl.h>
#endif

#ifdef HAVE_SDL_NET_H
#include <SDL_net.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif


// Data directories
vector<DirectorySpecifier> data_search_path;	// List of directories in which data files are searched for
DirectorySpecifier local_data_dir;		// Local (per-user) data file directory
DirectorySpecifier preferences_dir;		// Directory for preferences
DirectorySpecifier saved_games_dir;		// Directory for saved games
DirectorySpecifier recordings_dir;		// Directory for recordings (except film buffer, which is stored in local_data_dir)

// Command-line options
bool option_nogl = false;				// Disable OpenGL
bool option_nosound = false;			// Disable sound output
static bool force_fullscreen = false;	// Force fullscreen mode
static bool force_windowed = false;		// Force windowed mode


#ifdef __BEOS__
// From csfiles_beos.cpp
extern string get_application_directory(void);
extern string get_preferences_directory(void);
#endif

// From preprocess_map_sdl.cpp
extern bool get_default_music_spec(FileSpecifier &file);

// From vbl_sdl.cpp
void execute_timer_tasks(void);

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
#ifdef __WIN32__
	MessageBox(NULL,
		"Command line switches:\n\n"
#else
	printf(
		"\nUsage: %s\n"
#endif
		"\t[-h | --help]          Display this help message\n"
		"\t[-v | --version]       Display the game version\n"
		"\t[-f | --fullscreen]    Run the game fullscreen\n"
		"\t[-w | --windowed]      Run the game in a window\n"
#ifdef HAVE_OPENGL
		"\t[-g | --nogl]          Do not use OpenGL\n"
#endif
		"\t[-s | --nosound]       Do not access the sound card\n"
#if defined(__unix__) || defined(__BEOS__) || defined(__WIN32__)
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
	printf(
		"Aleph One " VERSION "\n"
		"http://source.bungie.org/\n\n"
		"Original code by Bungie Software <http://www.bungie.com/>\n"
		"Additional work by Loren Petrich, Chris Pruett, Rhys Hill et al.\n"
	    "Expat XML library by James Clark\n"
		"SDL port by Christian Bauer <Christian.Bauer@uni-mainz.de>\n\n"
		"This is free software with ABSOLUTELY NO WARRANTY.\n"
		"You are welcome to redistribute it under certain conditions.\n"
		"For details, see the file COPYING.\n"
#ifdef __BEOS__
		// BeOS versions is statically linked against SDL, so we have to include this:
		"\nSimple DirectMedia Layer (SDL) Library included under the terms of the\n"
		"GNU Library General Public License.\n"
		"For details, see the file COPYING.SDL.\n"
#endif
	);

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
			force_fullscreen = true;
		} else if (strcmp(*argv, "-w") == 0 || strcmp(*argv, "--windowed") == 0) {
			force_windowed = true;
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

	try {

		// Initialize everything
		initialize_application();

		// Run the main loop
		main_event_loop();

	} catch (exception &e) {

		fprintf(stderr, "Unhandled exception: %s\n", e.what());
		exit(1);

	} catch (...) {

		fprintf(stderr, "Unknown exception\n");
		exit(1);

	}

	return 0;
}


/*
 *  Initialize everything
 */

static void initialize_application(void)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | (option_nosound ? 0 : SDL_INIT_AUDIO)) < 0) {
		fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
		exit(1);
	}
	SDL_WM_SetCaption("Aleph One", "Aleph One");
	atexit(shutdown_application);

#ifdef HAVE_SDL_NET
	// Initialize SDL_net
	if (SDLNet_Init() < 0) {
		fprintf(stderr, "Couldn't initialize SDL_net (%s)\n", SDLNet_GetError());
		exit(1);
	}
#endif

	// Find data directories, construct search path
	DirectorySpecifier default_data_dir;

#if defined(__unix__)

	default_data_dir = PKGDATADIR;
	const char *home = getenv("HOME");
	if (home)
		local_data_dir = home;
	local_data_dir += ".alephone";

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

	bool hasName = GetUserName((LPSTR)login, &len);
	if (!hasName || strpbrk(login, "\\/:*?\"<>|") != NULL) 
		strcpy(login, "Bob User");

	local_data_dir = file_name;
	local_data_dir += "Prefs";
	local_data_dir.CreateDirectory();
	local_data_dir += login;
	 
#else
#error Data file paths must be set for this platform.
#endif

#if defined(__unix__) || defined(__BEOS__)
#define LIST_SEP ':'
#elif defined(__WIN32__)
#define LIST_SEP ';'
#endif

#if defined(__unix__) || defined(__BEOS__) || defined(__WIN32__)
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
	} else
#endif
		data_search_path.push_back(default_data_dir);
	data_search_path.push_back(local_data_dir);

	// Subdirectories
	preferences_dir = local_data_dir;
	saved_games_dir = local_data_dir + "Saved Games";
	recordings_dir = local_data_dir + "Recordings";

	// Create local directories
	local_data_dir.CreateDirectory();
	saved_games_dir.CreateDirectory();
	recordings_dir.CreateDirectory();
	DirectorySpecifier local_mml_dir = local_data_dir + "MML";
	local_mml_dir.CreateDirectory();
	DirectorySpecifier local_themes_dir = local_data_dir + "Themes";
	local_themes_dir.CreateDirectory();

	// Setup resource manager
	initialize_resources();

	// Parse MML files
	SetupParseTree();
	XML_Loader_SDL loader;
	loader.CurrentElement = &RootParser;
	{
		vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			DirectorySpecifier path = *i + "MML";
			loader.ParseDirectory(path);
			i++;
		}
	}

	// Check for presence of strings
	if (!TS_IsPresent(strERRORS) || !TS_IsPresent(strFILENAMES)) {
		fprintf(stderr, "Can't find required text strings (missing MML?).\n");
		exit(1);
	}

	// Load preferences
	initialize_preferences();
	graphics_preferences->screen_mode.acceleration = _no_acceleration;
#ifdef HAVE_OPENGL
	// Can't be switched later because of an SDL bug
	if (!option_nogl && graphics_preferences->screen_mode.bit_depth == 16)
		graphics_preferences->screen_mode.acceleration = _opengl_acceleration;
#endif
	if (force_fullscreen)
		graphics_preferences->screen_mode.fullscreen = true;
	if (force_windowed)	// takes precedence over fullscreen because windowed is safer
		graphics_preferences->screen_mode.fullscreen = false;
	write_preferences();

	// Initialize everything
	initialize_fonts();
	initialize_sound_manager(sound_preferences);
	initialize_marathon_music_handler();
	initialize_keyboard_controller();
	initialize_screen(&graphics_preferences->screen_mode);
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


/*
 *  Shutdown application
 */

static void shutdown_application(void)
{
#ifdef HAVE_SDL_NET
	SDLNet_Quit();
#endif

	SDL_Quit();
}


/*
 *  Networking available?
 */

bool networking_available(void)
{
#ifdef HAVE_SDL_NET
	return true;
#else
	return false;
#endif
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
		if (severity == infoError && top_dialog == NULL)
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

class w_levels : public w_list<entry_point> {
public:
	w_levels(const vector<entry_point> &items, dialog *d) : w_list<entry_point>(items, 400, 8, 0), parent(d) {}

	void item_selected(void)
	{
		parent->quit(0);
	}

	void draw_item(vector<entry_point>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const
	{
		y += font->get_ascent();
		char str[256];
		sprintf(str, "%d - %s", i->level_number + 1, i->level_name);
		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, str, x, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

private:
	dialog *parent;
};

short get_level_number_from_user(void)
{
	// Get levels
	vector<entry_point> levels;
	if (!get_entry_points(levels, _single_player_entry_point | _multiplayer_carnage_entry_point | _multiplayer_cooperative_entry_point)) {
		entry_point dummy;
		dummy.level_number = 0;
		strcpy(dummy.level_name, "untitled");
		levels.push_back(dummy);
	}

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
	d.add(new w_static_text("Start at level:", MESSAGE_FONT, TITLE_COLOR));
	w_levels *level_w = new w_levels(levels, &d);
	d.add(level_w);
	d.add(new w_spacer());
	d.add(new w_button("CANCEL", dialog_cancel, &d));

	// Run dialog
	int level;
	if (d.run() == 0)	// OK
		level = level_w->get_selection();
	else
		level = NONE;

	// Redraw main menu
	update_game_window();
	return level;
}


/*
 *  File selection dialogs
 */

class w_file_list : public w_list<dir_entry> {
public:
	w_file_list(const vector<dir_entry> &items) : w_list<dir_entry>(items, 400, 15, 0) {}

	void draw_item(vector<dir_entry>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const
	{
		y += font->get_ascent();
		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, i->name.c_str(), x, y, selected ? get_dialog_color(ITEM_ACTIVE_COLOR) : get_dialog_color(ITEM_COLOR), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}
};

class w_read_file_list : public w_file_list {
public:
	w_read_file_list(const vector<dir_entry> &items, dialog *d) : w_file_list(items), parent(d) {}

	void item_selected(void)
	{
		parent->quit(0);
	}

private:
	dialog *parent;
};

bool FileSpecifier::ReadDialog(int type, char *prompt)
{
	// Set default prompt
	if (prompt == NULL) {
		switch (type) {
			case _typecode_savegame:
				prompt = "CONTINUE SAVED GAME";
				break;
			case _typecode_film:
				prompt = "REPLAY SAVED FILM";
				break;
			default:
				prompt = "OPEN FILE";
				break;
		}
	}

	// Read directory
	FileSpecifier dir;
	switch (type) {
		case _typecode_savegame:
			dir.SetToSavedGamesDir();
			break;
		case _typecode_film:
			dir.SetToRecordingsDir();
			break;
		default:
			dir.SetToLocalDataDir();
			break;
	}
	vector<dir_entry> entries;
	if (!dir.ReadDirectory(entries))
		return false;
	sort(entries.begin(), entries.end());

	// Create dialog
	dialog d;
	d.add(new w_static_text(prompt, TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_read_file_list *list_w = new w_read_file_list(entries, &d);
	d.add(list_w);
	d.add(new w_spacer());
	d.add(new w_button("CANCEL", dialog_cancel, &d));

	// Run dialog
	bool result = false;
	if (d.run() == 0) { // OK
		if (entries.size()) {
			name = dir.name;
			AddPart(entries[list_w->get_selection()].name);
			result = true;
		}
	}

	// Redraw game window
	update_game_window();
	return result;
}

class w_file_name : public w_text_entry {
public:
	w_file_name(const char *name, dialog *d, const char *initial_name = NULL) : w_text_entry(name, 31, initial_name), parent(d) {}
	~w_file_name() {}

	void event(SDL_Event &e)
	{
		// Return = close dialog
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
			parent->quit(0);
		w_text_entry::event(e);
	}

private:
	dialog *parent;
};

class w_write_file_list : public w_file_list {
public:
	w_write_file_list(const vector<dir_entry> &items, const char *selection, dialog *d, w_file_name *w) : w_file_list(items), parent(d), name_widget(w)
	{
		if (selection) {
			vector<dir_entry>::const_iterator i, end = items.end();
			int num = 0;
			for (i = items.begin(); i != end; i++, num++) {
				if (i->name == selection) {
					set_selection(num);
					break;
				}
			}
		}
	}

	void item_selected(void)
	{
		name_widget->set_text(items[selection].name.c_str());
		parent->quit(0);
	}

private:
	dialog *parent;
	w_file_name *name_widget;
};

static bool confirm_save_choice(FileSpecifier &file);

bool FileSpecifier::WriteDialog(int type, char *prompt, char *default_name)
{
	// Set default prompt
	if (prompt == NULL) {
		switch (type) {
			case _typecode_savegame:
				prompt = "SAVE GAME";
				break;
			case _typecode_film:
				prompt = "SAVE FILM";
				break;
			default:
				prompt = "SAVE FILE";
				break;
		}
	}

	// Read directory
	FileSpecifier dir;
	switch (type) {
		case _typecode_savegame:
			dir.SetToSavedGamesDir();
			break;
		case _typecode_film:
			dir.SetToRecordingsDir();
			break;
		default:
			dir.SetToLocalDataDir();
			break;
	}
	vector<dir_entry> entries;
	if (!dir.ReadDirectory(entries))
		return false;
	sort(entries.begin(), entries.end());

	// Create dialog
	dialog d;
	d.add(new w_static_text(prompt, TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_file_name *name_w = new w_file_name("File Name", &d, default_name);
	w_write_file_list *list_w = new w_write_file_list(entries, default_name, &d, name_w);
	d.add(list_w);
	d.add(new w_spacer());
	d.add(name_w);
	d.add(new w_spacer());
	d.add(new w_left_button("OK", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Run dialog
again:
	bool result = false;
	if (d.run() == 0) { // OK
		if (strlen(name_w->get_text()) == 0) {
			play_dialog_sound(DIALOG_ERROR_SOUND);
			name_w->set_text(default_name);
			goto again;
		}
		name = dir.name;
		AddPart(name_w->get_text());
		result = confirm_save_choice(*this);
		if (!result)
			goto again;
	}

	// Redraw game window
	update_game_window();
	return result;
}

bool FileSpecifier::WriteDialogAsync(int type, char *prompt, char *default_name)
{
	return FileSpecifier::WriteDialog(type, prompt, default_name);
}

static bool confirm_save_choice(FileSpecifier &file)
{
	// If the file doesn't exist, everything is alright
	if (!file.Exists())
		return true;

	// Construct message
	char name[256];
	file.GetName(name);
	char message[512];
	sprintf(message, "'%s' already exists.", name);

	// Create dialog
	dialog d;
	d.add(new w_static_text(message));
	d.add(new w_static_text("Ok to overwrite?"));
	d.add(new w_spacer());
	d.add(new w_left_button("YES", dialog_ok, &d));
	d.add(new w_right_button("NO", dialog_cancel, &d));

	// Run dialog
	return d.run() == 0;
}


/*
 *  Main event loop
 */

const uint32 TICKS_BETWEEN_EVENT_POLL = 167;	// 6 Hz

static void main_event_loop(void)
{
	uint32 last_event_poll = 0;

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

		execute_timer_tasks();
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
		case SDLK_PERIOD:		// Sound volume up
			changed_prefs = adjust_sound_volume_up(sound_preferences, _snd_adjust_volume);
			break;
		case SDLK_COMMA:		// Sound volume down
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
					default: break;
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
				default: break;
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
	for (int y=0; y<video->h; y++)
		memcpy((uint8 *)t->pixels + t->pitch * y, (uint8 *)pixels + video->w * 3 * (video->h - y - 1), video->w * 3);
	free(pixels);

	// Save surface
	SDL_SaveBMP(t, file.GetPath());
	SDL_FreeSurface(t);
#endif
}

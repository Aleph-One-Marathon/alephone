/*
 *  preferences_sdl.cpp - Preferences handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"
#include "screen.h"
#include "images.h"
#include "find_files.h"
#include "screen_drawing.h"

#include <string.h>
#include <vector>

#ifdef HAVE_UNISTD_H
#include <unistd.h>	// for getlogin()
#endif

#ifdef __WIN32__
#include <windows.h> // for GetUserName()
#endif


// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;

// Prototypes
static void player_dialog(void *arg);
static void opengl_dialog(void *arg);
static void graphics_dialog(void *arg);
static void sound_dialog(void *arg);
static void controls_dialog(void *arg);
static void environment_dialog(void *arg);
static void keyboard_dialog(void *arg);


/*
 *  Get user name
 */

static void get_name_from_system(char *name)
{
#if defined(__unix__) || defined(__BEOS__)

	char *login = getlogin();
	strcpy(name, login ? login : "Bob User");

#elif defined(__WIN32__)

	char login[17];
	DWORD len = 17;

	bool hasName = GetUserName((LPSTR)login, &len);
	if (hasName && strpbrk(login, "\\/:*?\"<>|") == NULL) // Ignore illegal names
		strcpy(name, login);
	else
		strcpy(name, "Bob User");

#else
#error get_name_from_system() not implemented for this platform
#endif
}


/*
 *  Ethernet always available
 */

static bool ethernet_active(void)
{
	return true;
}


/*
 *  Main preferences dialog
 */

void handle_preferences(void)
{
	// Save the existing preferences, in case we have to reload them
	write_preferences();

	// Load sensible palette
	struct color_table *system_colors = build_8bit_system_color_table();
	assert_world_color_table(system_colors, system_colors);
	delete system_colors;

	// Create top-level dialog
	dialog d;
	d.add(new w_static_text("PREFERENCES", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	d.add(new w_button("PLAYER", player_dialog, &d));
	d.add(new w_button("GRAPHICS", graphics_dialog, &d));
	d.add(new w_button("SOUND", sound_dialog, &d));
	d.add(new w_button("CONTROLS", controls_dialog, &d));
	d.add(new w_button("ENVIRONMENT", environment_dialog, &d));
	d.add(new w_spacer());
	d.add(new w_button("RETURN", dialog_cancel, &d));

	// Clear menu screen
	clear_screen();

	// Run dialog
	d.run();

	// Redraw main menu
	display_main_menu();
}


/*
 *  Player dialog
 */

static const char *level_labels[6] = {
	"Kindergarten", "Easy", "Normal", "Major Damage", "Total Carnage", NULL
};

static void player_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("PLAYER SETTINGS", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_select *level_w = new w_select("Difficulty", player_preferences->difficulty_level, level_labels);
	d.add(level_w);
	d.add(new w_spacer());
	d.add(new w_static_text("Network Appearance"));
	w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, player_preferences->name);
	d.add(name_w);
	w_player_color *pcolor_w = new w_player_color("Color", player_preferences->color);
	d.add(pcolor_w);
	w_player_color *tcolor_w = new w_player_color("Team Color", player_preferences->team);
	d.add(tcolor_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		const char *name = name_w->get_text();
		if (strcmp(name, player_preferences->name)) {
			strcpy(player_preferences->name, name);
			changed = true;
		}

		int level = level_w->get_selection();
		if (level != player_preferences->difficulty_level) {
			player_preferences->difficulty_level = level;
			changed = true;
		}

		int color = pcolor_w->get_selection();
		if (color != player_preferences->color) {
			player_preferences->color = color;
			changed = true;
		}

		int team = tcolor_w->get_selection();
		if (team != player_preferences->team) {
			player_preferences->team = team;
			changed = true;
		}

		if (changed)
			write_preferences();
	}
}


/*
 *  Handle graphics dialog
 */

static const char *depth_labels[3] = {
	"8 Bit", "16 Bit", NULL
};

static const char *resolution_labels[3] = {
	"Low", "High", NULL
};

static const char *size_labels[9] = {
	"320x160", "480x240", "640x320", "640x480 (no HUD)",
	"800x400", "800x600 (no HUD)", "1024x512", "1024x768 (no HUD)", NULL
};

static const char *gamma_labels[9] = {
	"Darkest", "Darker", "Dark", "Normal", "Light", "Really Light", "Even Lighter", "Lightest", NULL
};

static void graphics_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("GRAPHICS SETUP", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_toggle *depth_w = new w_toggle("Color Depth", graphics_preferences->screen_mode.bit_depth == 16, depth_labels);
	w_toggle *resolution_w = new w_toggle("Resolution", graphics_preferences->screen_mode.high_resolution, resolution_labels);
	if (graphics_preferences->screen_mode.acceleration == _no_acceleration) {
		d.add(depth_w);
		d.add(resolution_w);
	}
	w_select *size_w = new w_select("Screen Size", graphics_preferences->screen_mode.size, size_labels);
	d.add(size_w);
	w_toggle *fullscreen_w = new w_toggle("Fullscreen", graphics_preferences->screen_mode.fullscreen);
	d.add(fullscreen_w);
	w_select *gamma_w = new w_select("Brightness", graphics_preferences->screen_mode.gamma_level, gamma_labels);
	d.add(gamma_w);
#ifdef HAVE_OPENGL
	d.add(new w_spacer());
	d.add(new w_button("OPENGL OPTIONS", opengl_dialog, &d));
#endif
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		bool fullscreen = fullscreen_w->get_selection();
		if (fullscreen != graphics_preferences->screen_mode.fullscreen) {
			graphics_preferences->screen_mode.fullscreen = fullscreen;
			// This is the only setting that has an immediate effect
			toggle_fullscreen(fullscreen);
			parent->draw();
			changed = true;
		}

		int depth = (depth_w->get_selection() ? 16 : 8);
		if (depth != graphics_preferences->screen_mode.bit_depth) {
			graphics_preferences->screen_mode.bit_depth = depth;
			changed = true;
			// don't change mode now; it will be changed when the game starts
		}

		bool hi_res = resolution_w->get_selection();
		if (hi_res != graphics_preferences->screen_mode.high_resolution) {
			graphics_preferences->screen_mode.high_resolution = hi_res;
			changed = true;
		}

		int size = size_w->get_selection();
		if (size != graphics_preferences->screen_mode.size) {
			graphics_preferences->screen_mode.size = size;
			changed = true;
			// don't change mode now; it will be changed when the game starts
		}

		int gamma = gamma_w->get_selection();
		if (gamma != graphics_preferences->screen_mode.gamma_level) {
			graphics_preferences->screen_mode.gamma_level = gamma;
			changed = true;
		}

		if (changed) {
			write_preferences();
			parent->draw();		// DirectX seems to need this
		}
	}
}


/*
 *  OpenGL dialog
 */

static void opengl_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;
	OGL_ConfigureData &prefs = Get_OGL_ConfigureData();

	// Create dialog
	dialog d;
	d.add(new w_static_text("OPENGL OPTIONS", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_toggle *zbuffer_w = new w_toggle("Z Buffer", prefs.Flags & OGL_Flag_ZBuffer);
	d.add(zbuffer_w);
	w_toggle *landscape_w = new w_toggle("Landscapes", !(prefs.Flags & OGL_Flag_FlatLand));
	d.add(landscape_w);
	w_toggle *fog_w = new w_toggle("Fog", prefs.Flags & OGL_Flag_Fog);
	d.add(fog_w);
	w_toggle *multi_w = new w_toggle("Multitexturing", prefs.Flags & OGL_Flag_SnglPass);
	d.add(multi_w);
	w_toggle *static_w = new w_toggle("Static Effect", !(prefs.Flags & OGL_Flag_FlatStatic));
	d.add(static_w);
	w_toggle *fader_w = new w_toggle("Color Effects", prefs.Flags & OGL_Flag_Fader);
	d.add(fader_w);
	w_toggle *liq_w = new w_toggle("Transparent Liquids", prefs.Flags & OGL_Flag_LiqSeeThru);
	d.add(liq_w);
	w_toggle *map_w = new w_toggle("OpenGL Overhead Map", prefs.Flags & OGL_Flag_Map);
	d.add(map_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		uint16 flags = 0;
		if (zbuffer_w->get_selection()) flags |= OGL_Flag_ZBuffer;
		if (!(landscape_w->get_selection())) flags |= OGL_Flag_FlatLand;
		if (fog_w->get_selection()) flags |= OGL_Flag_Fog;
		if (multi_w->get_selection()) flags |= OGL_Flag_SnglPass;
		if (!(static_w->get_selection())) flags |= OGL_Flag_FlatStatic;
		if (fader_w->get_selection()) flags |= OGL_Flag_Fader;
		if (liq_w->get_selection()) flags |= OGL_Flag_LiqSeeThru;
		if (map_w->get_selection()) flags |= OGL_Flag_Map;

		if (flags != prefs.Flags) {
			prefs.Flags = flags;
			changed = true;
		}

		if (changed)
			write_preferences();
	}
}


/*
 *  Sound dialog
 */

class w_toggle *stereo_w, *dynamic_w;

class w_stereo_toggle : public w_toggle {
public:
	w_stereo_toggle(const char *name, bool selection) : w_toggle(name, selection) {}

	void selection_changed(void)
	{
		// Turning off stereo turns off dynamic tracking
		w_toggle::selection_changed();
		if (selection == false)
			dynamic_w->set_selection(false);
	}
};

class w_dynamic_toggle : public w_toggle {
public:
	w_dynamic_toggle(const char *name, bool selection) : w_toggle(name, selection) {}

	void selection_changed(void)
	{
		// Turning on dynamic tracking turns on stereo
		w_toggle::selection_changed();
		if (selection == true)
			stereo_w->set_selection(true);
	}
};

static const char *channel_labels[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", NULL};

class w_volume_slider : public w_slider {
public:
	w_volume_slider(const char *name, int vol) : w_slider(name, NUMBER_OF_SOUND_VOLUME_LEVELS, vol) {}
	~w_volume_slider() {}

	void item_selected(void)
	{
		test_sound_volume(selection, _snd_adjust_volume);
	}
};

static void sound_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("SOUND SETUP", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	static const char *quality_labels[3] = {"8 Bit", "16 Bit", NULL};
	w_toggle *quality_w = new w_toggle("Quality", sound_preferences->flags & _16bit_sound_flag, quality_labels);
	d.add(quality_w);
	stereo_w = new w_stereo_toggle("Stereo", sound_preferences->flags & _stereo_flag);
	d.add(stereo_w);
	dynamic_w = new w_dynamic_toggle("Active Panning", sound_preferences->flags & _dynamic_tracking_flag);
	d.add(dynamic_w);
	w_toggle *ambient_w = new w_toggle("Ambient Sounds", sound_preferences->flags & _ambient_sound_flag);
	d.add(ambient_w);
	w_toggle *more_w = new w_toggle("More Sounds", sound_preferences->flags & _more_sounds_flag);
	d.add(more_w);
	w_select *channels_w = new w_select("Channels", sound_preferences->channel_count, channel_labels);
	d.add(channels_w);
	w_volume_slider *volume_w = new w_volume_slider("Volume", sound_preferences->volume);
	d.add(volume_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		uint16 flags = 0;
		if (quality_w->get_selection()) flags |= _16bit_sound_flag;
		if (stereo_w->get_selection()) flags |= _stereo_flag;
		if (dynamic_w->get_selection()) flags |= _dynamic_tracking_flag;
		if (ambient_w->get_selection()) flags |= _ambient_sound_flag;
		if (more_w->get_selection()) flags |= _more_sounds_flag;

		if (flags != sound_preferences->flags) {
			sound_preferences->flags = flags;
			changed = true;
		}

		int channel_count = channels_w->get_selection();
		if (channel_count != sound_preferences->channel_count) {
			sound_preferences->channel_count = channel_count;
			changed = true;
		}

		int volume = volume_w->get_selection();
		if (volume != sound_preferences->volume) {
			sound_preferences->volume = volume;
			changed = true;
		}

		if (changed) {
			set_sound_manager_parameters(sound_preferences);
			write_preferences();
		}
	}
}


/*
 *  Controls dialog
 */

static w_toggle *mouse_w;

static void controls_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("CONTROLS", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	mouse_w = new w_toggle("Mouse Control", input_preferences->input_device);
	d.add(mouse_w);
	w_toggle *invert_mouse_w = new w_toggle("Invert Mouse", input_preferences->modifiers & _inputmod_invert_mouse);
	d.add(invert_mouse_w);
	w_toggle *always_run_w = new w_toggle("Always Run", input_preferences->modifiers & _inputmod_interchange_run_walk);
	d.add(always_run_w);
	w_toggle *always_swim_w = new w_toggle("Always Swim", input_preferences->modifiers & _inputmod_interchange_swim_sink);
	d.add(always_swim_w);
	w_toggle *weapon_w = new w_toggle("Auto-Switch Weapons", !(input_preferences->modifiers & _inputmod_dont_switch_to_new_weapon));
	d.add(weapon_w);
	d.add(new w_spacer());
	d.add(new w_button("CONFIGURE KEYBOARD", keyboard_dialog, &d));
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		int device = mouse_w->get_selection();
		if (device != input_preferences->input_device) {
			input_preferences->input_device = device;
			changed = true;
		}

		uint16 flags = 0;
		if (always_run_w->get_selection()) flags |= _inputmod_interchange_run_walk;
		if (always_swim_w->get_selection()) flags |= _inputmod_interchange_swim_sink;
		if (!(weapon_w->get_selection())) flags |= _inputmod_dont_switch_to_new_weapon;
		if (invert_mouse_w->get_selection()) flags |= _inputmod_invert_mouse;

		if (flags != input_preferences->modifiers) {
			input_preferences->modifiers = flags;
			changed = true;
		}

		if (changed)
			write_preferences();
	}
}


/*
 *  Keyboard dialog
 */

const int NUM_KEYS = 20;

static const char *action_name[NUM_KEYS] = {
	"Move Forward", "Move Backward", "Turn Left", "Turn Right", "Sidestep Left", "Sidestep Right",
	"Glance Left", "Glance Right", "Look Up", "Look Down", "Look Ahead",
	"Previous Weapon", "Next Weapon", "Trigger", "2nd Trigger",
	"Sidestep", "Run/Swim", "Look",
	"Action", "Auto Map"
};

static SDLKey default_keys[NUM_KEYS] = {
	SDLK_KP8, SDLK_KP5, SDLK_KP4, SDLK_KP6,		// moving/turning
	SDLK_z, SDLK_x,								// sidestepping
	SDLK_a, SDLK_s,								// horizontal looking
	SDLK_d, SDLK_c, SDLK_v,						// vertical looking
	SDLK_KP7, SDLK_KP9,							// weapon cycling
	SDLK_SPACE, SDLK_LALT,						// weapon trigger
	SDLK_LSHIFT, SDLK_LCTRL, SDLK_LMETA,		// modifiers
	SDLK_TAB,									// action trigger
	SDLK_m										// map
};

static SDLKey default_mouse_keys[NUM_KEYS] = {
	SDLK_w, SDLK_x, SDLK_LEFT, SDLK_RIGHT,		// moving/turning
	SDLK_a, SDLK_d,								// sidestepping
	SDLK_q, SDLK_e,								// horizontal looking
	SDLK_UP, SDLK_DOWN, SDLK_KP0,				// vertical looking
	SDLK_c, SDLK_z,								// weapon cycling
	SDLK_RCTRL, SDLK_SPACE,						// weapon trigger
	SDLK_RSHIFT, SDLK_LSHIFT, SDLK_LCTRL,		// modifiers
	SDLK_s,										// action trigger
	SDLK_TAB									// map
};

class w_prefs_key;

static w_prefs_key *key_w[NUM_KEYS];

class w_prefs_key : public w_key {
public:
	w_prefs_key(const char *name, SDLKey key) : w_key(name, key) {}

	void set_key(SDLKey new_key)
	{
		// Key used for in-game function?
		int error = NONE;
		switch (new_key) {
			case SDLK_PERIOD:		// Sound volume up/down
			case SDLK_COMMA:
				error = keyIsUsedForSound;
				break;

			case SDLK_EQUALS:		// Map zoom
			case SDLK_MINUS:
				error = keyIsUsedForMapZooming;
				break;

			case SDLK_LEFTBRACKET:	// Inventory scrolling
			case SDLK_RIGHTBRACKET:
				error = keyIsUsedForScrolling;
				break;

			case SDLK_BACKSPACE:
			case SDLK_BACKSLASH:
			case SDLK_F1:
			case SDLK_F2:
			case SDLK_F3:
			case SDLK_F4:
			case SDLK_F5:
			case SDLK_F6:
			case SDLK_F7:
			case SDLK_F8:
			case SDLK_F9:
			case SDLK_F10:
			case SDLK_F11:
			case SDLK_F12:
				error = keyIsUsedAlready;
				break;

			default:
				break;
		}
		if (error != NONE) {
			alert_user(infoError, strERRORS, error, 0);
			return;
		}

		w_key::set_key(new_key);
		if (new_key == SDLK_UNKNOWN)
			return;

		// Remove binding to this key from all other widgets
		for (int i=0; i<NUM_KEYS; i++) {
			if (key_w[i] && key_w[i] != this && key_w[i]->get_key() == new_key) {
				key_w[i]->set_key(SDLK_UNKNOWN);
				key_w[i]->dirty = true;
			}
		}
	}
};

static void load_default_keys(void *arg)
{
	// Load default keys, depending on state of "Mouse control" widget
	dialog *d = (dialog *)arg;
	SDLKey *keys = (mouse_w->get_selection() ? default_mouse_keys : default_keys);
	for (int i=0; i<NUM_KEYS; i++)
		key_w[i]->set_key(keys[i]);
	d->draw();
}

static void keyboard_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Clear array of key widgets (because w_prefs_key::set_key() scans it)
	for (int i=0; i<NUM_KEYS; i++)
		key_w[i] = NULL;

	// Create dialog
	dialog d;
	d.add(new w_static_text("CONFIGURE KEYBOARD", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	for (int i=0; i<NUM_KEYS; i++) {
		key_w[i] = new w_prefs_key(action_name[i], SDLKey(input_preferences->keycodes[i]));
		d.add(key_w[i]);
	}
	d.add(new w_spacer());
	d.add(new w_button("DEFAULTS", load_default_keys, &d));
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		for (int i=0; i<NUM_KEYS; i++) {
			SDLKey key = key_w[i]->get_key();
			if (key != input_preferences->keycodes[i]) {
				input_preferences->keycodes[i] = short(key);
				changed = true;
			}
		}

		set_keys(input_preferences->keycodes);
		if (changed)
			write_preferences();
	}
}


/*
 *  Environment dialog
 */

// Find available themes in directory and append to vector
class FindThemes : public FileFinder {
public:
	FindThemes(vector<FileSpecifier> &v) : dest_vector(v) {dest_vector.clear();}

private:
	bool found(FileSpecifier &file)
	{
		// Look for "theme.mml" files
		string base, part;
		file.SplitPath(base, part);
		if (part == "theme.mml")
			dest_vector.push_back(base);
		return false;
	}

	vector<FileSpecifier> &dest_vector;
};

// Environment item
class env_item {
public:
	env_item() : indent(0), selectable(false)
	{
		name[0] = 0;
	}

	env_item(const FileSpecifier &fs, int i, bool sel) : spec(fs), indent(i), selectable(sel)
	{
		spec.GetName(name);
	}

	FileSpecifier spec;	// Specifier of associated file
	char name[256];		// Last part of file name
	int indent;			// Indentation level
	bool selectable;	// Flag: item refers to selectable file (otherwise to directory name)
};

// Environment file list widget
class w_env_list : public w_list<env_item> {
public:
	w_env_list(const vector<env_item> &items, const char *selection, dialog *d) : w_list<env_item>(items, 400, 15, 0), parent(d)
	{
		vector<env_item>::const_iterator i, end = items.end();
		int num = 0;
		for (i = items.begin(); i != end; i++, num++) {
			if (strcmp(i->spec.GetPath(), selection) == 0) {
				set_selection(num);
				break;
			}
		}
	}

	bool is_item_selectable(int i)
	{
		return items[i].selectable;
	}

	void item_selected(void)
	{
		parent->quit(0);
	}

	void draw_item(vector<env_item>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const
	{
		y += font->get_ascent();

		int color;
		if (i->selectable) {
			color = selected ? ITEM_ACTIVE_COLOR : ITEM_COLOR;
		} else
			color = LABEL_COLOR;

		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, i->name, x + i->indent * 8, y, get_dialog_color(color), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

private:
	dialog *parent;
};

// Environment selection button
class w_env_select : public w_select_button {
public:
	w_env_select(const char *name, const char *path, const char *m, int t, dialog *d) : w_select_button(name, item_name, select_item_callback, this), parent(d), menu_title(m), type(t)
	{
		set_path(path);
	}
	~w_env_select() {}

	void set_path(const char *p)
	{
		item = p;
		item.GetName(item_name);
		set_selection(item_name);
	}

	const char *get_path(void) const
	{
		return item.GetPath();
	}

	FileSpecifier &get_file_specifier(void)
	{
		return item;
	}

private:
	void select_item(dialog *parent);
	static void select_item_callback(void *arg)
	{
		w_env_select *obj = (w_env_select *)arg;
		obj->select_item(obj->parent);
	}

	dialog *parent;
	const char *menu_title;	// Selection menu title

	FileSpecifier item;		// File specification
	int type;				// File type
	char item_name[256];	// File name (excluding directory part)
};

void w_env_select::select_item(dialog *parent)
{
	// Find available files
	vector<FileSpecifier> files;
	if (type == _typecode_theme) {

		// Theme, find by theme script
		FindThemes finder(files);
		vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			FileSpecifier dir = *i + "Themes";
			finder.Find(dir, WILDCARD_TYPE);
			i++;
		}

	} else {

		// Map/phyics/shapes/sounds, find by type
		FindAllFiles finder(files);
		vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
		while (i != end) {
			FileSpecifier dir = *i;
			finder.Find(dir, type);
			i++;
		}
	}

	// Create structured list of files
	vector<env_item> items;
	vector<FileSpecifier>::const_iterator i = files.begin(), end = files.end();
	string last_base;
	int indent_level = 0;
	for (i = files.begin(); i != end; i++) {
		string base, part;
		i->SplitPath(base, part);
		if (base != last_base) {

			// New directory
			FileSpecifier base_spec = base;
//			if (base_spec != global_dir && base_spec != local_dir) {

				// Subdirectory, insert name as unselectable item, put items on indentation level 1
				items.push_back(env_item(base_spec, 0, false));
				indent_level = 1;

//			} else {
//
//				// Top-level directory, put items on indentation level 0
//				indent_level = 0;
//			}
			last_base = base;
		}
		items.push_back(env_item(*i, indent_level, true));
	}

	// Create dialog
	dialog d;
	d.add(new w_static_text(menu_title, TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_env_list *list_w = new w_env_list(items, item.GetPath(), &d);
	d.add(list_w);
	d.add(new w_spacer());
	d.add(new w_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) { // Accepted
		if (items.size())
			set_path(items[list_w->get_selection()].spec.GetPath());
	}
}

static void environment_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("ENVIRONMENT SETTINGS", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_env_select *map_w = new w_env_select("Map", environment_preferences->map_file, "AVAILABLE MAPS", _typecode_scenario, &d);
	d.add(map_w);
	w_env_select *physics_w = new w_env_select("Physics", environment_preferences->physics_file, "AVAILABLE PHYSICS MODELS", _typecode_physics, &d);
	d.add(physics_w);
	w_env_select *shapes_w = new w_env_select("Shapes", environment_preferences->shapes_file, "AVAILABLE SHAPES", _typecode_shapes, &d);
	d.add(shapes_w);
	w_env_select *sounds_w = new w_env_select("Sounds", environment_preferences->sounds_file, "AVAILABLE SOUNDS", _typecode_sounds, &d);
	d.add(sounds_w);
	w_env_select *theme_w = new w_env_select("Theme", environment_preferences->theme_dir, "AVAILABLE THEMES", _typecode_theme, &d);
	d.add(theme_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	bool theme_changed = false;
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		const char *path = map_w->get_path();
		if (strcmp(path, environment_preferences->map_file)) {
			strcpy(environment_preferences->map_file, path);
			environment_preferences->map_checksum = read_wad_file_checksum(map_w->get_file_specifier());
			changed = true;
		}

		path = physics_w->get_path();
		if (strcmp(path, environment_preferences->physics_file)) {
			strcpy(environment_preferences->physics_file, path);
			environment_preferences->physics_checksum = read_wad_file_checksum(physics_w->get_file_specifier());
			changed = true;
		}

		path = shapes_w->get_path();
		if (strcmp(path, environment_preferences->shapes_file)) {
			strcpy(environment_preferences->shapes_file, path);
			environment_preferences->shapes_mod_date = shapes_w->get_file_specifier().GetDate();
			changed = true;
		}

		path = sounds_w->get_path();
		if (strcmp(path, environment_preferences->sounds_file)) {
			strcpy(environment_preferences->sounds_file, path);
			environment_preferences->sounds_mod_date = sounds_w->get_file_specifier().GetDate();
			changed = true;
		}

		path = theme_w->get_path();
		if (strcmp(path, environment_preferences->theme_dir)) {
			strcpy(environment_preferences->theme_dir, path);
			changed = theme_changed = true;
		}

		if (changed)
			load_environment_from_preferences();

		if (theme_changed) {
			FileSpecifier theme = environment_preferences->theme_dir;
			load_theme(theme);
		}

		if (changed || theme_changed)
			write_preferences();
	}

	// Redraw parent dialog
	if (theme_changed)
		parent->quit(0);	// Quit the parent dialog so it won't draw in the old theme
}

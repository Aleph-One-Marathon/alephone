/*
 *  preferences_sdl.cpp - Preferences handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "sdl_dialogs.h"
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

// From shell_sdl.cpp
extern FileSpecifier global_data_dir, local_data_dir;

// Prototypes
static void player_dialog(void *arg);
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
 *  Handle preferences dialog
 */

void handle_preferences(void)
{
	// Save the existing preferences, in case we have to reload them
	write_preferences();

	// Load sensible palette
	struct color_table *system_colors = build_8bit_system_color_table();
	assert_world_color_table(system_colors, NULL);
	delete system_colors;

	// Create top-level dialog
	dialog d;
	d.add(new w_pict(8000));
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
 *  Handle player dialog
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
	w_text_entry *name_w = new w_text_entry("Name", PREFERENCES_NAME_LENGTH, player_preferences->name);
	d.add(name_w);
	w_select *level_w = new w_select("Difficulty", player_preferences->difficulty_level, level_labels);
	d.add(level_w);
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

	// Redraw parent dialog
	clear_screen();
	parent->draw();
}


/*
 *  Handle graphics dialog
 */

static const char *depth_labels[3] = {
	"8 Bit", "16 Bit", NULL
};

static const char *size_labels[9] = {
	"320x160", "480x240", "640x320", "640x480 (no HUD)",
	"800x400", "800x600 (no HUD)", "1024x512", "1024x768 (no HUD)", NULL
};

static const char *gamma_labels[9] = {
	"Darkest", "Darker", "Dark", "Normal", "Light", "Really Light", "Even Lighter", "Lightest"
};

static void graphics_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_static_text("GRAPHICS SETUP", TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_toggle *depth_w = new w_toggle("Color Depth", graphics_preferences->screen_mode.bit_depth == 16, depth_labels);
	if (graphics_preferences->screen_mode.acceleration == _no_acceleration)
		d.add(depth_w);
	w_select *size_w = new w_select("Resolution", graphics_preferences->screen_mode.size, size_labels);
	d.add(size_w);
	w_select *gamma_w = new w_select("Brightness", graphics_preferences->screen_mode.gamma_level, gamma_labels);
	d.add(gamma_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		int depth = (depth_w->get_selection() ? 16 : 8);
		if (depth != graphics_preferences->screen_mode.bit_depth) {
			graphics_preferences->screen_mode.bit_depth = depth;
			changed = true;
			// don't change mode now; it will be changed when the game starts
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

		if (changed)
			write_preferences();
	}

	// Redraw parent dialog
	clear_screen();
	parent->draw();
}


/*
 *  Handle sound dialog
 */

class w_toggle *stereo_w;
class w_toggle *dynamic_w;

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

		if (changed) {
			set_sound_manager_parameters(sound_preferences);
			write_preferences();
		}
	}

	// Redraw parent dialog
	clear_screen();
	parent->draw();
}


/*
 *  Handle controls dialog
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

		if (flags != input_preferences->modifiers) {
			input_preferences->modifiers = flags;
			changed = true;
		}

		if (changed)
			write_preferences();
	}

	// Redraw parent dialog
	clear_screen();
	parent->draw();
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

static short default_keys[NUM_KEYS] = {
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

static short default_mouse_keys[NUM_KEYS] = {
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

static w_key *key_w[NUM_KEYS];

static void load_default_keys(void *arg)
{
	// Load default keys, depending on state of "Mouse control" widget
	dialog *d = (dialog *)arg;
	short *keys = (mouse_w->get_selection() ? default_mouse_keys : default_keys);
	for (int i=0; i<NUM_KEYS; i++)
		key_w[i]->set_key(SDLKey(input_preferences->keycodes[i] = keys[i]));
	d->draw();
}

static void keyboard_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	d.add(new w_pict(8001));
	d.add(new w_spacer());
	for (int i=0; i<NUM_KEYS; i++) {
		key_w[i] = new w_key(action_name[i], SDLKey(input_preferences->keycodes[i]));
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

	// Redraw parent dialog
	clear_screen();
	parent->draw();
}


/*
 *  Handle environment dialog
 */

class w_env_list : public w_list<FileSpecifier> {
public:
	w_env_list(const vector<FileSpecifier> &items, const char *selection, dialog *d) : w_list<FileSpecifier>(items, 400, 15, 0), parent(d)
	{
		vector<FileSpecifier>::const_iterator i, end = items.end();
		int num = 0;
		for (i = items.begin(); i != end; i++, num++) {
			if (strcmp(i->GetName(), selection) == 0) {
				set_selection(num);
				break;
			}
		}
	}

	void item_selected(void)
	{
		parent->quit(0);
	}

	void draw_item(vector<FileSpecifier>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const
	{
		y += font_ascent(font);
		char str[256];
		i->GetLastPart(str);
		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, str, x, y, selected ? get_dialog_color(s, ITEM_ACTIVE_COLOR) : get_dialog_color(s, ITEM_COLOR), font, style);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}

private:
	dialog *parent;
};

class w_env_select : public w_select_button {
public:
	w_env_select(const char *name, const char *path, const char *m, int t, dialog *d) : w_select_button(name, item_name, select_item_callback, this), parent(d), menu_title(m), type(t)
	{
		set_path(path);
	}
	~w_env_select() {}

	void set_path(const char *p)
	{
		item.SetName(p, type);
		item.GetLastPart(item_name);
		set_selection(item_name);
	}

	const char *get_path(void) const
	{
		return item.GetName();
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
	FindAllFiles finder(files);
	finder.Find(global_data_dir, type);
	finder.Find(local_data_dir, type);

	// Create dialog
	dialog d;
	d.add(new w_static_text(menu_title, TITLE_FONT, TITLE_COLOR));
	d.add(new w_spacer());
	w_env_list *list_w = new w_env_list(files, item.GetName(), &d);
	d.add(list_w);
	d.add(new w_spacer());
	d.add(new w_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) // Accepted
		set_path(files[list_w->get_selection()].GetName());

	// Redraw parent dialog
	clear_screen();
	parent->draw();
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
//	w_env_select *physics_w = new w_env_select("Physics", environment_preferences->physics_file, "AVAILABLE PHYSICS MODELS", _typecode_physics, &d);
//	d.add(physics_w);
	w_env_select *shapes_w = new w_env_select("Shapes", environment_preferences->shapes_file, "AVAILABLE SHAPES", _typecode_shapes, &d);
	d.add(shapes_w);
	w_env_select *sounds_w = new w_env_select("Sounds", environment_preferences->sounds_file, "AVAILABLE SOUNDS", _typecode_sounds, &d);
	d.add(sounds_w);
	d.add(new w_spacer());
	d.add(new w_left_button("ACCEPT", dialog_ok, &d));
	d.add(new w_right_button("CANCEL", dialog_cancel, &d));

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		const char *path = map_w->get_path();
		if (strcmp(path, environment_preferences->map_file)) {
			strcpy(environment_preferences->map_file, path);
			environment_preferences->map_checksum = read_wad_file_checksum(map_w->get_file_specifier());
			changed = true;
		}

//		path = physics_w->get_path();
//		if (strcmp(path, environment_preferences->physics_file)) {
//			strcpy(environment_preferences->physics_file, path);
//			environment_preferences->physics_checksum = read_wad_file_checksum(physics_w->get_file_specifier());
//			changed = true;
//		}

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

		if (changed) {
			load_environment_from_preferences();
			write_preferences();
		}
	}

	// Redraw parent dialog
	clear_screen();
	parent->draw();
}

/*

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

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Default to keyboard and mouse control under Carbon
	for there are no InputSprockets

Apr 30, 2002 (Loren Petrich):
	Converting to a MML-based preferences system

May 16, 2002 (Woody Zenfell):
    Added UI/preferences elements for configurable mouse sensitivity
    Support for "don't auto-recenter" behavior modifier
    Routines to let other code disable/reenable/query behavior modification
   
Jul 21, 2002 (Loren Petrich):
	AS had added some code to fix the OSX preferences behavior;
	I modified it so that it would not be used in the Classic version

Apr 10-22, 2003 (Woody Zenfell):
        Join hinting and autogathering have Preferences entries now
        Being less obnoxious with unrecognized Prefs stuff
        Macintosh Enviroprefs popup style can be set in Preferences file

May 22, 2003 (Woody Zenfell):
	Support for preferences for multiple network game protocols; configurable local game port.

 May 27, 2003 (Gregory Smith):
	Preferences for speex netmic

 August 27, 2003 (Woody Zenfell):
	Preferences for netscript.  Some reworking of index-based Mac FSSpec reading along the way.
 */

/*
 *  preferences.cpp - Preferences handling
 */

#include "cseries.h"
#include "FileHandler.h"

#include "map.h"
#include "shell.h" /* For the screen_mode structure */
#include "interface.h"
#include "SoundManager.h"
#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprocket Support */

#include "preferences.h"
#include "wad.h"
#include "wad_prefs.h"
#include "game_errors.h"
#include "network.h" // for _ethernet, etc.
#include "find_files.h"
#include "game_wad.h" // for set_map_file
#include "screen.h"
#include "fades.h"
#include "extensions.h"
#include "Console.h"
#include "Plugins.h"

#include "XML_ElementParser.h"
#include "XML_DataBlock.h"
#include "ColorParser.h"
#include "StarGameProtocol.h"
#include "RingGameProtocol.h"

#include "tags.h"
#include "Logging.h"

#include <string.h>
#include <stdlib.h>

#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"
#include "images.h"
#include "preference_dialogs.h"
#include "preferences_widgets_sdl.h"
#include "mouse.h"

#include "Music.h"

#include <cmath>
#include <sstream>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef __WIN32__
#include <windows.h> // for GetUserName()
#endif

#include "joystick.h"

// 8-bit support is still here if you undefine this, but you'll need to fix it
#define TRUE_COLOR_ONLY 1

using namespace alephone;

static const char sPasswordMask[] = "reverof nohtaram";

static const char* sNetworkGameProtocolNames[] =
{	// These should match up with _network_game_protocol_ring, etc.
	"ring",
	"star"
};

static const size_t NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES = sizeof(sNetworkGameProtocolNames) / sizeof(sNetworkGameProtocolNames[0]);

// MML-like Preferences Stuff; it makes obsolete
// w_open_preferences_file(), w_get_data_from_preferences(), and w_write_preferences_file()
// in wad_prefs.*

// The absolute root element ...
static XML_ElementParser PrefsRootParser("");

// This is the canonical root element in the XML-format preference file:
static XML_ElementParser MarathonPrefsParser("mara_prefs");

// Interpreter of read-in file
static XML_DataBlock XML_DataBlockLoader;

// Sets up the parser, of course
static void SetupPrefsParseTree();

// Have the prefs been inited?
static bool PrefsInited = false;


// Global preferences data
struct graphics_preferences_data *graphics_preferences = NULL;
struct serial_number_data *serial_preferences = NULL;
struct network_preferences_data *network_preferences = NULL;
struct player_preferences_data *player_preferences = NULL;
struct input_preferences_data *input_preferences = NULL;
SoundManager::Parameters *sound_preferences = NULL;
struct environment_preferences_data *environment_preferences = NULL;

// LP: fake portable-files stuff
inline short memory_error() {return 0;}

// Prototypes
#ifndef SDL
static void *get_player_pref_data(void);
static void *get_input_pref_data(void);
static void *get_sound_pref_data(void);
static void *get_graphics_pref_data(void);
static void *get_environment_pref_data(void);
#endif

static bool ethernet_active(void);
static void get_name_from_system(unsigned char *name);

// LP: getting rid of the (void *) mechanism as inelegant and non-type-safe
static void default_graphics_preferences(graphics_preferences_data *preferences);
static bool validate_graphics_preferences(graphics_preferences_data *preferences);
static void default_serial_number_preferences(serial_number_data *preferences);
static bool validate_serial_number_preferences(serial_number_data *preferences);
static void default_network_preferences(network_preferences_data *preferences);
static bool validate_network_preferences(network_preferences_data *preferences);
static void default_player_preferences(player_preferences_data *preferences);
static bool validate_player_preferences(player_preferences_data *preferences);
static void default_input_preferences(input_preferences_data *preferences);
static bool validate_input_preferences(input_preferences_data *preferences);
static void default_environment_preferences(environment_preferences_data *preferences);
static bool validate_environment_preferences(environment_preferences_data *preferences);

// Prototypes
static void player_dialog(void *arg);
static void graphics_dialog(void *arg);
static void sound_dialog(void *arg);
static void controls_dialog(void *arg);
static void environment_dialog(void *arg);
static void keyboard_dialog(void *arg);
//static void texture_options_dialog(void *arg);

/*
 *  Get user name
 */

static void get_name_from_system(unsigned char *outName)
{
    // Skipping usual string safety pickiness, I'm tired tonight.
    // Hope caller's buffer is big enough.
    char* name = (char*) outName;

#if defined(unix) || defined(__BEOS__) || (defined (__APPLE__) && defined (__MACH__)) || defined(__NetBSD__) || defined(__OpenBSD__)

	char *login = getlogin();
	strcpy(name, login ? login : "Bob User");

#elif defined(__WIN32__)

	char login[17];
	DWORD len = 17;

	bool hasName = (GetUserName((LPSTR)login, &len) == TRUE);
	if (hasName && strpbrk(login, "\\/:*?\"<>|") == NULL) // Ignore illegal names
		strcpy(name, login);
	else
		strcpy(name, "Bob User");

#else
//#error get_name_from_system() not implemented for this platform
#endif

    // In-place conversion to pstring
    a1_c2pstr(name);
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
	if (SDL_GetVideoSurface()->format->BitsPerPixel == 8) {
		struct color_table *system_colors = build_8bit_system_color_table();
		assert_world_color_table(system_colors, system_colors);
		delete system_colors;
	}

	// Create top-level dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	w_title *w_header = new w_title("PREFERENCES");
	d.add(w_header);
	w_button *w_player = new w_button("PLAYER", player_dialog, &d);
	d.add(w_player);

	w_button *w_graphics = new w_button("GRAPHICS", graphics_dialog, &d);
	d.add(w_graphics);
	w_button *w_sound = new w_button("SOUND", sound_dialog, &d);
	d.add(w_sound);
	w_button *w_controls = new w_button("CONTROLS", controls_dialog, &d);
	d.add(w_controls);
	w_button *w_environment = new w_button("ENVIRONMENT", environment_dialog, &d);
	d.add(w_environment);
	
	w_button *w_return = new w_button("RETURN", dialog_cancel, &d);
	d.add(w_return);

	placer->add(w_header);
	placer->add(new w_spacer, true);
	placer->add(w_player);
	placer->add(w_graphics);
	placer->add(w_sound);
	placer->add(w_controls);
	placer->add(w_environment);
	placer->add(new w_spacer, true);
	placer->add(w_return);

	d.set_widget_placer(placer);

	// Clear menu screen
	clear_screen();

	// Run dialog
	d.run();

	// Redraw main menu
	display_main_menu();
}

class CrosshairPref : public Bindable<int>
{
public:
	CrosshairPref(short& pref) : m_pref(pref) { }

	virtual int bind_export() {
		return (m_pref - 1);
	}

	virtual void bind_import(int value) {
		m_pref = value + 1;
	}

protected:
	short& m_pref;
};

class ColorComponentPref : public Bindable<int>
{
public:
	ColorComponentPref(uint16& pref) : m_pref(pref) { }
	
	virtual int bind_export() {
		return (m_pref >> 12);
	}

	virtual void bind_import(int value) {
		m_pref = value << 12;
	}

protected:
	uint16& m_pref;
};

class OpacityPref : public Bindable<int>
{
public:
	OpacityPref(float& pref) : m_pref(pref) { }
	
	virtual int bind_export() {
		return (static_cast<int>(floor(m_pref * 16)));
	}

	virtual void bind_import(int value) {
		m_pref = ((float) value / 16.0);
	}
protected:
	float& m_pref;
};

static const char *shape_labels[3] = {
	"Cross", "Octagon", NULL
};

enum { kCrosshairWidget };

static auto_ptr<BinderSet> crosshair_binders;

struct update_crosshair_display
{
	void operator()(dialog *d) {
		crosshair_binders->migrate_all_first_to_second();
	}
};

static void crosshair_dialog(void *arg)
{
	CrosshairData OldCrosshairs = player_preferences->Crosshairs;
	crosshair_binders.reset(new BinderSet);

	dialog *parent = (dialog *) arg;

	dialog d;
	vertical_placer *placer = new vertical_placer;
	w_title *w_header = new w_title("CROSSHAIR SETTINGS");
	placer->dual_add(w_header, d);
	placer->add(new w_spacer, true);

	w_crosshair_display *crosshair_w = new w_crosshair_display();
	placer->dual_add(crosshair_w, d);

	placer->add(new w_spacer, true);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET));
	table->col_flags(0, placeable::kAlignRight);

	w_toggle *crosshairs_active_w = new w_toggle(player_preferences->crosshairs_active);
	table->dual_add(crosshairs_active_w->label("Show crosshairs"), d);
	table->dual_add(crosshairs_active_w, d);	
	
	// Shape
	w_select *shape_w = new w_select(0, shape_labels);
	SelectSelectorWidget shapeWidget(shape_w);
	Int16Pref shapePref(player_preferences->Crosshairs.Shape);
	crosshair_binders->insert<int> (&shapeWidget, &shapePref);
	table->dual_add(shape_w->label("Shape"), d);
	table->dual_add(shape_w, d);

	table->add_row(new w_spacer(), true);

	// Thickness
	w_slider* thickness_w = new w_slider(7, 0);
	SliderSelectorWidget thicknessWidget(thickness_w);
	CrosshairPref thicknessPref(player_preferences->Crosshairs.Thickness);
	crosshair_binders->insert<int> (&thicknessWidget, &thicknessPref);
	table->dual_add(thickness_w->label("Width"), d);
	table->dual_add(thickness_w, d);

	// From Center
	w_slider *from_center_w = new w_slider(15, 0);
	SliderSelectorWidget fromCenterWidget(from_center_w);
	Int16Pref fromCenterPref(player_preferences->Crosshairs.FromCenter);
	crosshair_binders->insert<int> (&fromCenterWidget, &fromCenterPref);
	table->dual_add(from_center_w->label("Gap"), d);
	table->dual_add(from_center_w, d);

	// Length
	w_slider *length_w = new w_slider(15, 0);
	SliderSelectorWidget lengthWidget(length_w);
	CrosshairPref lengthPref(player_preferences->Crosshairs.Length);
	crosshair_binders->insert<int> (&lengthWidget, &lengthPref);
	table->dual_add(length_w->label("Size"), d);
	table->dual_add(length_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("Color"), d);

	// Color
	w_slider *red_w = new w_slider(16, 0);
	SliderSelectorWidget redWidget(red_w);
	ColorComponentPref redPref(player_preferences->Crosshairs.Color.red);
	crosshair_binders->insert<int> (&redWidget, &redPref);
	table->dual_add(red_w->label("Red"), d);
	table->dual_add(red_w, d);

	w_slider *green_w = new w_slider(16, 0);
	SliderSelectorWidget greenWidget(green_w);;
	ColorComponentPref greenPref(player_preferences->Crosshairs.Color.green);
	crosshair_binders->insert<int> (&greenWidget, &greenPref);
	table->dual_add(green_w->label("Green"), d);
	table->dual_add(green_w, d);

	w_slider *blue_w = new w_slider(16, 0);
	SliderSelectorWidget blueWidget(blue_w);
	ColorComponentPref bluePref(player_preferences->Crosshairs.Color.blue);
	crosshair_binders->insert<int> (&blueWidget, &bluePref);
	table->dual_add(blue_w->label("Blue"), d);
	table->dual_add(blue_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("OpenGL Only (no preview)"), d);

	w_slider *opacity_w = new w_slider(16, 0);
	SliderSelectorWidget opacityWidget(opacity_w);
	OpacityPref opacityPref(player_preferences->Crosshairs.Opacity);
	crosshair_binders->insert<int> (&opacityWidget, &opacityPref);
	table->dual_add(opacity_w->label("Opacity"), d);
	table->dual_add(opacity_w, d);

	placer->add(table, true);
	placer->add(new w_spacer, true);

	horizontal_placer *button_placer = new horizontal_placer;
	w_button *w_accept = new w_button("ACCEPT", dialog_ok, &d);
	button_placer->dual_add(w_accept, d);
	w_button *w_cancel = new w_button("CANCEL", dialog_cancel, &d);
	button_placer->dual_add(w_cancel, d);
	placer->add(button_placer, true);

	d.set_widget_placer(placer);
	d.set_processing_function(update_crosshair_display());

	crosshair_binders->migrate_all_second_to_first();

	clear_screen();

	if (d.run() == 0) // Accepted
	{
		crosshair_binders->migrate_all_first_to_second();
		player_preferences->Crosshairs.PreCalced = false;
		player_preferences->crosshairs_active = crosshairs_active_w->get_selection();
		write_preferences();
	}
	else
	{
		player_preferences->Crosshairs = OldCrosshairs;
	}

	crosshair_binders.reset(0);
}

/*
 *  Player dialog
 */

enum {
	NAME_W
};

static void player_dialog(void *arg)
{
	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("PLAYER SETTINGS"), d);
	placer->add(new w_spacer());

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table->col_flags(0, placeable::kAlignRight);
	table->col_flags(1, placeable::kAlignLeft);

	w_select *level_w = new w_select(player_preferences->difficulty_level, NULL /*level_labels*/);
	level_w->set_labels_stringset(kDifficultyLevelsStringSetID);
	table->dual_add(level_w->label("Difficulty"), d);
	table->dual_add(level_w, d);

	table->add_row(new w_spacer(), true);

	table->dual_add_row(new w_static_text("Appearance"), d);

	w_text_entry *name_w = new w_text_entry(PREFERENCES_NAME_LENGTH, "");
	name_w->set_identifier(NAME_W);
	name_w->set_enter_pressed_callback(dialog_try_ok);
	name_w->set_value_changed_callback(dialog_disable_ok_if_empty);
	name_w->enable_mac_roman_input();
	table->dual_add(name_w->label("Name"), d);
	table->dual_add(name_w, d);

	w_player_color *pcolor_w = new w_player_color(player_preferences->color);
	table->dual_add(pcolor_w->label("Color"), d);
	table->dual_add(pcolor_w, d);

	w_player_color *tcolor_w = new w_player_color(player_preferences->team);
	table->dual_add(tcolor_w->label("Team"), d);
	table->dual_add(tcolor_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("\322Find Internet Game\323 Server"), d);

	w_enabling_toggle *login_as_guest_w = new w_enabling_toggle(strcmp(network_preferences->metaserver_login, "guest") == 0, false);
	table->dual_add(login_as_guest_w->label("Guest"), d);
	table->dual_add(login_as_guest_w, d);

	w_text_entry *login_w = new w_text_entry(network_preferences_data::kMetaserverLoginLength, network_preferences->metaserver_login);
	table->dual_add(login_w->label("Login"), d);
	table->dual_add(login_w, d);

	w_password_entry *password_w = new w_password_entry(network_preferences_data::kMetaserverLoginLength, network_preferences->metaserver_password);
	table->dual_add(password_w->label("Password"), d);
	table->dual_add(password_w, d);
	w_toggle *mute_guests_w = new w_toggle(network_preferences->mute_metaserver_guests);
	table->dual_add(mute_guests_w->label("Mute All Guest Chat"), d);
	table->dual_add(mute_guests_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("Custom Internet Chat Colors"), d);
	w_enabling_toggle *custom_colors_w = new w_enabling_toggle(network_preferences->use_custom_metaserver_colors);
	table->dual_add(custom_colors_w->label("Use Custom Colors"), d);
	table->dual_add(custom_colors_w, d);
	
	w_color_picker *primary_w = new w_color_picker(network_preferences->metaserver_colors[0]);
	table->dual_add(primary_w->label("Primary"), d);
	table->dual_add(primary_w, d);
	
	w_color_picker *secondary_w = new w_color_picker(network_preferences->metaserver_colors[1]);
	table->dual_add(secondary_w->label("Secondary"), d);
	table->dual_add(secondary_w, d);

	custom_colors_w->add_dependent_widget(primary_w);
	custom_colors_w->add_dependent_widget(secondary_w);

	login_as_guest_w->add_dependent_widget(login_w);
	login_as_guest_w->add_dependent_widget(password_w);
	login_as_guest_w->add_dependent_widget(mute_guests_w);

	placer->add(table, true);

	placer->add(new w_spacer(), true);

	w_button *crosshair_button = new w_button("CROSSHAIR", crosshair_dialog, &d);
	placer->dual_add(crosshair_button, d);

	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	
	w_button* ok_button = new w_button("ACCEPT", dialog_ok, &d);
	ok_button->set_identifier(iOK);
	button_placer->dual_add(ok_button, d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	// We don't do this earlier because it (indirectly) invokes the name_typing callback, which needs iOK
	copy_pstring_to_text_field(&d, NAME_W, player_preferences->name);

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		const char *name = name_w->get_text();
		unsigned char theOldNameP[PREFERENCES_NAME_LENGTH+1];
		pstrncpy(theOldNameP, player_preferences->name, PREFERENCES_NAME_LENGTH+1);
		char *theOldName = a1_p2cstr(theOldNameP);
		if (strcmp(name, theOldName)) {
			copy_pstring_from_text_field(&d, NAME_W, player_preferences->name);
			changed = true;
		}

		const char *metaserver_login = (login_as_guest_w->get_selection() != 0) ? "guest" : login_w->get_text();
		if (strcmp(metaserver_login, network_preferences->metaserver_login)) {
			strncpy(network_preferences->metaserver_login, metaserver_login, network_preferences_data::kMetaserverLoginLength);
			changed = true;
		}

		const char *metaserver_password = (login_as_guest_w->get_selection() != 0) ? "" : password_w->get_text();
		if (strcmp(metaserver_password, network_preferences->metaserver_password)) {
			strncpy(network_preferences->metaserver_password, metaserver_password, network_preferences_data::kMetaserverLoginLength);
			changed = true;
		}

		bool mute_metaserver_guests = (login_as_guest_w->get_selection() != 0) ? false : mute_guests_w->get_selection() == 1;
		if (mute_metaserver_guests != network_preferences->mute_metaserver_guests)
		{
			network_preferences->mute_metaserver_guests = mute_metaserver_guests;
			changed = true;
		}

		bool use_custom_metaserver_colors = custom_colors_w->get_selection();
		if (use_custom_metaserver_colors != network_preferences->use_custom_metaserver_colors)
		{
			network_preferences->use_custom_metaserver_colors = use_custom_metaserver_colors;
			changed = true;
		}

		if (use_custom_metaserver_colors)
		{
			rgb_color primary_color = primary_w->get_selection();
			if (primary_color.red != network_preferences->metaserver_colors[0].red || primary_color.green != network_preferences->metaserver_colors[0].green || primary_color.blue != network_preferences->metaserver_colors[0].blue)
			{
				network_preferences->metaserver_colors[0] = primary_color;
				changed = true;
			}

			rgb_color secondary_color = secondary_w->get_selection();
			if (secondary_color.red != network_preferences->metaserver_colors[1].red || secondary_color.green != network_preferences->metaserver_colors[1].green || secondary_color.blue != network_preferences->metaserver_colors[1].blue)			{
				network_preferences->metaserver_colors[1] = secondary_color;
				changed = true;
			}

		}
					
		int16 level = static_cast<int16>(level_w->get_selection());
		assert(level >= 0);
		if (level != player_preferences->difficulty_level) {
			player_preferences->difficulty_level = level;
			changed = true;
		}

		int16 color = static_cast<int16>(pcolor_w->get_selection());
		assert(color >= 0);
		if (color != player_preferences->color) {
			player_preferences->color = color;
			changed = true;
		}

		int16 team = static_cast<int16>(tcolor_w->get_selection());
		assert(team >= 0);
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

#ifdef TRUE_COLOR_ONLY
static const char* depth_labels[3] = {
	"16 Bit", "32 Bit", NULL
};
#else
static const char *depth_labels[4] = {
	"8 Bit", "16 Bit", "32 Bit", NULL
};
#endif

static const char *resolution_labels[3] = {
	"Low", "High", NULL
};

static const char *sw_alpha_blending_labels[4] = {
	"Off", "Fast", "Nice", NULL
};

static const char *gamma_labels[9] = {
	"Darkest", "Darker", "Dark", "Normal", "Light", "Really Light", "Even Lighter", "Lightest", NULL
};

static const char* renderer_labels[] = {
	"Software", "OpenGL (Classic)", "OpenGL (Shader)", NULL
};

static const char* hud_scale_labels[] = {
"Normal", "Double", "Largest", NULL
};

static const char* term_scale_labels[] = {
"Normal", "Double", "Largest", NULL
};

static const char* max_saves_labels[] = {
	"20", "100", "500", "Unlimited", NULL
};
static const uint32 max_saves_values[] = {
	20, 100, 500, 0
};


enum {
    iRENDERING_SYSTEM = 1000
};

static const vector<string> build_stringvector_from_cstring_array (const char** label_array)
{
	std::vector<std::string> label_vector;
	for (int i = 0; label_array[i] != NULL; ++i)
		label_vector.push_back(std::string(label_array[i]));
		
	return label_vector;
}


static void software_rendering_options_dialog(void* arg)
{
	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("SOFTWARE RENDERING OPTIONS"), d);
	placer->add(new w_spacer(), true);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table->col_flags(0, placeable::kAlignRight);

#ifdef TRUE_COLOR_ONLY
	w_select *depth_w = new w_select(graphics_preferences->screen_mode.bit_depth == 16 ? 0 : 1, depth_labels);
#else
	w_select *depth_w = new w_select(graphics_preferences->screen_mode.bit_depth == 8 ? 0 : graphics_preferences->screen_mode.bit_depth == 16 ? 1 : 2, depth_labels);
#endif
	table->dual_add(depth_w->label("Color Depth"), d);
	table->dual_add(depth_w, d);

	w_toggle *resolution_w = new w_toggle(graphics_preferences->screen_mode.high_resolution, resolution_labels);
	table->dual_add(resolution_w->label("Resolution"), d);
	table->dual_add(resolution_w, d);

	table->add_row(new w_spacer(), true);

	w_select *sw_alpha_blending_w = new w_select(graphics_preferences->software_alpha_blending, sw_alpha_blending_labels);
	table->dual_add(sw_alpha_blending_w->label("Transparent Liquids"), d);
	table->dual_add(sw_alpha_blending_w, d);

	placer->add(table, true);

	placer->add(new w_spacer(), true);
	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("ACCEPT", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);
	placer->add(button_placer, true);

	d.set_widget_placer(placer);
	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

#ifdef TRUE_COLOR_ONLY
		int depth = (depth_w->get_selection() == 0 ? 16 : 32);
#else
		int depth = (depth_w->get_selection() == 0 ? 8 : depth_w->get_selection() == 1 ? 16 : 32);
#endif
		if (depth != graphics_preferences->screen_mode.bit_depth) {
			graphics_preferences->screen_mode.bit_depth = depth;
			changed = true;
			// don't change mode now; it will be changed when the game starts
		}

		bool hi_res = resolution_w->get_selection() != 0;
		if (hi_res != graphics_preferences->screen_mode.high_resolution) {
			graphics_preferences->screen_mode.high_resolution = hi_res;
			changed = true;
		}

		if (sw_alpha_blending_w->get_selection() != graphics_preferences->software_alpha_blending)
		{
			graphics_preferences->software_alpha_blending = sw_alpha_blending_w->get_selection();
			changed = true;
		}

		if (changed)
			write_preferences();
	}
}

// ZZZ addition: bounce to correct renderer-config box based on selected rendering system.
static void rendering_options_dialog_demux(void* arg)
{
	int theSelectedRenderer = get_selection_control_value((dialog*) arg, iRENDERING_SYSTEM) - 1;

	switch(theSelectedRenderer) {
		case _no_acceleration:
			software_rendering_options_dialog(arg);
			break;

		case _opengl_acceleration:
		case _shader_acceleration:
			OpenGLDialog::Create (theSelectedRenderer)->OpenGLPrefsByRunning ();
			break;

		default:
			assert(false);
			break;
	}
}

std::vector<std::string> build_resolution_labels()
{
	std::vector<std::string> result;
	bool first_mode = true;
	for (std::vector<std::pair<int, int> >::const_iterator it = Screen::instance()->GetModes().begin(); it != Screen::instance()->GetModes().end(); ++it)
	{
		ostringstream os;
		os << it->first << "x" << it->second;
		if (first_mode)
		{
			result.push_back("Automatic");
			first_mode = false;
		}
		result.push_back(os.str());
	}

	return result;
}

static void graphics_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;

	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("GRAPHICS SETUP"), d);
	placer->add(new w_spacer(), true);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table->col_flags(0, placeable::kAlignRight);
	
	w_select* renderer_w = new w_select(graphics_preferences->screen_mode.acceleration, renderer_labels);
	renderer_w->set_identifier(iRENDERING_SYSTEM);
#ifndef HAVE_OPENGL
	renderer_w->set_selection(_no_acceleration);
	renderer_w->set_enabled(false);
#endif
	table->dual_add(renderer_w->label("Rendering System"), d);
	table->dual_add(renderer_w, d);

	w_select_popup *size_w = new w_select_popup();
	size_w->set_labels(build_resolution_labels());
	if (graphics_preferences->screen_mode.auto_resolution)
		size_w->set_selection(0);
	else
		size_w->set_selection(Screen::instance()->FindMode(graphics_preferences->screen_mode.width, graphics_preferences->screen_mode.height) + 1);
	table->dual_add(size_w->label("Screen Size"), d);
	table->dual_add(size_w, d);

	w_toggle *fill_screen_w;
	const SDL_version *version = SDL_Linked_Version();
	if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
	{
		fill_screen_w = new w_toggle(graphics_preferences->screen_mode.fill_the_screen);
		table->dual_add(fill_screen_w->label("Fill the Screen"), d);
		table->dual_add(fill_screen_w, d);
	}

	w_toggle *fixh_w = new w_toggle(!graphics_preferences->screen_mode.fix_h_not_v);
	table->dual_add(fixh_w->label("Limit Vertical View"), d);
	table->dual_add(fixh_w, d);
    
	w_select_popup *gamma_w = new w_select_popup();
	gamma_w->set_labels(build_stringvector_from_cstring_array(gamma_labels));
	gamma_w->set_selection(graphics_preferences->screen_mode.gamma_level);
	table->dual_add(gamma_w->label("Brightness"), d);
	table->dual_add(gamma_w, d);

	table->add_row(new w_spacer(), true);

	w_toggle *fullscreen_w = new w_toggle(!graphics_preferences->screen_mode.fullscreen);
	table->dual_add(fullscreen_w->label("Windowed Mode"), d);
	table->dual_add(fullscreen_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("Heads-Up Display"), d);
	w_enabling_toggle *hud_w = new w_enabling_toggle(graphics_preferences->screen_mode.hud);
	table->dual_add(hud_w->label("Show HUD"), d);
	table->dual_add(hud_w, d);
	
	w_select_popup *hud_scale_w = new w_select_popup();
	hud_scale_w->set_labels(build_stringvector_from_cstring_array(hud_scale_labels));
	hud_scale_w->set_selection(graphics_preferences->screen_mode.hud_scale_level);
	table->dual_add(hud_scale_w->label("HUD Size"), d);
	table->dual_add(hud_scale_w, d);
	hud_w->add_dependent_widget(hud_scale_w);
	
	w_select_popup *term_scale_w = new w_select_popup();
	term_scale_w->set_labels(build_stringvector_from_cstring_array(term_scale_labels));
	term_scale_w->set_selection(graphics_preferences->screen_mode.term_scale_level);
	table->dual_add(term_scale_w->label("Terminal Size"), d);
	table->dual_add(term_scale_w, d);
	
	w_toggle *map_w = new w_toggle(graphics_preferences->screen_mode.translucent_map);
	table->dual_add(map_w->label("Overlay Map"), d);
	table->dual_add(map_w, d);

	placer->add(table, true);

	placer->add(new w_spacer(), true);
	placer->dual_add(new w_button("RENDERING OPTIONS", rendering_options_dialog_demux, &d), d);
	placer->add(new w_spacer(), true);

#ifndef HAVE_OPENGL
	expand_app_variables(temporary, "This copy of $appName$ was built without OpenGL support.");
	placer->dual_add(new w_static_text(temporary), d);
#endif
	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("ACCEPT", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	placer->add(button_placer, true);
    
	d.set_widget_placer(placer);
	
	// Clear screen
	clear_screen();
    
    // Run dialog
    if (d.run() == 0) {	// Accepted
	    bool changed = false;
	    
	    bool fullscreen = fullscreen_w->get_selection() == 0;
	    if (fullscreen != graphics_preferences->screen_mode.fullscreen) {
		    graphics_preferences->screen_mode.fullscreen = fullscreen;
		    changed = true;
	    }

	    short renderer = static_cast<short>(renderer_w->get_selection());
	    assert(renderer >= 0);
	    if(renderer != graphics_preferences->screen_mode.acceleration) {
		    graphics_preferences->screen_mode.acceleration = renderer;
		    if (renderer) graphics_preferences->screen_mode.bit_depth = 32;
		    changed = true;
	    }
	    
	    short resolution = static_cast<short>(size_w->get_selection());
		if (resolution == 0)
		{
			if (!graphics_preferences->screen_mode.auto_resolution) {
				graphics_preferences->screen_mode.auto_resolution = true;
				changed = true;
			}
		}
	    else if (Screen::instance()->ModeWidth(resolution - 1) != graphics_preferences->screen_mode.width || Screen::instance()->ModeHeight(resolution - 1) != graphics_preferences->screen_mode.height || graphics_preferences->screen_mode.auto_resolution)
	    {
		    graphics_preferences->screen_mode.width = Screen::instance()->ModeWidth(resolution - 1);
		    graphics_preferences->screen_mode.height = Screen::instance()->ModeHeight(resolution - 1);
			graphics_preferences->screen_mode.auto_resolution = false;
		    changed = true;
	    }
	    
	    short gamma = static_cast<short>(gamma_w->get_selection());
	    if (gamma != graphics_preferences->screen_mode.gamma_level) {
		    graphics_preferences->screen_mode.gamma_level = gamma;
		    changed = true;
	    }
        
        bool fix_h_not_v = fixh_w->get_selection() == 0;
        if (fix_h_not_v != graphics_preferences->screen_mode.fix_h_not_v) {
            graphics_preferences->screen_mode.fix_h_not_v = fix_h_not_v;
            changed = true;
        }

	    bool hud = hud_w->get_selection() != 0;
	    if (hud != graphics_preferences->screen_mode.hud)
	    {
		    graphics_preferences->screen_mode.hud = hud;
		    changed = true;
	    }
	    
	    short hud_scale = static_cast<short>(hud_scale_w->get_selection());
	    if (hud_scale != graphics_preferences->screen_mode.hud_scale_level)
	    {
		    graphics_preferences->screen_mode.hud_scale_level = hud_scale;
		    changed = true;
	    }
	    
	    short term_scale = static_cast<short>(term_scale_w->get_selection());
	    if (term_scale != graphics_preferences->screen_mode.term_scale_level)
	    {
		    graphics_preferences->screen_mode.term_scale_level = term_scale;
		    changed = true;
	    }
		
		bool translucent_map = map_w->get_selection() != 0;
		if (translucent_map != graphics_preferences->screen_mode.translucent_map) {
			graphics_preferences->screen_mode.translucent_map = translucent_map;
			changed = true;
		}
	    
	    const SDL_version *version = SDL_Linked_Version();
	    if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
	    {
		    bool fill_the_screen = fill_screen_w->get_selection() != 0;
		    if (fill_the_screen != graphics_preferences->screen_mode.fill_the_screen) {
			    graphics_preferences->screen_mode.fill_the_screen = fill_the_screen;
			    changed = true;
		    }
	    }

	    if (changed) {
		    write_preferences();
		    change_screen_mode(&graphics_preferences->screen_mode, true);
		    clear_screen(true);
		    parent->layout();
		    parent->draw();		// DirectX seems to need this
	    }
    }
}

/*
 *  Sound dialog
 */

class w_toggle *stereo_w, *dynamic_w;

class w_stereo_toggle : public w_toggle {
public:
	w_stereo_toggle(bool selection) : w_toggle(selection) {}

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
	w_dynamic_toggle(bool selection) : w_toggle(selection) {}

	void selection_changed(void)
	{
		// Turning on dynamic tracking turns on stereo
		w_toggle::selection_changed();
		if (selection == true)
			stereo_w->set_selection(true);
	}
};

static const char *channel_labels[] = {"1", "2", "4", "8", "16", "32", NULL};

class w_volume_slider : public w_slider {
public:
	w_volume_slider(int vol) : w_slider(NUMBER_OF_SOUND_VOLUME_LEVELS, vol) {}
	~w_volume_slider() {}

	void item_selected(void)
	{
		SoundManager::instance()->TestVolume(selection, _snd_adjust_volume);
	}
};

static void sound_dialog(void *arg)
{
	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("SOUND SETUP"), d);
	placer->add(new w_spacer(), true);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table->col_flags(0, placeable::kAlignRight);

	static const char *quality_labels[3] = {"8 Bit", "16 Bit", NULL};
	w_toggle *quality_w = new w_toggle(TEST_FLAG(sound_preferences->flags, _16bit_sound_flag), quality_labels);
	table->dual_add(quality_w->label("Quality"), d);
	table->dual_add(quality_w, d);

	stereo_w = new w_stereo_toggle(sound_preferences->flags & _stereo_flag);
	table->dual_add(stereo_w->label("Stereo"), d);
	table->dual_add(stereo_w, d);

	dynamic_w = new w_dynamic_toggle(TEST_FLAG(sound_preferences->flags, _dynamic_tracking_flag));
	table->dual_add(dynamic_w->label("Active Panning"), d);
	table->dual_add(dynamic_w, d);

	w_toggle *ambient_w = new w_toggle(TEST_FLAG(sound_preferences->flags, _ambient_sound_flag));
	table->dual_add(ambient_w->label("Ambient Sounds"), d);
	table->dual_add(ambient_w, d);

	w_toggle *more_w = new w_toggle(TEST_FLAG(sound_preferences->flags, _more_sounds_flag));
	table->dual_add(more_w->label("More Sounds"), d);
	table->dual_add(more_w, d);

	w_toggle *button_sounds_w = new w_toggle(TEST_FLAG(input_preferences->modifiers, _inputmod_use_button_sounds));
	table->dual_add(button_sounds_w->label("Interface Button Sounds"), d);
	table->dual_add(button_sounds_w, d);

	w_select *channels_w = new w_select(static_cast<int>(std::floor(std::log(static_cast<float>(sound_preferences->channel_count)) / std::log(2.0) + 0.5)), channel_labels);
	table->dual_add(channels_w->label("Channels"), d);
	table->dual_add(channels_w, d);

	w_volume_slider *volume_w = new w_volume_slider(sound_preferences->volume);
	table->dual_add(volume_w->label("Volume"), d);
	table->dual_add(volume_w, d);

	w_slider *music_volume_w = new w_slider(NUMBER_OF_SOUND_VOLUME_LEVELS, sound_preferences->music);
	table->dual_add(music_volume_w->label("Music Volume"), d);
	table->dual_add(music_volume_w, d);


	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("Network Microphone"), d);

	w_toggle* mute_while_transmitting_w = new w_toggle(!sound_preferences->mute_while_transmitting);
	table->dual_add(mute_while_transmitting_w->label("Headset Mic Mode"), d);
	table->dual_add(mute_while_transmitting_w, d);

	table->add_row(new w_spacer(), true);
	table->dual_add_row(new w_static_text("Experimental Sound Options"), d);
		w_toggle *zrd_w = new w_toggle(TEST_FLAG(sound_preferences->flags, _zero_restart_delay));
	table->dual_add(zrd_w->label("Zero Restart Delay"), d);
	table->dual_add(zrd_w, d);

	placer->add(table, true);

	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("ACCEPT", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);
	
	placer->add(button_placer, true);

	d.set_widget_placer(placer);
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
		if (zrd_w->get_selection()) flags |= _zero_restart_delay;

		if (flags != sound_preferences->flags) {
			sound_preferences->flags = flags;
			changed = true;
		}

		flags = input_preferences->modifiers & ~_inputmod_use_button_sounds;
		if (button_sounds_w->get_selection()) flags |= _inputmod_use_button_sounds;
		if (flags != input_preferences->modifiers) {
			input_preferences->modifiers = flags;
			changed = true;
		}

		int16 channel_count = 1 << channels_w->get_selection();
		if (channel_count != sound_preferences->channel_count) {
			sound_preferences->channel_count = channel_count;
			changed = true;
		}

		int volume = volume_w->get_selection();
		if (volume != sound_preferences->volume) {
			sound_preferences->volume = volume;
			changed = true;
		}

		int music_volume = music_volume_w->get_selection();
		if (music_volume != sound_preferences->music) {
			sound_preferences->music = music_volume;
			changed = true;
		}

		bool mute_while_transmitting = !mute_while_transmitting_w->get_selection();
		if (mute_while_transmitting != sound_preferences->mute_while_transmitting)
		{
			sound_preferences->mute_while_transmitting = mute_while_transmitting;
			changed = true;
		}

		if (changed) {
//			set_sound_manager_parameters(sound_preferences);
			SoundManager::instance()->SetParameters(*sound_preferences);
			write_preferences();
		}
	}
}


/*
 *  Controls dialog
 */

w_select_popup* joystick_axis_w[NUMBER_OF_JOYSTICK_MAPPINGS];

static void axis_mapped(void* pv)
{
	w_select_popup* w = reinterpret_cast<w_select_popup*>(pv);
	for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
	{
		if (w != joystick_axis_w[i] && w->get_selection() == joystick_axis_w[i]->get_selection()) 
		{
			joystick_axis_w[i]->set_selection(0);
		}
	}
}

static w_enabling_toggle *mouse_w;
static w_enabling_toggle *joystick_w;

static void input_selected(w_select* w)
{
	if (w == mouse_w && w->get_selection())
		joystick_w->set_selection(0, true);
	else if (w == joystick_w && w->get_selection())
		mouse_w->set_selection(0, true);
}

static void controls_dialog(void *arg)
{
	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("CONTROLS"), d);
	placer->add(new w_spacer(), true);

	tab_placer* tabs = new tab_placer();

	std::vector<std::string> labels;
	labels.push_back("GENERAL");
	labels.push_back("MOUSE");
	labels.push_back("JOYSTICK");
	w_tab *tab_w = new w_tab(labels, tabs);

	placer->dual_add(tab_w, d);
	placer->add(new w_spacer(), true);

	vertical_placer *general = new vertical_placer();
	table_placer* mouse = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table_placer* joystick = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	mouse->col_flags(0, placeable::kAlignRight);
	joystick->col_flags(0, placeable::kAlignRight);

	mouse_w = new w_enabling_toggle(input_preferences->input_device == 1, true);
	mouse_w->set_selection_changed_callback(input_selected);
	mouse->dual_add(mouse_w->label("Use Mouse"), d);
	mouse->dual_add(mouse_w, d);

	mouse->add_row(new w_spacer(), true);

	w_toggle *mouse_acceleration_w = new w_toggle(input_preferences->mouse_acceleration);
	mouse->dual_add(mouse_acceleration_w->label("Accelerate Mouse"), d);
	mouse->dual_add(mouse_acceleration_w, d);

	mouse_w->add_dependent_widget(mouse_acceleration_w);

	w_toggle *invert_mouse_w = new w_toggle(TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse));
	mouse->dual_add(invert_mouse_w->label("Invert Mouse"), d);
	mouse->dual_add(invert_mouse_w, d);
	
	mouse_w->add_dependent_widget(invert_mouse_w);

	const float kMinSensitivityLog = -3.0f;
	const float kMaxSensitivityLog = 3.0f;
	const float kSensitivityLogRange = kMaxSensitivityLog - kMinSensitivityLog;
	
	// LP: split this into horizontal and vertical sensitivities
	float theSensitivity, theSensitivityLog;
	
	theSensitivity = ((float) input_preferences->sens_vertical) / FIXED_ONE;
	if (theSensitivity <= 0.0f) theSensitivity = 1.0f;
	theSensitivityLog = std::log(theSensitivity);
	int theVerticalSliderPosition =
		(int) ((theSensitivityLog - kMinSensitivityLog) * (1000.0f / kSensitivityLogRange));
	
	w_slider* sens_vertical_w = new w_slider(1000, theVerticalSliderPosition);
	mouse->dual_add(sens_vertical_w->label("Mouse Vertical Sensitivity"), d);
	mouse->dual_add(sens_vertical_w, d);

	mouse_w->add_dependent_widget(sens_vertical_w);
	
	theSensitivity = ((float) input_preferences->sens_horizontal) / FIXED_ONE;
	if (theSensitivity <= 0.0f) theSensitivity = 1.0f;
	theSensitivityLog = std::log(theSensitivity);
	int theHorizontalSliderPosition =
		(int) ((theSensitivityLog - kMinSensitivityLog) * (1000.0f / kSensitivityLogRange));

	w_slider* sens_horizontal_w = new w_slider(1000, theHorizontalSliderPosition);
	mouse->dual_add(sens_horizontal_w->label("Mouse Horizontal Sensitivity"), d);
	mouse->dual_add(sens_horizontal_w, d);

	mouse_w->add_dependent_widget(sens_horizontal_w);

	joystick_w = new w_enabling_toggle(input_preferences->input_device == 0 && SDL_NumJoysticks() > 0 && input_preferences->joystick_id >= 0, true);
	joystick_w->set_selection_changed_callback(input_selected);
	joystick->dual_add(joystick_w->label("Use Joystick / Gamepad"), d);
	joystick->dual_add(joystick_w, d);

	joystick->add_row(new w_spacer(), true);

	std::vector<std::string> joystick_labels;
	if (SDL_NumJoysticks()) 
	{
		for (int i = 0; i < SDL_NumJoysticks(); ++i)
		{
			joystick_labels.push_back(SDL_JoystickName(i));
		}
	}
	else
	{
		joystick_labels.push_back("No Joysticks");
	}
	w_select_popup* which_joystick_w = new w_select_popup();
	which_joystick_w->set_labels(joystick_labels);
	which_joystick_w->set_selection(std::max(0, std::min(SDL_NumJoysticks(), static_cast<int>(input_preferences->joystick_id))));
	joystick->dual_add(which_joystick_w->label("Joystick / Gamepad"), d);
	joystick->dual_add(which_joystick_w, d);
	joystick_w->add_dependent_widget(which_joystick_w);

	joystick->add_row(new w_spacer(), true);
	joystick->dual_add_row(new w_static_text("Axis Mappings"), d);

	std::vector<std::string> axis_labels;
	axis_labels.push_back("Unassigned");
	for (int i = 1; i <= 8; ++i)
	{
		stringstream s;
		s << "Axis " << i;
		axis_labels.push_back(s.str());
	}
	for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
	{
		joystick_axis_w[i] = new w_select_popup();
		joystick_axis_w[i]->set_labels(axis_labels);
		joystick_axis_w[i]->set_selection(input_preferences->joystick_axis_mappings[i] + 1);
		joystick_axis_w[i]->set_popup_callback(axis_mapped, joystick_axis_w[i]);
	}

	joystick->dual_add(joystick_axis_w[_joystick_strafe]->label("Sidestep Left/Right"), d);
	joystick->dual_add(joystick_axis_w[_joystick_strafe], d);

	joystick->dual_add(joystick_axis_w[_joystick_velocity]->label("Move Forward/Backward"), d);
	joystick->dual_add(joystick_axis_w[_joystick_velocity], d);

	joystick->dual_add(joystick_axis_w[_joystick_yaw]->label("Turn Left/Right"), d);
	joystick->dual_add(joystick_axis_w[_joystick_yaw], d);

	joystick->dual_add(joystick_axis_w[_joystick_pitch]->label("Look Up/Down"), d);
	joystick->dual_add(joystick_axis_w[_joystick_pitch], d);

	for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
	{
		joystick_w->add_dependent_widget(joystick_axis_w[i]);
	}

	joystick->add_row(new w_spacer(), true);

	table_placer* general_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	general_table->col_flags(0, placeable::kAlignRight);

	w_toggle *always_run_w = new w_toggle(input_preferences->modifiers & _inputmod_interchange_run_walk);
	general_table->dual_add(always_run_w->label("Always Run"), d);
	general_table->dual_add(always_run_w, d);

	w_toggle *always_swim_w = new w_toggle(TEST_FLAG(input_preferences->modifiers, _inputmod_interchange_swim_sink));
	general_table->dual_add(always_swim_w->label("Always Swim"), d);
	general_table->dual_add(always_swim_w, d);

	general_table->add_row(new w_spacer(), true);

	w_toggle *weapon_w = new w_toggle(!(input_preferences->modifiers & _inputmod_dont_switch_to_new_weapon));
	general_table->dual_add(weapon_w->label("Auto-Switch Weapons"), d);
	general_table->dual_add(weapon_w, d);

	w_toggle* auto_recenter_w = new w_toggle(!(input_preferences->modifiers & _inputmod_dont_auto_recenter));
	general_table->dual_add(auto_recenter_w->label("Auto-Recenter View"), d);
	general_table->dual_add(auto_recenter_w, d);

	general->add(general_table, true);

	general->add(new w_spacer(), true);
	general->dual_add(new w_static_text("Warning: Auto-Switch Weapons and Auto-Recenter View"), d);
	general->dual_add(new w_static_text("are always ON in network play.  Turning either one OFF"), d);
	general->dual_add(new w_static_text("will also disable film recording for single-player games."), d);
		
	tabs->add(general, true);
	tabs->add(mouse, true);
	tabs->add(joystick, true);

	placer->add(tabs, true);

	placer->add(new w_spacer(), true);
	placer->add(new w_spacer(), true);
	placer->dual_add(new w_button("CONFIGURE KEYS / BUTTONS", keyboard_dialog, &d), d);

	placer->add(new w_spacer(), true);
	
	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("ACCEPT", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);
	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	// Clear screen
	clear_screen();

	// Run dialog
	if (d.run() == 0) {	// Accepted
		bool changed = false;

		int16 device = static_cast<int16>(mouse_w->get_selection());
		if (device != input_preferences->input_device) {
			input_preferences->input_device = device;
			changed = true;
		}

		// LP: split the sensitivity into vertical and horizontal sensitivities
		float theNewSensitivityLog;
		int theNewSliderPosition;
		
		theNewSliderPosition = sens_vertical_w->get_selection();
        if(theNewSliderPosition != theVerticalSliderPosition) {
            theNewSensitivityLog = kMinSensitivityLog + ((float) theNewSliderPosition) * (kSensitivityLogRange / 1000.0f);
            input_preferences->sens_vertical = _fixed(std::exp(theNewSensitivityLog) * FIXED_ONE);
            changed = true;
        }
		
		theNewSliderPosition = sens_horizontal_w->get_selection();
        if(theNewSliderPosition != theHorizontalSliderPosition) {
            theNewSensitivityLog = kMinSensitivityLog + ((float) theNewSliderPosition) * (kSensitivityLogRange / 1000.0f);
            input_preferences->sens_horizontal = _fixed(std::exp(theNewSensitivityLog) * FIXED_ONE);
            changed = true;
        }

/*
        int theNewSliderPosition = sensitivity_w->get_selection();
        if(theNewSliderPosition != theSliderPosition) {
            float theNewSensitivityLog = kMinSensitivityLog + ((float) theNewSliderPosition) * (kSensitivityLogRange / 1000.0f);
            input_preferences->sensitivity = _fixed(std::exp(theNewSensitivityLog) * FIXED_ONE);
            changed = true;
        }
*/
		uint16 flags = input_preferences->modifiers & _inputmod_use_button_sounds;
		if (always_run_w->get_selection()) flags |= _inputmod_interchange_run_walk;
		if (always_swim_w->get_selection()) flags |= _inputmod_interchange_swim_sink;
		if (!(weapon_w->get_selection())) flags |= _inputmod_dont_switch_to_new_weapon;
		if (!(auto_recenter_w->get_selection())) flags |= _inputmod_dont_auto_recenter;
		if (invert_mouse_w->get_selection()) flags |= _inputmod_invert_mouse;

		if (flags != input_preferences->modifiers) {
			input_preferences->modifiers = flags;
			changed = true;
		}

		if (mouse_acceleration_w->get_selection() != input_preferences->mouse_acceleration)
		{
			input_preferences->mouse_acceleration = mouse_acceleration_w->get_selection();
			changed = true;
		}

		if (joystick_w->get_selection())
		{
			if (which_joystick_w->get_selection() != input_preferences->joystick_id)
			{
				input_preferences->joystick_id = which_joystick_w->get_selection();
				changed = true;
			}
		}
		else if (input_preferences->joystick_id != -1)
		{
			input_preferences->joystick_id = -1;
			changed = true;
		}

		for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
		{
			if (joystick_axis_w[i]->get_selection() - 1 != input_preferences->joystick_axis_mappings[i]) {
				input_preferences->joystick_axis_mappings[i] = joystick_axis_w[i]->get_selection() - 1;
				changed = true;
			}
		}

		if (changed)
			write_preferences();
	}
}


/*
 *  Keyboard dialog
 */

const int NUM_KEYS = 21;

static const char *action_name[NUM_KEYS] = {
	"Move Forward", "Move Backward", "Turn Left", "Turn Right", "Sidestep Left", "Sidestep Right",
	"Glance Left", "Glance Right", "Look Up", "Look Down", "Look Ahead",
	"Previous Weapon", "Next Weapon", "Trigger", "2nd Trigger",
	"Sidestep", "Run/Swim", "Look",
	"Action", "Auto Map", "Microphone"
};

static const char *shell_action_name[NUMBER_OF_SHELL_KEYS] = {
	"Inventory Left", "Inventory Right", "Switch Player View", "Volume Up", "Volume Down", "Zoom Map In", "Zoom Map Out", "Toggle FPS", "Chat/Console", "Network Stats"
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
	SDLK_m,										// map
    SDLK_BACKQUOTE                              // microphone (ZZZ)
};

static SDLKey default_mouse_keys[NUM_KEYS] = {
	SDLK_w, SDLK_x, SDLK_LEFT, SDLK_RIGHT,		// moving/turning
	SDLK_a, SDLK_d,								// sidestepping
	SDLK_q, SDLK_e,								// horizontal looking
	SDLK_UP, SDLK_DOWN, SDLK_KP0,				// vertical looking
	SDLK_c, SDLK_z,								// weapon cycling
	SDLKey(SDLK_BASE_MOUSE_BUTTON), SDLKey(SDLK_BASE_MOUSE_BUTTON+2), // weapon trigger
	SDLK_RSHIFT, SDLK_LSHIFT, SDLK_LCTRL,		// modifiers
	SDLK_s,										// action trigger
	SDLK_TAB,									// map
    SDLK_BACKQUOTE                              // microphone (ZZZ)
};

static SDLKey default_shell_keys[NUMBER_OF_SHELL_KEYS] = {
	SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_BACKSPACE, SDLK_PERIOD, SDLK_COMMA, SDLK_EQUALS, SDLK_MINUS, SDLK_SLASH, SDLK_BACKSLASH, SDLK_1
};

class w_prefs_key;

static w_prefs_key *key_w[NUM_KEYS];
static w_prefs_key *shell_key_w[NUMBER_OF_SHELL_KEYS];

class w_prefs_key : public w_key {
public:
	w_prefs_key(SDLKey key) : w_key(key) {}

	void set_key(SDLKey new_key)
	{
		// Key used for in-game function?
		int error = NONE;
		switch (new_key) {
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
		case SDLK_ESCAPE: // (ZZZ: for quitting)
			error = keyIsUsedAlready;
			break;
			
		default:
			break;
		}
		if (new_key == SDLKey(SDLK_BASE_MOUSE_BUTTON + 3) || new_key == SDLKey(SDLK_BASE_MOUSE_BUTTON + 4))
		{
			error = keyScrollWheelDoesntWork;
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

		for (int i =0; i < NUMBER_OF_SHELL_KEYS; i++) {
			if (shell_key_w[i] && shell_key_w[i] != this && shell_key_w[i]->get_key() == new_key) {
				shell_key_w[i]->set_key(SDLK_UNKNOWN);
				shell_key_w[i]->dirty = true;
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

	for (int i=0; i<NUMBER_OF_SHELL_KEYS;i++)
		shell_key_w[i]->set_key(default_shell_keys[i]);
	d->draw();
}

enum {
	KEYBOARD_TABS,
	TAB_KEYS,
	TAB_MORE_KEYS
};

static void keyboard_dialog(void *arg)
{
	// Clear array of key widgets (because w_prefs_key::set_key() scans it)
	for (int i=0; i<NUM_KEYS; i++)
		key_w[i] = NULL;

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title("CONFIGURE KEYS / BUTTONS"), d);

	placer->add(new w_spacer(), true);
	
	table_placer *left_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	left_table->col_flags(0, placeable::kAlignRight);
	table_placer *right_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	right_table->col_flags(0, placeable::kAlignRight);

	for (int i=0; i<19; i++)
	{
		
		key_w[i] = new w_prefs_key(SDLKey(input_preferences->keycodes[i]));
		left_table->dual_add(key_w[i]->label(action_name[i]), d);
		left_table->dual_add(key_w[i], d);
	}

	for (int i=19; i<NUM_KEYS; i++) {
		key_w[i] = new w_prefs_key(SDLKey(input_preferences->keycodes[i]));
		right_table->dual_add(key_w[i]->label(action_name[i]), d);
		right_table->dual_add(key_w[i], d);
	}

	for (int i = 0; i < NUMBER_OF_SHELL_KEYS; i++) {
		shell_key_w[i] = new w_prefs_key(SDLKey(input_preferences->shell_keycodes[i]));
		right_table->dual_add(shell_key_w[i]->label(shell_action_name[i]), d);
		right_table->dual_add(shell_key_w[i], d);
	}

	horizontal_placer *table = new horizontal_placer(get_theme_space(ITEM_WIDGET), true);

	table->add(left_table, true);
	table->add(right_table, true);

	placer->add(table, true);

	placer->add(new w_spacer(), true);
	placer->dual_add(new w_button("DEFAULTS", load_default_keys, &d), d);
	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("ACCEPT", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);
	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	// Clear screen
	clear_screen();

	enter_joystick();

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

		for (int i=0; i<NUMBER_OF_SHELL_KEYS;i++) {
			SDLKey key = shell_key_w[i]->get_key();
			if (key != input_preferences->shell_keycodes[i]) {
				input_preferences->shell_keycodes[i] = short(key);
				changed = true;
			}
		}

		set_keys(input_preferences->keycodes);
		if (changed)
			write_preferences();
	}

	exit_joystick();
}

extern void ResetAllMMLValues();

static void plugins_dialog(void *)
{
	dialog d;
	vertical_placer *placer = new vertical_placer;
	w_title *w_header = new w_title("PLUGINS");
	placer->dual_add(w_header, d);
	placer->add(new w_spacer, true);

	std::vector<Plugin> plugins(Plugins::instance()->begin(), Plugins::instance()->end());
	w_plugins* plugins_w = new w_plugins(plugins, 400, 7);
	placer->dual_add(plugins_w, d);
	
	placer->add(new w_spacer, true);

	horizontal_placer* button_placer = new horizontal_placer;
	w_button* accept_w = new w_button("ACCEPT", dialog_ok, &d);
	button_placer->dual_add(accept_w, d);
	w_button* cancel_w = new w_button("CANCEL", dialog_cancel, &d);
	button_placer->dual_add(cancel_w, d);

	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	if (d.run() == 0) {
		bool changed = false;
		Plugins::iterator plugin = Plugins::instance()->begin();
		for (Plugins::iterator it = plugins.begin(); it != plugins.end(); ++it, ++plugin) {
			changed |= (plugin->enabled != it->enabled);
			plugin->enabled = it->enabled;
		}

		if (changed) {
			Plugins::instance()->invalidate();
			write_preferences();

			ResetAllMMLValues();
			LoadBaseMMLScripts();
			Plugins::instance()->load_mml();
		}
	}
}


/*
 *  Environment dialog
 */

static const char* film_profile_labels[] = {
	"Aleph One",
	"Marathon 2",
	"Marathon Infinity",
	0
};

static void environment_dialog(void *arg)
{
	dialog *parent = (dialog *)arg;

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	w_title *w_header = new w_title("ENVIRONMENT SETTINGS");
	placer->dual_add(w_header, d);
	placer->add(new w_spacer, true);

	table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
	table->col_flags(0, placeable::kAlignRight);
	
	w_env_select *map_w = new w_env_select(environment_preferences->map_file, "AVAILABLE MAPS", _typecode_scenario, &d);
	table->dual_add(map_w->label("Map"), d);
	table->dual_add(map_w, d);
	
	w_env_select *physics_w = new w_env_select(environment_preferences->physics_file, "AVAILABLE PHYSICS MODELS", _typecode_physics, &d);
	table->dual_add(physics_w->label("Physics"), d);
	table->dual_add(physics_w, d);

	w_env_select *shapes_w = new w_env_select(environment_preferences->shapes_file, "AVAILABLE SHAPES", _typecode_shapes, &d);
	table->dual_add(shapes_w->label("Shapes"), d);
	table->dual_add(shapes_w, d);

	w_env_select *sounds_w = new w_env_select(environment_preferences->sounds_file, "AVAILABLE SOUNDS", _typecode_sounds, &d);
	table->dual_add(sounds_w->label("Sounds"), d);
	table->dual_add(sounds_w, d);

	w_env_select* resources_w = new w_env_select(environment_preferences->resources_file, "AVAILABLE FILES", _typecode_unknown, &d);
	table->dual_add(resources_w->label("External Resources"), d);
	table->dual_add(resources_w, d);

	table->add_row(new w_spacer, true);
	table->dual_add_row(new w_button("PLUGINS", plugins_dialog, &d), d);

	table->add_row(new w_spacer, true);
	table->dual_add_row(new w_static_text("Solo Script"), d);
	w_enabling_toggle* use_solo_lua_w = new w_enabling_toggle(environment_preferences->use_solo_lua);
	table->dual_add(use_solo_lua_w->label("Use Solo Script"), d);
	table->dual_add(use_solo_lua_w, d);

	w_file_chooser *solo_lua_w = new w_file_chooser("Choose Script", _typecode_netscript);
	solo_lua_w->set_file(environment_preferences->solo_lua_file);
	table->dual_add(solo_lua_w->label("Script File"), d);
	table->dual_add(solo_lua_w, d);
	use_solo_lua_w->add_dependent_widget(solo_lua_w);

	table->add_row(new w_spacer, true);
	table->dual_add_row(new w_static_text("Film Playback"), d);
	
	w_select* film_profile_w = new w_select(environment_preferences->film_profile, film_profile_labels);
	table->dual_add(film_profile_w->label("Default Playback Profile"), d);
	table->dual_add(film_profile_w, d);
	
	w_enabling_toggle* use_replay_net_lua_w = new w_enabling_toggle(environment_preferences->use_replay_net_lua);
	table->dual_add(use_replay_net_lua_w->label("Use Netscript in Films"), d);
	table->dual_add(use_replay_net_lua_w, d);
	
	w_file_chooser *replay_net_lua_w = new w_file_chooser("Choose Script", _typecode_netscript);
	replay_net_lua_w->set_file(network_preferences->netscript_file);
	table->dual_add(replay_net_lua_w->label("Netscript File"), d);
	table->dual_add(replay_net_lua_w, d);
	use_replay_net_lua_w->add_dependent_widget(replay_net_lua_w);
	
	table->add_row(new w_spacer, true);
	table->dual_add_row(new w_static_text("Options"), d);

	w_toggle *hide_extensions_w = new w_toggle(environment_preferences->hide_extensions);
	table->dual_add(hide_extensions_w->label("Hide File Extensions"), d);
	table->dual_add(hide_extensions_w, d);

	w_select *max_saves_w = new w_select(0, max_saves_labels);
	for (int i = 0; max_saves_labels[i] != NULL; ++i) {
		if (max_saves_values[i] == environment_preferences->maximum_quick_saves)
			max_saves_w->set_selection(i);
	}
	table->dual_add(max_saves_w->label("Unnamed Saves to Keep"), d);
	table->dual_add(max_saves_w, d);

	placer->add(table, true);

	placer->add(new w_spacer, true);

	horizontal_placer *button_placer = new horizontal_placer;
	w_button *w_accept = new w_button("ACCEPT", dialog_ok, &d);
	button_placer->dual_add(w_accept, d);
	w_button *w_cancel = new w_button("CANCEL", dialog_cancel, &d);
	button_placer->dual_add(w_cancel, d);
	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	// Clear screen
	clear_screen();

	// Run dialog
	bool theme_changed = false;
	FileSpecifier old_theme;
	const Plugin* theme_plugin = Plugins::instance()->find_theme();
	if (theme_plugin)
	{
		old_theme = theme_plugin->directory + theme_plugin->theme;
	}

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
		
		path = resources_w->get_path();
		if (strcmp(path, environment_preferences->resources_file) != 0) 
		{
			strcpy(environment_preferences->resources_file, path);
			changed = true;
		}
		
		bool use_solo_lua = use_solo_lua_w->get_selection() != 0;
		if (use_solo_lua != environment_preferences->use_solo_lua)
		{
			environment_preferences->use_solo_lua = use_solo_lua;
			changed = true;
		}
		
		path = solo_lua_w->get_file().GetPath();
		if (strcmp(path, environment_preferences->solo_lua_file)) {
			strcpy(environment_preferences->solo_lua_file, path);
			changed = true;
		}

		bool use_replay_net_lua = use_replay_net_lua_w->get_selection() != 0;
		if (use_replay_net_lua != environment_preferences->use_replay_net_lua)
		{
			environment_preferences->use_replay_net_lua = use_replay_net_lua;
			changed = true;
		}
		
		path = replay_net_lua_w->get_file().GetPath();
		if (strcmp(path, network_preferences->netscript_file)) {
			strcpy(network_preferences->netscript_file, path);
			changed = true;
		}
		
		FileSpecifier new_theme;
		theme_plugin = Plugins::instance()->find_theme();
		if (theme_plugin)
		{
			new_theme = theme_plugin->directory + theme_plugin->theme;
		}

		if (new_theme != old_theme)
		{
			theme_changed = true;
		}

		bool hide_extensions = hide_extensions_w->get_selection() != 0;
		if (hide_extensions != environment_preferences->hide_extensions)
		{
			environment_preferences->hide_extensions = hide_extensions;
			changed = true;
		}

		if (film_profile_w->get_selection() != environment_preferences->film_profile)
		{
			environment_preferences->film_profile = static_cast<FilmProfileType>(film_profile_w->get_selection());
		
			changed = true;
		}

		bool saves_changed = false;
		int saves = max_saves_values[max_saves_w->get_selection()];
		if (saves != environment_preferences->maximum_quick_saves) {
			environment_preferences->maximum_quick_saves = saves;
			saves_changed = true;
		}

		if (changed)
			load_environment_from_preferences();

		if (theme_changed) {
			load_theme(new_theme);
		}

		if (changed || theme_changed || saves_changed)
			write_preferences();
	}

	// Redraw parent dialog
	if (theme_changed)
		parent->quit(0);	// Quit the parent dialog so it won't draw in the old theme
}

// For writing out boolean values
const char *BoolString(bool B) {return (B ? "true" : "false");}

// For writing out color values
const float CNorm = 1/float(65535);	// Maximum uint16

// These are template classes so as to be able to handle both
// "rgb_color" and "RGBColor" declarations (both are uint16 red, green, blue)

template<class CType> void WriteColor(FILE *F,
	const char *Prefix, CType& Color, const char *Suffix)
{
	fprintf(F,"%s<color red=\"%f\" green=\"%f\" blue=\"%f\"/>%s",
		Prefix,CNorm*Color.red,CNorm*Color.green,CNorm*Color.blue,Suffix);
}

template<class CType> void WriteColorWithIndex(FILE *F,
	const char *Prefix, int Index, CType& Color, const char *Suffix)
{
	fprintf(F,"%s<color index=\"%d\" red=\"%f\" green=\"%f\" blue=\"%f\"/>%s",
		Prefix,Index,CNorm*Color.red,CNorm*Color.green,CNorm*Color.blue,Suffix);
}

// For writing out text strings: have both Pascal and C versions
// These special routines are necessary in order to make the writing-out XML-friendly,
// converting XML's reserved characters into appropriate strings.
void WriteXML_PasString(FILE *F, const char *Prefix, const unsigned char *String, const char *Suffix);
void WriteXML_CString(FILE *F, const char *Prefix, const char *String, int MaxLen, const char *Suffix);
void WriteXML_Pathname(FILE *F, const char *Prefix, const char *String, const char *Suffix);
void WriteXML_Char(FILE *F, unsigned char c);

extern void hub_set_minimum_send_period(int32);
extern int32& hub_get_minimum_send_period();

struct set_latency_tolerance
{
	void operator() (const std::string& arg) const {
		hub_set_minimum_send_period(atoi(arg.c_str()));
		screen_printf("latency tolerance is now %i", atoi(arg.c_str()));
		write_preferences();
	}
};

struct get_latency_tolerance
{
	void operator() (const std::string&) const {
		screen_printf("latency tolerance is %i", hub_get_minimum_send_period());
	}
};

void transition_preferences(const DirectorySpecifier& legacy_preferences_dir)
{
	FileSpecifier prefs;
	prefs.SetToPreferencesDir();
	prefs += getcstr(temporary, strFILENAMES, filenamePREFERENCES);
	if (!prefs.Exists())
	{
		FileSpecifier oldPrefs;
		oldPrefs = legacy_preferences_dir;
		oldPrefs += getcstr(temporary, strFILENAMES, filenamePREFERENCES);
		if (oldPrefs.Exists())
		{
			oldPrefs.Rename(prefs);
		}
	}
}

/*
 *  Initialize preferences (load from file or setup defaults)
 */

void initialize_preferences(
	void)
{
	logContext("initializing preferences");

	// In case this function gets called more than once...
	if (!PrefsInited)
	{
		SetupPrefsParseTree();
		
		graphics_preferences= new graphics_preferences_data;
		player_preferences= new player_preferences_data;
		input_preferences= new input_preferences_data;
		sound_preferences = new SoundManager::Parameters;
		serial_preferences= new serial_number_data;
		network_preferences= new network_preferences_data;
		environment_preferences= new environment_preferences_data;

		XML_DataBlockLoader.CurrentElement = &PrefsRootParser;
		XML_DataBlockLoader.SourceName = "[Preferences]";
				
		PrefsInited = true;

		CommandParser PreferenceSetCommandParser;
		PreferenceSetCommandParser.register_command("latency_tolerance", set_latency_tolerance());
		CommandParser PreferenceGetCommandParser;
		PreferenceGetCommandParser.register_command("latency_tolerance", get_latency_tolerance());

		CommandParser PreferenceCommandParser;
		PreferenceCommandParser.register_command("set", PreferenceSetCommandParser);
		PreferenceCommandParser.register_command("get", PreferenceGetCommandParser);
		Console::instance()->register_command("preferences", PreferenceCommandParser);
		
		read_preferences ();
	}
}

void read_preferences ()
{
	// Set to defaults; will be overridden by reading in the XML stuff
	default_graphics_preferences(graphics_preferences);
	default_serial_number_preferences(serial_preferences);
	default_network_preferences(network_preferences);
	default_player_preferences(player_preferences);
	default_input_preferences(input_preferences);
	*sound_preferences = SoundManager::Parameters();
	default_environment_preferences(environment_preferences);

	// Slurp in the file and parse it

	FileSpecifier FileSpec;

	FileSpec.SetToPreferencesDir();
	FileSpec += getcstr(temporary, strFILENAMES, filenamePREFERENCES);

	OpenedFile OFile;
	bool defaults = false;
	bool opened = FileSpec.Open(OFile);

	if (!opened) {
		defaults = true;
		FileSpec.SetNameWithPath(getcstr(temporary, strFILENAMES, filenamePREFERENCES));
		opened = FileSpec.Open(OFile);
	}

	if (opened) {
		int32 Len = 0;
		OFile.GetLength(Len);
		if (Len > 0) {
			vector<char> FileContents(Len);

			if (OFile.Read(Len, &FileContents[0])) {
				OFile.Close();
				if (!XML_DataBlockLoader.ParseData(&FileContents[0], Len)) {
					if (defaults)
						alert_user(expand_app_variables("There were default preferences-file parsing errors (see $appLogFile$ for details)").c_str(), infoError);
					else
						alert_user(expand_app_variables("There were preferences-file parsing errors (see $appLogFile$ for details)").c_str(), infoError);
				}
			}
		}
	}

	// Check on the read-in prefs
	validate_graphics_preferences(graphics_preferences);
	validate_serial_number_preferences(serial_preferences);
	validate_network_preferences(network_preferences);
	validate_player_preferences(player_preferences);
	validate_input_preferences(input_preferences);
	validate_environment_preferences(environment_preferences);
	
	// jkvw: If we try to load a default file, but can't, we'll have set the game error.
	//       But that's not useful, because we're just going to try loading the file
	//       from user preferences.  It used to be this code was only called in initialisation,
	//       and any game error generated here was simply clobbered by an init time
	//       clear_game_error().  Since I'm using this more generally now, I'm clearing the
	//       error right here, because it's not like we're bothered when we can't load a
	//       default file.
	//       (Problem is SDL specific - socre one for Carbon? :) )
	clear_game_error ();
}


/*
 *  Write preferences to file
 */

void write_preferences(
	void)
{
	// LP: I've used plain stdio here because it's simple to do formatted writing with it.
	
	// Fix courtesy of mdadams@ku.edu
	FileSpecifier FileSpec;
	FileSpec.SetToPreferencesDir();
	FileSpec += getcstr(temporary, strFILENAMES, filenamePREFERENCES);
	
	// Open the file
	FILE *F = fopen(FileSpec.GetPath(),"w");
	
	if (!F)
	{
		return;
	}

	fprintf(F,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(F,"<!-- Preferences file for the Marathon Open Source \"Aleph One\" engine -->\n\n");
	
	fprintf(F,"<mara_prefs>\n\n");
	
	fprintf(F,"<graphics\n");
	fprintf(F,"  scmode_width=\"%hd\"\n", graphics_preferences->screen_mode.width);
	fprintf(F,"  scmode_height=\"%hd\"\n", graphics_preferences->screen_mode.height);
	fprintf(F,"  scmode_auto_resolution=\"%s\"\n", BoolString(graphics_preferences->screen_mode.auto_resolution));
	fprintf(F,"  scmode_hud=\"%s\"\n", BoolString(graphics_preferences->screen_mode.hud));
	fprintf(F,"  scmode_hud_scale=\"%hd\"\n", graphics_preferences->screen_mode.hud_scale_level);
	fprintf(F,"  scmode_term_scale=\"%hd\"\n", graphics_preferences->screen_mode.term_scale_level);
	fprintf(F,"  scmode_translucent_map=\"%s\"\n", BoolString(graphics_preferences->screen_mode.translucent_map));
	fprintf(F,"  scmode_accel=\"%hd\"\n",graphics_preferences->screen_mode.acceleration);
	fprintf(F,"  scmode_highres=\"%s\"\n",BoolString(graphics_preferences->screen_mode.high_resolution));
	fprintf(F,"  scmode_fullscreen=\"%s\"\n",BoolString(graphics_preferences->screen_mode.fullscreen));
	fprintf(F,"  scmode_fill_the_screen=\"%s\"\n", BoolString(graphics_preferences->screen_mode.fill_the_screen));
	fprintf(F,"  scmode_bitdepth=\"%hd\"\n",graphics_preferences->screen_mode.bit_depth);
	fprintf(F,"  scmode_gamma=\"%hd\"\n",graphics_preferences->screen_mode.gamma_level);
    fprintf(F,"  scmode_fix_h_not_v=\"%s\"\n", BoolString(graphics_preferences->screen_mode.fix_h_not_v));
	fprintf(F,"  ogl_flags=\"%hu\"\n",graphics_preferences->OGL_Configure.Flags);
	fprintf(F,"  software_alpha_blending=\"%i\"\n", graphics_preferences->software_alpha_blending);
        fprintf(F,"  anisotropy_level=\"%f\"\n", graphics_preferences->OGL_Configure.AnisotropyLevel);
	fprintf(F,"  multisamples=\"%i\"\n", graphics_preferences->OGL_Configure.Multisamples);
	fprintf(F,"  geforce_fix=\"%s\"\n", BoolString(graphics_preferences->OGL_Configure.GeForceFix));
	fprintf(F,"  wait_for_vsync=\"%s\"\n", BoolString(graphics_preferences->OGL_Configure.WaitForVSync));
	fprintf(F,"  gamma_corrected_blending=\"%s\"\n", BoolString(graphics_preferences->OGL_Configure.Use_sRGB));
	fprintf(F,"  use_npot=\"%s\"\n", BoolString(graphics_preferences->OGL_Configure.Use_NPOT));
	fprintf(F,"  double_corpse_limit=\"%s\"\n", BoolString(graphics_preferences->double_corpse_limit));
	fprintf(F,"  hog_the_cpu=\"%s\"\n", BoolString(graphics_preferences->hog_the_cpu));
	fprintf(F,"  movie_export_video_quality=\"%hd\"\n",graphics_preferences->movie_export_video_quality);
	fprintf(F,"  movie_export_audio_quality=\"%hd\"\n",graphics_preferences->movie_export_audio_quality);
	fprintf(F,">\n");
	fprintf(F,"  <void>\n");
	WriteColor(F,"    ",graphics_preferences->OGL_Configure.VoidColor,"\n");
	fprintf(F,"  </void>\n");
	fprintf(F,"  <landscapes>\n");
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			WriteColorWithIndex(F,"    ",(2*i+j),
				graphics_preferences->OGL_Configure.LscpColors[i][j],"\n");
	fprintf(F,"  </landscapes>\n");
	for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
	{
		OGL_Texture_Configure& TxtrConfig = graphics_preferences->OGL_Configure.TxtrConfigList[k];
		fprintf(F,"  <texture index=\"%hd\" near_filter=\"%hd\" far_filter=\"%hd\" resolution=\"%hd\" color_format=\"%d\" max_size=\"%d\"/>\n",
			k, TxtrConfig.NearFilter, TxtrConfig.FarFilter, TxtrConfig.Resolution, TxtrConfig.ColorFormat, TxtrConfig.MaxSize);
	}
	OGL_Texture_Configure& TxtrConfig = graphics_preferences->OGL_Configure.ModelConfig;
	fprintf(F,"  <texture index=\"%hd\" near_filter=\"%hd\" far_filter=\"%hd\" resolution=\"%hd\" color_format=\"%d\" max_size=\"%d\"/>\n",
		OGL_NUMBER_OF_TEXTURE_TYPES, TxtrConfig.NearFilter, TxtrConfig.FarFilter, TxtrConfig.Resolution, TxtrConfig.ColorFormat, TxtrConfig.MaxSize);
	fprintf(F,"</graphics>\n\n");
	
	fprintf(F,"<player\n");
	fprintf(F, "  name=\"%s\"\n", mac_roman_to_utf8(pstring_to_string(player_preferences->name)).c_str());
	fprintf(F,"  color=\"%hd\"\n",player_preferences->color);
	fprintf(F,"  team=\"%hd\"\n",player_preferences->team);
	fprintf(F,"  last_time_ran=\"%u\"\n",player_preferences->last_time_ran);
	fprintf(F,"  difficulty=\"%hd\"\n",player_preferences->difficulty_level);
	fprintf(F,"  bkgd_music=\"%s\"\n",BoolString(player_preferences->background_music_on));
	fprintf(F,"  crosshairs_active=\"%s\"\n",BoolString(player_preferences->crosshairs_active));
	fprintf(F,">\n");
	ChaseCamData& ChaseCam = player_preferences->ChaseCam;
	fprintf(F,"  <chase_cam behind=\"%hd\" upward=\"%hd\" rightward=\"%hd\" flags=\"%hd\"\n",
		ChaseCam.Behind, ChaseCam.Upward, ChaseCam.Rightward, ChaseCam.Flags);
	fprintf(F,"    damping=\"%f\" spring=\"%f\" opacity=\"%f\"/>\n",
		ChaseCam.Damping, ChaseCam.Spring, ChaseCam.Opacity);
	CrosshairData& Crosshairs = player_preferences->Crosshairs;
	fprintf(F,"  <crosshairs\n");
	fprintf(F,"    thickness=\"%hd\" from_center=\"%hd\" length=\"%hd\"\n",
		Crosshairs.Thickness, Crosshairs.FromCenter, Crosshairs.Length);
	fprintf(F,"    shape=\"%hd\" opacity=\"%f\"\n",
		Crosshairs.Shape, Crosshairs.Opacity);
	fprintf(F,"  >\n"),
	WriteColor(F,"    ",Crosshairs.Color,"\n");
	fprintf(F,"  </crosshairs>\n");
	fprintf(F,"</player>\n\n");
	
	fprintf(F,"<input\n");
	fprintf(F,"  device=\"%hd\"\n",input_preferences->input_device);
	fprintf(F,"  modifiers=\"%hu\"\n",input_preferences->modifiers);
	fprintf(F,"  sens_horizontal=\"%d\"\n",input_preferences->sens_horizontal); // ZZZ, LP
	fprintf(F,"  sens_vertical=\"%d\"\n",input_preferences->sens_vertical); // ZZZ, LP
	fprintf(F,"  mouse_acceleration=\"%s\"\n",BoolString(input_preferences->mouse_acceleration)); // SB
	fprintf(F,"  joystick_id=\"%hd\"\n", input_preferences->joystick_id);

	fprintf(F,">\n");
	for (int i = 0; i < MAX_BUTTONS; i++)
		fprintf(F,"  <mouse_button index=\"%hd\" action=\"%s\"/>\n", i,
			input_preferences->mouse_button_actions[i] == _mouse_button_fires_left_trigger ? "left_trigger" : 
			input_preferences->mouse_button_actions[i] == _mouse_button_fires_right_trigger ? "right_trigger" : "none");
	for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
		fprintf(F,"  <joystick_axis_mapping index=\"%hd\" axis=\"%hd\" axis_sensitivity=\"%f\" bound=\"%hd\"/>\n", i, input_preferences->joystick_axis_mappings[i], input_preferences->joystick_axis_sensitivities[i], input_preferences->joystick_axis_bounds[i]);
	for (int k=0; k<NUMBER_OF_KEYS; k++)
		fprintf(F,"  <sdl_key index=\"%hd\" value=\"%hd\"/>\n",
			k,input_preferences->keycodes[k]);
	for (int k=0; k<NUMBER_OF_SHELL_KEYS;k++)
		fprintf(F,"  <sdl_key index=\"%hd\" value=\"%hd\"/>\n",
			k + NUMBER_OF_KEYS, input_preferences->shell_keycodes[k]);
	fprintf(F,"</input>\n\n");
	
	fprintf(F,"<sound\n");
	fprintf(F,"  channels=\"%hd\"\n",sound_preferences->channel_count);
	fprintf(F,"  volume=\"%hd\"\n",sound_preferences->volume);
	fprintf(F,"  music_volume=\"%hd\"\n",sound_preferences->music);
	fprintf(F,"  flags=\"%hu\"\n",sound_preferences->flags);
	fprintf(F,"  rate=\"\%hu\"\n", sound_preferences->rate);
	fprintf(F,"  samples=\"\%hu\"\n", sound_preferences->samples);
	fprintf(F,"  volume_while_speaking=\"\%hu\"\n", sound_preferences->volume_while_speaking);
	fprintf(F,"  mute_while_transmitting=\"%s\"\n", BoolString(sound_preferences->mute_while_transmitting));
	fprintf(F,"/>\n\n");
	
#if !defined(DISABLE_NETWORKING)
	fprintf(F,"<network\n");
	fprintf(F,"  microphone=\"%s\"\n",BoolString(network_preferences->allow_microphone));
	fprintf(F,"  untimed=\"%s\"\n",BoolString(network_preferences->game_is_untimed));
	fprintf(F,"  type=\"%hd\"\n",network_preferences->type);
	fprintf(F,"  game_type=\"%hd\"\n",network_preferences->game_type);
	fprintf(F,"  difficulty=\"%hd\"\n",network_preferences->difficulty_level);
	fprintf(F,"  game_options=\"%hu\"\n",network_preferences->game_options);
	fprintf(F,"  time_limit=\"%d\"\n",network_preferences->time_limit);
	fprintf(F,"  kill_limit=\"%hd\"\n",network_preferences->kill_limit);
	fprintf(F,"  entry_point=\"%hd\"\n",network_preferences->entry_point);
        fprintf(F,"  autogather=\"%s\"\n",BoolString(network_preferences->autogather));
        fprintf(F,"  join_by_address=\"%s\"\n",BoolString(network_preferences->join_by_address));
        WriteXML_CString(F, "  join_address=\"",network_preferences->join_address,256,"\"\n");
        fprintf(F,"  local_game_port=\"%hu\"\n",network_preferences->game_port);
	fprintf(F,"  game_protocol=\"%s\"\n",sNetworkGameProtocolNames[network_preferences->game_protocol]);
	fprintf(F,"  use_speex_netmic_encoder=\"%s\"\n", BoolString(network_preferences->use_speex_encoder));
	fprintf(F,"  use_netscript=\"%s\"\n", BoolString(network_preferences->use_netscript));
	WriteXML_Pathname(F,"  netscript_file=\"", network_preferences->netscript_file, "\"\n");
	fprintf(F,"  cheat_flags=\"%hu\"\n",network_preferences->cheat_flags);
	fprintf(F,"  advertise_on_metaserver=\"%s\"\n",BoolString(network_preferences->advertise_on_metaserver));
	fprintf(F,"  attempt_upnp=\"%s\"\n", BoolString(network_preferences->attempt_upnp));
	fprintf(F,"  check_for_updates=\"%s\"\n", BoolString(network_preferences->check_for_updates));
	fprintf(F,"  metaserver_login=\"%.16s\"\n", network_preferences->metaserver_login);
	
	fprintf(F,"  metaserver_password=\"");
	for (int i = 0; i < 16; i++) {
		fprintf(F, "%.2x", network_preferences->metaserver_password[i] ^ sPasswordMask[i]);
	}
	fprintf(F, "\"\n");
	fprintf(F,"  use_custom_metaserver_colors=\"%s\"\n", BoolString(network_preferences->use_custom_metaserver_colors));
	fprintf(F,"  mute_metaserver_guests=\"%s\"\n", BoolString(network_preferences->mute_metaserver_guests));
	
	fprintf(F,">\n");
#ifndef SDL
	WriteXML_FSSpec(F,"  ", kNetworkScriptFileSpecIndex, network_preferences->netscript_file);
#endif
	for (int i = 0; i < 2; i++)
	{
		WriteColorWithIndex(F, "  ", i, network_preferences->metaserver_colors[i], "\n");
	}
	WriteStarPreferences(F);
	WriteRingPreferences(F);
	fprintf(F,"</network>\n\n");
#endif // !defined(DISABLE_NETWORKING)

	fprintf(F,"<environment\n");
	WriteXML_Pathname(F,"  map_file=\"",environment_preferences->map_file,"\"\n");
	WriteXML_Pathname(F,"  physics_file=\"",environment_preferences->physics_file,"\"\n");
	WriteXML_Pathname(F,"  shapes_file=\"",environment_preferences->shapes_file,"\"\n");
	WriteXML_Pathname(F,"  sounds_file=\"",environment_preferences->sounds_file,"\"\n");
	WriteXML_Pathname(F,"  resources_file=\"",environment_preferences->resources_file,"\"\n");
	fprintf(F,"  map_checksum=\"%u\"\n",environment_preferences->map_checksum);
	fprintf(F,"  physics_checksum=\"%u\"\n",environment_preferences->physics_checksum);
	fprintf(F,"  shapes_mod_date=\"%u\"\n",uint32(environment_preferences->shapes_mod_date));
	fprintf(F,"  sounds_mod_date=\"%u\"\n",uint32(environment_preferences->sounds_mod_date));
	fprintf(F,"  group_by_directory=\"%s\"\n",BoolString(environment_preferences->group_by_directory));
	fprintf(F,"  reduce_singletons=\"%s\"\n",BoolString(environment_preferences->reduce_singletons));
	fprintf(F,"  smooth_text=\"%s\"\n", BoolString(environment_preferences->smooth_text));
	WriteXML_Pathname(F,"  solo_lua_file=\"", environment_preferences->solo_lua_file, "\"\n");
	fprintf(F,"  use_solo_lua=\"%s\"\n", BoolString(environment_preferences->use_solo_lua));
	fprintf(F,"  use_replay_net_lua=\"%s\"\n", BoolString(environment_preferences->use_replay_net_lua));
	fprintf(F,"  hide_alephone_extensions=\"%s\"\n", BoolString(environment_preferences->hide_extensions));
	fprintf(F,"  film_profile=\"%u\"\n", static_cast<uint32>(environment_preferences->film_profile));
	fprintf(F,"  maximum_quick_saves=\"%u\"\n",environment_preferences->maximum_quick_saves);
	fprintf(F,">\n");
	for (Plugins::iterator it = Plugins::instance()->begin(); it != Plugins::instance()->end(); ++it) {
		if (it->compatible() && !it->enabled) {
			WriteXML_Pathname(F,"  <disable_plugin path=\"", it->directory.GetPath(), "\"/>\n");
		}
	}
	fprintf(F,"</environment>\n\n");
			
	fprintf(F,"</mara_prefs>\n\n");
	
	fclose(F);
	
}

/*
 *  Get prefs data from prefs file (or defaults)
 */

#ifndef SDL
static void *get_graphics_pref_data() {return graphics_preferences;}
static void *get_player_pref_data() {return player_preferences;}
static void *get_sound_pref_data() {return sound_preferences;}
static void *get_input_pref_data() {return input_preferences;}
static void *get_environment_pref_data() {return environment_preferences;}
#endif

/*
 *  Setup default preferences
 */

static void default_graphics_preferences(graphics_preferences_data *preferences)
{
  memset(&preferences->screen_mode, '\0', sizeof(screen_mode_data));
	preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;

	preferences->screen_mode.width = 640;
	preferences->screen_mode.height = 480;
	preferences->screen_mode.auto_resolution = true;
	preferences->screen_mode.hud = true;
	preferences->screen_mode.hud_scale_level = 0;
	preferences->screen_mode.term_scale_level = 0;
	preferences->screen_mode.translucent_map = false;
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__WIN32__)
	preferences->screen_mode.acceleration = _opengl_acceleration;
#else
	preferences->screen_mode.acceleration = _no_acceleration;
#endif
	preferences->screen_mode.high_resolution = true;
	preferences->screen_mode.fullscreen = true;
	preferences->screen_mode.fix_h_not_v = true;
	
	const SDL_version *version = SDL_Linked_Version();
	if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
		preferences->screen_mode.fill_the_screen = false;
	else
		preferences->screen_mode.fill_the_screen = true;

	if (preferences->screen_mode.acceleration == _no_acceleration)
		preferences->screen_mode.bit_depth = 16;
	else
		preferences->screen_mode.bit_depth = 32;
	
	preferences->screen_mode.draw_every_other_line= false;
	
	OGL_SetDefaults(preferences->OGL_Configure);

	preferences->double_corpse_limit= false;
	preferences->hog_the_cpu = false;

	preferences->software_alpha_blending = _sw_alpha_off;

	preferences->movie_export_video_quality = 50;
	preferences->movie_export_audio_quality = 50;
}

static void default_serial_number_preferences(serial_number_data *preferences)
{
	memset(preferences, 0, sizeof(struct serial_number_data));
}

static void default_network_preferences(network_preferences_data *preferences)
{
	preferences->type= _ethernet;

	preferences->allow_microphone = true;
	preferences->game_is_untimed = false;
	preferences->difficulty_level = 2;
	preferences->game_options =	_multiplayer_game | _ammo_replenishes | _weapons_replenish
		| _specials_replenish |	_monsters_replenish | _burn_items_on_death | _suicide_is_penalized 
		| _force_unique_teams | _live_network_stats;
	preferences->time_limit = 10 * TICKS_PER_SECOND * 60;
	preferences->kill_limit = 10;
	preferences->entry_point= 0;
	preferences->game_type= _game_of_kill_monsters;
	preferences->autogather= false;
	preferences->join_by_address= false;
	obj_clear(preferences->join_address);
	preferences->game_port= DEFAULT_GAME_PORT;
	preferences->game_protocol= _network_game_protocol_default;
#if !defined(DISABLE_NETWORKING)
	DefaultStarPreferences();
	DefaultRingPreferences();
#endif // !defined(DISABLE_NETWORKING)
	preferences->use_speex_encoder = true;
	preferences->use_netscript = false;
	preferences->netscript_file[0] = '\0';
	preferences->cheat_flags = _allow_tunnel_vision | _allow_crosshair | _allow_behindview | _allow_overlay_map;
	preferences->advertise_on_metaserver = false;
	preferences->attempt_upnp = false;
	preferences->check_for_updates = true;
	strcpy(preferences->metaserver_login, "guest");
	memset(preferences->metaserver_password, 0, 16);
	preferences->mute_metaserver_guests = false;
	preferences->use_custom_metaserver_colors = false;
	preferences->metaserver_colors[0] = get_interface_color(PLAYER_COLOR_BASE_INDEX);
	preferences->metaserver_colors[1] = get_interface_color(PLAYER_COLOR_BASE_INDEX);
}

static void default_player_preferences(player_preferences_data *preferences)
{
	obj_clear(*preferences);

	preferences->difficulty_level= 2;
	get_name_from_system(preferences->name);
	
	// LP additions for new fields:
	
	preferences->ChaseCam.Behind = 1536;
	preferences->ChaseCam.Upward = 0;
	preferences->ChaseCam.Rightward = 0;
	preferences->ChaseCam.Flags = 0;
	preferences->ChaseCam.Damping = 0.5;
	preferences->ChaseCam.Spring = 0;
	preferences->ChaseCam.Opacity = 1;
	
	preferences->Crosshairs.Thickness = 2;
	preferences->Crosshairs.FromCenter = 8;
	preferences->Crosshairs.Length = 16;
	preferences->Crosshairs.Shape = CHShape_RealCrosshairs;
	preferences->Crosshairs.Color = rgb_white;
	preferences->Crosshairs.Opacity = 0.5;
	preferences->Crosshairs.PreCalced = false;
}

static void default_input_preferences(input_preferences_data *preferences)
{
	preferences->input_device= _keyboard_or_game_pad;
	set_default_keys(preferences->keycodes, _standard_keyboard_setup);
	for (int i = 0; i < NUMBER_OF_SHELL_KEYS; i++)
	{
		preferences->shell_keycodes[i] = default_shell_keys[i];
	}
	
	// LP addition: set up defaults for modifiers:
	// interchange run and walk, but don't interchange swim and sink.
	preferences->modifiers = _inputmod_interchange_run_walk;

	// LP: split into horizontal and vertical sensitivities
	// ZZZ addition: sensitivity factor starts at 1 (no adjustment)
	preferences->sens_horizontal = FIXED_ONE;
	preferences->sens_vertical = FIXED_ONE;
	
	// SB
	preferences->mouse_acceleration = true;

	// default mouse settings
	preferences->mouse_button_actions[0] = _mouse_button_fires_left_trigger;
	preferences->mouse_button_actions[1] = _mouse_button_fires_right_trigger;
	for (int i = 2; i < MAX_BUTTONS; i++)
		preferences->mouse_button_actions[i] = _mouse_button_does_nothing;

	preferences->joystick_id = -1;
	for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; ++i)
		preferences->joystick_axis_mappings[i] = i;

	preferences->joystick_axis_sensitivities[_joystick_strafe] = 1.0;
	preferences->joystick_axis_sensitivities[_joystick_velocity] = -5.0;
	preferences->joystick_axis_sensitivities[_joystick_yaw] = 0.1;
	preferences->joystick_axis_sensitivities[_joystick_pitch] = -1.0;

	preferences->joystick_axis_bounds[_joystick_strafe] = 10000;
	preferences->joystick_axis_bounds[_joystick_velocity] = 3000;
	preferences->joystick_axis_bounds[_joystick_yaw] = 3000;
	preferences->joystick_axis_bounds[_joystick_pitch] = 4500;
}

static void default_environment_preferences(environment_preferences_data *preferences)
{
	obj_set(*preferences, NONE);

	FileSpecifier DefaultMapFile;
	FileSpecifier DefaultShapesFile;
	FileSpecifier DefaultSoundsFile;
	FileSpecifier DefaultPhysicsFile;
	FileSpecifier DefaultExternalResourcesFile;
    
	get_default_map_spec(DefaultMapFile);
	get_default_physics_spec(DefaultPhysicsFile);
	get_default_shapes_spec(DefaultShapesFile);
	get_default_sounds_spec(DefaultSoundsFile);
	get_default_external_resources_spec(DefaultExternalResourcesFile);
	                
	preferences->map_checksum= read_wad_file_checksum(DefaultMapFile);
	strncpy(preferences->map_file, DefaultMapFile.GetPath(), 256);
	preferences->map_file[255] = 0;
	
	preferences->physics_checksum= read_wad_file_checksum(DefaultPhysicsFile);
	strncpy(preferences->physics_file, DefaultPhysicsFile.GetPath(), 256);
	preferences->physics_file[255] = 0;
	
	preferences->shapes_mod_date = DefaultShapesFile.GetDate();
	strncpy(preferences->shapes_file, DefaultShapesFile.GetPath(), 256);
	preferences->shapes_file[255] = 0;

	preferences->sounds_mod_date = DefaultSoundsFile.GetDate();
	strncpy(preferences->sounds_file, DefaultSoundsFile.GetPath(), 256);
	preferences->sounds_file[255] = 0;

	strncpy(preferences->resources_file, DefaultExternalResourcesFile.GetPath(), 256);
	preferences->resources_file[255] = 0;

	preferences->group_by_directory = true;
	preferences->reduce_singletons = false;
	preferences->smooth_text = true;

	preferences->solo_lua_file[0] = 0;
	preferences->use_solo_lua = false;
	preferences->use_replay_net_lua = false;
	preferences->hide_extensions = true;
	preferences->film_profile = FILM_PROFILE_DEFAULT;
	preferences->maximum_quick_saves = 0;
}


/*
 *  Validate preferences
 */

static bool validate_graphics_preferences(graphics_preferences_data *preferences)
{
	bool changed= false;

	// Fix bool options
	preferences->screen_mode.high_resolution = !!preferences->screen_mode.high_resolution;
	preferences->screen_mode.fullscreen = !!preferences->screen_mode.fullscreen;
	preferences->screen_mode.draw_every_other_line = !!preferences->screen_mode.draw_every_other_line;
    preferences->screen_mode.fix_h_not_v = !!preferences->screen_mode.fix_h_not_v;

	if(preferences->screen_mode.gamma_level<0 || preferences->screen_mode.gamma_level>=NUMBER_OF_GAMMA_LEVELS)
	{
		preferences->screen_mode.gamma_level= DEFAULT_GAMMA_LEVEL;
		changed= true;
	}

	// OpenGL requires at least 16 bit color depth
	if (preferences->screen_mode.acceleration != _no_acceleration && preferences->screen_mode.bit_depth == 8)
	{
		preferences->screen_mode.bit_depth= 16;
		changed= true;
	}

#ifdef TRUE_COLOR_ONLY
	if (preferences->screen_mode.bit_depth == 8)
	{
		preferences->screen_mode.bit_depth = 16;
		changed = true;
	}
#endif

	return changed;
}

static bool validate_serial_number_preferences(serial_number_data *preferences)
{
	(void) (preferences);
	return false;
}

static bool validate_network_preferences(network_preferences_data *preferences)
{
	bool changed= false;

	// Fix bool options
	preferences->allow_microphone = !!preferences->allow_microphone;
	preferences->game_is_untimed = !!preferences->game_is_untimed;

	if(preferences->type<0||preferences->type>_ethernet)
	{
		if(ethernet_active())
		{
			preferences->type= _ethernet;
		} else {
			preferences->type= _localtalk;
		}
		changed= true;
	}
	
	if(preferences->game_is_untimed != true && preferences->game_is_untimed != false)
	{
		preferences->game_is_untimed= false;
		changed= true;
	}

	if(preferences->allow_microphone != true && preferences->allow_microphone != false)
	{
		preferences->allow_microphone= true;
		changed= true;
	}

	if(preferences->game_type<0 || preferences->game_type >= NUMBER_OF_GAME_TYPES)
	{
		preferences->game_type= _game_of_kill_monsters;
		changed= true;
	}

        // ZZZ: is this relevant anymore now with XML prefs?  if so, should validate autogather, join_by_address, and join_address.

	if(preferences->game_protocol >= NUMBER_OF_NETWORK_GAME_PROTOCOLS)
	{
		preferences->game_protocol= _network_game_protocol_default;
		changed= true;
	}

	return changed;
}

static bool validate_player_preferences(player_preferences_data *preferences)
{
	// Fix bool options
	preferences->background_music_on = !!preferences->background_music_on;

	return false;
}

static bool validate_input_preferences(input_preferences_data *preferences)
{
	(void) (preferences);
	return false;
}

static bool validate_environment_preferences(environment_preferences_data *preferences)
{
	(void) (preferences);
	return false;
}


/*
 *  Load the environment
 */

/* Load the environment.. */
void load_environment_from_preferences(
	void)
{
	FileSpecifier File;
	struct environment_preferences_data *prefs= environment_preferences;

	File = prefs->map_file;
	if (File.Exists()) {
		set_map_file(File);
	} else {
		/* Try to find the checksum */
		if(find_wad_file_that_has_checksum(File,
			_typecode_scenario, strPATHS, prefs->map_checksum))	{
			set_map_file(File);
		} else {
			set_to_default_map();
		}
	}

	File = prefs->physics_file;
	if (File.Exists()) {
		set_physics_file(File);
		import_definition_structures();
	} else {
		if(find_wad_file_that_has_checksum(File,
			_typecode_physics, strPATHS, prefs->physics_checksum)) {
			set_physics_file(File);
			import_definition_structures();
		} else {
			/* Didn't find it.  Don't change them.. */
		}
	}
	
	File = prefs->shapes_file;
	if (File.Exists()) {
		open_shapes_file(File);
	} else {
		if(find_file_with_modification_date(File,
			_typecode_shapes, strPATHS, prefs->shapes_mod_date))
		{
			open_shapes_file(File);
		} else {
			/* What should I do? */
		}
	}

	File = prefs->sounds_file;
	if (File.Exists()) {
		SoundManager::instance()->OpenSoundFile(File);
	} else {
		if(find_file_with_modification_date(File,
			_typecode_sounds, strPATHS, prefs->sounds_mod_date)) {
			SoundManager::instance()->OpenSoundFile(File);
		} else {
			/* What should I do? */
		}
	}

	File = prefs->resources_file;
	if (File.Exists())
	{
		set_external_resources_file(File);
	}
	set_external_resources_images_file(File);
}


// LP addition: get these from the preferences data
ChaseCamData& GetChaseCamData() {return player_preferences->ChaseCam;}
CrosshairData& GetCrosshairData() {return player_preferences->Crosshairs;}
OGL_ConfigureData& Get_OGL_ConfigureData() {return graphics_preferences->OGL_Configure;}


// ZZZ: override player-behavior modifiers
static bool sStandardizeModifiers = false;


void
standardize_player_behavior_modifiers() {
    sStandardizeModifiers = true;
}


void
restore_custom_player_behavior_modifiers() {
    sStandardizeModifiers = false;
}


bool
is_player_behavior_standard() {
    return (!dont_switch_to_new_weapon() && !dont_auto_recenter());
}


// LP addition: modification of Josh Elsasser's dont-switch-weapons patch
// so as to access preferences stuff here
bool dont_switch_to_new_weapon() {
    // ZZZ: let game require standard modifiers for a while
    if(!sStandardizeModifiers)
	    return TEST_FLAG(input_preferences->modifiers,_inputmod_dont_switch_to_new_weapon);
    else
        return false;
}


// ZZZ addition: like dont_switch_to_new_weapon()
bool
dont_auto_recenter() {
    if(!sStandardizeModifiers)
        return TEST_FLAG(input_preferences->modifiers, _inputmod_dont_auto_recenter);
    else
        return false;
}


// For writing out text strings: have both Pascal and C versions
// These special routines are necessary in order to make the writing-out XML-friendly,
// converting XML's reserved characters into appropriate strings.

void WriteXML_PasString(FILE *F, const char *Prefix, const unsigned char *String, const char *Suffix)
{
	fprintf(F,"%s",Prefix);
	for (int k=1; k<=String[0]; k++)
		WriteXML_Char(F,String[k]);
	fprintf(F,"%s",Suffix);
}

void WriteXML_CString(FILE *F, const char *Prefix, const char *String, int MaxLen, const char *Suffix)
{
	fprintf(F,"%s",Prefix);
	size_t Len = strlen(String);
	for (size_t k=0; k<Len; k++)
		WriteXML_Char(F,String[k]);
	fprintf(F,"%s",Suffix);
}

void WriteXML_Pathname(FILE *F, const char *Prefix, const char *String, const char *Suffix)
{
	char tempstr[256];
	contract_symbolic_paths(tempstr, String, 255);
	fprintf(F,"%s",Prefix);
	size_t Len = strlen(tempstr);
	for (size_t k=0; k<Len; k++)
		WriteXML_Char(F,tempstr[k]);
	fprintf(F,"%s",Suffix);
}

void WriteXML_Char(FILE *F, unsigned char c)
{
	// Make XML-friendly
	// Are the characters normal ASCII printable characters?
	if (c < 0x20)
	{
		// Turn the bad character into a good one,
		// because Expat dislikes characters below 0x20
		fprintf(F,"$");
	}
	else
	{
		switch(c)
		{
		// XML reserved characters
		case '&':
			fprintf(F,"&amp;");
			break;
		
		case '<':
			fprintf(F,"&lt;");
			break;
		
		case '>':
			fprintf(F,"&gt;");
			break;
		
		case '"':
			fprintf(F,"&quot;");
			break;
		
		case '\'':
			fprintf(F,"&apos;");
			break;
		
		// OK character
		default:
			fputc(int(c),F);
			break;
		}
	}
}

// LP additions: MML-like prefs stuff
// These parsers are intended to work correctly on both Mac and SDL prefs files;
// including one crossing over to the other platform (uninterpreted fields become defaults)

// To get around both RGBColor and rgb_color being used in the code
template<class CType1, class CType2> void CopyColor(CType1& Dest, CType2& Src)
{
	Dest.red = Src.red;
	Dest.green = Src.green;
	Dest.blue = Src.blue;
}


class XML_VoidPrefsParser: public XML_ElementParser
{
	rgb_color Color;

public:
	bool Start();
	bool End();

	XML_VoidPrefsParser(): XML_ElementParser("void") {}
};

bool XML_VoidPrefsParser::Start()
{
	CopyColor(Color,graphics_preferences->OGL_Configure.VoidColor);
	
	Color_SetArray(&Color);
	
	return true;
}

bool XML_VoidPrefsParser::End()
{
	CopyColor(graphics_preferences->OGL_Configure.VoidColor,Color);

	return true;
}

static XML_VoidPrefsParser VoidPrefsParser;


class XML_LandscapePrefsParser: public XML_ElementParser
{
	rgb_color Colors[8];

public:
	bool Start();
	bool End();

	XML_LandscapePrefsParser(): XML_ElementParser("landscapes") {}
};

bool XML_LandscapePrefsParser::Start()
{
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			CopyColor(Colors[2*i+j],graphics_preferences->OGL_Configure.LscpColors[i][j]);
	
	Color_SetArray(Colors,8);
	
	return true;
}

bool XML_LandscapePrefsParser::End()
{
	for (int i=0; i<4; i++)
		for (int j=0; j<2; j++)
			CopyColor(graphics_preferences->OGL_Configure.LscpColors[i][j],Colors[2*i+j]);

	return true;
}

static XML_LandscapePrefsParser LandscapePrefsParser;


class XML_TexturePrefsParser: public XML_ElementParser
{
	bool IndexPresent, ValuesPresent[5];
	int16 Index, Values[5];
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_TexturePrefsParser(): XML_ElementParser("texture") {}
};

bool XML_TexturePrefsParser::Start()
{
	IndexPresent = false;
	for (int k=0; k<4; k++)
		ValuesPresent[k] = false;
	
	return true;
}

bool XML_TexturePrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,OGL_NUMBER_OF_TEXTURE_TYPES))
		{
			IndexPresent = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"near_filter"))
	{
		if (ReadInt16Value(Value,Values[0]))
		{
			ValuesPresent[0] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"far_filter"))
	{
		if (ReadInt16Value(Value,Values[1]))
		{
			ValuesPresent[1] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"resolution"))
	{
		if (ReadInt16Value(Value,Values[2]))
		{
			ValuesPresent[2] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"color_format"))
	{
		if (ReadInt16Value(Value,Values[3]))
		{
			ValuesPresent[3] = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag, "max_size"))
	{
		if (ReadInt16Value(Value, Values[4]))
		{
			ValuesPresent[4] = true;
			return true;
		}
		else
			return false;
	}
	return true;
}


bool XML_TexturePrefsParser::AttributesDone()
{
	// Verify...
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}

	OGL_Texture_Configure& Config = (Index == OGL_NUMBER_OF_TEXTURE_TYPES) ?  graphics_preferences->OGL_Configure.ModelConfig : graphics_preferences->OGL_Configure.TxtrConfigList[Index];

	if (ValuesPresent[0])
		Config.NearFilter = Values[0];
	
	if (ValuesPresent[1])
		Config.FarFilter = Values[1];
	
	if (ValuesPresent[2])
		Config.Resolution = Values[2];
	
	if (ValuesPresent[3])
		Config.ColorFormat = Values[3];

	if (ValuesPresent[4])
		Config.MaxSize = Values[4];
	
	return true;
}

static XML_TexturePrefsParser TexturePrefsParser;


class XML_GraphicsPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_GraphicsPrefsParser(): XML_ElementParser("graphics") {}
};

struct ViewSizeData
{
	short Width, Height;
	bool HUD;
};

const ViewSizeData LegacyViewSizes[32] = 
{
	{ 320, 160, true},
	{ 480, 240, true},
	{ 640, 480, true},
	{ 640, 480, false},
	{ 800, 600, true},
	{ 800, 600, false},
	{ 1024, 768, true},
	{ 1024, 768, false},
	{ 1280, 1024, true},
	{ 1280, 1024, false},
	{ 1600, 1200, true},
	{ 1600, 1200, false},
	{ 1024, 640, true},
	{ 1024, 640, false},
	{ 1280, 800, true},
	{ 1280, 800, false},
	{ 1280, 854, true},
	{ 1280, 854, false},
	{ 1440, 900, true},
	{ 1440, 900, false},
	{ 1680, 1050, true},
	{ 1680, 1050, false},
	{ 1920, 1200, true},
	{ 1920, 1200, false},
	{ 2560, 1600, true},
	{ 2560, 1600, false},
	{ 1280, 768, true},
	{ 1280, 768, false},
	{ 1280, 960, true},
	{ 1280, 960, false},
	{ 1280, 720, true},
	{ 1280, 720, false}
};

bool XML_GraphicsPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"scmode_size"))
	{
		short scmode;
		if (ReadInt16Value(Value, scmode))
		{
			if (scmode >= 0 && scmode < 32)
			{
				graphics_preferences->screen_mode.height = LegacyViewSizes[scmode].Height;
				graphics_preferences->screen_mode.width = LegacyViewSizes[scmode].Width;
				graphics_preferences->screen_mode.hud = LegacyViewSizes[scmode].HUD;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else if (StringsEqual(Tag,"scmode_height"))
	{
		return ReadInt16Value(Value, graphics_preferences->screen_mode.height);
	}
	else if (StringsEqual(Tag,"scmode_width"))
	{
		return ReadInt16Value(Value, graphics_preferences->screen_mode.width);
	}
	else if (StringsEqual(Tag,"scmode_auto_resolution"))
	{
		return ReadBooleanValue(Value, graphics_preferences->screen_mode.auto_resolution);
	}
	else if (StringsEqual(Tag,"scmode_hud"))
	{
		return ReadBooleanValue(Value, graphics_preferences->screen_mode.hud);
	}
	else if (StringsEqual(Tag,"scmode_hud_scale"))
	{
		return ReadInt16Value(Value, graphics_preferences->screen_mode.hud_scale_level);
	}
	else if (StringsEqual(Tag,"scmode_term_scale"))
	{
		return ReadInt16Value(Value, graphics_preferences->screen_mode.term_scale_level);
	}
	else if (StringsEqual(Tag,"scmode_translucent_map"))
	{
		return ReadBooleanValue(Value, graphics_preferences->screen_mode.translucent_map);
	}
	else if (StringsEqual(Tag,"scmode_accel"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.acceleration);
	}
	else if (StringsEqual(Tag,"scmode_highres"))
	{
		return ReadBooleanValue(Value,graphics_preferences->screen_mode.high_resolution);
	}
	else if (StringsEqual(Tag,"scmode_fullscreen"))
	{
		return ReadBooleanValue(Value,graphics_preferences->screen_mode.fullscreen);
	}
	else if (StringsEqual(Tag, "scmode_fill_the_screen"))
	{
		const SDL_version *version = SDL_Linked_Version();
		if (SDL_VERSIONNUM(version->major, version->minor, version->patch) >= SDL_VERSIONNUM(1, 2, 10))
			return ReadBooleanValue(Value, graphics_preferences->screen_mode.fill_the_screen);
		else
		{
			graphics_preferences->screen_mode.fill_the_screen = true;
			return true;
		}
	}
    else if (StringsEqual(Tag,"scmode_fix_h_not_v"))
    {
        return ReadBooleanValue(Value,graphics_preferences->screen_mode.fix_h_not_v);
    }
	else if (StringsEqual(Tag,"scmode_bitdepth"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.bit_depth);
	}
	else if (StringsEqual(Tag,"scmode_gamma"))
	{
		return ReadInt16Value(Value,graphics_preferences->screen_mode.gamma_level);
	}
	else if (StringsEqual(Tag,"ogl_flags"))
	{
		return ReadUInt16Value(Value,graphics_preferences->OGL_Configure.Flags);
	}
        else if (StringsEqual(Tag,"experimental_rendering"))
        {
		// obsolete
		return true;
        }
	else if (StringsEqual(Tag, "software_alpha_blending"))
	{
		return ReadInt16Value(Value, graphics_preferences->software_alpha_blending);
	}
        else if (StringsEqual(Tag,"anisotropy_level"))
	  {
	    return ReadFloatValue(Value, graphics_preferences->OGL_Configure.AnisotropyLevel);
	  }
	else if (StringsEqual(Tag,"multisamples"))
	  {
	    return ReadInt16Value(Value, graphics_preferences->OGL_Configure.Multisamples);
	  }
	else if (StringsEqual(Tag,"geforce_fix"))
	{
		return ReadBooleanValue(Value, graphics_preferences->OGL_Configure.GeForceFix);
	}
	else if (StringsEqual(Tag,"wait_for_vsync"))
	{
		return ReadBooleanValue(Value, graphics_preferences->OGL_Configure.WaitForVSync);
	}
	else if (StringsEqual(Tag,"gamma_corrected_blending"))
	{
		return ReadBooleanValue(Value, graphics_preferences->OGL_Configure.Use_sRGB);
	}
	else if (StringsEqual(Tag,"use_npot"))
	{
		return ReadBooleanValue(Value, graphics_preferences->OGL_Configure.Use_NPOT);
	}
	else if (StringsEqual(Tag,"double_corpse_limit"))
	  {
	    return ReadBooleanValue(Value,graphics_preferences->double_corpse_limit);
	  }
	else if (StringsEqual(Tag,"hog_the_cpu"))
	{
		return ReadBooleanValue(Value, graphics_preferences->hog_the_cpu);
	}
	else if (StringsEqual(Tag,"movie_export_video_quality"))
	{
		return ReadBoundedInt16Value(Value, graphics_preferences->movie_export_video_quality, 0, 100);
	}
	else if (StringsEqual(Tag,"movie_export_audio_quality"))
	{
		return ReadBoundedInt16Value(Value, graphics_preferences->movie_export_audio_quality, 0, 100);
	}
	else if (StringsEqual(Tag,"use_directx_backend"))
	{
		return true;
	}
	return true;
}

static XML_GraphicsPrefsParser GraphicsPrefsParser;



class XML_ChaseCamPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_ChaseCamPrefsParser(): XML_ElementParser("chase_cam") {}
};

bool XML_ChaseCamPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"behind"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Behind);
	}
	else if (StringsEqual(Tag,"upward"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Upward);
	}
	else if (StringsEqual(Tag,"rightward"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Rightward);
	}
	else if (StringsEqual(Tag,"flags"))
	{
		return ReadInt16Value(Value,player_preferences->ChaseCam.Flags);
	}
	else if (StringsEqual(Tag,"damping"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Damping);
	}
	else if (StringsEqual(Tag,"spring"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Spring);
	}
	else if (StringsEqual(Tag,"opacity"))
	{
		return ReadFloatValue(Value,player_preferences->ChaseCam.Opacity);
	}
	return true;
}

static XML_ChaseCamPrefsParser ChaseCamPrefsParser;


class XML_CrosshairsPrefsParser: public XML_ElementParser
{
	rgb_color Color;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool End();

	XML_CrosshairsPrefsParser(): XML_ElementParser("crosshairs") {}
};

bool XML_CrosshairsPrefsParser::Start()
{
	CopyColor(Color,player_preferences->Crosshairs.Color);
	Color_SetArray(&Color);

	return true;
}

bool XML_CrosshairsPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"thickness"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Thickness);
	}
	else if (StringsEqual(Tag,"from_center"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.FromCenter);
	}
	else if (StringsEqual(Tag,"length"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Length);
	}
	else if (StringsEqual(Tag,"shape"))
	{
		return ReadInt16Value(Value,player_preferences->Crosshairs.Shape);
	}
	else if (StringsEqual(Tag,"opacity"))
	{
		return ReadFloatValue(Value,player_preferences->Crosshairs.Opacity);
	}
	return true;
}

bool XML_CrosshairsPrefsParser::End()
{
	CopyColor(player_preferences->Crosshairs.Color,Color);

	return true;
}

static XML_CrosshairsPrefsParser CrosshairsPrefsParser;


class XML_PlayerPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_PlayerPrefsParser(): XML_ElementParser("player") {}
};

bool XML_PlayerPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"name"))
	{
		// Copy in as Pascal string
		DeUTF8_Pas(Value,strlen(Value),player_preferences->name,PREFERENCES_NAME_LENGTH);
		return true;
	}
	else if (StringsEqual(Tag,"color"))
	{
		return ReadInt16Value(Value,player_preferences->color);
	}
	else if (StringsEqual(Tag,"team"))
	{
		return ReadInt16Value(Value,player_preferences->team);
	}
	else if (StringsEqual(Tag,"last_time_ran"))
	{
		return ReadUInt32Value(Value,player_preferences->last_time_ran);
	}
	else if (StringsEqual(Tag,"difficulty"))
	{
		return ReadInt16Value(Value,player_preferences->difficulty_level);
	}
	else if (StringsEqual(Tag,"bkgd_music"))
	{
		return ReadBooleanValue(Value,player_preferences->background_music_on);
	}
	else if (StringsEqual(Tag,"crosshairs_active"))
	{
		return ReadBooleanValue(Value,player_preferences->crosshairs_active);
	}
	return true;
}

static XML_PlayerPrefsParser PlayerPrefsParser;


class XML_MouseButtonPrefsParser: public XML_ElementParser
{
	bool IndexPresent, ActionPresent;
	int16 Index, Action;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MouseButtonPrefsParser(const char *_Name): XML_ElementParser(_Name) {}
};

bool XML_MouseButtonPrefsParser::Start()
{
	IndexPresent = ActionPresent = false;
	
	return true;
}

bool XML_MouseButtonPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,MAX_BUTTONS-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"action"))
	{
		if (StringsEqual(Value, "none"))
		{
			Action = _mouse_button_does_nothing;
			ActionPresent = true;
			return true;
		}
		else if (StringsEqual(Value, "left_trigger"))
		{
			Action = _mouse_button_fires_left_trigger;
			ActionPresent = true;
			return true;
		}
		else if (StringsEqual(Value, "right_trigger"))
		{
			Action = _mouse_button_fires_right_trigger;
			ActionPresent = true;
			return true;
		}
		else return false;
	}
	return true;
}

bool XML_MouseButtonPrefsParser::AttributesDone()
{
	// Verify...
	if (!(IndexPresent && ActionPresent))
	{
		AttribsMissing();
		return false;
	}
	
	input_preferences->mouse_button_actions[Index] = Action;
			
	return true;
}

static XML_MouseButtonPrefsParser MouseButtonPrefsParser("mouse_button");

class XML_AxisMappingPrefsParser : public XML_ElementParser
{
	bool IndexPresent, AxisPresent, SensitivityPresent, BoundPresent;
	int16 Index, Axis, Bound;
	float Sensitivity;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_AxisMappingPrefsParser(const char *_Name) : XML_ElementParser(_Name) { }
};

bool XML_AxisMappingPrefsParser::Start()
{
	IndexPresent = AxisPresent = SensitivityPresent = BoundPresent = false;

	return true;
}

bool XML_AxisMappingPrefsParser::HandleAttribute(const char* Tag, const char* Value)
{
	if (StringsEqual(Tag, "index"))
	{
		if (ReadBoundedInt16Value(Value, Index, 0, NUMBER_OF_JOYSTICK_MAPPINGS - 1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag, "axis"))
	{
		if (ReadBoundedInt16Value(Value, Axis, -1, 7))
		{
			AxisPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag, "axis_sensitivity"))
	{
		if (ReadFloatValue(Value, Sensitivity))
		{
			SensitivityPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag, "bound"))
	{
		if (ReadBoundedInt16Value(Value, Bound, 0, SHRT_MAX))
		{
			BoundPresent = true;
			return true;
		}
		else return false;
	}

	return true;
}

bool XML_AxisMappingPrefsParser::AttributesDone()
{
	if (!(IndexPresent && AxisPresent))
	{
		AttribsMissing();
		return false;
	}

	input_preferences->joystick_axis_mappings[Index] = Axis;
	if (SensitivityPresent)
	{
		input_preferences->joystick_axis_sensitivities[Index] = Sensitivity;
	}
	if (BoundPresent)
	{
		input_preferences->joystick_axis_bounds[Index] = Bound;
	}
	return true;
}

static XML_AxisMappingPrefsParser AxisMappingPrefsParser("joystick_axis_mapping");


class XML_KeyPrefsParser: public XML_ElementParser
{
	bool IndexPresent, KeyValPresent;
	int16 Index, KeyVal;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_KeyPrefsParser(const char *_Name): XML_ElementParser(_Name) {}
};

bool XML_KeyPrefsParser::Start()
{
	IndexPresent = KeyValPresent = false;
	
	return true;
}

bool XML_KeyPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,NUMBER_OF_KEYS+NUMBER_OF_SHELL_KEYS-1))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	if (StringsEqual(Tag,"value"))
	{
		if (ReadInt16Value(Value,KeyVal))
		{
			KeyValPresent = true;
			return true;
		}
		else return false;
	}
	return true;
}

bool XML_KeyPrefsParser::AttributesDone()
{
	// Verify...
	if (!(IndexPresent && KeyValPresent))
	{
		AttribsMissing();
		return false;
	}
	if (Index >= NUMBER_OF_KEYS)
		input_preferences->shell_keycodes[Index - NUMBER_OF_KEYS] = KeyVal;
	else
		input_preferences->keycodes[Index] = KeyVal;
			
	return true;
}


// This compilation trick guarantees that both Mac and SDL versions will ignore the other's
// key values; for each platform, the parser of the other platform's key values
// is a dummy parser.
static XML_ElementParser MacKeyPrefsParser("mac_key");
static XML_KeyPrefsParser SDLKeyPrefsParser("sdl_key");


class XML_InputPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_InputPrefsParser(): XML_ElementParser("input") {}
};

bool XML_InputPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"device"))
	{
		return ReadInt16Value(Value,input_preferences->input_device);
	}
	else if (StringsEqual(Tag,"modifiers"))
	{
		return ReadUInt16Value(Value,input_preferences->modifiers);
	}
	// ZZZ: sensitivity scaling factor
	// LP addition: split into separate horizontal and vertical sensitivities
	else if (StringsEqual(Tag, "sensitivity"))
	{
		_fixed sensitivity;
		if (ReadInt32Value(Value, sensitivity))
		{
			input_preferences->sens_horizontal = sensitivity;
			input_preferences->sens_vertical = sensitivity;
			return true;
		}
        else
        	return false;
	}
	else if (StringsEqual(Tag, "sens_horizontal"))
	{
		return ReadInt32Value(Value, input_preferences->sens_horizontal);
	}
	else if (StringsEqual(Tag, "sens_vertical"))
	{
		return ReadInt32Value(Value, input_preferences->sens_vertical);
	}
	else if(StringsEqual(Tag, "mouse_acceleration")) {
		return ReadBooleanValue(Value, input_preferences->mouse_acceleration);
	}
	else if (StringsEqual(Tag, "joystick_id"))
	{
		return ReadInt16Value(Value, input_preferences->joystick_id);
	}
	return true;
}

static XML_InputPrefsParser InputPrefsParser;


class XML_SoundPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_SoundPrefsParser(): XML_ElementParser("sound") {}
};

bool XML_SoundPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"channels"))
	{
		return ReadInt16Value(Value,sound_preferences->channel_count);
	}
	else if (StringsEqual(Tag,"volume"))
	{
		return ReadInt16Value(Value,sound_preferences->volume);
	}
	else if (StringsEqual(Tag,"music_volume"))
	{
		return ReadInt16Value(Value,sound_preferences->music);
	}
	else if (StringsEqual(Tag,"flags"))
	{
		return ReadUInt16Value(Value,sound_preferences->flags);
	}
	else if (StringsEqual(Tag,"rate"))
	{
		return ReadUInt16Value(Value, sound_preferences->rate);
	}
	else if (StringsEqual(Tag,"samples"))
	{
		return ReadUInt16Value(Value, sound_preferences->samples);
	}
	else if (StringsEqual(Tag,"volume_while_speaking"))
	{
		return ReadInt16Value(Value, sound_preferences->volume_while_speaking);
	}
	else if (StringsEqual(Tag,"mute_while_transmitting"))
	{
		return ReadBooleanValue(Value, sound_preferences->mute_while_transmitting);
	}
	return true;
}

static XML_SoundPrefsParser SoundPrefsParser;

class XML_NetworkPrefsParser: public XML_ElementParser
{
	rgb_color Colors[2];
public:
	bool Start();
	bool End();
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_NetworkPrefsParser(): XML_ElementParser("network") {}
};

bool XML_NetworkPrefsParser::Start()
{
	for (int i = 0; i < 2; i++)
	{
		CopyColor(Colors[i], network_preferences->metaserver_colors[i]);
	}
	Color_SetArray(Colors, 2);

	return true;
}

bool XML_NetworkPrefsParser::End()
{
	for (int i = 0; i < 2; i++)
	{
		CopyColor(network_preferences->metaserver_colors[i], Colors[i]);
	}

	return true;
}

bool XML_NetworkPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"microphone"))
	{
		return ReadBooleanValue(Value,network_preferences->allow_microphone);
	}
	else if (StringsEqual(Tag,"untimed"))
	{
		return ReadBooleanValue(Value,network_preferences->game_is_untimed);
	}
	else if (StringsEqual(Tag,"type"))
	{
		return ReadInt16Value(Value,network_preferences->type);
	}
	else if (StringsEqual(Tag,"game_type"))
	{
		return ReadInt16Value(Value,network_preferences->game_type);
	}
	else if (StringsEqual(Tag,"difficulty"))
	{
		return ReadInt16Value(Value,network_preferences->difficulty_level);
	}
	else if (StringsEqual(Tag,"game_options"))
	{
		return ReadUInt16Value(Value,network_preferences->game_options);
	}
	else if (StringsEqual(Tag,"time_limit"))
	{
		return ReadInt32Value(Value,network_preferences->time_limit);
	}
	else if (StringsEqual(Tag,"kill_limit"))
	{
		return ReadInt16Value(Value,network_preferences->kill_limit);
	}
	else if (StringsEqual(Tag,"entry_point"))
	{
		return ReadInt16Value(Value,network_preferences->entry_point);
	}
        else if (StringsEqual(Tag,"autogather"))
        {
                return ReadBooleanValue(Value,network_preferences->autogather);
        }
        else if (StringsEqual(Tag,"join_by_address"))
        {
                return ReadBooleanValue(Value,network_preferences->join_by_address);
        }
        else if (StringsEqual(Tag,"join_address"))
        {
                DeUTF8_C(Value,strlen(Value),network_preferences->join_address,255);
                return true;
        }
        else if (StringsEqual(Tag,"local_game_port"))
        {
                return ReadUInt16Value(Value,network_preferences->game_port);
        }
	else if (StringsEqual(Tag,"game_protocol"))
	{
		size_t i;
		for(i = 0; i < NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES; i++)
		{
			if(StringsEqual(Value,sNetworkGameProtocolNames[i]))
				break;
		}
		if(i < NUMBER_OF_NETWORK_GAME_PROTOCOL_NAMES)
		{
			network_preferences->game_protocol= i;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"use_speex_netmic_encoder"))
	{
		return ReadBooleanValue(Value,network_preferences->use_speex_encoder);
	}
	else if (StringsEqual(Tag,"use_netscript"))
	{
		return ReadBooleanValue(Value,network_preferences->use_netscript);
	}
	else if (StringsEqual(Tag,"netscript_file"))
	{
		char tempstr[256];
		expand_symbolic_paths(tempstr, Value, 255);
		DeUTF8_C(tempstr,strlen(tempstr),network_preferences->netscript_file,sizeof(network_preferences->netscript_file));
	}
	else if (StringsEqual(Tag,"cheat_flags"))
        {
		return ReadUInt16Value(Value,network_preferences->cheat_flags);
	}
	else if (StringsEqual(Tag,"advertise_on_metaserver"))
	{
		return ReadBooleanValue(Value,network_preferences->advertise_on_metaserver);
	} 
	else if (StringsEqual(Tag,"attempt_upnp"))
	{
		return ReadBooleanValue(Value, network_preferences->attempt_upnp);
	}
	else if (StringsEqual(Tag,"check_for_updates"))
	{
		return ReadBooleanValue(Value, network_preferences->check_for_updates);
	}
	else if (StringsEqual(Tag,"use_custom_metaserver_colors"))
	{
		return ReadBooleanValue(Value, network_preferences->use_custom_metaserver_colors);
	}
	else if (StringsEqual(Tag,"metaserver_login"))
	{
		DeUTF8_C(Value, strlen(Value),network_preferences->metaserver_login, sizeof(network_preferences->metaserver_login));
		return true;
	}
	else if (StringsEqual(Tag,"mute_metaserver_guests"))
	{
		return ReadBooleanValue(Value, network_preferences->mute_metaserver_guests);
	}
	else if (StringsEqual(Tag,"metaserver_clear_password"))
	{
		DeUTF8_C(Value, strlen(Value),network_preferences->metaserver_password, sizeof(network_preferences->metaserver_password));
		return true;
	}
	else if (StringsEqual(Tag,"metaserver_password"))
	{
		char obscure_password[32];
		DeUTF8(Value, strlen(Value), obscure_password, sizeof(obscure_password));
		for (int i = 0; i < 16; i++)
		{
			unsigned int c;
			sscanf(obscure_password + i*2, "%2x", &c);
			network_preferences->metaserver_password[i] = (char) c ^ sPasswordMask[i];
		}
		return true;
	}
	return true;
}

static XML_NetworkPrefsParser NetworkPrefsParser;


static XML_ElementParser MacFSSpecPrefsParser("mac_fsspec");

class XML_DisablePluginsParser : public XML_ElementParser
{
public:
	bool HandleAttribute(const char* Tag, const char* Value);

	XML_DisablePluginsParser() : XML_ElementParser("disable_plugin") {}
};

bool XML_DisablePluginsParser::HandleAttribute(const char* Tag, const char* Value) {
	if (StringsEqual(Tag, "path")) {
		char tempstr[256];
		Plugins::instance()->disable(expand_symbolic_paths(tempstr, Value, 255));
		return true;
	}

	return true;
}

static XML_DisablePluginsParser DisablePluginsParser;

class XML_EnvironmentPrefsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_EnvironmentPrefsParser(): XML_ElementParser("environment") {}
};

bool XML_EnvironmentPrefsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"map_file"))
	{
		expand_symbolic_paths(environment_preferences->map_file, Value, 255);
		return true;
	}
	else if (StringsEqual(Tag,"physics_file"))
	{
		expand_symbolic_paths(environment_preferences->physics_file, Value, 255);
		return true;
	}
	else if (StringsEqual(Tag,"shapes_file"))
	{
		expand_symbolic_paths(environment_preferences->shapes_file, Value, 255);
		return true;
	}
	else if (StringsEqual(Tag,"sounds_file"))
	{
		expand_symbolic_paths(environment_preferences->sounds_file, Value, 255);
		return true;
	}
	else if (StringsEqual(Tag, "resources_file"))
	{
		expand_symbolic_paths(environment_preferences->resources_file, Value, 255);
	}
	else if (StringsEqual(Tag,"theme_dir"))
	{
		return true;
	}
	else if (StringsEqual(Tag,"map_checksum"))
	{
		return ReadUInt32Value(Value,environment_preferences->map_checksum);
	}
	else if (StringsEqual(Tag,"physics_checksum"))
	{
		return ReadUInt32Value(Value,environment_preferences->physics_checksum);
	}
	else if (StringsEqual(Tag,"shapes_mod_date"))
	{
		uint32 ModDate;
		if (ReadUInt32Value(Value,ModDate))
		{
			environment_preferences->shapes_mod_date = TimeType(ModDate);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"sounds_mod_date"))
	{
		uint32 ModDate;
		if (ReadUInt32Value(Value,ModDate))
		{
			environment_preferences->sounds_mod_date = TimeType(ModDate);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"group_by_directory"))
	{
		return ReadBooleanValue(Value,environment_preferences->group_by_directory);
	}
	else if (StringsEqual(Tag,"reduce_singletons"))
	{
		return ReadBooleanValue(Value,environment_preferences->reduce_singletons);
	}
	else if (StringsEqual(Tag,"non_bungie_warning"))
	{
		// obsolete
		return true;
	}
	else if (StringsEqual(Tag,"smooth_text"))
	{
		return ReadBooleanValue(Value, environment_preferences->smooth_text);
	}
	else if (StringsEqual(Tag,"solo_lua_file"))
	{
		expand_symbolic_paths(environment_preferences->solo_lua_file, Value, 255);
		return true;
	}
	else if (StringsEqual(Tag,"use_solo_lua"))
	{
		return ReadBooleanValue(Value, environment_preferences->use_solo_lua);
	}
	else if (StringsEqual(Tag,"use_replay_net_lua"))
	{
		return ReadBooleanValue(Value, environment_preferences->use_replay_net_lua);
	}
	else if (StringsEqual(Tag,"hud_lua_file"))
	{
		return true;
	}
	else if (StringsEqual(Tag,"use_hud_lua"))
	{
		return true;
	}
	else if (StringsEqual(Tag, "hide_alephone_extensions"))
	{
		return ReadBooleanValue(Value, environment_preferences->hide_extensions);
	}
	else if (StringsEqual(Tag, "film_profile"))
	{
		uint32 profile;
		if (ReadUInt32Value(Value, profile))
		{
			environment_preferences->film_profile = static_cast<FilmProfileType>(profile);
			return true;
		} 
		return false;
	}
	else if (StringsEqual(Tag,"maximum_quick_saves"))
	{
		return ReadUInt32Value(Value,environment_preferences->maximum_quick_saves);
	}
	return true;
}

static XML_EnvironmentPrefsParser EnvironmentPrefsParser;



void SetupPrefsParseTree()
{
	// Add the root object here
	PrefsRootParser.AddChild(&MarathonPrefsParser);
	
	// Add all the others
	
	VoidPrefsParser.AddChild(Color_GetParser());
	GraphicsPrefsParser.AddChild(&VoidPrefsParser);
	LandscapePrefsParser.AddChild(Color_GetParser());
	GraphicsPrefsParser.AddChild(&LandscapePrefsParser);
	GraphicsPrefsParser.AddChild(&TexturePrefsParser);
	MarathonPrefsParser.AddChild(&GraphicsPrefsParser);
	
	PlayerPrefsParser.AddChild(&ChaseCamPrefsParser);
	CrosshairsPrefsParser.AddChild(Color_GetParser());
	PlayerPrefsParser.AddChild(&CrosshairsPrefsParser);
	MarathonPrefsParser.AddChild(&PlayerPrefsParser);

	InputPrefsParser.AddChild(&MouseButtonPrefsParser);
	InputPrefsParser.AddChild(&AxisMappingPrefsParser);
	InputPrefsParser.AddChild(&MacKeyPrefsParser);
	InputPrefsParser.AddChild(&SDLKeyPrefsParser);
	MarathonPrefsParser.AddChild(&InputPrefsParser);
	
	MarathonPrefsParser.AddChild(&SoundPrefsParser);

#if !defined(DISABLE_NETWORKING)
	NetworkPrefsParser.AddChild(Color_GetParser());
	NetworkPrefsParser.AddChild(StarGameProtocol::GetParser());
	NetworkPrefsParser.AddChild(RingGameProtocol::GetParser());
#endif // !defined(DISABLE_NETWORKING)
	NetworkPrefsParser.AddChild(&MacFSSpecPrefsParser);
	MarathonPrefsParser.AddChild(&NetworkPrefsParser);
	
	EnvironmentPrefsParser.AddChild(&MacFSSpecPrefsParser);
	EnvironmentPrefsParser.AddChild(&DisablePluginsParser);
	MarathonPrefsParser.AddChild(&EnvironmentPrefsParser);
}

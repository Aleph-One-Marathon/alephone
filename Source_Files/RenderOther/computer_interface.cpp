/*
	computer_interface.c

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

	Monday, May 8, 1995 7:18:55 PM- rdm created.

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 16, 2000 (Loren Petrich):
	Set up for more graceful degradation when
	there is no terminal data in a map;
	also had get_indexed_terminal_data() return NULL
	if terminal data could not be found.

Feb 18, 2000 (Loren Petrich):
	Suppressed all the dprintf warnings, such as one for a picture being
	too large for a terminal.

Feb 19, 2000 (Loren Petrich):
	Had a frustrating time getting get_date_string() to work properly;
	SecondsToDate() seems to be broken.

Feb 21, 2000 (Loren Petrich):
	Corrected a troublesome assert in find_checkpoint_location()
	Moved drawing of borders to after drawing of other stuff,
	to fix checkpoint-overdraw bug.

Feb 26, 2000 (Loren Petrich):
	Fixed level-0 teleportation bug; the hack is to move the destination
	down by 1.
	
	Set font in PICT-error-message code to "systemFont".

Mar 5, 2000 (Loren Petrich):
	Improved handling of clipping in _render_computer_interface();
	it is trimmed to the terminal window when rendering there, and restored
	to the screen's full size when leaving.

May 11, 2000 (Loren Petrich):
	Suppressed vhalts here; these were for unrecognized terminal states.
	Changed them into vwarns.

May 20, 2000 (Loren Petrich):
	Put in more graceful degradation for
	get_indexed_grouping()
	get_indexed_font_changes()
	These will return NULL for an invalid index

July 5, 2000 (Loren Petrich):
	Using world_pixels instead of screen_window

Aug 10, 2000 (Loren Petrich):
	Added Chris Pruett's Pfhortran changes

Aug 22, 2000 (Loren Petrich):
	Added object-oriented resource handling

Sep 24, 2000 (Loren Petrich):
	Banished OverallBounds as unnecessary; world_pixels->portRect does fine here

Jun 23, 2001 (Loren Petrich):
	Suppressed some of the asserts in the code; tried to make degradation more graceful

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
*/

// add logon/logoff keywords. (& make terminal display them)
// cameras
// static
// delayed teleport

// activate tags
// picture with scrolling text.
// checkpoint with scrolling text
// don't use charwidth

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <vector>

#include "cseries.h"
#include "FileHandler.h"

#include "world.h"
#include "map.h"
#include "player.h"
#include "computer_interface.h"
#include "screen_drawing.h"
#include "overhead_map.h"
#include "SoundManager.h"
#include "interface.h" // for the error strings.
#include "shell.h"
#include "platforms.h" // for tagged platforms
#include "lightsource.h" // for tagged lightsources
#include "screen.h"

#include "images.h"
#include "Packing.h"

// MH: Lua scripting
#include "lua_script.h"

#include "Logging.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <sstream>

namespace algo = boost::algorithm;
namespace io = boost::iostreams;

#define LABEL_INSET 3
#define LOG_DURATION_BEFORE_TIMEOUT (2*TICKS_PER_SECOND)
#define BORDER_HEIGHT 18
#define BORDER_INSET 9
#define FUDGE_FACTOR 1

#define MAC_LINE_END 13

enum {
	_reading_terminal,
	_no_terminal_state,
	NUMBER_OF_TERMINAL_STATES
};

enum {
	_terminal_is_dirty= 0x01
};

enum {
	_any_abort_key_mask= _action_trigger_state,
	_terminal_up_arrow= _moving_forward,
	_terminal_down_arrow= _moving_backward,
	_terminal_page_down= _turning_right,
	_terminal_page_up= _turning_left,
	_terminal_next_state= _left_trigger_state
};

#define strCOMPUTER_LABELS 135
enum
{
	_marathon_name,
	_computer_starting_up,
	_computer_manufacturer,
	_computer_address,
	_computer_terminal,
	_scrolling_message,
	_acknowledgement_message,
	_disconnecting_message,
	_connection_terminated_message,
	_date_format
};

#define TERMINAL_IS_DIRTY(term) ((term)->flags & _terminal_is_dirty)
#define SET_TERMINAL_IS_DIRTY(term, v) ((void)((v)? ((term)->flags |= _terminal_is_dirty) : ((term)->flags &= ~_terminal_is_dirty)))

/* Maximum face changes per text grouping.. */
#define MAXIMUM_FACE_CHANGES_PER_TEXT_GROUPING (128)

enum {
	_text_is_encoded_flag= 0x0001
};

enum {
	_logon_group,
	_unfinished_group,
	_success_group,
	_failure_group,
	_information_group,
	_end_group,
	_interlevel_teleport_group, // permutation is level to go to
	_intralevel_teleport_group, // permutation is polygon to go to.
	_checkpoint_group, // permutation is the goal to show
	_sound_group, // permutation is the sound id to play
	_movie_group, // permutation is the movie id to play
	_track_group, // permutation is the track to play
	_pict_group, // permutation is the pict to display
	_logoff_group,
	_camera_group, //  permutation is the object index
	_static_group, // permutation is the duration of static.
	_tag_group, // permutation is the tag to activate

	NUMBER_OF_GROUP_TYPES
};

enum // flags to indicate text styles for paragraphs
{
	_plain_text      = 0x00,
	_bold_text       = 0x01,
	_italic_text     = 0x02,
	_underline_text  = 0x04
};

enum { /* terminal grouping flags */
	_draw_object_on_right= 0x01,  // for drawing checkpoints, picts, movies.
	_center_object= 0x02,
	_group_is_marathon_1 = 0x100
};

struct terminal_groupings {
	int16 flags; /* varies.. */
	int16 type; /* _information_text, _checkpoint_text, _briefing_text, _movie, _sound_bite, _soundtrack */
	int16 permutation; /* checkpoint id for chkpt, level id for _briefing, movie id for movie, sound id for sound, soundtrack id for soundtrack */
	int16 start_index;
	int16 length;
	int16 maximum_line_count;
};
const int SIZEOF_terminal_groupings = 12;

struct text_face_data {
	int16 index;
	int16 face;
	int16 color;
};
const int SIZEOF_text_face_data = 6;

// This is externally visible, so its external size is defined in the header file
struct player_terminal_data
{
	int16 flags;
	int16 phase;
	int16 state;
	int16 current_group;
	int16 level_completion_state;
	int16 current_line;
	int16 maximum_line;
	int16 terminal_id;
	int32 last_action_flag;
};

struct terminal_key {
	int16 keycode;
	int16 offset;
	int16 mask;
	int32 action_flag;
};

struct font_dimensions {
	int16 lines_per_screen;
	int16 character_width;
};

/* Terminal data loaded from map (maintained by computer_interface.cpp) */
struct terminal_text_t {	// Object describing one terminal
	terminal_text_t() {}
	uint16 flags = 0;
	int16 lines_per_page = 0;
	vector<terminal_groupings> groupings;
	vector<text_face_data> font_changes;
	vector<uint8> text;
};

static vector<terminal_text_t> map_terminal_text;

// ghs: for Lua
short number_of_terminal_texts() { return map_terminal_text.size(); }

/* internal global structure */
static struct player_terminal_data *player_terminals;

#define NUMBER_OF_TERMINAL_KEYS (sizeof(terminal_keys)/sizeof(struct terminal_key))

// Get the interface font to use from screen_drawing_<platform>.cpp
extern TextSpec *_get_font_spec(short font_index);
extern font_info *GetInterfaceFont(short font_index);
extern uint16 GetInterfaceStyle(short font_index);

/* ------------ private prototypes */
static player_terminal_data *get_player_terminal_data(
	short player_index);

static void draw_logon_text(terminal_text_t *terminal_text,
	short current_group_index, short logon_shape_id);
static void draw_computer_text(Rect *bounds, 
	terminal_text_t *terminal_text, short current_group_index, short current_line);
static void _draw_computer_text(char *base_text, short start_index, Rect *bounds,
	terminal_text_t *terminal_text, short current_line);
static short find_group_type(terminal_text_t *data, 
	short group_type);
static void teleport_to_level(short level_number, int16 delay_before_teleport);
static void teleport_to_polygon(short player_index, short polygon_index);
static struct terminal_groupings *get_indexed_grouping(
	terminal_text_t *data, short index);
static struct text_face_data *get_indexed_font_changes(
	terminal_text_t *data, short index);
static char *get_text_base(terminal_text_t *data);
// LP change: added a flag to indicate whether stuff after the other
// terminal stuff is to be drawn; if not, then draw the stuff before the
// other terminal stuff.
static void draw_terminal_borders(struct player_terminal_data *terminal_data,
	bool after_other_terminal_stuff);
static void next_terminal_state(short player_index);
static void next_terminal_group(short player_index, terminal_text_t *terminal_text);
static void get_date_string(char *date_string, short flags);
static void present_checkpoint_text(terminal_text_t *terminal_text, short current_group_index,
	short current_line);
static bool find_checkpoint_location(short checkpoint_index, world_point2d *location, 
	short *polygon_index);
static void	set_text_face(struct text_face_data *text_face);
static void draw_line(char *base_text, short start_index, short end_index, Rect *bounds,
	terminal_text_t *terminal_text, short *text_face_start_index,
	short line_number);
static bool calculate_line(char *base_text, short width, short start_index, 
	short text_end_index, short *end_index);
static void handle_reading_terminal_keys(short player_index, int32 action_flags);
static void calculate_bounds_for_object(short flags, Rect *bounds, Rect *source);
static void display_picture(short picture_id, Rect *frame, short flags);
static void display_shape(short shape, Rect* frame);
static void display_picture_with_text(struct player_terminal_data *terminal_data, 
	Rect *bounds, terminal_text_t *terminal_text, short current_lien);
static short count_total_lines(char *base_text, short width, short start_index, short end_index);
static void calculate_bounds_for_text_box(short flags, Rect *bounds);
static void goto_terminal_group(short player_index, terminal_text_t *terminal_text, 
	short new_group_index);
static bool previous_terminal_group(short player_index, terminal_text_t *terminal_text);
static void fill_terminal_with_static(Rect *bounds);
static short calculate_lines_per_page(void);
static Rect get_term_rectangle(short index);

static terminal_text_t *get_indexed_terminal_data(short id);
static void encode_text(terminal_text_t *terminal_text);
static void decode_text(terminal_text_t *terminal_text);

#include "sdl_fonts.h"
#include "joystick.h" // for AO_SCANCODE_BASE_JOYSTICK_BUTTON


// Global variables
// static const sdl_font_info *terminal_font = NULL;
static uint32 current_pixel;				// Current color pixel value
static uint16 current_style = styleNormal;	// Current style flags

// From screen_sdl.cpp
extern SDL_Surface *world_pixels;


// Terminal key definitions
static struct terminal_key terminal_keys[]= {
	{SDL_SCANCODE_UP, 0, 0, _terminal_page_up},				// arrow up
	{SDL_SCANCODE_DOWN, 0, 0, _terminal_page_down},			// arrow down
	{SDL_SCANCODE_PAGEUP, 0, 0, _terminal_page_up},			// page up
	{SDL_SCANCODE_PAGEDOWN, 0, 0, _terminal_page_down},		// page down
	{SDL_SCANCODE_TAB, 0, 0, _terminal_next_state},			// tab
	{SDL_SCANCODE_KP_ENTER, 0, 0, _terminal_next_state},	// enter
	{SDL_SCANCODE_RETURN, 0, 0, _terminal_next_state},		// return
	{SDL_SCANCODE_SPACE, 0, 0, _terminal_next_state},		// space
	{SDL_SCANCODE_ESCAPE, 0, 0, _any_abort_key_mask},		// escape
	{AO_SCANCODE_JOYSTICK_ESCAPE, 0, 0, _any_abort_key_mask},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_DPAD_UP, 0, 0, _terminal_page_up},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_DPAD_DOWN, 0, 0, _terminal_page_down},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_A, 0, 0, _terminal_next_state},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_X, 0, 0, _terminal_next_state},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_Y, 0, 0, _terminal_next_state},
	{AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_B, 0, 0, _any_abort_key_mask}
};


// Emulation of MacOS functions
static void InsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom -= dy;
	r->right -= dx;
}

static void OffsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom += dy;
	r->right += dx;
}

extern SDL_Surface *draw_surface;

static void	set_text_face(struct text_face_data *text_face)
{
	current_style = styleNormal;

	// Set style
	if (text_face->face & _bold_text)
		current_style |= styleBold;
	if (text_face->face & _italic_text)
		current_style |= styleItalic;
	if (text_face->face & _underline_text)
		current_style |= styleUnderline;

	// Set color
	SDL_Color color;
	_get_interface_color(text_face->color + _computer_interface_text_color, &color);
	current_pixel = SDL_MapRGB(/*world_pixels*/draw_surface->format, color.r, color.g, color.b);
}

static bool can_break_after(char c)
{
	// determined empirically on my PowerBook
	switch (c)
	{
	case '&':
	case '*':
	case '+':
	case '-':
	case '\\':
	case '<':
	case '=':
	case '>':
	case '/':
	case '^':
	case '|':
		return true;
	default:
		return false;
	}
}

static bool calculate_line(char *base_text, short width, short start_index, short text_end_index, short *end_index)
{
	bool done = false;

	if (base_text[start_index]) {
		int index = start_index, running_width = 0;
		
		// terminal_font no longer a global, since it may change
		font_info *terminal_font = GetInterfaceFont(_computer_interface_font);

		while (running_width < width && base_text[index] && base_text[index] != MAC_LINE_END) {
			running_width += char_width(base_text[index], terminal_font, current_style);
			index++;
		}

		// Now go backwards, looking for a place to split
		if (base_text[index] == MAC_LINE_END)
			index++;
		else if (base_text[index]) {
			if (film_profile.better_terminal_word_wrap)
			{
				int break_point = index - 1;
				while (break_point > start_index)
				{
					if (base_text[break_point] == ' ')
					{
						index = break_point + 1; // eat the space
						break;
					}
					else if (break_point > start_index + 1 &&
							 can_break_after(base_text[break_point - 1]))
					{
						index = break_point;
						break;
					}

					--break_point;
				}
			}
			else
			{
				int break_point = index;
				
				while (break_point>start_index) {
					if (base_text[break_point] == ' ')
						break; 	// Non printing
					break_point--;	// this needs to be in front of the test
				}
				
				if (break_point != start_index)
					index = break_point+1;	// Space at the end of the line
			}
		}
		
		*end_index= index;
	} else
		done = true;
	
	return done;
}

/* ------------ code begins */

player_terminal_data *get_player_terminal_data(
	short player_index)
{
	struct player_terminal_data *player_terminal = GetMemberWithBounds(player_terminals,player_index,MAXIMUM_NUMBER_OF_PLAYERS);
	
	vassert(player_terminal, csprintf(temporary, "player index #%d is out of range", player_index));

	return player_terminal;
}

void initialize_terminal_manager(
	void)
{
	player_terminals= new player_terminal_data[MAXIMUM_NUMBER_OF_PLAYERS];
	assert(player_terminals);
	objlist_clear(player_terminals, MAXIMUM_NUMBER_OF_PLAYERS);

/*
	terminal_font = load_font(*_get_font_spec(_computer_interface_font));
*/
}

void initialize_player_terminal_info(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);

	//CP Addition: trap for logout!
	if (terminal->state != _no_terminal_state)
        {
                L_Call_Terminal_Exit(terminal->terminal_id, player_index);
        }

	terminal->flags= 0;
	terminal->phase = NONE; // not using a control panel.
	terminal->state= _no_terminal_state; // And there is no line..
	terminal->current_group= NONE;
	terminal->level_completion_state= 0;
	terminal->current_line= 0;
	terminal->maximum_line= 0;
	terminal->terminal_id= 0;
	terminal->last_action_flag= -1l; /* Eat the first key */
}

void enter_computer_interface(
	short player_index, 
	short text_number, 
	short completion_flag)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	struct player_data *player= get_player_data(player_index);

	// LP addition: if there is no terminal-data chunk, then just make the logon sound and quit
/*	if (map_terminal_text.size() == 0)
	{
		play_object_sound(player->object_index, Sound_TerminalLogon());
		return;
	}
*/
	struct terminal_text_t *terminal_text = get_indexed_terminal_data(text_number);
	if (terminal_text == NULL)
	{
		play_object_sound(player->object_index, Sound_TerminalLogon());
		return;
	}
	
	if(dynamic_world->player_count==1)
	{
		short lines_per_page;
	
		/* Reset the lines per page to the actual value for whatever fucked up font that they have */
		lines_per_page= calculate_lines_per_page();
		if(lines_per_page != terminal_text->lines_per_page)
		{
// dprintf("You have one confused font.");
			terminal_text->lines_per_page= lines_per_page;
		}
	}

	/* Tell everyone that this player is in the computer interface.. */	
	terminal->state= _reading_terminal;
	terminal->phase= NONE;
	terminal->current_group= NONE;
	terminal->level_completion_state= completion_flag;
	terminal->current_line= 0;
	terminal->maximum_line= 1; // any click or keypress will get us out.
	terminal->terminal_id= text_number;
	terminal->last_action_flag= -1l; /* Eat the first key */

	/* And select the first one. */
	next_terminal_group(player_index, terminal_text);
}

/*  Assumes ¶t==1 tick */
void update_player_for_terminal_mode(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	if (!terminal) return;

	if(terminal->state != _no_terminal_state)
	{
		/* Phase is a timer for logging in and out only.. */
		switch(terminal->state)
		{
			case _reading_terminal:
				if(terminal->phase != NONE)
				{
					if(--terminal->phase<=0)
					{
						terminal_text_t *terminal_text = get_indexed_terminal_data(terminal->terminal_id);
						if (terminal_text == NULL)
							break;
						next_terminal_group(player_index, terminal_text);
					}
				}
				break;
				
			default:
				break;
		}
	}
}

void update_player_keys_for_terminal(
	short player_index,
	uint32 action_flags)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	if (!terminal) return;

	switch(terminal->state)
	{
		case _reading_terminal:
			handle_reading_terminal_keys(player_index, action_flags);
			break;
		
		case _no_terminal_state:
		default:
			break;
	}
}

bool player_in_terminal_mode(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	bool in_terminal_mode;
	
	if(terminal->state==_no_terminal_state)
	{
		in_terminal_mode= false;
	} else {
		in_terminal_mode= true;
	}
	
	return in_terminal_mode;
}

void _render_computer_interface(void)
{
	struct player_terminal_data *terminal_data= get_player_terminal_data(current_player_index);
	
	if (terminal_data->state == _no_terminal_state) return;
	// assert(terminal_data->state != _no_terminal_state);
	if(TERMINAL_IS_DIRTY(terminal_data))
	{
		SET_TERMINAL_IS_DIRTY(terminal_data, false);

		terminal_text_t *terminal_text;
		struct terminal_groupings *current_group;
		Rect bounds;

		/* Get the terminal text.. */
		terminal_text = get_indexed_terminal_data(terminal_data->terminal_id);
		// LP addition: quit if none
		if (terminal_text == NULL) return;
		
		switch(terminal_data->state)
		{
			case _reading_terminal:
				/* Initialize it if it hasn't been done.. */
				current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
				// LP change: just in case...
				if (!current_group) return;
				
				/* Draw the borders! */
				// LP change: draw these after, so as to avoid overdraw bug
				draw_terminal_borders(terminal_data, false);
				switch(current_group->type)
				{
					case _logon_group:
					case _logoff_group:
						draw_logon_text(terminal_text, terminal_data->current_group,
							current_group->permutation);
						break;
						
					case _unfinished_group:
					case _success_group:
					case _failure_group:
						// dprintf("You shouldn't try to render this view.;g");
						break;
						
					case _information_group:
						/* Draw as normal... */
                        bounds= get_term_rectangle(_terminal_full_text_rect);
						draw_computer_text(&bounds, terminal_text, terminal_data->current_group, 
							terminal_data->current_line);
						break;
						
					case _checkpoint_group: // permutation is the goal to show
						/* note that checkpoints can only be equal to one screenful.. */
						present_checkpoint_text(terminal_text, terminal_data->current_group,
							terminal_data->current_line);
						break;
						
					case _end_group:
					case _interlevel_teleport_group: // permutation is level to go to
					case _intralevel_teleport_group: // permutation is polygon to go to.
					case _sound_group: // permutation is the sound id to play
					case _tag_group:
						// These are all handled elsewhere
						break;
						
					case _movie_group:
					case _track_group:
						if(!game_is_networked)
						{
							// dprintf("Movies/Music Tracks not supported on playback (yet);g");
						} else {
							// dprintf("On networked games, should we display a PICT here?;g");
						}
						break;
					
					case _pict_group:
						display_picture_with_text(terminal_data, &bounds, terminal_text, 
							terminal_data->current_line);
						break;
			
					case _static_group:
						bounds = get_term_rectangle(_terminal_screen_rect);
						fill_terminal_with_static(&bounds);
						SET_TERMINAL_IS_DIRTY(terminal_data, true);
						break;
						
					case _camera_group:
						break;

					default:
						break;
				}
				// Moved down here, so they'd overdraw the other stuff
				draw_terminal_borders(terminal_data, true);
				break;
				
			default:
				break;
		}
		
		RequestDrawingTerm();
	}
}

/* Only care about local_player_index */
uint32 build_terminal_action_flags(
	char *keymap)
{
	uint32 flags, raw_flags;
	struct terminal_key *key= terminal_keys;
	struct player_terminal_data *terminal= get_player_terminal_data(local_player_index);
	
	raw_flags= 0;
	for(unsigned index= 0; index<NUMBER_OF_TERMINAL_KEYS; ++index)
	{
		if (keymap[key->keycode]) raw_flags |= key->action_flag;
		key++;
	}

	/* Only catch the key the first time. */
	flags= raw_flags^terminal->last_action_flag;
	flags&= raw_flags;
	terminal->last_action_flag= raw_flags;

//dprintf("Flags: %x (Raw: %x Mask: %x);g", flags, raw_flags, terminal->last_action_flag);
	return flags;
}

/* called from walk_player_list.. */
void dirty_terminal_view(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	
	if(terminal->state != _no_terminal_state)
	{
		SET_TERMINAL_IS_DIRTY(terminal, true);
	}
}

/* Take damage, abort.. */
void abort_terminal_mode(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	
	if(terminal->state != _no_terminal_state)
	{
		terminal->state= _no_terminal_state;
		L_Call_Terminal_Exit(terminal->terminal_id, player_index);
	}
}

static uint16_t MARATHON_LOGON_SHAPE = 44;

/* --------- local code */
static void draw_logon_text(
	terminal_text_t *terminal_text,
	short current_group_index,
	short logon_shape_id)
{
	Rect picture_bounds= get_term_rectangle(_terminal_logon_graphic_rect);
	char *base_text= get_text_base(terminal_text);
	short width;
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, 
		current_group_index);
	// LP change: just in case...
	if (!current_group) return;
	
	if (current_group->flags & _group_is_marathon_1)
	{
		display_shape(MARATHON_LOGON_SHAPE, &picture_bounds);
        
        // draw static title below logo
        picture_bounds= get_term_rectangle(_terminal_logon_title_rect);
        getcstr(temporary, strCOMPUTER_LABELS, _marathon_name);
		_draw_screen_text(temporary, (screen_rectangle *) &picture_bounds, _center_vertical | _center_horizontal, _computer_interface_title_font, _computer_interface_text_color);

        picture_bounds= get_term_rectangle(_terminal_logon_location_rect);       
	}
	else
	{
        Rect bounds= picture_bounds;
		display_picture(logon_shape_id, &picture_bounds,  _center_object);

        /* Use the picture bounds to create the logon text crap . */	
        picture_bounds.top= picture_bounds.bottom;
        picture_bounds.bottom= bounds.bottom;
        picture_bounds.left= bounds.left;
        picture_bounds.right= bounds.right;
    }

	/* This is always just a line, so we can do this here.. */
	font_info *terminal_font = GetInterfaceFont(_computer_interface_font);
	uint16 terminal_style = GetInterfaceStyle(_computer_interface_font);
	width = text_width(base_text + current_group->start_index, current_group->length, terminal_font, terminal_style);
	// width = text_width(base_text + current_group->start_index, current_group->length, terminal_font, _get_font_spec(_computer_interface_font)->style);
	picture_bounds.left += (RECTANGLE_WIDTH(&picture_bounds) - width) / 2;
	
	_draw_computer_text(base_text, current_group_index, &picture_bounds, terminal_text, 0);
}

/* returns true for phase chagne */
static void draw_computer_text(
	Rect *bounds, 
	terminal_text_t *terminal_text, 
	short current_group_index,
	short current_line)
{
	char *base_text= get_text_base(terminal_text);
	
	_draw_computer_text(base_text, current_group_index, bounds, 
		terminal_text, current_line);
}

/* Returns true if current_line> end of text.. (for phase change) */
static void _draw_computer_text(
	char *base_text,
	short group_index,
	Rect *bounds,
	terminal_text_t *terminal_text,
	short current_line)
{
	bool done= false;
	short start_index;
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, group_index);
	// LP change: just in case...
	if (!current_group) return;
	struct text_face_data text_face;
	short index, last_index, last_text_index, end_index;
	unsigned text_index;

	uint16 old_style = current_style;
	current_style = GetInterfaceStyle(_computer_interface_font);
	// current_style = _get_font_spec(_computer_interface_font)->style;

	start_index= current_group->start_index;
	end_index= current_group->length+current_group->start_index;
	
	/* eat the previous lines */
	for(index= 0; index<current_line; ++index)
	{
		/* Calculate one line.. */
		if(!calculate_line(base_text, RECTANGLE_WIDTH(bounds), start_index, current_group->start_index+current_group->length, 
			&end_index))
		{
			if(end_index>current_group->start_index+current_group->length)
			{
				// dprintf("Start: %d Length: %d End: %d;g", current_group->start_index, current_group->length, end_index);
				// dprintf("Width: %d", RECTANGLE_WIDTH(bounds));
				end_index= current_group->start_index+current_group->length;
			}
			//dprintf("calculate line: %d start: %d end: %d", index, start_index, end_index);
			assert(end_index<=current_group->start_index+current_group->length);
			
			start_index= end_index;
		} else {
			/* End of text.. */
			done= true;
		}
	}

	if(!done)
	{
		/* Go backwards, and see if there were any other face changes... */
		last_index= current_group->start_index;
		last_text_index= NONE;
		for(text_index= 0; text_index<terminal_text->font_changes.size(); ++text_index)
		{
			struct text_face_data *font_face= get_indexed_font_changes(terminal_text, text_index);
			// LP change: just in case...
			if (!font_face) return;
			
			/* Go backwards from the scrolled starting location.. */
			if(font_face->index>last_index && font_face->index<start_index)
			{
				// dprintf("ff index: %d last: %d end: %d", font_face->index, last_index, start_index);
				last_index= font_face->index;
				last_text_index= text_index;
			}
		}
		
		// dprintf("last index: %d", last_text_index);
		/* Figure out the font.. */
		if(last_text_index==NONE)
		{
			/* Default-> plain, etc.. */
			text_face.color= 0;
			text_face.face= 0;
		} else {
			struct text_face_data *font_face= get_indexed_font_changes(terminal_text, 
				last_text_index);
			// LP change: just in case...
			if (!font_face) return;
			text_face= *font_face;
		}
	
		/* Set the font.. */
		set_text_face(&text_face);
	
		/* Draw what is one the screen */
		for(index= 0; !done && index<terminal_text->lines_per_page; ++index)
		{
			//dprintf("calculating the line");
			/* Calculate one line.. */
			if(!calculate_line(base_text, RECTANGLE_WIDTH(bounds), start_index, current_group->start_index+current_group->length, &end_index))
			{
				//dprintf("draw calculate line: %d start: %d end: %d text: %x length: %d lti: %d", index, start_index, end_index, base_text, current_group->length, last_text_index);
				if(end_index>current_group->start_index+current_group->length)
				{
					// dprintf("Start: %d Length: %d End: %d;g", current_group->start_index, current_group->length, end_index);
					// dprintf("Width: %d", RECTANGLE_WIDTH(bounds));
					end_index= current_group->start_index+current_group->length;
				}
				assert(end_index<=current_group->start_index+current_group->length);
				draw_line(base_text, start_index, end_index, bounds, terminal_text, &last_text_index, 
					index);
				start_index= end_index;
			} else {
				/* End of text. */
//				done= true;
			}
		}
	}

	current_style = old_style;
}

static short count_total_lines(
	char *base_text,
	short width,
	short start_index,
	short end_index)
{
	short total_line_count= 0;
	short text_end_index= end_index;

	uint16 old_style = current_style;
	current_style = GetInterfaceStyle(_computer_interface_font);
	// current_style = _get_font_spec(_computer_interface_font)->style;

	while(!calculate_line(base_text, width, start_index, text_end_index, &end_index))
	{
		total_line_count++;
		start_index= end_index;
	}

	current_style = old_style;
	
	return total_line_count;
}

static void draw_line(
	char *base_text, 
	short start_index, 
	short end_index, 
	Rect *bounds,
	terminal_text_t *terminal_text,
	short *text_face_start_index,
	short line_number)
{
	short line_height= _get_font_line_height(_computer_interface_font);
	bool done= false;
	short current_start, current_end= end_index;
	struct text_face_data *face_data= NULL;
	unsigned text_index;

	if((*text_face_start_index)==NONE) 
	{
		text_index= 0;
	} else {
		text_index= (*text_face_start_index);
	}
	
	/* Get to the first one that concerns us.. */
	if(text_index<terminal_text->font_changes.size())
	{
		do {
			face_data= get_indexed_font_changes(terminal_text, text_index);
			// LP change: just in case...
			if (!face_data) return;
			if(face_data->index<start_index) text_index++;
		} while(face_data->index<start_index && text_index<terminal_text->font_changes.size());
	}
	
	current_start= start_index;
	font_info *terminal_font = GetInterfaceFont(_computer_interface_font);
	int xpos = bounds->left;

	while(!done)
	{
		if(text_index<terminal_text->font_changes.size())
		{
			face_data= get_indexed_font_changes(terminal_text, text_index);
			// LP change: just in case...
			if (!face_data) return;

			if(face_data->index>=current_start && face_data->index<current_end)
			{
				current_end= face_data->index;
				text_index++;
				(*text_face_start_index)= text_index;
			}
		}

		xpos += draw_text(/*world_pixels*/ draw_surface, base_text + current_start, current_end - current_start,
		                  xpos, bounds->top + line_height * (line_number + FUDGE_FACTOR),
		                  current_pixel, terminal_font, current_style);
		if(current_end!=end_index)
		{
			current_start= current_end;
			current_end= end_index;
			assert(face_data);
			set_text_face(face_data);
		} else {
			done= true;
		}
	}
}

static short find_group_type(
	terminal_text_t *data, 
	short group_type)
{
	unsigned index;

	for(index= 0; index<data->groupings.size(); index++)
	{
		struct terminal_groupings *group= get_indexed_grouping(data, index);
		// LP change: just in case...
		if (!group) return NONE;
		if(group->type==group_type) break;
	}
	
	return index;
}

static void teleport_to_level(
	short level_number, int16_t delay_before_teleport)
{
	/* It doesn't matter which player we get. */
	struct player_data *player= get_player_data(0);
	
	// LP change: moved down by 1 so that level 0 will be valid
	player->teleporting_destination= -level_number - 1;
	player->delay_before_teleport = delay_before_teleport;
}
			
static void teleport_to_polygon(
	short player_index,
	short polygon_index)
{
	struct player_data *player= get_player_data(player_index);
	
	player->teleporting_destination= polygon_index;
	assert(!player->delay_before_teleport);
}

static void calculate_bounds_for_text_box(
	short flags,
	Rect *bounds)
{
	if(flags & _center_object)
	{
		// dprintf("splitting text not supported!");
		calculate_bounds_for_object(_draw_object_on_right, bounds, NULL);
	} 
	else if(flags & _draw_object_on_right)
	{
		calculate_bounds_for_object(0, bounds, NULL);
	} 
	else 
	{
		calculate_bounds_for_object(_draw_object_on_right, bounds, NULL);
	}
	if (flags & _group_is_marathon_1)
		bounds->top += _get_font_line_height(_computer_interface_font);
}

static void display_picture_with_text(
	struct player_terminal_data *terminal_data, 
	Rect *bounds, 
	terminal_text_t *terminal_text,
	short current_line)
{
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
	// LP change: just in case...
	if (!current_group) return;
	Rect text_bounds;
	Rect picture_bounds;

	assert(current_group->type==_pict_group);
    calculate_bounds_for_object(current_group->flags, &picture_bounds, NULL);
	display_picture(current_group->permutation, &picture_bounds, current_group->flags);

	/* Display the text */
	calculate_bounds_for_text_box(current_group->flags, &text_bounds);
	draw_computer_text(&text_bounds, terminal_text, terminal_data->current_group, current_line);
}

static void display_shape(short shape, Rect* frame)
{
	SDL_Surface* s = get_shape_surface(shape);
	if (s)
	{
		Rect bounds;
		Rect screen_bounds;

		bounds.left = bounds.top = 0;
		bounds.right = s->w;
		bounds.bottom = s->h;
		
		OffsetRect(&bounds, -bounds.left, -bounds.top);
		calculate_bounds_for_object(_center_object, &screen_bounds, &bounds);
		
		OffsetRect(&bounds, screen_bounds.left + (RECTANGLE_WIDTH(&screen_bounds)-RECTANGLE_WIDTH(&bounds))/2, screen_bounds.top + (RECTANGLE_HEIGHT(&screen_bounds)-RECTANGLE_HEIGHT(&bounds))/2);

		SDL_Rect r = { bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top };
		SDL_SetSurfaceAlphaMod(s, 255);
		SDL_BlitSurface(s, NULL, draw_surface, &r);

		SDL_FreeSurface(s);
		*frame = bounds;
	}
}

extern int get_pict_header_width(LoadedResource &);

static void display_picture(
	short picture_id,
	Rect *frame,
	short flags)
{
	LoadedResource PictRsrc;

	auto s = get_picture_resource_from_scenario(picture_id, PictRsrc) ? 
			 picture_to_surface(PictRsrc) :
			 std::shared_ptr<SDL_Surface>(nullptr, SDL_FreeSurface);

	if (s)
	{
		Rect bounds;
		Rect screen_bounds;

		bounds.left = bounds.top = 0;
		bounds.right = s->w;
		bounds.bottom = s->h;

		int pict_header_width = get_pict_header_width(PictRsrc);
		bool cinemascopeHack = false;
		if (bounds.right != pict_header_width)
		{
			cinemascopeHack = true;
			bounds.right = pict_header_width;
		}
		OffsetRect(&bounds, -bounds.left, -bounds.top);
		calculate_bounds_for_object(flags, &screen_bounds, &bounds);

		if(RECTANGLE_WIDTH(&bounds)<=RECTANGLE_WIDTH(&screen_bounds) && 
			RECTANGLE_HEIGHT(&bounds)<=RECTANGLE_HEIGHT(&screen_bounds))
		{
			/* It fits-> center it. */
			OffsetRect(&bounds, screen_bounds.left+(RECTANGLE_WIDTH(&screen_bounds)-RECTANGLE_WIDTH(&bounds))/2,
				screen_bounds.top+(RECTANGLE_HEIGHT(&screen_bounds)-RECTANGLE_HEIGHT(&bounds))/2);
		} else {
			/* Doesn't fit.  Make it, but preserve the aspect ratio like a good little boy */
			if(RECTANGLE_HEIGHT(&bounds)-RECTANGLE_HEIGHT(&screen_bounds)>=
				RECTANGLE_WIDTH(&bounds)-RECTANGLE_WIDTH(&screen_bounds))
			{
				short adjusted_width;
				
				adjusted_width= RECTANGLE_HEIGHT(&screen_bounds)*RECTANGLE_WIDTH(&bounds)/RECTANGLE_HEIGHT(&bounds);
				bounds= screen_bounds;
				InsetRect(&bounds, (RECTANGLE_WIDTH(&screen_bounds)-adjusted_width)/2, 0);
				// dprintf("Warning: Not large enough for pict: %d (height);g", picture_id);
			} else {
				/* Width is the predominant factor */
				short adjusted_height;
				
				adjusted_height= RECTANGLE_WIDTH(&screen_bounds)*RECTANGLE_HEIGHT(&bounds)/RECTANGLE_WIDTH(&bounds);
				bounds= screen_bounds;
				InsetRect(&bounds, 0, (RECTANGLE_HEIGHT(&screen_bounds)-adjusted_height)/2);
				// dprintf("Warning: Not large enough for pict: %d (width);g", picture_id);
			}
		}

//		warn(HGetState((Handle) picture) & 0x40); // assert it is purgable.

		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		if ((s->w == r.w && s->h == r.h) || cinemascopeHack)
			SDL_BlitSurface(s.get(), NULL, /*world_pixels*/draw_surface, &r);
		else {
			// Rescale picture
			SDL_Surface *s2 = rescale_surface(s.get(), r.w, r.h);
			if (s2) {
				SDL_BlitSurface(s2, NULL, /*world_pixels*/draw_surface, &r);
				SDL_FreeSurface(s2);
			}
		}
		/* And let the caller know where we drew the picture */
		*frame= bounds;
	} else {
		Rect bounds;
		char format_string[128];

		calculate_bounds_for_object(flags, &bounds, NULL);
	
		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		SDL_FillRect(/*world_pixels*/draw_surface, &r, SDL_MapRGB(/*world_pixels*/draw_surface->format, 0, 0, 0));

		getcstr(format_string, strERRORS, pictureNotFound);
		sprintf(temporary, format_string, picture_id);

		const font_info *font = GetInterfaceFont(_computer_interface_title_font);
		int width = text_width(temporary, font, styleNormal);
		draw_text(/*world_pixels*/draw_surface, temporary,
		          bounds.left + (RECTANGLE_WIDTH(&bounds) - width) / 2,
		          bounds.top + RECTANGLE_HEIGHT(&bounds) / 2,
		          SDL_MapRGB(/*world_pixels*/draw_surface->format, 0xff, 0xff, 0xff),
		          font, styleNormal);
	}
}

template <typename T>
static inline T randomize_pixel(uint16 pixel)
{
	return static_cast<T>(pixel);
}

extern SDL_PixelFormat pixel_format_32;

template <>
inline uint32 randomize_pixel(uint16 pixel)
{
	return ((uint32)pixel^(((uint32)pixel)<<8)) | pixel_format_32.Amask;
}

template <typename T>
static inline void randomize_line(T* start, uint32 count)
{
	static uint16 random_seed = 6906;
	for (int i = 0; i < count; ++i)
	{
		*start++ = randomize_pixel<T>(random_seed);
		if (random_seed&1) random_seed = (random_seed>>1)^0xb400;
		else random_seed = random_seed >> 1;
	}
}

static void fill_terminal_with_static(
	Rect *bounds)
{
	for (int y = bounds->top; y < bounds->bottom; ++y)
	{
		int bpp = draw_surface->format->BytesPerPixel;
		int width = bounds->right - bounds->left;

		uint8* p = (uint8*)draw_surface->pixels + y * draw_surface->pitch + bounds->left * bpp;
		if (bpp == 1)
		{
			randomize_line<uint8>(p, width);
		}
		else if (bpp == 2)
		{
			randomize_line<uint16>(reinterpret_cast<uint16*>(p), width);
		}
		else if (bpp == 4)
		{
			randomize_line<uint32>(reinterpret_cast<uint32*>(p), width);
		}
	}
}

// LP addition: will return NULL if no terminal data was found for this terminal number

static boost::scoped_ptr<terminal_text_t> resource_terminal;
static int resource_terminal_id = NONE;

void clear_compiled_terminal_cache()
{
	resource_terminal.reset();
	resource_terminal_id = NONE;
}

extern OpenedResourceFile M1ShapesFile;

static terminal_text_t* compile_marathon_terminal(char*, short);

static terminal_text_t *get_indexed_terminal_data(
	short id)
{
	if (id < 0 || id >= int(map_terminal_text.size()))
	{
		id = 1000 + dynamic_world->current_level_number * 10 + id;
		if (id == resource_terminal_id)
		{
			return resource_terminal.get();
		}
		else
		{
			LoadedResource rsrc;
			if (ExternalResources.IsOpen())
			{
				ExternalResources.Get('t', 'e', 'r', 'm', id, rsrc);
			}

			if (!rsrc.IsLoaded() && M1ShapesFile.IsOpen())
			{
				M1ShapesFile.Get('t', 'e', 'r', 'm', id, rsrc);
			}

			if (rsrc.IsLoaded())
			{
				resource_terminal.reset(compile_marathon_terminal(reinterpret_cast<char*>(rsrc.GetPointer()), rsrc.GetLength()));
				
				resource_terminal_id = id;
				return resource_terminal.get();
			}
		}

		return NULL;
	}

	terminal_text_t *t = &map_terminal_text[id];

	// Note that this will only decode the text once
	decode_text(t);
	return t;
}

static void decode_text(
	terminal_text_t *terminal_text)
{
	if(terminal_text->flags & _text_is_encoded_flag)
	{
		encode_text(terminal_text);
		terminal_text->flags &= ~_text_is_encoded_flag;
	}
}

static void encode_text(
	terminal_text_t *terminal_text)
{
	int length = terminal_text->text.size();
	uint8 *p = terminal_text->text.data();

	for (int i=0; i<length/4; i++) {
		p += 2;
		*p++ ^= 0xfe;
		*p++ ^= 0xed;
	}
	for (int i=0; i<length%4; i++)
		*p++ ^= 0xfe;

	terminal_text->flags |= _text_is_encoded_flag;
}

// LP change: added a flag to indicate whether stuff after the other
// terminal stuff is to be drawn; if not, then draw the stuff before the
// other terminal stuff.
static void draw_terminal_borders(
	struct player_terminal_data *terminal_data,
	bool after_other_terminal_stuff)
{
	Rect frame, border;
	short top_message, bottom_left_message, bottom_right_message;
	terminal_text_t *terminal_text= get_indexed_terminal_data(terminal_data->terminal_id);

	// LP addition: quit if none
	if (terminal_text == NULL) return;

	// LP change: just in case...
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
	if (current_group == NULL) return;
	
	switch(current_group->type)
	{
		case _logon_group:
			top_message= _computer_starting_up;
			bottom_left_message= _computer_manufacturer;
			bottom_right_message= _computer_address;
			break;
			
		case _logoff_group:
			top_message= _disconnecting_message;
			bottom_left_message= _computer_manufacturer;
			bottom_right_message= _computer_address;
			break;

		default:			
			top_message= _computer_terminal;
			bottom_left_message= _scrolling_message;
			bottom_right_message= _acknowledgement_message;
			break;
	}

	/* First things first: draw the border.. */
	/* Get the destination frame.. */
    frame= get_term_rectangle(_terminal_screen_rect);
		
	if (!after_other_terminal_stuff)
	{
		/* Erase the rectangle.. */
		_fill_screen_rectangle((screen_rectangle *) &frame, _black_color);
	} else {

		/* Draw the top rectangle */
		border= get_term_rectangle(_terminal_header_rect);
		_fill_screen_rectangle((screen_rectangle *) &border, _computer_border_background_text_color);

		/* Draw the top login header text... */
		border.left += LABEL_INSET; border.right -= LABEL_INSET;
		getcstr(temporary, strCOMPUTER_LABELS, top_message);
		_draw_screen_text(temporary, (screen_rectangle *) &border, _center_vertical, _computer_interface_font, _computer_border_text_color);
		get_date_string(temporary, current_group->flags);
		_draw_screen_text(temporary, (screen_rectangle *) &border, _right_justified | _center_vertical, 
			_computer_interface_font, _computer_border_text_color);
	
		/* Draw the the bottom rectangle & text */
		border= get_term_rectangle(_terminal_footer_rect);
		_fill_screen_rectangle((screen_rectangle *) &border, _computer_border_background_text_color);
		border.left += LABEL_INSET; border.right -= LABEL_INSET;
		getcstr(temporary, strCOMPUTER_LABELS, bottom_left_message);
		_draw_screen_text(temporary, (screen_rectangle *) &border, _center_vertical, _computer_interface_font, 
			_computer_border_text_color);
		getcstr(temporary, strCOMPUTER_LABELS, bottom_right_message);
		_draw_screen_text(temporary, (screen_rectangle *) &border, _right_justified | _center_vertical, 
			_computer_interface_font, _computer_border_text_color);
	
		// LP change: done with stuff for after the other rendering
	}
}

static void next_terminal_state(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);

	switch(terminal->state)
	{
		case _reading_terminal:
			initialize_player_terminal_info(player_index);
			break;
			
		case _no_terminal_state:
		default:
			break;
	}
}

static bool previous_terminal_group(
	short player_index,
	terminal_text_t *terminal_text)
{
	struct player_terminal_data *terminal_data= get_player_terminal_data(player_index);
	bool success= false;
	
	if(terminal_data->state==_reading_terminal)
	{
		short new_group_index= terminal_data->current_group-1;
		bool use_new_group= true;
		bool done= true;
		
		do 
		{
			if(new_group_index>=0)
			{
				struct terminal_groupings *new_group= get_indexed_grouping(terminal_text, new_group_index);
				// LP change: just in case...
				if (!new_group) return success;
				switch(new_group->type)
				{
					case _logon_group:
					case _end_group:
						use_new_group= false;
						break;
						
					case _interlevel_teleport_group:
					case _intralevel_teleport_group:
						// dprintf("This shouldn't happen!");
						break;

 					case _sound_group:
					case _tag_group:
						new_group_index--;
						done= false;
						break;
					
					case _movie_group:
					case _track_group:
					case _checkpoint_group:
					case _pict_group:
					case _information_group:
					case _camera_group:
						done = true;
						break;
						
					case _unfinished_group:
					case _success_group:
					case _failure_group:
					case _static_group:
						use_new_group= false;
						break;
					
					default:
						break;
				}
			} else {
				done= true;
				use_new_group= false;
			}
			if (!use_new_group) done = true; // this fixes perma-loop freeze
		} while(!done);
		
		if(use_new_group)
		{
			/* Go there.. */
			goto_terminal_group(player_index, terminal_text, new_group_index);
			success= true;
		}
	}
	
	return success;
}

static void next_terminal_group(
	short player_index,
	terminal_text_t *terminal_text)
{
	struct player_terminal_data *terminal_data= get_player_terminal_data(player_index);
	bool update_line_count= false;
	
	if(terminal_data->current_group==NONE)
	{
		update_line_count= true;
		
		switch(terminal_data->level_completion_state)
		{
			case _level_unfinished:
				terminal_data->current_group= find_group_type(terminal_text, _unfinished_group);
				break;
				
			case _level_finished:
				terminal_data->current_group= find_group_type(terminal_text, _success_group);
				if(terminal_data->current_group==static_cast<int16>(terminal_text->groupings.size()))
				{
					/* Fallback. */
					terminal_data->current_group= find_group_type(terminal_text, _unfinished_group);
					// assert(terminal_data->current_group != terminal_text->groupings.size());
				}
				break;
				
			case _level_failed:
				terminal_data->current_group= find_group_type(terminal_text, _failure_group);
				if(terminal_data->current_group==static_cast<int16>(terminal_text->groupings.size()))
				{
					/* Fallback. */
					terminal_data->current_group= find_group_type(terminal_text, _unfinished_group);
					// assert(terminal_data->current_group != terminal_text->groupings.size());
				}
				break;
			
			default:
				break;
		}

		if (terminal_data->current_group==static_cast<int16>(terminal_text->groupings.size()) && terminal_text->groupings.size() && (terminal_text->groupings[0].flags & _group_is_marathon_1))
		{
			// Marathon 1 fallback
			terminal_data->current_group = 0;
		} else {

			/* Note that the information groups are now keywords, and can have no data associated with them */
			next_terminal_group(player_index, terminal_text);
		}
	} else {
		terminal_data->current_group++;
		assert(terminal_data->current_group >= 0);
		if((size_t)terminal_data->current_group>=terminal_text->groupings.size())
		{
			next_terminal_state(player_index);
		} else {
			update_line_count= true;
		}
	}
	
	if(update_line_count)
	{
		goto_terminal_group(player_index, terminal_text, terminal_data->current_group);
	}
	
	SET_TERMINAL_IS_DIRTY(terminal_data, true);
}

static void goto_terminal_group(
	short player_index, 
	terminal_text_t *terminal_text, 
	short new_group_index)
{
	struct player_terminal_data *terminal_data= get_player_terminal_data(player_index);
	struct player_data *player= get_player_data(player_index);
	struct terminal_groupings *current_group;
	
	terminal_data->current_group= new_group_index;
	
	current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
	// LP change: just in case...
	if (!current_group)
	{
		// Copied from _end_group case
		next_terminal_state(player_index);
		terminal_data->maximum_line= 1; // any click or keypress will get us out...
		return;
	}
	
	terminal_data->current_line= 0;
		
	switch(current_group->type)
	{
		case _logon_group:
			play_object_sound(player->object_index, Sound_TerminalLogon());
			terminal_data->phase= LOG_DURATION_BEFORE_TIMEOUT;
			terminal_data->maximum_line= current_group->maximum_line_count;
			break;
			
		case _logoff_group:
			play_object_sound(player->object_index, Sound_TerminalLogoff());
			terminal_data->phase= LOG_DURATION_BEFORE_TIMEOUT;
			terminal_data->maximum_line= current_group->maximum_line_count;
			break;
			
		case _interlevel_teleport_group:
		case _intralevel_teleport_group:
		case _sound_group:
		case _tag_group:
		case _movie_group:
		case _track_group:
		case _camera_group:
			terminal_data->phase= NONE;
			terminal_data->maximum_line= current_group->maximum_line_count;
			break;

		case _checkpoint_group:
		case _pict_group:
			terminal_data->phase= NONE;
			if(dynamic_world->player_count>1)
			{
				/* Use what the server told us */
				terminal_data->maximum_line= current_group->maximum_line_count;
			} else {
				/* Calculate this for ourselves. */
				Rect text_bounds;
	
				/* The only thing we care about is the width. */
				calculate_bounds_for_text_box(current_group->flags, &text_bounds);
				terminal_data->maximum_line= count_total_lines(get_text_base(terminal_text),
					RECTANGLE_WIDTH(&text_bounds), current_group->start_index, 
					current_group->start_index+current_group->length);
			}
			break;
			
		case _information_group:
			terminal_data->phase= NONE;
			if(dynamic_world->player_count>1)
			{
				/* Use what the server told us */
				terminal_data->maximum_line= current_group->maximum_line_count;
			} else {
				/* Calculate this for ourselves. */
                Rect bounds= get_term_rectangle(_terminal_full_text_rect);
				terminal_data->maximum_line= count_total_lines(get_text_base(terminal_text),
					RECTANGLE_WIDTH(&bounds), current_group->start_index,
					current_group->start_index+current_group->length);
			}
			break;

		case _static_group:
			terminal_data->phase= current_group->permutation;
			terminal_data->maximum_line= current_group->maximum_line_count;
			break;

		case _end_group:
			/* Get Out! */
			next_terminal_state(player_index);
			terminal_data->maximum_line= 1; // any click or keypress will get us out...
			break;

		case _unfinished_group:
		case _success_group:
		case _failure_group:
			vwarn(0, "You shouldn't be coming to this group");
			break;
			
		default:
			break;
	}
}

/* I'll use this function, almost untouched.. */
static void get_date_string(
	char *date_string,
	short flags)
{
	char temp_string[101];
	int32 game_time_passed;
	time_t seconds;
	struct tm game_time;

	/* Treat the date as if it were recent. */
	game_time_passed= INT32_MAX - dynamic_world->game_information.game_time_remaining;
	
	/* convert the game seconds to machine seconds */
	if (flags & _group_is_marathon_1)
	{
		seconds = 809304137;
		seconds += 7*60*(game_time_passed / TICKS_PER_SECOND);
	}
	else
	{
		seconds = 800070137;
		seconds += game_time_passed / TICKS_PER_SECOND;
	}
	game_time = *localtime(&seconds);
	game_time.tm_year= 437;
	game_time.tm_yday= 0;
	game_time.tm_isdst= 0;

	getcstr(temp_string, strCOMPUTER_LABELS, _date_format);
	strftime(date_string, 100, temp_string, &game_time);
}

static void present_checkpoint_text(
	terminal_text_t *terminal_text,
	short current_group_index,
	short current_line)
{
	Rect bounds;
	struct overhead_map_data overhead_data;
	struct terminal_groupings *current_group;
	
	current_group= get_indexed_grouping(terminal_text, current_group_index);
	// LP change: just in case...
	if (!current_group) return;

	// draw the overhead map.
	calculate_bounds_for_object(current_group->flags, &bounds, NULL);
	overhead_data.scale =  1;
	
	if(find_checkpoint_location(current_group->permutation, &overhead_data.origin, 
		&overhead_data.origin_polygon_index))
	{
		overhead_data.top= bounds.top;
		overhead_data.left= bounds.left;
		overhead_data.half_width= RECTANGLE_WIDTH(&bounds)/2;
		overhead_data.half_height= RECTANGLE_HEIGHT(&bounds)/2;
		overhead_data.width= RECTANGLE_WIDTH(&bounds);
		overhead_data.height= RECTANGLE_HEIGHT(&bounds);
		overhead_data.mode= _rendering_checkpoint_map;
        set_drawing_clip_rectangle(bounds.top, bounds.left, bounds.bottom, bounds.right);
		_render_overhead_map(&overhead_data);
        set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	} else {
		char format_string[128];
	
		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		SDL_FillRect(/*world_pixels*/draw_surface, &r, SDL_MapRGB(/*world_pixels*/draw_surface->format, 0, 0, 0));

		getcstr(format_string, strERRORS, checkpointNotFound);
		sprintf(temporary, format_string, current_group->permutation);

		const font_info *font = GetInterfaceFont(_computer_interface_title_font);
		// const sdl_font_info *font = load_font(*_get_font_spec(_computer_interface_title_font));
		int width = text_width(temporary, font, styleNormal);
		draw_text(/*world_pixels*/draw_surface, temporary,
		          bounds.left + (RECTANGLE_WIDTH(&bounds) - width) / 2,
		          bounds.top + RECTANGLE_HEIGHT(&bounds) / 2,
		          SDL_MapRGB(/*world_pixels*/draw_surface->format, 0xff, 0xff, 0xff),
		          font, styleNormal);
	}

	// draw the text
	calculate_bounds_for_text_box(current_group->flags, &bounds);
	draw_computer_text(&bounds, terminal_text, current_group_index, current_line);
}

static bool find_checkpoint_location(
	short checkpoint_index, 
	world_point2d *location, 
	short *polygon_index)
{
	bool success= false;
	short ii;
	struct map_object *saved_object;
	short match_count;
	
	location->x= location->y= match_count= 0;
	for (ii = 0, saved_object= saved_objects; ii < dynamic_world->initial_objects_count; ii++, saved_object++)
	{
		if (saved_object->type==_saved_goal && saved_object->index==checkpoint_index)
		{
			location->x+= saved_object->location.x;
			location->y+= saved_object->location.y;
//			*polygon_index= saved_object->polygon_index;
			match_count++;
		}
	}
	
	if(match_count)
	{
		/* Now average.. */	
		location->x /= match_count;
		location->y /= match_count;
		*polygon_index= world_point_to_polygon_index(location);
		success = (*polygon_index != NONE);
	}

	return success;		
}

static void handle_reading_terminal_keys(
	short player_index,
	int32 action_flags)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	terminal_text_t *terminal_text= get_indexed_terminal_data(terminal->terminal_id);
	// LP addition: quit if none
	if (!terminal_text) return;
	struct terminal_groupings *current_group;
	short initial_group= terminal->current_group;
	short initial_line= terminal->current_line;
	bool aborted= false;
	struct player_data *player= get_player_data(player_index);
	short line_delta= 0;
	bool change_state= false;
	
	current_group= get_indexed_grouping(terminal_text, terminal->current_group);
	// LP change: just in case...
	if (!current_group)
	{
		// Copied from _end_group case
		next_terminal_state(player_index);
		aborted= true;
	}
	
	switch(current_group->type)
	{
		case _logon_group:
		case _logoff_group:
		case _unfinished_group:
		case _success_group:
		case _failure_group:
		case _information_group:
		case _checkpoint_group:
		case _pict_group:
		case _camera_group:
		case _static_group:
			if(action_flags & _terminal_up_arrow) 
			{
				line_delta= -1;
			} 
			
			if(action_flags & _terminal_down_arrow)
			{
				line_delta= 1;
			} 
			
			if(action_flags & _terminal_page_down)
			{
				play_object_sound(player->object_index, Sound_TerminalPage());
				line_delta= terminal_text->lines_per_page;
			} 
			
			if(action_flags & _terminal_page_up)
			{
				play_object_sound(player->object_index, Sound_TerminalPage());
				line_delta= -terminal_text->lines_per_page;
			}

			/* this one should change state, if necessary */
			if(action_flags & _terminal_next_state)
			{
				play_object_sound(player->object_index, Sound_TerminalPage());
				/* Force a state change. */
				line_delta= terminal_text->lines_per_page;
				change_state= true;
			}
			
			if(action_flags & _any_abort_key_mask)
			{
				/* Abort! */
				initialize_player_terminal_info(player_index);
				aborted= true;
			}
			break;

		case _movie_group:
		case _track_group:
			break;
			
		case _end_group:
			next_terminal_state(player_index);
			aborted= true;
			break;
		
		case _interlevel_teleport_group: // permutation is level to go to
			if (film_profile.m1_teleport_without_delay && (current_group->flags & _group_is_marathon_1))
			{
				teleport_to_level(current_group->permutation, 0);
			}
			else
			{
				teleport_to_level(current_group->permutation, TICKS_PER_SECOND / 2);
			}
			initialize_player_terminal_info(player_index);
			aborted= true;
			break;

		case _intralevel_teleport_group: // permutation is polygon to go to.
			teleport_to_polygon(player_index, current_group->permutation);
			initialize_player_terminal_info(player_index);
			aborted= true;
			break;

		case _sound_group: // permutation is the sound id to play
			/* Play the sound immediately, and then go to the next level.. */
			{
				struct player_data *player= get_player_data(player_index);
				play_object_sound(player->object_index, current_group->permutation);
				next_terminal_group(player_index, terminal_text);
				aborted= true;
			}
			break;

		case _tag_group:
			set_tagged_light_statuses(current_group->permutation, true);
			try_and_change_tagged_platform_states(current_group->permutation, true);
			next_terminal_group(player_index, terminal_text);
			aborted= true;
			break;

		default:
			break;
	}

	/* If we are dirty.. */
	terminal->current_line+= line_delta;
	if(!aborted && (initial_group != terminal->current_group ||  
			initial_line != terminal->current_line))
	{
		if(terminal->current_line<0)
		{
			if(!previous_terminal_group(player_index, terminal_text))
			{
				terminal->current_line= 0;
			}
		}
		
		if(terminal->current_line>=terminal->maximum_line)
		{
			assert(terminal->current_group >= 0);
			if(static_cast<size_t>(terminal->current_group)+1>=terminal_text->groupings.size())
			{
				if(change_state)
				{
					/* Go ahead an let the terminal group deal with it.. */
					next_terminal_group(player_index, terminal_text);
				} 
				else 
				{
					/* renumber the lines */
					terminal->current_line-= line_delta;
				}
			} 
			else 
			{
				next_terminal_group(player_index, terminal_text);
			}
		}

		SET_TERMINAL_IS_DIRTY(terminal, true);
	}
}
	
static void calculate_bounds_for_object(
	short flags,
	Rect *bounds,
	Rect *source)
{
	if(source && flags & _center_object)
	{
        *bounds = get_term_rectangle(_terminal_logon_graphic_rect);
		if(RECTANGLE_WIDTH(source)>RECTANGLE_WIDTH(bounds)
			|| RECTANGLE_HEIGHT(source)>RECTANGLE_HEIGHT(bounds))
		{
			/* Just return the normal frame.  Aspect ratio will take care of us.. */
		} else {
			InsetRect(bounds, (RECTANGLE_WIDTH(bounds)-RECTANGLE_WIDTH(source))/2,
				(RECTANGLE_HEIGHT(bounds)-RECTANGLE_HEIGHT(source))/2);
		}
	} 
	else if(flags & _draw_object_on_right)
	{
        *bounds = get_term_rectangle(_terminal_right_rect);
	} 
	else 
	{
		*bounds = get_term_rectangle(_terminal_left_rect);
	}
}

#define MAXIMUM_GROUPS_PER_TERMINAL 15

static void calculate_maximum_lines_for_groups(struct terminal_groupings *groups, 
	short group_count, char *text_base);

class MarathonTerminalCompiler
{
public:
	MarathonTerminalCompiler(char* text, short length) : terminal(new terminal_text_t), in_buffer(text, length), in(&in_buffer) {
		information_group.type = 0;
		briefing_group.type = 0;
		success_group.type = 0;
		failure_group.type = 0;
		unfinished_group.type = 0;
		group.type = NONE;
	}
	terminal_text_t* Compile();

private:
	void FinishGroup();
	void CompileLine(const std::string& line);

	void BuildUnfinishedGroup();
	void BuildSuccessGroup();
	void BuildFailureGroup();

	std::unique_ptr<terminal_text_t> terminal;
	
	io::stream_buffer<io::array_source> in_buffer;
	std::istream in;

	std::vector<uint8_t> out;

	int16 current_group_type;
	terminal_groupings group;

	terminal_groupings logon_group;
	terminal_groupings information_group;
	terminal_groupings briefing_group;
	terminal_groupings unfinished_group;
	terminal_groupings failure_group;
	terminal_groupings success_group;

	std::vector<terminal_groupings> information_checkpoints;
	std::vector<terminal_groupings> unfinished_checkpoints;
	// does Marathon allow #checkpoints inside success/failure/briefing?
	std::vector<terminal_groupings>* checkpoints;
};

terminal_text_t* MarathonTerminalCompiler::Compile()
{
	std::string line;
	while (std::getline(in, line, static_cast<char>(MAC_LINE_END)))
	{
		if (line[0] == '#')
		{
			FinishGroup();

			group.flags = _group_is_marathon_1;
			group.permutation = 0;
			
			if (algo::istarts_with(line, "#logon"))
			{
				group.type = _logon_group;
			}
			else if (algo::istarts_with(line, "#information"))
			{
				group.type = _information_group;
			}
			else if (algo::istarts_with(line, "#checkpoint"))
			{
				group.type = _checkpoint_group;
				std::istringstream permutation(line.substr(1 + strlen("#checkpoint")));
				permutation >> group.permutation;
			}
			else if (algo::istarts_with(line, "#briefing"))
			{
				group.type = _interlevel_teleport_group;
				std::istringstream permutation(line.substr(1 + strlen("#briefing")));
				permutation >> group.permutation;
			}
			else if (algo::istarts_with(line, "#unfinished"))
			{
				group.type = _unfinished_group;
			}
			else if (algo::istarts_with(line, "#success"))
			{
				group.type = _success_group;
			}
			else if (algo::istarts_with(line, "#failure"))
			{
				group.type = _failure_group;
			}
			else 
			{
				logWarning("Unrecognized group");
				terminal.reset(0);
				return 0;
			}

			group.start_index = out.size();
		}
		else if (line[0] != ';')
		{
			CompileLine(line);
		}
	}

	FinishGroup();

	BuildUnfinishedGroup();
	BuildSuccessGroup();
	BuildFailureGroup();

	terminal->text = out;

	terminal->lines_per_page = calculate_lines_per_page();
	calculate_maximum_lines_for_groups(&terminal->groupings[0], terminal->groupings.size(), reinterpret_cast<char*>(terminal->text.data()));

	return terminal.release();
}

static terminal_text_t* compile_marathon_terminal(char* text, short length)
{
	MarathonTerminalCompiler compiler(text, length);
	return compiler.Compile();
}

void MarathonTerminalCompiler::FinishGroup()
{
	group.length = out.size() - group.start_index;

	out.push_back('\0');

	switch (group.type)
	{
	case _logon_group:
		checkpoints = nullptr;
		logon_group = group;
		break;
	case _information_group:
		information_group = group;
		information_checkpoints.clear();
		checkpoints = &information_checkpoints;
		break;
	case _checkpoint_group:
		if (checkpoints)
		{
			checkpoints->push_back(group);
		}
		break;
	case _interlevel_teleport_group:
		checkpoints = nullptr;
		briefing_group = group;
		break;
	case _success_group:
		checkpoints = nullptr;
		success_group = group;
		break;
	case _failure_group:
		checkpoints = nullptr;
		failure_group = group;
		break;
	case _unfinished_group:
		unfinished_group = group;
		unfinished_checkpoints.clear();
		checkpoints = &unfinished_checkpoints;
		break;
	}
}

void MarathonTerminalCompiler::CompileLine(const std::string& line)
{
	// Marathon formatting resets at the beginning of the line
	text_face_data font_change;
	font_change.index = out.size();
	font_change.face = 0;
	font_change.color = 0;

	terminal->font_changes.push_back(font_change);
	
	for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
	{
		if (*it == '$' && (it + 1 != line.end()))
		{
			font_change.index = out.size();
			char c = *(it + 1);
			switch (c)
			{
			case 'B':
				font_change.face |= _bold_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'b':
				font_change.face &= ~_bold_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'I':
				font_change.face |= _italic_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'i':
				font_change.face &= ~_italic_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'U':
				font_change.face |= _underline_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'u':
				font_change.face &= ~_underline_text;
				terminal->font_changes.push_back(font_change);
				++it;
				break;
			case 'C':
				if (it + 2 != line.end())
				{
					char cc = *(it + 2);
					if (cc >= '0' && cc <= '7')
					{
						font_change.color = cc - '0';
						terminal->font_changes.push_back(font_change);
						it += 2;
					}
					else
					{
						out.push_back(*it);
					}
				}
				else
				{
					out.push_back(*it);
				}
				break;
			default:
				++it;
			}
		}
		else if (*it == '%' && (it + 1 != line.end()))
		{
			static char replacement[] = "The colony has been wiped out. Phhht! Just like that.";
            
			char c = *(it + 1);
			switch (c)
			{
			case 'r':
				out.insert(out.end(), replacement, replacement + strlen(replacement));
				++it;
				break;
			case '%':
				out.push_back(*it);
				++it;
				break;
			default:
				out.push_back(*it);
			}
		}
		else
		{
			out.push_back(*it);
		}
	}

	out.push_back(MAC_LINE_END);
}

void MarathonTerminalCompiler::BuildUnfinishedGroup()
{
	terminal_groupings group;
	group.flags = 0;
	group.type = _unfinished_group;
	group.permutation = 0;
	group.start_index = 0;
	group.length = 0;
	terminal->groupings.push_back(group);

	terminal->groupings.push_back(logon_group);

	if (information_group.type)
	{
		terminal->groupings.push_back(information_group);
		terminal->groupings.insert(terminal->groupings.end(),
								   information_checkpoints.begin(),
								   information_checkpoints.end());
	}

	if (unfinished_group.type)
	{
		group = unfinished_group;
		group.type = _information_group;
		terminal->groupings.push_back(group);

		terminal->groupings.insert(terminal->groupings.end(),
								   unfinished_checkpoints.begin(),
								   unfinished_checkpoints.end());
	}

	group = logon_group;
	group.type = _logoff_group;
	terminal->groupings.push_back(group);

	group.type = _end_group;
	group.flags = 0;
	group.permutation = 0;
	group.start_index = 0;
	group.length = 0;
	terminal->groupings.push_back(group);
}

void MarathonTerminalCompiler::BuildSuccessGroup()
{
	if (success_group.type || briefing_group.type)
	{
		terminal_groupings group;
		group.flags = 0;
		group.type = _success_group;
		group.permutation = 0;
		group.start_index = 0;
		group.length = 0;
		terminal->groupings.push_back(group);

		terminal->groupings.push_back(logon_group);

		if (success_group.type == _success_group)
		{
			group = success_group;
			group.type = _information_group;
			terminal->groupings.push_back(group);
		}

		if (briefing_group.type)
		{
			group = briefing_group;
			group.type = _information_group;
			group.permutation = 0;
			terminal->groupings.push_back(group);
		}

		group = logon_group;
		group.type = _logoff_group;
		terminal->groupings.push_back(group);

		if (briefing_group.type)
		{
			group.type = _interlevel_teleport_group;
			group.permutation = briefing_group.permutation;
			group.start_index = 0;
			group.length = 0;
			terminal->groupings.push_back(group);
		}

		group.type = _end_group;
		group.flags = 0;
		group.permutation = 0;
		group.start_index = 0;
		group.length = 0;
		terminal->groupings.push_back(group);
	}
}

void MarathonTerminalCompiler::BuildFailureGroup()
{
	if (failure_group.type)
	{
		terminal_groupings group;
		group.flags = 0;
		group.type = _failure_group;
		group.permutation = 0;
		group.start_index = 0;
		group.length = 0;
		terminal->groupings.push_back(group);

		terminal->groupings.push_back(logon_group);

		group = failure_group;
		group.type = _information_group;
		terminal->groupings.push_back(group);

		if (briefing_group.type)
		{
			group = briefing_group;
			group.type = _information_group;
			group.permutation = 0;
			terminal->groupings.push_back(group);
		}

		group = logon_group;
		group.type = _logoff_group;
		terminal->groupings.push_back(group);

		if (briefing_group.type)
		{
			group.type = _interlevel_teleport_group;
			group.permutation = briefing_group.permutation;
			group.start_index = 0;
			group.length = 0;
			terminal->groupings.push_back(group);
		}

		group.type = _end_group;
		group.flags = 0;
		group.permutation = 0;
		group.start_index = 0;
		group.length = 0;
		terminal->groupings.push_back(group);
	}
}

static void calculate_maximum_lines_for_groups(
	struct terminal_groupings *groups,
	short group_count,
	char *text_base)
{
	short index;
	
	for(index= 0; index<group_count; ++index)
	{
		switch(groups[index].type)
		{
			case _logon_group:
			case _logoff_group:
			case _interlevel_teleport_group:
			case _intralevel_teleport_group:
			case _sound_group:
			case _tag_group:
			case _movie_group:
			case _track_group:
			case _camera_group:
			case _static_group:
			case _end_group:
				groups[index].maximum_line_count= 1; // any click or keypress gets us out.
				break;
	
			case _unfinished_group:
			case _success_group:
			case _failure_group:
				groups[index].maximum_line_count= 0; // should never get to one of these groups.
				break;
	
			case _checkpoint_group:
			case _pict_group:
				{
					Rect text_bounds;
		
					/* The only thing we care about is the width. */
					calculate_bounds_for_text_box(groups[index].flags, &text_bounds);
					groups[index].maximum_line_count= count_total_lines(text_base,
						RECTANGLE_WIDTH(&text_bounds), groups[index].start_index, 
						groups[index].start_index+groups[index].length);
				}
				break;
								
			case _information_group:
				{
					Rect text_bounds= get_term_rectangle(_terminal_full_text_rect);
		
					groups[index].maximum_line_count= count_total_lines(text_base,
						RECTANGLE_WIDTH(&text_bounds), groups[index].start_index, 
						groups[index].start_index+groups[index].length);
				}
				break;
                
			default:
				break;
		}
	}
}

static struct terminal_groupings *get_indexed_grouping(
	terminal_text_t *data,
	short index)
{
	if (index < 0 || index >= int(data->groupings.size()))
		return NULL;

	return &data->groupings[index];
}

static struct text_face_data *get_indexed_font_changes(
	terminal_text_t *data,
	short index)
{
	if (index < 0 || index >= int(data->font_changes.size()))
		return NULL;

	return &data->font_changes[index];
}

static char *get_text_base(
	terminal_text_t *data)
{
	return (char *)data->text.data();
}

static short calculate_lines_per_page(
	void)
{
	Rect bounds;
	short lines_per_page;

    if (!film_profile.calculate_terminal_lines_correctly)
    {
        bounds= get_term_rectangle(_terminal_screen_rect);
        lines_per_page= (RECTANGLE_HEIGHT(&bounds)-2*BORDER_HEIGHT)/_get_font_line_height(_computer_interface_font);
        lines_per_page-= FUDGE_FACTOR;
    }
    else
    {
        bounds= get_term_rectangle(_terminal_full_text_rect);
        lines_per_page= RECTANGLE_HEIGHT(&bounds)/_get_font_line_height(_computer_interface_font);
    }

	return lines_per_page;
}

static Rect get_term_rectangle(short index)
{
    Rect bounds;
    screen_rectangle *term_rect = get_interface_rectangle(_terminal_screen_rect);
    screen_rectangle *target_rect = get_interface_rectangle(index);
    bounds.left = target_rect->left - term_rect->left;
    bounds.top = target_rect->top - term_rect->top;
    bounds.right = target_rect->right - term_rect->left;
    bounds.bottom = target_rect->bottom - term_rect->top;
    return bounds;
}


/*
 *  Calculate the length the loaded terminal data would take up on disk
 *  (for saving)
 */

static size_t packed_terminal_length(const terminal_text_t &t)
{
	return SIZEOF_static_preprocessed_terminal_data
	     + t.groupings.size() * SIZEOF_terminal_groupings
	     + t.font_changes.size() * SIZEOF_text_face_data
	     + t.text.size();
}

size_t calculate_packed_terminal_data_length(void)
{
	size_t total = 0;

	// Loop for all terminals
	vector<terminal_text_t>::const_iterator t = map_terminal_text.begin(), tend = map_terminal_text.end();
	while (t != tend) {
		total += packed_terminal_length(*t);
		t++;
	}

	return total;
}


/*
 *  Unpack terminal data from stream
 */

void unpack_map_terminal_data(uint8 *p, size_t count)
{
	// Clear existing terminals
	map_terminal_text.clear();

	// Unpack all terminals
	while (count > 0) {

		// Create new terminal_text_t
		terminal_text_t t;
		map_terminal_text.push_back(t);
		terminal_text_t &data = map_terminal_text.back();

		// Read header
		uint8 *p_start = p, *p_header = p;
		uint16 total_length, grouping_count, font_changes_count;
		StreamToValue(p, total_length);
		StreamToValue(p, data.flags);
		StreamToValue(p, data.lines_per_page);
		StreamToValue(p, grouping_count);
		StreamToValue(p, font_changes_count);
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_static_preprocessed_terminal_data));

		// Reserve memory for groupings and font changes
		data.groupings.reserve(grouping_count);
		data.font_changes.reserve(font_changes_count);

		// Read groupings
		p_start = p;
		for (int i=0; i<grouping_count; i++) {
			terminal_groupings g;
			StreamToValue(p, g.flags);
			StreamToValue(p, g.type);
			StreamToValue(p, g.permutation);
			StreamToValue(p, g.start_index);
			StreamToValue(p, g.length);
			StreamToValue(p, g.maximum_line_count);
			data.groupings.push_back(g);
		}
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_terminal_groupings) * grouping_count);

		// Read font changes
		p_start = p;
		for (int i=0; i<font_changes_count; i++) {
			text_face_data f;
			StreamToValue(p, f.index);
			StreamToValue(p, f.face);
			StreamToValue(p, f.color);
			data.font_changes.push_back(f);
		}
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_text_face_data) * font_changes_count);

		// Read text (no conversion)
		const int text_length = total_length - static_cast<int>(p - p_header);
		assert(text_length >= 0);
		data.text.resize(text_length);
		StreamToBytes(p, data.text.data(), data.text.size());

		// Continue with next terminal
		count -= total_length;
	}
}


/*
 *  Pack terminal data to stream
 */

void pack_map_terminal_data(uint8 *p, size_t count)
{
	// Pack all terminals
	vector<terminal_text_t>::const_iterator t = map_terminal_text.begin(), tend = map_terminal_text.end();
	while (t != tend) {

		// Write header
		uint8 *p_start = p;
		uint16 total_length = static_cast<uint16>(packed_terminal_length(*t));
		uint16 grouping_count = static_cast<uint16>(t->groupings.size());
		uint16 font_changes_count = static_cast<uint16>(t->font_changes.size());
		ValueToStream(p, total_length);
		ValueToStream(p, t->flags);
		ValueToStream(p, t->lines_per_page);
		ValueToStream(p, grouping_count);
		ValueToStream(p, font_changes_count);
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_static_preprocessed_terminal_data));

		// Write groupings
		p_start = p;
		vector<terminal_groupings>::const_iterator g = t->groupings.begin(), gend = t->groupings.end();
		while (g != gend) {
			ValueToStream(p, g->flags);
			ValueToStream(p, g->type);
			ValueToStream(p, g->permutation);
			ValueToStream(p, g->start_index);
			ValueToStream(p, g->length);
			ValueToStream(p, g->maximum_line_count);
			g++;
		}
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_terminal_groupings) * grouping_count);

		// Write font changes
		p_start = p;
		vector<text_face_data>::const_iterator f = t->font_changes.begin(), fend = t->font_changes.end();
		while (f != fend) {
			ValueToStream(p, f->index);
			ValueToStream(p, f->face);
			ValueToStream(p, f->color);
			f++;
		}
		assert((p - p_start) == static_cast<ptrdiff_t>(SIZEOF_text_face_data) * font_changes_count);

		// Write text (no conversion)
		BytesToStream(p, t->text.data(), t->text.size());

		// Continue with next terminal
		t++;
	}
}


uint8 *unpack_player_terminal_data(uint8 *Stream, size_t Count)
{
	uint8* S = Stream;
	player_terminal_data* ObjPtr = player_terminals;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		StreamToValue(S,ObjPtr->phase);
		StreamToValue(S,ObjPtr->state);
		StreamToValue(S,ObjPtr->current_group);
		StreamToValue(S,ObjPtr->level_completion_state);
		StreamToValue(S,ObjPtr->current_line);
		StreamToValue(S,ObjPtr->maximum_line);
		StreamToValue(S,ObjPtr->terminal_id);
		StreamToValue(S,ObjPtr->last_action_flag);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_player_terminal_data));
	return S;
}

uint8 *pack_player_terminal_data(uint8 *Stream, size_t Count)
{
	uint8* S = Stream;
	player_terminal_data* ObjPtr = player_terminals;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		ValueToStream(S,ObjPtr->phase);
		ValueToStream(S,ObjPtr->state);
		ValueToStream(S,ObjPtr->current_group);
		ValueToStream(S,ObjPtr->level_completion_state);
		ValueToStream(S,ObjPtr->current_line);
		ValueToStream(S,ObjPtr->maximum_line);
		ValueToStream(S,ObjPtr->terminal_id);
		ValueToStream(S,ObjPtr->last_action_flag);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_player_terminal_data));
	return S;
}

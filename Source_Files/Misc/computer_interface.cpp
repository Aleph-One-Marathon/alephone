/*

	computer_interface.c
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
*/

// add logon/logoff keywords. (& make terminal display them)
// cameras
// static
// delayed teleport

// activate tags
// picture with scrolling text.
// checkpoint with scrolling text
// don't use charwidth

#pragma segment drawing

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "cseries.h"
#include "byte_swapping.h"
#include "FileHandler.h"

#include "world.h"
#include "map.h"
#include "player.h"
#include "computer_interface.h"
#include "screen_drawing.h"
#include "overhead_map.h"
#include "mysound.h"
#include "interface.h" // for the error strings.
#include "shell.h"
#include "platforms.h" // for tagged platforms
#include "lightsource.h" // for tagged lightsources
#include "screen.h"

#include "images.h"

//CP Addition: scripting support
#include "scripting.h"

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
	_center_object= 0x02
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

struct player_terminal_data
{
	short flags;
	short phase;
	short state;
	short current_group;
	short level_completion_state;
	short current_line;
	short maximum_line;
	short terminal_id;
	long last_action_flag;
};

struct terminal_key {
	short keycode;
	short offset;
	short mask;
	long action_flag;
};

struct font_dimensions {
	short lines_per_screen;
	short character_width;
};

/* globals - I would really like to make these static to computer_interface.c, but game_wad.c needs them */
byte *map_terminal_data;
long map_terminal_data_length;

/* internal global structure */
static struct player_terminal_data *player_terminals;

#define NUMBER_OF_TERMINAL_KEYS (sizeof(terminal_keys)/sizeof(struct terminal_key))

extern TextSpec *_get_font_spec(short font_index);

inline player_terminal_data *get_player_terminal_data(
	short player_index)
{
	struct player_terminal_data *player_terminal = GetMemberWithBounds(player_terminals,player_index,MAXIMUM_NUMBER_OF_PLAYERS);
	
	vassert(player_terminal, csprintf(temporary, "player index #%d is out of range", player_index));

	return player_terminal;
}

// LP addition: overall terminal boundary rect (needed for resetting the clipping rectangle)
static Rect OverallBounds;

/* ------------ byte-swapping definitions */
static _bs_field _bs_static_preprocessed_terminal_data[] = {
	_2byte, _2byte, _2byte, _2byte, _2byte
};

static _bs_field _bs_terminal_groupings[] = {
	_2byte, _2byte, _2byte, _2byte, _2byte, _2byte
};

static _bs_field _bs_text_face_data[] = {
	_2byte, _2byte, _2byte
};

/* ------------ private prototypes */
static void draw_logon_text(Rect *bounds, struct static_preprocessed_terminal_data *terminal_text,
	short current_group_index, short logon_shape_id);
static void draw_computer_text(Rect *bounds, 
	struct static_preprocessed_terminal_data *terminal_text, short current_group_index, short current_line);
static void _draw_computer_text(char *base_text, short start_index, Rect *bounds,
	struct static_preprocessed_terminal_data *terminal_text, short current_line);
static short find_group_type(struct static_preprocessed_terminal_data *data, 
	short group_type);
static void teleport_to_level(short level_number);
static void teleport_to_polygon(short player_index, short polygon_index);
static struct terminal_groupings *get_indexed_grouping(
	struct static_preprocessed_terminal_data *data, short index);
static struct text_face_data *get_indexed_font_changes(
	struct static_preprocessed_terminal_data *data, short index);
static char *get_text_base(struct static_preprocessed_terminal_data *data);
// LP change: added a flag to indicate whether stuff after the other
// terminal stuff is to be drawn; if not, then draw the stuff before the
// other terminal stuff.
static void draw_terminal_borders(struct view_terminal_data *data, 
	struct player_terminal_data *terminal_data, Rect *terminal_frame,
	bool after_other_terminal_stuff);
/*
static void draw_terminal_borders(struct view_terminal_data *data, 
	struct player_terminal_data *terminal_data, Rect *terminal_frame);
*/
static void next_terminal_state(short player_index);
static void next_terminal_group(short player_index, struct static_preprocessed_terminal_data *terminal_text);
static void get_date_string(char *date_string);
static void present_checkpoint_text(Rect *frame,
	struct static_preprocessed_terminal_data *terminal_text, short current_group_index,
	short current_line);
static bool find_checkpoint_location(short checkpoint_index, world_point2d *location, 
	short *polygon_index);
struct static_preprocessed_terminal_data *preprocess_text(char *text, short length);
static void pre_build_groups(struct terminal_groupings *groups,
	short *group_count, struct text_face_data *text_faces, short *text_face_count, 
	char *base_text, short *base_length);
static short matches_group(char *base_text, short length, short index, short possible_group, 
	short *permutation);
static void	set_text_face(struct text_face_data *text_face);
static void draw_line(char *base_text, short start_index, short end_index, Rect *bounds,
	struct static_preprocessed_terminal_data *terminal_text, short *text_face_start_index,
	short line_number);
static bool calculate_line(char *base_text, short width, short start_index, 
	short text_end_index, short *end_index);
static void handle_reading_terminal_keys(short player_index, long action_flags);
static void calculate_bounds_for_object(Rect *frame, short flags, Rect *bounds, Rect *source);
static void display_picture(short picture_id, Rect *frame, short flags);
static void display_picture_with_text(struct player_terminal_data *terminal_data, 
	Rect *bounds, struct static_preprocessed_terminal_data *terminal_text, short current_lien);
static short count_total_lines(char *base_text, short width, short start_index, short end_index);
static void calculate_bounds_for_text_box(Rect *frame, short flags, Rect *bounds);
static void goto_terminal_group(short player_index, struct static_preprocessed_terminal_data *terminal_text, 
	short new_group_index);
static bool previous_terminal_group(short player_index, struct static_preprocessed_terminal_data *terminal_text);
static void fill_terminal_with_static(Rect *bounds);
static short calculate_lines_per_page(void);

#ifndef PREPROCESSING_CODE
static struct static_preprocessed_terminal_data *get_indexed_terminal_data(short id);
static void encode_text(struct static_preprocessed_terminal_data *terminal_text);
static void decode_text(struct static_preprocessed_terminal_data *terminal_text);
#endif

/* ------------ machine-specific code */

#if defined(mac)
#include "computer_interface_macintosh.c"
#elif defined(SDL)
#include "computer_interface_sdl.cpp"
#endif

/* ------------ code begins */
void initialize_terminal_manager(
	void)
{
	player_terminals= new player_terminal_data[MAXIMUM_NUMBER_OF_PLAYERS];
	assert(player_terminals);
	objlist_clear(player_terminals, MAXIMUM_NUMBER_OF_PLAYERS);

#ifdef mac
	for(int index= 0; index<NUMBER_OF_TERMINAL_KEYS; ++index)
	{
		terminal_keys[index].mask= 1 << (terminal_keys[index].keycode&7);
		terminal_keys[index].offset= terminal_keys[index].keycode>>3;
	}
#endif

#ifdef SDL
	terminal_font = load_font(*_get_font_spec(_computer_interface_font));
#endif
}

void initialize_player_terminal_info(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);

	//CP Addition: trap for logout!
	if (terminal->state != _no_terminal_state && terminal->current_line != 0)
		activate_terminal_exit_trap(terminal->terminal_id);

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
	// LP: verify object sizes:
	assert(sizeof(static_preprocessed_terminal_data) == SIZEOF_static_preprocessed_terminal_data);
	assert(sizeof(terminal_groupings) == SIZEOF_terminal_groupings);
	assert(sizeof(text_face_data) == SIZEOF_text_face_data);
	
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	struct player_data *player= get_player_data(player_index);
	// LP addition: if there is no terminal-data chunk, then just make the logon sound and quit
	if (map_terminal_data_length <= 0)
	{
		play_object_sound(player->object_index, _snd_computer_interface_logon);
		return;
	}
	struct static_preprocessed_terminal_data *terminal_text= get_indexed_terminal_data(text_number);
	if (!terminal_text)
	{
		play_object_sound(player->object_index, _snd_computer_interface_logon);
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

/*  Assumes ╢t==1 tick */
void update_player_for_terminal_mode(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);

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
						struct static_preprocessed_terminal_data *terminal_text= get_indexed_terminal_data(terminal->terminal_id);
						// LP addition: quit if none
						if (!terminal_text) break;
						
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

	switch(terminal->state)
	{
		case _reading_terminal:
			handle_reading_terminal_keys(player_index, action_flags);
			break;
		
		case _no_terminal_state:
		default:
			// LP change:
			assert(false);
			// halt();
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

void *get_terminal_data_for_save_game(
	void)
{
	return player_terminals;
}

long calculate_terminal_data_length(
	void)
{
	long length= dynamic_world->player_count*sizeof(struct player_terminal_data);
	
	return length;
}

void *get_terminal_information_array(
	void)
{
	return map_terminal_data;
}

long calculate_terminal_information_length(
	void)
{
	return map_terminal_data_length;
}

void _render_computer_interface(
	struct view_terminal_data *data)
{
	struct player_terminal_data *terminal_data= get_player_terminal_data(current_player_index);
	
	assert(terminal_data->state != _no_terminal_state);
	if(TERMINAL_IS_DIRTY(terminal_data))
	{
		struct static_preprocessed_terminal_data *terminal_text;
		struct terminal_groupings *current_group;
		Rect bounds;

		/* Get the terminal text.. */
		terminal_text= get_indexed_terminal_data(terminal_data->terminal_id);
		// LP addition: quit if none
		if (!terminal_text) return;
		
		// LP addition:
		// Create overall frame for use in the checkpoint display;
		// this frame will be the default clipping frame for the terminal-display window,
		// in case something goes wrong.
		set_drawing_clip_rectangle(data->top, data->left, data->bottom, data->right);
		
		switch(terminal_data->state)
		{
			case _reading_terminal:
				/* Initialize it if it hasn't been done.. */
				current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
				// LP change: just in case...
				if (!current_group) return;
				
				/* Draw the borders! */
				// LP change: draw these after, so as to avoid overdraw bug
				// draw_terminal_borders(data, terminal_data, &bounds);
				draw_terminal_borders(data, terminal_data, &bounds, false);
				switch(current_group->type)
				{
					case _logon_group:
					case _logoff_group:
						draw_logon_text(&bounds, terminal_text, terminal_data->current_group, 
							current_group->permutation);
						break;
						
					case _unfinished_group:
					case _success_group:
					case _failure_group:
						// dprintf("You shouldn't try to render this view.;g");
						break;
						
					case _information_group:
						/* Draw as normal... */
						InsetRect(&bounds, 72-BORDER_INSET, 0); /* 1 inch in from each side */
						draw_computer_text(&bounds, terminal_text, terminal_data->current_group, 
							terminal_data->current_line);
						break;
						
					case _checkpoint_group: // permutation is the goal to show
						/* note that checkpoints can only be equal to one screenful.. */
						present_checkpoint_text(&bounds, terminal_text, terminal_data->current_group,
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
						fill_terminal_with_static(&bounds);
						break;
						
					case _camera_group:
						break;

					default:
						// vhalt(csprintf(temporary, "What is this group: %d", current_group->type));
						vwarn(false,csprintf(temporary, "What is this group: %d", current_group->type));
						break;
				}
				// Moved down here, so they'd overdraw the other stuff
				draw_terminal_borders(data, terminal_data, &bounds, true);
				break;
				
			default:
				// LP change:
				assert(false);
				// halt();
				break;
		}
		
#if defined(mac)
		// LP change: restore overall clipping
		// Changed to actually-used buffer
		ClipRect(&world_pixels->portRect);
#else
		// Disable clipping
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
#endif
		
		SET_TERMINAL_IS_DIRTY(terminal_data, false);
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
#if defined(mac)
		if (*(keymap + key->offset) & key->mask) raw_flags |= key->action_flag;
#elif defined(SDL)
		if (keymap[key->keycode]) raw_flags |= key->action_flag;
#endif
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
	}
}

/* --------- local code */
static void draw_logon_text(
	Rect *bounds, 
	struct static_preprocessed_terminal_data *terminal_text,
	short current_group_index,
	short logon_shape_id)
{
	Rect picture_bounds= *bounds;
	char *base_text= get_text_base(terminal_text);
	short width;
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, 
		current_group_index);
	// LP change: just in case...
	if (!current_group) return;
	
	/* Draw the login emblem.. */
	display_picture(logon_shape_id, &picture_bounds,  _center_object);

	/* Use the picture bounds to create the logon text crap . */	
	/* The top has the font line height subtracted due to the fudge factor... */
//	picture_bounds.top= picture_bounds.bottom-_get_font_line_height(_computer_interface_font)+5;
	picture_bounds.top= picture_bounds.bottom;
	picture_bounds.bottom= bounds->bottom;
	picture_bounds.left= bounds->left;
	picture_bounds.right= bounds->right;

#ifdef mac
	/* This is always just a line, so we can do this here.. */
	{
		TextSpec old_font;

		GetFont(&old_font);
		SetFont(_get_font_spec(_computer_interface_font));
	
		width= TextWidth(base_text, current_group->start_index, current_group->length);
		picture_bounds.left += (RECTANGLE_WIDTH(&picture_bounds)-width)/2;
		SetFont(&old_font);
	}
#else
	width = text_width(base_text + current_group->start_index, current_group->length, terminal_font, current_style);
	picture_bounds.left += (RECTANGLE_WIDTH(&picture_bounds) - width) / 2;
#endif
	
	_draw_computer_text(base_text, current_group_index, &picture_bounds, terminal_text, 0);

	return;
}

/* returns true for phase chagne */
static void draw_computer_text(
	Rect *bounds, 
	struct static_preprocessed_terminal_data *terminal_text, 
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
	struct static_preprocessed_terminal_data *terminal_text,
	short current_line)
{
	bool done= false;
	short line_count, start_index, text_index;
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, group_index);
	// LP change: just in case...
	if (!current_group) return;
	struct text_face_data text_face;
	short index, last_index, last_text_index, end_index;

#ifdef mac
	/* Set the font.. */
	TextSpec old_font;
	GetFont(&old_font);
	SetFont(_get_font_spec(_computer_interface_font));
#endif

	line_count= 0;
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
		for(text_index= 0; text_index<terminal_text->font_changes_count; ++text_index)
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

#ifdef mac
	SetFont(&old_font);
#endif

	return;
}

static short count_total_lines(
	char *base_text,
	short width,
	short start_index,
	short end_index)
{
	short total_line_count= 0;
	short text_end_index= end_index;

#ifdef mac
	/* Set the font.. */
	TextSpec old_font;
	GetFont(&old_font);
	SetFont(_get_font_spec(_computer_interface_font));
#endif

	while(!calculate_line(base_text, width, start_index, text_end_index, &end_index))
	{
		total_line_count++;
		start_index= end_index;
	}

#ifdef mac
	SetFont(&old_font);
#endif
	
	return total_line_count;
}

static void draw_line(
	char *base_text, 
	short start_index, 
	short end_index, 
	Rect *bounds,
	struct static_preprocessed_terminal_data *terminal_text,
	short *text_face_start_index,
	short line_number)
{
	short line_height= _get_font_line_height(_computer_interface_font);
	bool done= false;
	short text_index, current_start;
	short current_end= end_index;
	struct text_face_data *face_data= NULL;

	if((*text_face_start_index)==NONE) 
	{
		text_index= 0;
	} else {
		text_index= (*text_face_start_index);
	}
	
	/* Get to the first one that concerns us.. */
	if(text_index<terminal_text->font_changes_count)
	{
		do {
			face_data= get_indexed_font_changes(terminal_text, text_index);
			// LP change: just in case...
			if (!face_data) return;
			if(face_data->index<start_index) text_index++;
		} while(face_data->index<start_index && text_index<terminal_text->font_changes_count);
	}
	
	current_start= start_index;
#ifdef mac
	MoveTo(bounds->left, bounds->top+line_height*(line_number+FUDGE_FACTOR));
#else
	int xpos = bounds->left;
#endif

	while(!done)
	{
		if(text_index<terminal_text->font_changes_count)
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

#ifdef mac
		DrawText(base_text, current_start, current_end-current_start);
#else
		xpos += draw_text(world_pixels, base_text + current_start, current_end - current_start,
		                  xpos, bounds->top + line_height * (line_number + FUDGE_FACTOR),
		                  current_pixel, terminal_font, current_style);
#endif
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
	struct static_preprocessed_terminal_data *data, 
	short group_type)
{
	short index;
	
	for(index= 0; index<data->grouping_count; index++)
	{
		struct terminal_groupings *group= get_indexed_grouping(data, index);
		// LP change: just in case...
		if (!group) return NONE;
		if(group->type==group_type) break;
	}
	
	return index;
}

static void teleport_to_level(
	short level_number)
{
	/* It doesn't matter which player we get. */
	struct player_data *player= get_player_data(0);
	
	// LP change: won't be checking this
	// assert(level_number != 0);
	// LP change: moved down by 1 so that level 0 will be valid
	player->teleporting_destination= -level_number - 1;
	player->delay_before_teleport= TICKS_PER_SECOND/2; // delay before we teleport.
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
	Rect *frame,
	short flags,
	Rect *bounds)
{
	if(flags & _center_object)
	{
		// dprintf("splitting text not supported!");
	} 
	else if(flags & _draw_object_on_right)
	{
		calculate_bounds_for_object(frame, 0, bounds, NULL);
	} 
	else 
	{
		calculate_bounds_for_object(frame, _draw_object_on_right, bounds, NULL);
	}

	return;
}

static void display_picture_with_text(
	struct player_terminal_data *terminal_data, 
	Rect *bounds, 
	struct static_preprocessed_terminal_data *terminal_text,
	short current_line)
{
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
	// LP change: just in case...
	if (!current_group) return;
	Rect text_bounds;
	Rect picture_bounds;

	assert(current_group->type==_pict_group);
	picture_bounds= *bounds;
	display_picture(current_group->permutation, &picture_bounds, current_group->flags);

	/* Display the text */
	calculate_bounds_for_text_box(bounds, current_group->flags, &text_bounds);
	draw_computer_text(&text_bounds, terminal_text, terminal_data->current_group, current_line);
}

static void display_picture(
	short picture_id,
	Rect *frame,
	short flags)
{
	LoadedResource PictRsrc;
#ifdef mac
	PicHandle picture;
#else
	SDL_Surface *s = NULL;
#endif

	if (get_picture_resource_from_scenario(picture_id, PictRsrc))
	{
#ifdef mac
		picture = PicHandle(PictRsrc.GetHandle());
#else
		s = picture_to_surface(PictRsrc);
	}
	if (s)
	{
#endif
		Rect bounds;
		Rect screen_bounds;

#if defined(mac)
		bounds= (*picture)->picFrame;
#elif defined(SDL)
		bounds.left = bounds.top = 0;
		bounds.right = s->w;
		bounds.bottom = s->h;
#endif
		OffsetRect(&bounds, -bounds.left, -bounds.top);
		calculate_bounds_for_object(frame, flags, &screen_bounds, &bounds);

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

#if defined(mac)
		HLock((Handle) picture);
		DrawPicture(picture, &bounds);
		HUnlock((Handle) picture);
#elif defined(SDL)
		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		SDL_BlitSurface(s, NULL, world_pixels, &r);
		SDL_FreeSurface(s);
#endif
		/* And let the caller know where we drew the picture */
		*frame= bounds;
	} else {
		Rect bounds;
		char format_string[128];

		calculate_bounds_for_object(frame, flags, &bounds, NULL);
	
#if defined(mac)
		EraseRect(&bounds);
#elif defined(SDL)
		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		SDL_FillRect(world_pixels, &r, SDL_MapRGB(world_pixels->format, 0, 0, 0));
#endif

		getcstr(format_string, strERRORS, pictureNotFound);
		sprintf(temporary, format_string, picture_id);

#if defined(mac)
		// LP change: setting the font to the OS font
		TextFont(systemFont);
		short width= TextWidth(temporary, 0, strlen(temporary));

		/* Center the error message.. */
		MoveTo(bounds.left+(RECTANGLE_WIDTH(&bounds)-width)/2, 
			bounds.top+RECTANGLE_HEIGHT(&bounds)/2);
		DrawText(temporary, 0, strlen(temporary));
#elif defined(SDL)
		sdl_font_info *font = load_font(*_get_font_spec(_computer_interface_title_font));
		int width = text_width(temporary, font, normal);
		draw_text(world_pixels, temporary,
		          bounds.left + (RECTANGLE_WIDTH(&bounds) - width) / 2,
		          bounds.top + RECTANGLE_HEIGHT(&bounds) / 2,
		          SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff),
		          font, normal);
#endif
	}
}

/* Not completed. Remember 16/24 bit & valkyrie */
static void fill_terminal_with_static(
	Rect *bounds)
{
	(void) (bounds);
	// dprintf("Filling with static;g");
#ifdef OBSOLETE
	static long seed= TickCount();
	byte *base;
	short x, y;
	
	base= base_address_of_gworld;
	rowBytes= base_rowbytes;
	for(y= bounds->top; y<bounds->bottom; ++y)
	{
		for(x= bounds->left; x<bounds->right; ++x)
		{
			(*base)++= BASE_STATIC_COLOR+(seed&7);
			seed= (seed&1) ? ((seed>>1)^0xb400) : (seed>> 1);
		}
		base+= (rowBytes-width);
	}
#endif
}

#ifndef PREPROCESSING_CODE
// LP addition: will return NULL if no terminal data was found for this terminal number
static struct static_preprocessed_terminal_data *get_indexed_terminal_data(
	short id)
{
	struct static_preprocessed_terminal_data *data;
	long offset= 0l;
	short index= id;

	data= (struct static_preprocessed_terminal_data *) (map_terminal_data);
	while(index>0) {
		// LP change: be sure to handle this in this routine's caller
		if (offset >= map_terminal_data_length) return NULL;
		// vassert(offset<map_terminal_data_length, csprintf(temporary, "Unable to get data for terminal: %d", id));
		offset+= data->total_length;
		data= (struct static_preprocessed_terminal_data *) (map_terminal_data+offset);
		index--;
	}

	/* Note that this will only decode the text once. */	
	decode_text(data);

	return data;
}
#endif

#ifdef PREPROCESSING_CODE
void decode_text(
	struct static_preprocessed_terminal_data *terminal_text)
#else
static void decode_text(
	struct static_preprocessed_terminal_data *terminal_text)
#endif
{
	if(terminal_text->flags & _text_is_encoded_flag)
	{
		encode_text(terminal_text);
		
		terminal_text->flags &= ~_text_is_encoded_flag;
	}
}

#ifdef PREPROCESSING_CODE
void encode_text(
	struct static_preprocessed_terminal_data *terminal_text)
#else
static void encode_text(
	struct static_preprocessed_terminal_data *terminal_text)
#endif
{
	int length = terminal_text->total_length -
		(sizeof(static_preprocessed_terminal_data) + 
		terminal_text->grouping_count * sizeof(terminal_groupings) +
		terminal_text->font_changes_count * sizeof(text_face_data));

	uint8 *p = (uint8 *)get_text_base(terminal_text);
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
	struct view_terminal_data *data,
	struct player_terminal_data *terminal_data,
	Rect *terminal_frame,
	bool after_other_terminal_stuff)
	// Rect *terminal_frame)
{
	Rect frame, border;
	short top_message, bottom_left_message, bottom_right_message;
	struct static_preprocessed_terminal_data *terminal_text= get_indexed_terminal_data(terminal_data->terminal_id);
	// LP addition: quit if none
	if (!terminal_text) return;
	struct terminal_groupings *current_group= get_indexed_grouping(terminal_text, terminal_data->current_group);
	// LP change: just in case...
	if (!current_group) return;
	
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
	frame.top= data->top;
	frame.bottom= data->bottom;
	frame.left= data->left;
	frame.right= data->right;
		
	// LP change:
	if (!after_other_terminal_stuff)
	{
	/* Erase the rectangle.. */
	_fill_screen_rectangle((screen_rectangle *) &frame, _black_color);
	// LP addition:
	} else {

	/* Now letterbox it if necessary */
	frame.top+= data->vertical_offset;
	frame.bottom-= data->vertical_offset;
	
	/* Draw the top rectangle */
	border= frame;
	border.bottom= border.top+BORDER_HEIGHT;
	_fill_screen_rectangle((screen_rectangle *) &border, _computer_border_background_text_color);

	/* Draw the top login header text... */
	border.left += LABEL_INSET; border.right -= LABEL_INSET;
	getcstr(temporary, strCOMPUTER_LABELS, top_message);
	_draw_screen_text(temporary, (screen_rectangle *) &border, _center_vertical, _computer_interface_font, _computer_border_text_color);
	get_date_string(temporary);
	_draw_screen_text(temporary, (screen_rectangle *) &border, _right_justified | _center_vertical, 
		_computer_interface_font, _computer_border_text_color);
	
	/* Draw the the bottom rectangle & text */
	border= frame;
	border.top= border.bottom-BORDER_HEIGHT;
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
	
	/* The screen rectangle minus the border.. */
	*terminal_frame= frame;
	InsetRect(terminal_frame, BORDER_INSET, BORDER_HEIGHT+BORDER_INSET);
}

static void next_terminal_state(
	short player_index)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	struct player_data *player= get_player_data(player_index);

	switch(terminal->state)
	{
#ifdef OBSOLETE
	struct static_preprocessed_terminal_data *terminal_text;
	
		case _logging_in_terminal:
			terminal->state= _reading_terminal;
			terminal->phase= NONE; /* Different type.. */
			/* Get the terminal text.. */
			terminal_text= get_indexed_terminal_data(terminal->terminal_id);
			// LP addition: quit if none
			if (!terminal_text) return;
			terminal->current_group= NONE;
			next_terminal_group(player_index, terminal_text);
			break;

		case _logging_out_terminal:
			terminal->state= _no_terminal_state;
			terminal->phase= 0;
			terminal->current_group= NONE;
			terminal->current_line= 0;
			terminal->maximum_line= 1; // any click or keypress will get us out.
			initialize_player_terminal_info(player_index);
			break;
		case _reading_terminal:
			terminal->state= _logging_out_terminal;
			terminal->phase= LOG_DURATION_BEFORE_TIMEOUT;
			terminal->current_group= NONE;
			terminal->current_line= 0;
			terminal->maximum_line= 1; // any click or keypress will get us out.
			SET_TERMINAL_IS_DIRTY(terminal, true);
#ifndef PREPROCESSING_CODE
			play_object_sound(player->object_index, _snd_computer_interface_logout);
#endif			
#endif
		case _reading_terminal:
			initialize_player_terminal_info(player_index);
			break;
			
		case _no_terminal_state:
		default:
			// vhalt(csprintf(temporary, "What is %d?", terminal->state));
			vwarn(false,csprintf(temporary, "What is %d?", terminal->state));
			break;
	}

	return;
}

static bool previous_terminal_group(
	short player_index,
	struct static_preprocessed_terminal_data *terminal_text)
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
						break;
						
					case _camera_group:
						break;
						
					case _unfinished_group:
					case _success_group:
					case _failure_group:
					case _static_group:
						use_new_group= false;
						break;
					
					default:
						// LP change:
						assert(false);
						// halt();
						break;
				}
			} else {
				done= true;
				use_new_group= false;
			}
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
	struct static_preprocessed_terminal_data *terminal_text)
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
				if(terminal_data->current_group==terminal_text->grouping_count) 
				{
					/* Fallback. */
					terminal_data->current_group= find_group_type(terminal_text, _unfinished_group);
					assert(terminal_data->current_group != terminal_text->grouping_count);
				}
				break;
				
			case _level_failed:
				terminal_data->current_group= find_group_type(terminal_text, _failure_group);
				if(terminal_data->current_group==terminal_text->grouping_count) 
				{
					/* Fallback. */
					terminal_data->current_group= find_group_type(terminal_text, _unfinished_group);
					assert(terminal_data->current_group != terminal_text->grouping_count);
				}
				break;
			
			default:
				// vhalt(csprintf(temporary, "What is type %d?", 
				vwarn(false,csprintf(temporary, "What is type %d?", 
				 	terminal_data->level_completion_state));
				break;
		}

		/* Note that the information groups are now keywords, and can have no data associated with them */
		next_terminal_group(player_index, terminal_text);
	} else {
		terminal_data->current_group++;
		if(terminal_data->current_group>=terminal_text->grouping_count)
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

	return;
}

static void goto_terminal_group(
	short player_index, 
	struct static_preprocessed_terminal_data *terminal_text, 
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
#ifndef PREPROCESSING_CODE
			play_object_sound(player->object_index, _snd_computer_interface_logon);
#endif
			terminal_data->phase= LOG_DURATION_BEFORE_TIMEOUT;
			terminal_data->maximum_line= current_group->maximum_line_count;
			break;
			
		case _logoff_group:
#ifndef PREPROCESSING_CODE
			play_object_sound(player->object_index, _snd_computer_interface_logout);
#endif
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
				Rect text_bounds, bounds;
	
				/* The only thing we care about is the width. */
				SetRect(&bounds, 0, 0, 640, 480);
				InsetRect(&bounds, BORDER_INSET, BORDER_HEIGHT+BORDER_INSET);
				calculate_bounds_for_text_box(&bounds, current_group->flags, &text_bounds);
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
				short width= 640; // еее sync (Must guarantee 100 high res!)
	
				width-= 2*(72-BORDER_INSET); /* 1 inch in from each side */				
				terminal_data->maximum_line= count_total_lines(get_text_base(terminal_text), 
					width, current_group->start_index, 
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
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return;
}

/* I'll use this function, almost untouched.. */
static void get_date_string(
	char *date_string)
{
	char temp_string[101];
	long game_time_passed;
	time_t seconds;
	struct tm game_time;

	/* Treat the date as if it were recent. */
	game_time_passed= LONG_MAX - dynamic_world->game_information.game_time_remaining;
	
	/* convert the game seconds to machine seconds */
#ifdef mac
	seconds = 2882914937;
	seconds += (game_time_passed/TICKS_PER_SECOND)*MACINTOSH_TICKS_PER_SECOND; 
	DateTimeRec converted_date;
	SecondsToDate(seconds, &converted_date);
	
	game_time.tm_sec= converted_date.second;
	game_time.tm_min= converted_date.minute;
	game_time.tm_hour= converted_date.hour;
	game_time.tm_mday= converted_date.day;
	game_time.tm_mon= converted_date.month-1;
	game_time.tm_wday= converted_date.dayOfWeek;
#else
	seconds = 800070137;
	seconds += game_time_passed / TICKS_PER_SECOND;
	game_time = *localtime(&seconds);
#endif
	game_time.tm_year= 437;
	game_time.tm_yday= 0;
	game_time.tm_isdst= 0;

	getcstr(temp_string, strCOMPUTER_LABELS, _date_format);
	strftime(date_string, 100, temp_string, &game_time);
}

static void present_checkpoint_text(
	Rect *frame,
	struct static_preprocessed_terminal_data *terminal_text,
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
	bounds= *frame;
	calculate_bounds_for_object(frame, current_group->flags, &bounds, NULL);
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
#ifdef mac
		// LP change: set the clip window to that for the overhead data
		ClipRect(&bounds);
#endif
		_render_overhead_map(&overhead_data);
#ifdef mac
		// Reset it to the overall bounds
		ClipRect(&OverallBounds);
#endif
	} else {
		char format_string[128];
	
#if defined(mac)
		EraseRect(&bounds);
#elif defined(SDL)
		SDL_Rect r = {bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top};
		SDL_FillRect(world_pixels, &r, SDL_MapRGB(world_pixels->format, 0, 0, 0));
#endif

		getcstr(format_string, strERRORS, checkpointNotFound);
		sprintf(temporary, format_string, current_group->permutation);

#if defined(mac)
		short width= TextWidth(temporary, 0, strlen(temporary));

		/* Center the error message.. */
		MoveTo(bounds.left+(RECTANGLE_WIDTH(&bounds)-width)/2, 
			bounds.top+RECTANGLE_HEIGHT(&bounds)/2);
		DrawText(temporary, 0, strlen(temporary));
#elif defined(SDL)
		sdl_font_info *font = load_font(*_get_font_spec(_computer_interface_title_font));
		int width = text_width(temporary, font, normal);
		draw_text(world_pixels, temporary,
		          bounds.left + (RECTANGLE_WIDTH(&bounds) - width) / 2,
		          bounds.top + RECTANGLE_HEIGHT(&bounds) / 2,
		          SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff),
		          font, normal);
#endif
	}

	// draw the text
	calculate_bounds_for_text_box(frame, current_group->flags, &bounds);
	draw_computer_text(&bounds, terminal_text, current_group_index, current_line);

	return;
}

static bool find_checkpoint_location(
	short checkpoint_index, 
	world_point2d *location, 
	short *polygon_index)
{
	bool success= false;
#ifndef PREPROCESSING_CODE
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
		// LP change: made this much more correct
		// assert(*polygon_index);
		// success= true;
		success = (*polygon_index != NONE);
	}
#else
	(void)(checkpoint_index, location, polygon_index);
#endif
	
	return success;		
}

static void handle_reading_terminal_keys(
	short player_index,
	long action_flags)
{
	struct player_terminal_data *terminal= get_player_terminal_data(player_index);
	struct static_preprocessed_terminal_data *terminal_text= get_indexed_terminal_data(terminal->terminal_id);
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
#ifndef PREPROCESSING_CODE
				play_object_sound(player->object_index, _snd_computer_interface_page);
#endif
				line_delta= terminal_text->lines_per_page;
			} 
			
			if(action_flags & _terminal_page_up)
			{
#ifndef PREPROCESSING_CODE
				play_object_sound(player->object_index, _snd_computer_interface_page);
#endif
				line_delta= -terminal_text->lines_per_page;
			}

			/* this one should change state, if necessary */
			if(action_flags & _terminal_next_state)
			{
#ifndef PREPROCESSING_CODE
				play_object_sound(player->object_index, _snd_computer_interface_page);
#endif
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
#ifndef PREPROCESSING_CODE
			teleport_to_level(current_group->permutation);
#else	
			// dprintf("Terminal Editor: Teleporting to level: %d", current_group->permutation);
#endif
			initialize_player_terminal_info(player_index);
			aborted= true;
			break;

		case _intralevel_teleport_group: // permutation is polygon to go to.
#ifndef PREPROCESSING_CODE
			teleport_to_polygon(player_index, current_group->permutation);
#else	
			// dprintf("Terminal Editor: Teleporting to polygon: %d", current_group->permutation);
#endif
			initialize_player_terminal_info(player_index);
			aborted= true;
			break;

		case _sound_group: // permutation is the sound id to play
			/* Play the sound immediately, and then go to the next level.. */
			{
				struct player_data *player= get_player_data(player_index);
#ifndef PREPROCESSING_CODE
				play_object_sound(player->object_index, current_group->permutation);
#endif
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
			// vhalt(csprintf(temporary, "What is group: %d", current_group->type));
			vwarn(false,csprintf(temporary, "What is group: %d", current_group->type));
			break;
	}

	/* If we are dirty.. */
	terminal->current_line+= line_delta;
	if(!aborted && initial_group != terminal->current_group ||  
		initial_line != terminal->current_line)
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
			if(terminal->current_group+1>=terminal_text->grouping_count)
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
	Rect *frame,
	short flags,
	Rect *bounds,
	Rect *source)
{
	*bounds= *frame;
	
	if(source && flags & _center_object)
	{
		if(RECTANGLE_WIDTH(source)>RECTANGLE_WIDTH(frame) 
			|| RECTANGLE_HEIGHT(source)>RECTANGLE_HEIGHT(frame))
		{
			/* Just return the normal frame.  Aspect ratio will take care of us.. */
		} else {
			InsetRect(bounds, (RECTANGLE_WIDTH(frame)-RECTANGLE_WIDTH(source))/2,
				(RECTANGLE_HEIGHT(frame)-RECTANGLE_WIDTH(source))/2);
		}
	} 
	else if(flags & _draw_object_on_right)
	{
		bounds->left= bounds->right - RECTANGLE_WIDTH(bounds)/2 + BORDER_INSET/2;
	} 
	else 
	{
		bounds->right= bounds->left + RECTANGLE_WIDTH(bounds)/2 - BORDER_INSET/2;
	}
}

#ifdef PREPROCESSING_CODE
/*------------ */
/* -----> Preprocessing code... */
struct group_header_data {
	char *name;
	bool has_permutation;
};

static struct group_header_data group_data[]= {
	{"LOGON", true }, // permutation is the logo id to draw...
	{"UNFINISHED", false },
 	{"FINISHED", false },
	{"FAILED", false },
	{"INFORMATION", false },
	{"END", false },
	{"INTERLEVEL TELEPORT", true },
	{"INTRALEVEL TELEPORT", true },
	{"CHECKPOINT", true },
	{"SOUND", true },
	{"MOVIE", true },
	{"TRACK", true },
	{"PICT", true},
	{"LOGOFF", true }, // permutation is the logo id to draw...
	{"CAMERA", true}, // permutation is the object index
	{"STATIC", true}, // permutation is the duration of static.
	{"TAG", true} // permutation is the tag to activate
};

#define MAXIMUM_GROUPS_PER_TERMINAL 15

static void calculate_maximum_lines_for_groups(struct terminal_groupings *groups, 
	short group_count, char *text_base);

/* Note that this is NOT a marathon function, but an editor function... */
struct static_preprocessed_terminal_data *preprocess_text(
	char *text, 
	short length)
{
	short group_count, text_face_count;
	struct terminal_groupings groups[MAXIMUM_GROUPS_PER_TERMINAL];
	struct text_face_data text_faces[MAXIMUM_FACE_CHANGES_PER_TEXT_GROUPING];
	short new_length;
	struct static_preprocessed_terminal_data *data_structure;
	long total_length;
	short index;
	char *text_destination;

	new_length= length;
	pre_build_groups(groups, &group_count, text_faces, &text_face_count,
		text, &new_length);

	/* Allocate our conglomerated structure */
	total_length= sizeof(static_preprocessed_terminal_data) +
		group_count * sizeof(terminal_groupings) +
		text_face_count * sizeof(text_face_data) +
		new_length;

	data_structure= (struct static_preprocessed_terminal_data *) malloc(total_length);
	assert(data_structure);

	/* set these up.. */
	data_structure->total_length= total_length;
	data_structure->flags= 0; /* Don't encode (yet) */
	data_structure->grouping_count= group_count;
	data_structure->font_changes_count= text_face_count;

	/* Calculate the default lines per page for this font */
	data_structure->lines_per_page= calculate_lines_per_page();

	/* Calculate the maximum lines for each group */
	calculate_maximum_lines_for_groups(groups, group_count, text);

	for(index= 0; index<group_count; ++index)
	{
		struct terminal_groupings *destination;
		
		destination= get_indexed_grouping(data_structure, index);
		obj_copy(*destination, groups[index]);
	}

	for(index= 0; index<text_face_count; ++index)
	{
		struct text_face_data *destination;
		
		destination= get_indexed_font_changes(data_structure, index);
		obj_copy(*destination, text_faces[index]);

		// dprintf("%d/%d index: %d face: %d color: %d;g", index, text_face_count, text_faces[index].index, text_faces[index].face, text_faces[index].color);
	}

	text_destination= get_text_base(data_structure);
	memcpy(text_destination, text, new_length);
	//dprintf("Base: %x new_length: %d", text_destination, new_length);

	/* We be done! */
	return data_structure;
}

/* Life would be better if these were encoded like this for me.. */
static void pre_build_groups(
	struct terminal_groupings *groups, 
	short *group_count,
	struct text_face_data *text_faces,
	short *text_face_count,
	char *base_text,
	short *base_length)
{
	long index, current_length;
	bool in_group= false;
	short current_face, face_count, color_index, grp_count, data_length;
	bool last_was_return= true; /* in case the first line is a comment */

	/* Initial color, and face (plain) */
	color_index= current_face= 0;

	grp_count= face_count= 0;
	data_length= (*base_length);
	current_length= index= 0;
	/* Find the text groupings! */
	while(index<data_length)
	{
		if(base_text[index]=='#')
		{
			short possible_group;
			char first_char;

			/* This is some form of descriptive keyword.. */
			first_char= toupper(base_text[index+1]);
			for(possible_group= 0; possible_group<NUMBER_OF_GROUP_TYPES; ++possible_group)
			{
				if(first_char==group_data[possible_group].name[0])
				{
					short start_index, permutation;
					
					start_index= matches_group(base_text, data_length, index+1, possible_group, 
						&permutation);
					if(start_index != NONE)
					{
						short destination_index;
						
						if(in_group) 
						{
							/* Minus one, because we have the # included.. */
							assert(grp_count+1<MAXIMUM_GROUPS_PER_TERMINAL);
							groups[grp_count++].length= current_length;
							base_text[index]= 0; // null out the end of each group.
							destination_index= index+1;
						} else {	
							destination_index= index;
						}
						
						assert(data_length-start_index>=0);
						/* move it down-> check this! */
						memmove(&base_text[destination_index], 
							&base_text[start_index], data_length-start_index);
						data_length -= (start_index-destination_index);
						groups[grp_count].flags= 0;
						groups[grp_count].type= possible_group;
						groups[grp_count].permutation= permutation;
						groups[grp_count].start_index= destination_index;
						current_length= 0;
						in_group= true;
						break; /* out of for loop */
					}
				}
			}

			/* If we didn't match it, continue on..... */
			if(possible_group==NUMBER_OF_GROUP_TYPES)
			{
				index++;
				current_length++;
			}
//			vassert(possible_group==NUMBER_OF_GROUP_TYPES || base_text[index]!='#', 
//				csprintf(temporary, "Base: %x index: %d", base_text, index));
		} 
		else if(base_text[index]=='$')
		{
			bool changed_font= true;
			short move_down_by;
			
			move_down_by= 2;
			switch(base_text[index+1])
			{
				case 'B': /* Bold on! */
					current_face |= _bold_text;
					break;
					
				case 'b': /* bold off */
					current_face &= ~_bold_text;
					break;

				case 'I': /* Italic on */
					current_face |= _italic_text;
					break;
					
				case 'i': /* Italic off */
					current_face &= ~_italic_text;
					break;
					
				case 'U': /* Underline on */
					current_face |= _underline_text;
					break;
					
				case 'u': /* Underline off */
					current_face &= ~_underline_text;
					break;

				case 'C':
					switch(base_text[index+2])
					{
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
							color_index= base_text[index+2]-'0';
							move_down_by= 3;
							break;
							
						default:
							break;
					}
					break;
					
				default:
					/* Pass it on through, unchanged */
					changed_font= false;
					move_down_by= 0;
					break;
			}
			
			/* If we changed the font, move the text down.. */
			if(changed_font)
			{
				assert(face_count+1<MAXIMUM_FACE_CHANGES_PER_TEXT_GROUPING);
				text_faces[face_count].index= index;
				text_faces[face_count].color= color_index;
				text_faces[face_count++].face= current_face;
				
				/* And move the data down by 2 characters.. */
				memmove(&base_text[index], 
					&base_text[index+move_down_by], data_length-index-move_down_by);
				data_length -= move_down_by;
	
				/* Note that index doesn't change.. */
			} else {
				index++;
				current_length++;
			}
		} 
		else if (base_text[index]==';' && last_was_return)
		{
			short destination_index, start_index;

			/* this is a comment */
			destination_index= start_index= index;
			while(start_index<data_length && base_text[start_index] != MAC_LINE_END) start_index++;

			/* Eat the return on the comment line. */
			if(start_index<data_length && base_text[start_index]==MAC_LINE_END) start_index++;
			
			/* Nix the comment */			
			memmove(&base_text[destination_index], &base_text[start_index], data_length-start_index);
			data_length -= (start_index-destination_index);
		} else {
			if(base_text[index]==MAC_LINE_END)
			{
				last_was_return= true;
			} else {
				last_was_return= false;
			}
			index++;
			current_length++;
		}
	}

	/* Save the proper positions.. */
	(*text_face_count)= face_count;
	(*group_count)= grp_count;
	(*base_length)= data_length;
	if(in_group) groups[grp_count].length= current_length;

	return;
}

/* Return NONE if it doesn't matches.. */
static short matches_group(
	char *base_text, 
	short length,
	short index, 
	short possible_group, 
	short *permutation)
{
	short start_index= NONE;

	if(memcmp(&base_text[index], group_data[possible_group].name, 
		strlen(group_data[possible_group].name))==0)
	{
		/* This is a match.  find the end... */
		start_index= index+strlen(group_data[possible_group].name);
		
		if(group_data[possible_group].has_permutation)
		{
			/* Find the permutation... */
			*permutation= atoi(&base_text[start_index]);
		}
		
		/* Eat the rest of it.. */
		while(start_index<length && base_text[start_index] != MAC_LINE_END) start_index++;
		if(base_text[start_index]==MAC_LINE_END) start_index++;
	}
	
	return start_index;
}

static void find_all_permutations_of_type(
	byte *terminals,
	short terminal_count,
	short group_type,
	short *permutations,
	short *permutation_count)
{
	short terminal_index;
	struct static_preprocessed_terminal_data *data;
	long offset= 0l;
	short count= 0;

	for(terminal_index= 0; terminal_index<terminal_count; terminal_index++)
	{
		short group_index;
	
		data= (struct static_preprocessed_terminal_data *) (terminals+offset);
		for(group_index= 0; group_index<data->grouping_count; ++group_index)
		{
			struct terminal_groupings *group= get_indexed_grouping(data, group_index);
			// LP addition: just in case...
			if (!group) continue;
			if(group->type==group_type)
			{
				permutations[count++]= group->permutation;
			}
		}
		
		offset+= data->total_length;
	}

	*permutation_count= count;
}

void find_all_picts_references_by_terminals(
	byte *compiled_text, 
	short terminal_count,
	short *picts, 
	short *picture_count)
{
	find_all_permutations_of_type((byte *) compiled_text, terminal_count, _pict_group, picts, 
		picture_count);
}

void find_all_checkpoints_references_by_terminals(
	byte *compiled_text, 
	short terminal_count,
	short *checkpoints, 
	short *checkpoint_count)
{
	find_all_permutations_of_type((byte *) compiled_text, terminal_count, _checkpoint_group, 
		checkpoints, checkpoint_count);
}

bool terminal_has_finished_text_type(
	short terminal_id,
	short finished_type)
{
	bool has_type= false;
	struct static_preprocessed_terminal_data *terminal_text= get_indexed_terminal_data(terminal_id);
	// LP addition: quit if none
	if (!terminal_text) return false;
	short index;
	
	index= find_group_type(terminal_text, finished_type);
	if(index==terminal_text->grouping_count) 
	{
		has_type= false;
	} else {
		has_type= true;
	}

	return has_type;
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
					Rect text_bounds, bounds;
		
					/* The only thing we care about is the width. */
					SetRect(&bounds, 0, 0, 640, 480);
					InsetRect(&bounds, BORDER_INSET, BORDER_HEIGHT+BORDER_INSET);
					calculate_bounds_for_text_box(&bounds, groups[index].flags, &text_bounds);
					groups[index].maximum_line_count= count_total_lines(text_base,
						RECTANGLE_WIDTH(&text_bounds), groups[index].start_index, 
						groups[index].start_index+groups[index].length);
				}
				break;
				
			case _information_group:
				{
					short width= 640; // еее sync (Must guarantee 100 high res!)
	
					width-= 2*(72-BORDER_INSET); /* 1 inch in from each side */				
					groups[index].maximum_line_count= count_total_lines(text_base, 
						width, groups[index].start_index, 
						groups[index].start_index+groups[index].length);
				}
				break;
				
			default:
				// LP change:
				assert(false);
				// halt();
				break;

		}
	}

	return;
}
#endif

static struct terminal_groupings *get_indexed_grouping(
	struct static_preprocessed_terminal_data *data,
	short index)
{
	byte *start;
	
	// LP change: put in more graceful degradation
	if (!(index>=0 && index<data->grouping_count)) return NULL;
	// assert(index>=0 && index<data->grouping_count);
	start= (byte *) data;
	start += sizeof(static_preprocessed_terminal_data) + 
		index * sizeof(terminal_groupings);

	return (struct terminal_groupings *) start;
}

static struct text_face_data *get_indexed_font_changes(
	struct static_preprocessed_terminal_data *data,
	short index)
{
	byte *start;
	
	// LP change: put in more graceful degradation
	if (!(index>=0 && index<data->font_changes_count)) return NULL;
	// assert(index>=0 && index<data->font_changes_count);
	start= (byte *) data;
	start += sizeof(static_preprocessed_terminal_data) + 
		data->grouping_count * sizeof(terminal_groupings) +
		index * sizeof(text_face_data);

	return (struct text_face_data *) start;
}

static char *get_text_base(
	struct static_preprocessed_terminal_data *data)
{
	byte *start;

	start= (byte *) data;
	start += sizeof(static_preprocessed_terminal_data) + 
		data->grouping_count * sizeof(terminal_groupings) +
		data->font_changes_count * sizeof(text_face_data);

	return (char *) start;
}

static short calculate_lines_per_page(
	void)
{
	Rect bounds;
	short lines_per_page;

	calculate_destination_frame(_100_percent, true, &bounds);
	lines_per_page= (RECTANGLE_HEIGHT(&bounds)-2*BORDER_HEIGHT)/_get_font_line_height(_computer_interface_font);
	lines_per_page-= FUDGE_FACTOR;

	return lines_per_page;
}


/*
 *  Byte-swap terminal data
 */

void byte_swap_terminal_data(uint8 *data, int length)
{
	// LP: verify object sizes:
	assert(sizeof(static_preprocessed_terminal_data) == SIZEOF_static_preprocessed_terminal_data);
	assert(sizeof(terminal_groupings) == SIZEOF_terminal_groupings);
	assert(sizeof(text_face_data) == SIZEOF_text_face_data);
	
	while (length > 0) {
		uint8 *p = data;

		// Swap static_preprocessed_terminal_data
		static_preprocessed_terminal_data *d = (static_preprocessed_terminal_data *)p;
		byte_swap_object(*d, _bs_static_preprocessed_terminal_data);
		p += sizeof(static_preprocessed_terminal_data);

		// Swap groupings
		terminal_groupings *g = (terminal_groupings *)p;
		byte_swap_object_list(g, d->grouping_count, _bs_terminal_groupings);
		p += sizeof(terminal_groupings) * d->grouping_count;

		// Swap font changes
		text_face_data *f = (text_face_data *)p;
		byte_swap_object_list(f, d->font_changes_count, _bs_text_face_data);
		p += sizeof(text_face_data) * d->font_changes_count;

		data += d->total_length;
		length -= d->total_length;
	}
}

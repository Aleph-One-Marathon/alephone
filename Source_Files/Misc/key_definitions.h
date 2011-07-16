#ifndef __KEY_DEFINITIONS_H
#define __KEY_DEFINITIONS_H

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

*/

/*
 * Monday, September 12, 1994 12:45:17 PM  (alain)
 *   This header file can only be included by one other file. right now that's vbl.c
 *
 */


/* Constants */
enum /* special flag types */
{
	_double_flag,
	_latched_flag
};

/* Structures */
struct blacklist_data
{
	int16 offset1, offset2; /* the combination of keys that should be blacklisted */
	int16 mask1, mask2;     /* help for finding them in the keymap */
};

struct special_flag_data
{
	int16 type;
	int32 flag, alternate_flag;
	int16 persistence;
};

struct key_definition
{
#ifdef SDL
	SDLKey offset;
	uint32 action_flag;
#else
	int16 offset;
	int32 action_flag;
	int16 mask;
#endif
};

/*
 * various key setups that the user can get.
 * NOTE that these arrays must all be in the same order, and they must
 * be in the same order as the text edit boxes in the "setup keys" dialog
 *
 */

#define NUMBER_OF_STANDARD_KEY_DEFINITIONS (sizeof(standard_key_definitions)/sizeof(struct key_definition))
static struct key_definition standard_key_definitions[]=
{
#ifdef SDL
	/* keypad */
	{SDLK_KP8, _moving_forward},
	{SDLK_KP5, _moving_backward},
	{SDLK_KP4, _turning_left},
	{SDLK_KP6, _turning_right},
	
	/* zx translation */
	{SDLK_z, _sidestepping_left},
	{SDLK_x, _sidestepping_right},

	/* as looking */
	{SDLK_a, _looking_left},
	{SDLK_s, _looking_right},

	/* dcv vertical looking */
	{SDLK_d, _looking_up},
	{SDLK_c, _looking_down},
	{SDLK_v, _looking_center},
	
	/* KP7/KP9 for weapon cycling */
	{SDLK_KP7, _cycle_weapons_backward},
	{SDLK_KP9, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDLK_SPACE, _left_trigger_state},
	{SDLK_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDLK_LSHIFT, _sidestep_dont_turn},
	{SDLK_LCTRL, _run_dont_walk},
	{SDLK_LMETA, _look_dont_turn},
	
	/* tab for action */
	{SDLK_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDLK_m, _toggle_map},
	
	/* ` for using the microphone */
	{SDLK_BACKQUOTE, _microphone_button}
#else
	/* keypad */
	{0x5b, _moving_forward},
	{0x57, _moving_backward},
	{0x56, _turning_left},
	{0x58, _turning_right},
	
	/* zx translation */
	{0x06, _sidestepping_left},
	{0x07, _sidestepping_right},

	/* as looking */
	{0x00, _looking_left},
	{0x01, _looking_right},

	/* dcv vertical looking */
	{0x02, _looking_up},
	{0x08, _looking_down},
	{0x09, _looking_center},
	
	/* KP7/KP9 for weapon cycling */
	{0x59, _cycle_weapons_backward},
	{0x5c, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{0x31, _left_trigger_state},
	{0x3a, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{0x37, _sidestep_dont_turn},
	{0x3b, _run_dont_walk},
	{0x38, _look_dont_turn},
	
	/* tab for action */
	{0x30, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{0x2e, _toggle_map},
	
	/* ` for using the microphone */
	{0x32, _microphone_button}
#endif
};

#define NUMBER_OF_LEFT_HANDED_KEY_DEFINITIONS (sizeof(left_handed_key_definitions)/sizeof(struct key_definition))
static struct key_definition left_handed_key_definitions[]=
{
#ifdef SDL
	/* arrows */
	{SDLK_UP, _moving_forward},
	{SDLK_DOWN, _moving_backward},
	{SDLK_LEFT, _turning_left},
	{SDLK_RIGHT, _turning_right},
	
	/* zx translation */
	{SDLK_z, _sidestepping_left},
	{SDLK_x, _sidestepping_right},

	/* as looking */
	{SDLK_a, _looking_left},
	{SDLK_s, _looking_right},

	/* dcv vertical looking */
	{SDLK_d, _looking_up},
	{SDLK_c, _looking_down},
	{SDLK_v, _looking_center},
	
	/* ;' for weapon cycling */
	{SDLK_SEMICOLON, _cycle_weapons_backward},
	{SDLK_QUOTE, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDLK_SPACE, _left_trigger_state},
	{SDLK_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDLK_LSHIFT, _sidestep_dont_turn},
	{SDLK_LCTRL, _run_dont_walk},
	{SDLK_LMETA, _look_dont_turn},
	
	/* tab for action */
	{SDLK_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDLK_m, _toggle_map},
	
	/* ` for using the microphone */
	{SDLK_BACKQUOTE, _microphone_button}
#else
	/* arrows */
	{0x7e, _moving_forward},
	{0x7d, _moving_backward},
	{0x7b, _turning_left},
	{0x7c, _turning_right},
	
	/* zx translation */
	{0x06, _sidestepping_left},
	{0x07, _sidestepping_right},

	/* as looking */
	{0x00, _looking_left},
	{0x01, _looking_right},

	/* dcv vertical looking */
	{0x02, _looking_up},
	{0x08, _looking_down},
	{0x09, _looking_center},
	
	/* ;' for weapon cycling */
	{0x0c, _cycle_weapons_backward},
	{0x0d, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{0x31, _left_trigger_state},
	{0x3a, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{0x37, _sidestep_dont_turn},
	{0x3b, _run_dont_walk},
	{0x38, _look_dont_turn},
	
	/* tab for action */
	{0x30, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{0x2e, _toggle_map},
	
	/* ` for using the microphone */
	{0x32, _microphone_button}
#endif
};

#define NUMBER_OF_POWERBOOK_KEY_DEFINITIONS (sizeof(powerbook_key_definitions)/sizeof(struct key_definition))
static struct key_definition powerbook_key_definitions[]=
{
#ifdef SDL
	/* olk; */
	{SDLK_o, _moving_forward},
	{SDLK_l, _moving_backward},
	{SDLK_k, _turning_left},
	{SDLK_SEMICOLON, _turning_right},
	
	/* zx translation */
	{SDLK_z, _sidestepping_left},
	{SDLK_x, _sidestepping_right},

	/* as looking */
	{SDLK_a, _looking_left},
	{SDLK_s, _looking_right},

	/* dcv vertical looking */
	{SDLK_d, _looking_up},
	{SDLK_c, _looking_down},
	{SDLK_v, _looking_center},
	
	/* ip for weapon cycling */
	{SDLK_i, _cycle_weapons_backward},
	{SDLK_p, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDLK_SPACE, _left_trigger_state},
	{SDLK_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDLK_LSHIFT, _sidestep_dont_turn},
	{SDLK_LCTRL, _run_dont_walk},
	{SDLK_LMETA, _look_dont_turn},
	
	/* tab for action */
	{SDLK_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDLK_m, _toggle_map},
	
	/* ` for using the microphone */
	{SDLK_BACKQUOTE, _microphone_button}
#else
	/* olk; */
	{0x1f, _moving_forward},
	{0x25, _moving_backward},
	{0x28, _turning_left},
	{0x29, _turning_right},
	
	/* zx translation */
	{0x06, _sidestepping_left},
	{0x07, _sidestepping_right},

	/* as looking */
	{0x00, _looking_left},
	{0x01, _looking_right},

	/* dcv vertical looking */
	{0x02, _looking_up},
	{0x08, _looking_down},
	{0x09, _looking_center},
	
	/* ip for weapon cycling */
	{0x22, _cycle_weapons_backward},
	{0x23, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{0x31, _left_trigger_state},
	{0x3a, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{0x37, _sidestep_dont_turn},
	{0x3b, _run_dont_walk},
	{0x38, _look_dont_turn},
	
	/* tab for action */
	{0x30, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{0x2e, _toggle_map},
	
	/* ` for using the microphone */
	{0x32, _microphone_button}
#endif
};

static struct key_definition *all_key_definitions[NUMBER_OF_KEY_SETUPS]=
{
	standard_key_definitions,
	left_handed_key_definitions,
	powerbook_key_definitions
};

/* Externed because both vbl.c and vbl_macintosh.c use this. */
extern struct key_definition current_key_definitions[NUMBER_OF_STANDARD_KEY_DEFINITIONS];

#endif


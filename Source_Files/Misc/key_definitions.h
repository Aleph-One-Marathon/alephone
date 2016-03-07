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

#include "interface.h"
#include "player.h"


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
	SDL_Scancode offset;
	uint32 action_flag;
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
	/* keypad */
	{SDL_SCANCODE_KP_8, _moving_forward},
	{SDL_SCANCODE_KP_5, _moving_backward},
	{SDL_SCANCODE_KP_4, _turning_left},
	{SDL_SCANCODE_KP_6, _turning_right},
	
	/* zx translation */
	{SDL_SCANCODE_Z, _sidestepping_left},
	{SDL_SCANCODE_X, _sidestepping_right},

	/* as looking */
	{SDL_SCANCODE_A, _looking_left},
	{SDL_SCANCODE_S, _looking_right},

	/* dcv vertical looking */
	{SDL_SCANCODE_D, _looking_up},
	{SDL_SCANCODE_C, _looking_down},
	{SDL_SCANCODE_V, _looking_center},
	
	/* KP7/KP9 for weapon cycling */
	{SDL_SCANCODE_KP_7, _cycle_weapons_backward},
	{SDL_SCANCODE_KP_9, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDL_SCANCODE_SPACE, _left_trigger_state},
	{SDL_SCANCODE_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDL_SCANCODE_LSHIFT, _sidestep_dont_turn},
	{SDL_SCANCODE_LCTRL, _run_dont_walk},
	{SDL_SCANCODE_LGUI, _look_dont_turn},
	
	/* tab for action */
	{SDL_SCANCODE_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDL_SCANCODE_M, _toggle_map},
	
	/* ` for using the microphone */
	{SDL_SCANCODE_GRAVE, _microphone_button}
};

#define NUMBER_OF_LEFT_HANDED_KEY_DEFINITIONS (sizeof(left_handed_key_definitions)/sizeof(struct key_definition))
static struct key_definition left_handed_key_definitions[]=
{
	/* arrows */
	{SDL_SCANCODE_UP, _moving_forward},
	{SDL_SCANCODE_DOWN, _moving_backward},
	{SDL_SCANCODE_LEFT, _turning_left},
	{SDL_SCANCODE_RIGHT, _turning_right},
	
	/* zx translation */
	{SDL_SCANCODE_Z, _sidestepping_left},
	{SDL_SCANCODE_X, _sidestepping_right},

	/* as looking */
	{SDL_SCANCODE_A, _looking_left},
	{SDL_SCANCODE_S, _looking_right},

	/* dcv vertical looking */
	{SDL_SCANCODE_D, _looking_up},
	{SDL_SCANCODE_C, _looking_down},
	{SDL_SCANCODE_V, _looking_center},
	
	/* ;' for weapon cycling */
	{SDL_SCANCODE_SEMICOLON, _cycle_weapons_backward},
	{SDL_SCANCODE_APOSTROPHE, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDL_SCANCODE_SPACE, _left_trigger_state},
	{SDL_SCANCODE_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDL_SCANCODE_LSHIFT, _sidestep_dont_turn},
	{SDL_SCANCODE_LCTRL, _run_dont_walk},
	{SDL_SCANCODE_LGUI, _look_dont_turn},
	
	/* tab for action */
	{SDL_SCANCODE_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDL_SCANCODE_M, _toggle_map},
	
	/* ` for using the microphone */
	{SDL_SCANCODE_GRAVE, _microphone_button}
};

#define NUMBER_OF_POWERBOOK_KEY_DEFINITIONS (sizeof(powerbook_key_definitions)/sizeof(struct key_definition))
static struct key_definition powerbook_key_definitions[]=
{
	/* olk; */
	{SDL_SCANCODE_O, _moving_forward},
	{SDL_SCANCODE_L, _moving_backward},
	{SDL_SCANCODE_K, _turning_left},
	{SDL_SCANCODE_SEMICOLON, _turning_right},
	
	/* zx translation */
	{SDL_SCANCODE_Z, _sidestepping_left},
	{SDL_SCANCODE_X, _sidestepping_right},

	/* as looking */
	{SDL_SCANCODE_A, _looking_left},
	{SDL_SCANCODE_S, _looking_right},

	/* dcv vertical looking */
	{SDL_SCANCODE_D, _looking_up},
	{SDL_SCANCODE_C, _looking_down},
	{SDL_SCANCODE_V, _looking_center},
	
	/* ip for weapon cycling */
	{SDL_SCANCODE_I, _cycle_weapons_backward},
	{SDL_SCANCODE_P, _cycle_weapons_forward},
	
	/* space for primary trigger, option for alternate trigger */
	{SDL_SCANCODE_SPACE, _left_trigger_state},
	{SDL_SCANCODE_LALT, _right_trigger_state},
	
	/* shift, control and command modifiers */
	{SDL_SCANCODE_LSHIFT, _sidestep_dont_turn},
	{SDL_SCANCODE_LCTRL, _run_dont_walk},
	{SDL_SCANCODE_LGUI, _look_dont_turn},
	
	/* tab for action */
	{SDL_SCANCODE_TAB, _action_trigger_state},

	/* m for toggle between normal and overhead map view */
	{SDL_SCANCODE_M, _toggle_map},
	
	/* ` for using the microphone */
	{SDL_SCANCODE_GRAVE, _microphone_button}
};

static struct key_definition *all_key_definitions[NUMBER_OF_KEY_SETUPS]=
{
	standard_key_definitions,
	left_handed_key_definitions,
	powerbook_key_definitions
};

#endif


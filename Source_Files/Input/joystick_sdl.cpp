/*
JOYSTICK_SDL.CPP

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

May 18, 2009 (Eric Peterson):
    Initial revision.

*/

#include <SDL.h>
#include <boost/ptr_container/ptr_map.hpp>

#include "player.h" // for mask_in_absolute_positioning_information
#include "preferences.h"
#include "joystick.h"
#include "Logging.h"
#include "FileHandler.h"

// internal handles
int joystick_active = true;
static boost::ptr_map<int, SDL_GameController*> active_instances;
int axis_values[SDL_CONTROLLER_AXIS_MAX] = {};
bool button_values[NUM_SDL_JOYSTICK_BUTTONS] = {};

void initialize_joystick(void) {
	// Look for "gamecontrollerdb.txt" in default search path
	FileSpecifier fs;
	if (fs.SetNameWithPath("gamecontrollerdb.txt")) {
		SDL_GameControllerAddMappingsFromFile(fs.GetPath());
	}
	
	SDL_GameControllerEventState(SDL_ENABLE);
	for (int i = 0; i < SDL_NumJoysticks(); ++i)
		joystick_added(i);
}

void enter_joystick(void) {
	joystick_active = input_preferences->use_joystick;
}

void exit_joystick(void) {
	joystick_active = false;
}

void joystick_added(int device_index) {
	if (!SDL_IsGameController(device_index)) {
		SDL_Joystick *joystick = SDL_JoystickOpen(device_index);
		char guidStr[255] = "";
		SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guidStr, 255);
		logWarning("No mapping found for controller \"%s\" (%s)",
				   SDL_JoystickName(joystick), guidStr);
		return;
	}
	SDL_GameController *controller = SDL_GameControllerOpen(device_index);
	if (!controller)
		return;
	int instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
	active_instances[instance_id] = controller;
}

void joystick_removed(int instance_id) {
	SDL_GameController *controller = active_instances[instance_id];
	if (controller) {
		SDL_GameControllerClose(controller);
		active_instances.erase(instance_id);
	}
}

void joystick_axis_moved(int instance_id, int axis, int value) {
	switch (axis) {
		case SDL_CONTROLLER_AXIS_LEFTY:
		case SDL_CONTROLLER_AXIS_RIGHTY:
			// flip Y axes to better match default movement
			axis_values[axis] = value * -1;
			break;
		default:
			axis_values[axis] = value;
			break;
	}
	button_values[AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE - AO_SCANCODE_BASE_JOYSTICK_BUTTON + axis] = (value >= 16384);
	button_values[AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE - AO_SCANCODE_BASE_JOYSTICK_BUTTON + axis] = (value <= -16384);
}
void joystick_button_pressed(int instance_id, int button, bool down) {
	if (button >= 0 && button < NUM_SDL_JOYSTICK_BUTTONS)
		button_values[button] = down;
}

void joystick_buttons_become_keypresses(Uint8* ioKeyMap) {
    // if we're not using the joystick, avoid this
    if (!joystick_active)
        return;

    // toggle joystick buttons until we run out of slots or buttons
	for (int i = 0; i < NUM_SDL_JOYSTICK_BUTTONS; ++i) {
		ioKeyMap[AO_SCANCODE_BASE_JOYSTICK_BUTTON + i] = button_values[i];
    }

    return;
}

int process_joystick_axes(int flags, int tick) {
    if (!joystick_active)
        return flags;

	int axis_data[NUMBER_OF_JOYSTICK_MAPPINGS] = { 0, 0, 0, 0 };
    for (int i = 0; i < NUMBER_OF_JOYSTICK_MAPPINGS; i++) {
		int axis = input_preferences->joystick_axis_mappings[i];
		if (axis < 0)
			continue;
		
		// apply dead zone
		if (ABS(axis_values[axis]) < input_preferences->joystick_axis_bounds[i]) {
			axis_data[i] = 0;
			continue;
		}
		float val = axis_values[axis]/32767.f * input_preferences->joystick_axis_sensitivities[i];
		
		// apply axis-specific sensitivity, to match keyboard
		// velocities with user sensitivity at 1
		switch (i) {
			case _joystick_yaw:
				val *= 6.f/63.f;
				break;
			case _joystick_pitch:
				val *= 6.f/15.f;
				break;
		}
		
		// pin to largest d for which both -d and +d can be
		// represented in 1 action flags bitset
		float limit = 0.5f - 1.f / (1<<FIXED_FRACTIONAL_BITS);
		switch (i) {
			case _joystick_yaw:
				limit = 0.5f - 1.f / (1<<ABSOLUTE_YAW_BITS);
				break;
			case _joystick_pitch:
				limit = 0.5f - 1.f / (1<<ABSOLUTE_PITCH_BITS);
				break;
			case _joystick_velocity:
				// forward and backward limits are independent and capped,
				// so there's no need to ensure symmetry
				limit = 0.5f;
				break;
			case _joystick_strafe:
			default:
				break;
		}
		axis_data[i] = PIN(val, -limit, limit) * FIXED_ONE;
    }
    // we have intelligently set up ways to allow variably throttled movement
    // for these controls
    flags = mask_in_absolute_positioning_information(flags,
													 axis_data[_joystick_yaw],
													 axis_data[_joystick_pitch],
													 axis_data[_joystick_velocity]);
    // but we don't for strafing!  so we do some PULSE MODULATION instead
#define PULSE_PATTERNS 5
#define PULSE_PERIOD 4
	int pulses[PULSE_PATTERNS][PULSE_PERIOD] = {
						 { 0, 0, 0, 0 },
					     { 0, 0, 0, 1 },
					     { 0, 1, 0, 1 },
					     { 0, 1, 1, 1 },
					     { 1, 1, 1, 1 } };
	int which_pulse = MIN(PULSE_PATTERNS - 1, ABS(axis_data[_joystick_strafe])*PULSE_PATTERNS/32768);
	if (pulses[which_pulse][tick % PULSE_PERIOD]) {
		if (axis_data[_joystick_strafe] > 0)
			flags |= _sidestepping_right;
		else
			flags |= _sidestepping_left;
	}

    // finally, return this tick's action flags augmented with movement data
    return flags;
}

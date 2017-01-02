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

// controls the gradation of the pulse modulated strafing
int strafe_bounds[3] = {14000, 20000, 28000};

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
		case SDL_CONTROLLER_AXIS_LEFTX:
		case SDL_CONTROLLER_AXIS_RIGHTX:
			axis_values[axis] = value;
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
		case SDL_CONTROLLER_AXIS_RIGHTY:
			// flip Y axes to better match default movement
			axis_values[axis] = value * -1;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			button_values[AO_CONTROLLER_BUTTON_LEFTTRIGGER] = (value > 0);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			button_values[AO_CONTROLLER_BUTTON_RIGHTTRIGGER] = (value > 0);
			break;
		default:
			break;
	}
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
    int yaw, pitch, vel, strafe;
    int* axis_assignments[4] = {&strafe, &vel, &yaw, &pitch};

    if (!joystick_active)
        return flags;

    // loop through the axes, getting their information
    for (int i = 0; i < 4; i++) {
        int *store_location = axis_assignments[i];
        if (!store_location)
            continue;

	int axis = input_preferences->joystick_axis_mappings[i];
	if (axis < 0)
	{
		*store_location = 0;
		continue;
	}

        // scale and store the joystick axis to the relevant movement controller
		*store_location = static_cast<int>(input_preferences->joystick_axis_sensitivities[i] * axis_values[i]);
        // clip if the value is too low
        if ((*store_location < input_preferences->joystick_axis_bounds[i]) && (*store_location > -input_preferences->joystick_axis_bounds[i]))
            *store_location = 0;
    }
    //printf("tick: %d\tstrafe: %d\tyaw: %d\tpitch: %d\tvel: %d\n", tick, strafe, yaw, pitch, vel);

    // we have intelligently set up ways to allow variably throttled movement
    // for these controls
    flags = mask_in_absolute_positioning_information(flags, yaw, pitch, vel);
    // but we don't for strafing!  so we do some PULSE MODULATION instead
    int abs_strafe = strafe > 0 ? strafe : -strafe;
    if (abs_strafe < input_preferences->joystick_axis_bounds[_joystick_strafe]) {
        // do nothin, you're not pushing hard enough
    } else if ((abs_strafe > 0) && (abs_strafe < strafe_bounds[0])) {
        // jitter slowly
        if (!(tick % 4)) {
            if (strafe > 0)
                flags |= _sidestepping_right;
            else
                flags |= _sidestepping_left;
        }
    } else if ((abs_strafe >= strafe_bounds[0]) && (abs_strafe < strafe_bounds[1])) {
        // jitter a little more quickly
        if (tick % 2) {
            if (strafe > 0)
                flags |= _sidestepping_right;
            else
                flags |= _sidestepping_left;
        }
    } else if ((abs_strafe >= strafe_bounds[1]) && (abs_strafe < strafe_bounds[2])) {
        // jitter damn quickly
        if (tick % 4) {
            if (strafe > 0)
                flags |= _sidestepping_right;
            else
                flags |= _sidestepping_left;
        }
    } else if (abs_strafe >= strafe_bounds[2]) {
        // honest movement
        if (strafe > 0)
            flags |= _sidestepping_right;
        else
            flags |= _sidestepping_left;
    }

    // finally, return this tick's action flags augmented with movement data
    return flags;
}

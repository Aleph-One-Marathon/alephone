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
	joystick_active = true;
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

enum {
	_flags_yaw,
	_flags_pitch,
	NUMBER_OF_ABSOLUTE_POSITION_VALUES
};

typedef struct AxisInfo {
	int key_binding_index;
	int abs_pos_index;
	bool negative;
	
} AxisInfo;

static const std::vector<AxisInfo> axis_mappings = {
	{ 2, _flags_yaw, true },
	{ 3, _flags_yaw, false },
	{ 8, _flags_pitch, true },
	{ 9, _flags_pitch, false }
};

static const float axis_limits[NUMBER_OF_ABSOLUTE_POSITION_VALUES] = {
	0.5f - 1.f / (1<<ABSOLUTE_YAW_BITS),
	0.5f - 1.f / (1<<ABSOLUTE_PITCH_BITS)
};

static int axis_mapped_to_action(int action, bool* negative) {
	auto codeset = input_preferences->key_bindings[action];
	for (auto it = codeset.begin(); it != codeset.end(); ++it) {
		const SDL_Scancode code = *it;
		
		if (code < AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE)
			continue;
		if (code > (AO_SCANCODE_BASE_JOYSTICK_BUTTON + NUM_SDL_JOYSTICK_BUTTONS))
			continue;
		
		*negative = false;
		int axis = code - AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE;
		if (code >= AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE) {
			*negative = true;
			axis = code - AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE;
		}
		return axis;
	}
	return -1;
}

void joystick_buttons_become_keypresses(Uint8* ioKeyMap) {
    // if we're not using the joystick, avoid this
    if (!joystick_active)
        return;
	if (active_instances.empty())
		return;

	std::set<int> buttons_to_avoid;
	if (input_preferences->controller_analog) {
		// avoid setting buttons mapped to analog aiming
		for (auto it = axis_mappings.begin(); it != axis_mappings.end(); ++it) {
			const AxisInfo info = *it;
			bool negative = false;
			int axis = axis_mapped_to_action(info.key_binding_index, &negative);
			if (axis >= 0) {
				buttons_to_avoid.insert(axis + (negative ? AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE : AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE));
			}
		}
	}

	for (int i = 0; i < NUM_SDL_JOYSTICK_BUTTONS; ++i) {
		int code = AO_SCANCODE_BASE_JOYSTICK_BUTTON + i;
		if (buttons_to_avoid.count(code) == 0)
			ioKeyMap[code] = button_values[i];
    }
	
    return;
}

int process_joystick_axes(int flags, int tick) {
    if (!joystick_active)
        return flags;
	if (active_instances.empty())
		return flags;
	if (!input_preferences->controller_analog)
		return flags;
	
	float angular_deltas[NUMBER_OF_ABSOLUTE_POSITION_VALUES] = { 0, 0 };
	for (auto it = axis_mappings.begin(); it != axis_mappings.end(); ++it) {
		const AxisInfo info = *it;
		bool negative = false;
		int axis = axis_mapped_to_action(info.key_binding_index, &negative);
		if (axis < 0)
			continue;
		
		int val = axis_values[axis] * (negative ? -1 : 1);
		if (val > input_preferences->controller_deadzone) {
			float norm = val/32767.f * (static_cast<float>(input_preferences->controller_sensitivity) / FIXED_ONE);
			const float angle_per_norm = 768/63.f;
			angular_deltas[info.abs_pos_index] += norm * (info.negative ? -1.0 : 1.0) * angle_per_norm;
		}
	}
	
	// return this tick's action flags augmented with movement data
	const fixed_angle dyaw = static_cast<fixed_angle>(angular_deltas[_flags_yaw] * FIXED_ONE);
	const fixed_angle dpitch = static_cast<fixed_angle>(angular_deltas[_flags_pitch] * FIXED_ONE);
	if (dyaw != 0 || dpitch != 0)
		flags = process_aim_input(flags, {dyaw, dpitch});
	return flags;
}

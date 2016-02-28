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
#include "player.h" // for mask_in_absolute_positioning_information
#include "preferences.h"
#include "joystick.h"

// internal handles
static SDL_Joystick *joystick = NULL;
int joystick_active = true;

// controls the gradation of the pulse modulated strafing
int strafe_bounds[3] = {14000, 20000, 28000};

void enter_joystick(void) {
    // de-filter joystick events from event polling
    SDL_JoystickEventState(SDL_ENABLE);
    // attempt to open the first joystick if we haven't already.  it doesn't
    // really matter if we fail, since then we just won't be receiving any
    // joystick events :)
    joystick = joystick ? joystick : SDL_JoystickOpen(input_preferences->joystick_id);

    //printf("Opened joystick at %p\n", joystick);

    // initialization was easy!
    return;
}

void exit_joystick(void) {
    // throw out the old joystick handle
    if (joystick)
        SDL_JoystickClose(joystick);
    joystick = NULL;

    // filter joystick events from event polling
    SDL_JoystickEventState(SDL_DISABLE);

    return;
}

void joystick_buttons_become_keypresses(Uint8* ioKeyMap) {
    // if we're not using the joystick, avoid this
    if (!joystick || !joystick_active)
        return;

    // toggle joystick buttons until we run out of slots or buttons
    for (int i = 0; i < std::max(SDL_JoystickNumButtons(joystick), NUM_SDL_JOYSTICK_BUTTONS); i++) {
        ioKeyMap[AO_SCANCODE_BASE_JOYSTICK_BUTTON + i] = SDL_JoystickGetButton(joystick, i);
    }

    return;
}

int process_joystick_axes(int flags, int tick) {
    int yaw, pitch, vel, strafe;
    int* axis_assignments[4] = {&strafe, &vel, &yaw, &pitch};

    if (!joystick || !joystick_active)
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
        *store_location = static_cast<int>(input_preferences->joystick_axis_sensitivities[i] * SDL_JoystickGetAxis(joystick, axis));
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

/*
JOYSTICK.H

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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "cstypes.h"

// this is where we start stuffing button presses into the big keymap array,
// comfortably past SDL2's defined scancodes
const int AO_SCANCODE_BASE_JOYSTICK_BUTTON = 415;
const int AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE = AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_MAX;
const int AO_SCANCODE_BASE_JOYSTICK_AXIS_NEGATIVE = AO_SCANCODE_BASE_JOYSTICK_AXIS_POSITIVE + SDL_CONTROLLER_AXIS_MAX;
const int NUM_SDL_JOYSTICK_BUTTONS = SDL_CONTROLLER_BUTTON_MAX + 2*SDL_CONTROLLER_AXIS_MAX;

const SDL_Scancode AO_SCANCODE_JOYSTICK_ESCAPE = static_cast<SDL_Scancode>(AO_SCANCODE_BASE_JOYSTICK_BUTTON + SDL_CONTROLLER_BUTTON_START);

void initialize_joystick(void);
void enter_joystick(void);
void exit_joystick(void);
void joystick_buttons_become_keypresses(Uint8* ioKeyMap);
int process_joystick_axes(int flags, int tick);
void joystick_axis_moved(int instance_id, int axis, int value);
void joystick_button_pressed(int instance_id, int button, bool down);
void joystick_added(int device_index);
void joystick_removed(int instance_id);

#endif // JOYSTICK_H

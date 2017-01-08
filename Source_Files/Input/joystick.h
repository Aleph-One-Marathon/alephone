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
// since 65 is where uppercase letters start + room for 8 mouse presses
#define AO_SCANCODE_BASE_JOYSTICK_BUTTON 415
// keys resume at key 91, so that's room for 18 joystick buttons
#define NUM_SDL_JOYSTICK_BUTTONS 18
// Buttons 0-14 match SDL's controller enums; we support left/right triggers
// as additional buttons instead of analog axis controls
typedef enum
{
	AO_CONTROLLER_BUTTON_LEFTTRIGGER = 15,
	AO_CONTROLLER_BUTTON_RIGHTTRIGGER
} AO_GameControllerExtraButton;
// joystick axes match the first four from SDL's controller API;
// triggers are treated as buttons instead
#define NUM_SDL_JOYSTICK_AXES 4

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

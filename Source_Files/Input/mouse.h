#ifndef __MOUSE_H
#define __MOUSE_H

/*
MOUSE.H

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

Tuesday, January 17, 1995 2:53:17 PM  (Jason')

    May 16, 2002 (Woody Zenfell):
        semi-hacky scheme in SDL to let mouse buttons simulate keypresses
*/

#include "world.h"

void enter_mouse(short type);
fixed_yaw_pitch pull_mouselook_delta();
void exit_mouse(short type);
void mouse_idle(short type);
void recenter_mouse(void);

// ZZZ: stuff of various hackiness levels to pretend mouse buttons are keys
void mouse_buttons_become_keypresses(Uint8* ioKeyMap);
void mouse_scroll(bool up);
void mouse_moved(int delta_x, int delta_y);

#define NUM_SDL_REAL_MOUSE_BUTTONS 5
#define NUM_SDL_MOUSE_BUTTONS 7   		  // two scroll-wheel buttons
#define AO_SCANCODE_BASE_MOUSE_BUTTON 400 // this is button 1's pseudo-keysym
#define AO_SCANCODE_MOUSESCROLL_UP 405    // stored as mouse button 6
#define AO_SCANCODE_MOUSESCROLL_DOWN 406  // stored as mouse button 7

#if defined(__ANDROID__)
// Flat-panel Quest stopgap: the only input is one pointer + the trigger (mouse button 1). The
// trigger is multiplexed by gesture instead of firing the weapon: hold = walk forward (drag to
// look), quick tap = action (use), quick tap in the top-left corner = map toggle. Real 6DoF
// controller input arrives with the OpenXR/TBXR layer in Phase 2.
void android_pointer_down(int x, int y);
void android_pointer_up(void);
void android_pointer_move(int x, int y);
bool android_input_walking(void);      // trigger held (past a short threshold) -> walk
bool android_input_take_fire(void);    // consume a pending fire tap (main area)
bool android_input_take_action(void);  // consume a pending action tap (top-right corner)
bool android_input_take_map(void);     // consume a pending map-toggle tap (top-left corner)
#endif

#endif

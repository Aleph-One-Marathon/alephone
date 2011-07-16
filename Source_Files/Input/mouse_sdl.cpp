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
 *  mouse_sdl.cpp - Mouse handling, SDL specific implementation
 *
 *  May 16, 2002 (Woody Zenfell):
 *      Configurable mouse sensitivity
 *      Semi-hacky scheme to let mouse buttons simulate keypresses
 */

#include "cseries.h"

#include "mouse.h"
#include "player.h"
#include "shell.h"
#include "preferences.h"


// Global variables
static bool mouse_active = false;
static uint8 button_mask = 0;		// Mask of enabled buttons
static int center_x, center_y;		// X/Y center of screen
static _fixed snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity;
static _fixed snapshot_delta_scrollwheel;


/*
 *  Initialize in-game mouse handling
 */

void enter_mouse(short type)
{
	if (type != _keyboard_or_game_pad) {
#ifndef DEBUG
		SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
		SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
		mouse_active = true;
		snapshot_delta_yaw = snapshot_delta_pitch = snapshot_delta_velocity = 0;
		snapshot_delta_scrollwheel = 0;
		button_mask = 0;	// Disable all buttons (so a shot won't be fired if we enter the game with a mouse button down from clicking a GUI widget)
		recenter_mouse();
	}
}


/*
 *  Shutdown in-game mouse handling
 */

void exit_mouse(short type)
{
	if (type != _keyboard_or_game_pad) {
#ifndef DEBUG
		SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
		SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
		mouse_active = false;
	}
}


/*
 *  Calculate new center mouse position when screen size has changed
 */

void recenter_mouse(void)
{
	if (mouse_active) {
		SDL_Surface *s = SDL_GetVideoSurface();
		center_x = s->w / 2;
		center_y = s->h / 2;
		SDL_WarpMouse(center_x, center_y);
	}
}


/*
 *  Take a snapshot of the current mouse state
 */

void mouse_idle(short type)
{
	if (mouse_active) {
		static uint32 last_tick_count = 0;
		uint32 tick_count = SDL_GetTicks();
		int32 ticks_elapsed = tick_count - last_tick_count;

		if (ticks_elapsed < 1)
			return;

		int x, y;
		SDL_GetMouseState(&x, &y);
		SDL_WarpMouse(center_x, center_y);

		// Calculate axis deltas
		// Bit-shifting is always done with positive numbers,
		// for consistent rounding regardless of direction
		int xdiff = x - center_x;		
		_fixed vx = (ABS(xdiff) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;
		int ydiff = y - center_y;
		_fixed vy = -(ABS(ydiff) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;
		
		// Mouse inversion
		if (TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse))
			vy *= -1;

		// LP: modified for doing each axis separately;
		// ZZZ: scale input by sensitivity
		if (input_preferences->sens_horizontal != FIXED_ONE)
			vx = _fixed((float(input_preferences->sens_horizontal)*vx)/float(FIXED_ONE));
		if (input_preferences->sens_vertical != FIXED_ONE)
			vy = _fixed((float(input_preferences->sens_vertical)*vy)/float(FIXED_ONE));
		
		if(input_preferences->mouse_acceleration) {
			/* pin and do nonlinearity */
			vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1, vx*= (vx<0) ? -vx : vx, vx>>= 14;
			vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1, vy*= (vy<0) ? -vy : vy, vy>>= 14;
		}
		else {
			/* pin and do NOT do nonlinearity */
			vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1;
			vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1;
		}

		// Bit-shifting is complete, restore direction
		if (xdiff < 0)
			vx = -vx;
		if (ydiff < 0)
			vy = -vy;
		
		// X axis = yaw
		snapshot_delta_yaw = vx;

		// Y axis = pitch or move, depending on mouse input type
		if (type == _mouse_yaw_pitch) {
			snapshot_delta_pitch = vy;
			snapshot_delta_velocity = 0;
		} else {
			snapshot_delta_pitch = 0;
			snapshot_delta_velocity = vy;
		}

		last_tick_count = tick_count;
	}
}


/*
 *  Return mouse state
 */

void test_mouse(short type, uint32 *flags, _fixed *delta_yaw, _fixed *delta_pitch, _fixed *delta_velocity)
{
	if (mouse_active) {
		*delta_yaw = snapshot_delta_yaw;
		*delta_pitch = snapshot_delta_pitch;
		*delta_velocity = snapshot_delta_velocity;

		snapshot_delta_yaw = snapshot_delta_pitch = snapshot_delta_velocity = 0;

		if (snapshot_delta_scrollwheel > 0) *flags |= _cycle_weapons_forward;
		else if (snapshot_delta_scrollwheel < 0) *flags |= _cycle_weapons_backward;
		snapshot_delta_scrollwheel = 0;
	} else {
	  delta_yaw = 0;
	  delta_pitch = 0;
	  delta_velocity = 0;
	}
}


void
mouse_buttons_become_keypresses(Uint8* ioKeyMap)
{
		uint8 buttons = SDL_GetMouseState(NULL, NULL);
		uint8 orig_buttons = buttons;
		buttons &= button_mask;				// Mask out disabled buttons

        for(int i = 0; i < NUM_SDL_MOUSE_BUTTONS; i++) {
            ioKeyMap[SDLK_BASE_MOUSE_BUTTON + i] =
                (buttons & SDL_BUTTON(i+1)) ? SDL_PRESSED : SDL_RELEASED;
        }

        button_mask |= ~orig_buttons;		// A button must be released at least once to become enabled
}

/*
 *  Hide/show mouse pointer
 */

void hide_cursor(void)
{
	SDL_ShowCursor(0);
}

void show_cursor(void)
{
	SDL_ShowCursor(1);
}


/*
 *  Get current mouse position
 */

void get_mouse_position(short *x, short *y)
{
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	*x = mx;
	*y = my;
}


/*
 *  Mouse button still down?
 */

bool mouse_still_down(void)
{
	SDL_PumpEvents();
	Uint8 buttons = SDL_GetMouseState(NULL, NULL);
	return buttons & SDL_BUTTON_LMASK;
}

void mouse_scroll(bool up)
{
	if (up)
		snapshot_delta_scrollwheel += 1;
	else
		snapshot_delta_scrollwheel -= 1;
}

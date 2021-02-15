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
#include <math.h>

#include "mouse.h"
#include "player.h"
#include "shell.h"
#include "preferences.h"
#include "screen.h"

// Global variables
static bool mouse_active = false;
static uint8 button_mask = 0;		// Mask of enabled buttons
static fixed_yaw_pitch mouselook_delta = {0, 0};
static _fixed snapshot_delta_scrollwheel;
static int snapshot_delta_x, snapshot_delta_y;


/*
 *  Initialize in-game mouse handling
 */

void enter_mouse(short type)
{
	if (type != _keyboard_or_game_pad) {
		SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, input_preferences->raw_mouse_input ? "0" : "1");
		SDL_SetRelativeMouseMode(SDL_TRUE);
		mouse_active = true;
		mouselook_delta = {0, 0};
		snapshot_delta_scrollwheel = 0;
		snapshot_delta_x = snapshot_delta_y = 0;
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
		SDL_SetRelativeMouseMode(SDL_FALSE);
		mouse_active = false;
	}
}


/*
 *  Calculate new center mouse position when screen size has changed
 */

void recenter_mouse(void)
{
	if (mouse_active) {
		MainScreenCenterMouse();
	}
}

static inline float MIX(float start, float end, float factor)
{
	return (start * (1.f - factor)) + (end * factor);
}

/*
 *  Take a snapshot of the current mouse state
 */

void mouse_idle(short type)
{
	if (mouse_active) {
		static uint32 last_tick_count = 0;
		uint32 tick_count = machine_tick_count();
		int32 ticks_elapsed = tick_count - last_tick_count;

		if (ticks_elapsed < 1)
			return;

		// Calculate axis deltas
		float dx = snapshot_delta_x;
		float dy = -snapshot_delta_y;
		snapshot_delta_x = 0;
		snapshot_delta_y = 0;
		
		// Mouse inversion
		if (TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse))
			dy = -dy;
		
		// Delta sensitivities
		const float angle_per_scaled_delta = 128/66.f; // assuming _mouse_accel_none
		float sx = angle_per_scaled_delta * (input_preferences->sens_horizontal / float{FIXED_ONE});
		float sy = angle_per_scaled_delta * (input_preferences->sens_vertical / float{FIXED_ONE}) * (input_preferences->classic_vertical_aim ? 0.25f : 1.f);
		switch (input_preferences->mouse_accel_type)
		{
			case _mouse_accel_classic:
				sx *= MIX(1.f, (1/32.f) * fabs(dx * sx), input_preferences->mouse_accel_scale);
				sy *= MIX(1.f, (1/(input_preferences->classic_vertical_aim ? 8.f : 32.f)) * fabs(dy * sy), input_preferences->mouse_accel_scale);
				break;
			case _mouse_accel_none:
			default:
				break;
		}
		
		// Angular deltas
		const fixed_angle dyaw = static_cast<fixed_angle>(sx * dx * FIXED_ONE);
		const fixed_angle dpitch = static_cast<fixed_angle>(sy * dy * FIXED_ONE);
		
		// Push mouselook delta
		mouselook_delta = {dyaw, dpitch};
	}
}

fixed_yaw_pitch pull_mouselook_delta()
{
	auto delta = mouselook_delta;
	mouselook_delta = {0, 0};
	return delta;
}


void
mouse_buttons_become_keypresses(Uint8* ioKeyMap)
{
		uint8 buttons = SDL_GetMouseState(NULL, NULL);
		uint8 orig_buttons = buttons;
		buttons &= button_mask;				// Mask out disabled buttons

        for(int i = 0; i < NUM_SDL_REAL_MOUSE_BUTTONS; i++) {
            ioKeyMap[AO_SCANCODE_BASE_MOUSE_BUTTON + i] =
                (buttons & SDL_BUTTON(i+1)) ? SDL_PRESSED : SDL_RELEASED;
        }
		ioKeyMap[AO_SCANCODE_MOUSESCROLL_UP] = (snapshot_delta_scrollwheel > 0) ? SDL_PRESSED : SDL_RELEASED;
		ioKeyMap[AO_SCANCODE_MOUSESCROLL_DOWN] = (snapshot_delta_scrollwheel < 0) ? SDL_PRESSED : SDL_RELEASED;
		snapshot_delta_scrollwheel = 0;

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


void mouse_scroll(bool up)
{
	if (up)
		snapshot_delta_scrollwheel += 1;
	else
		snapshot_delta_scrollwheel -= 1;
}

void mouse_moved(int delta_x, int delta_y)
{
	snapshot_delta_x += delta_x;
	snapshot_delta_y += delta_y;
}

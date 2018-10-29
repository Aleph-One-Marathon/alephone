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

#ifdef __APPLE__
#include "mouse_cocoa.h"
#endif


// Global variables
static bool mouse_active = false;
static uint8 button_mask = 0;		// Mask of enabled buttons
static _fixed snapshot_delta_yaw, snapshot_delta_pitch;
static _fixed snapshot_delta_scrollwheel;
static int snapshot_delta_x, snapshot_delta_y;

static float lost_x, lost_y; //Stores the unrealized mouselook precision.
static float lost_x_at_last_sample, lost_y_at_last_sample; //Stores the unrealized mouse presion values as of sample time, which can be used by the renderer. Capturing this value eliminates jitter in multiplayer.
static bool smooth_mouselook;


/*
 *  Initialize in-game mouse handling
 */

void enter_mouse(short type)
{
	if (type != _keyboard_or_game_pad) {
#ifdef __APPLE__
		if (input_preferences->raw_mouse_input)
			OSX_Mouse_Init();
#endif
		SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, input_preferences->raw_mouse_input ? "0" : "1");
		SDL_SetRelativeMouseMode(SDL_TRUE);
		mouse_active = true;
		snapshot_delta_yaw = snapshot_delta_pitch = 0;
		snapshot_delta_scrollwheel = 0;
		snapshot_delta_x = snapshot_delta_y = 0;
        lost_x = lost_y = 0;
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
#ifdef __APPLE__
		OSX_Mouse_Shutdown();
#endif
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
#ifdef __APPLE__
		// In raw mode, get unaccelerated deltas from HID system
		if (input_preferences->raw_mouse_input)
			OSX_Mouse_GetMouseMovement(&snapshot_delta_x, &snapshot_delta_y);
#endif
		
		// Calculate axis deltas
		float dx = snapshot_delta_x;
		float dy = -snapshot_delta_y;
		snapshot_delta_x = 0;
		snapshot_delta_y = 0;
		
		// Mouse inversion
		if (TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse))
			dy = -dy;
		
		// scale input by sensitivity
		const float sensitivityScale = 1.f / (66.f * FIXED_ONE);
		float sx = sensitivityScale * input_preferences->sens_horizontal;
		float sy = sensitivityScale * input_preferences->sens_vertical;
		switch (input_preferences->mouse_accel_type)
		{
			case _mouse_accel_classic:
				sx *= MIX(1.f, fabs(dx * sx) * 4.f, input_preferences->mouse_accel_scale);
				sy *= MIX(1.f, fabs(dy * sy) * 4.f, input_preferences->mouse_accel_scale);
				break;
			case _mouse_accel_none:
			default:
				break;
		}
		dx *= sx;
		dy *= sy;
        
        //Add post-sensitivity lost precision, in case we can use it this time around.
        dx += lost_x;
        dy += lost_y;

		
		// 1 dx unit = 1 * 2^ABSOLUTE_YAW_BITS * (360 deg / 2^ANGULAR_BITS)
		//           = 90 deg
		//
		// 1 dy unit = 1 * 2^ABSOLUTE_PITCH_BITS * (360 deg / 2^ANGULAR_BITS)
		//           = 22.5 deg
		
		// Largest dx for which both -dx and +dx can be represented in 1 action flags bitset
		float dxLimit = 0.5f - 1.f / (1<<ABSOLUTE_YAW_BITS);  // 0.4921875 dx units (~44.30 deg)
		
		// Largest dy for which both -dy and +dy can be represented in 1 action flags bitset
		float dyLimit = 0.5f - 1.f / (1<<ABSOLUTE_PITCH_BITS);  // 0.46875 dy units (~10.55 deg)
		
		dxLimit = MIN(dxLimit, input_preferences->mouse_max_speed);
		dyLimit = MIN(dyLimit, input_preferences->mouse_max_speed);
		
		dx = PIN(dx, -dxLimit, dxLimit);
		dy = PIN(dy, -dyLimit, dyLimit);
		
		snapshot_delta_yaw   = static_cast<_fixed>(dx * FIXED_ONE);
		snapshot_delta_pitch = static_cast<_fixed>(dy * FIXED_ONE);
        
        //Shift off the bits which cannot yet be visualized, so we can add in that lost precision on the next call.
        snapshot_delta_yaw >>= (FIXED_FRACTIONAL_BITS-ABSOLUTE_YAW_BITS);
        snapshot_delta_yaw <<= (FIXED_FRACTIONAL_BITS-ABSOLUTE_YAW_BITS);
        snapshot_delta_pitch >>= (FIXED_FRACTIONAL_BITS-ABSOLUTE_PITCH_BITS);
        snapshot_delta_pitch <<= (FIXED_FRACTIONAL_BITS-ABSOLUTE_PITCH_BITS);
        
        //Track how much precision is ignored, so we can stuff it back into the input next time this function is called.
        lost_x = (dx * (float)FIXED_ONE) - (float)snapshot_delta_yaw;
        lost_y = (dy * (float)FIXED_ONE) - (float)snapshot_delta_pitch;
        lost_x /= (float)FIXED_ONE;
        lost_y /= (float)FIXED_ONE;
        
        smooth_mouselook = TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_SmoothLook) && player_controlling_game() && !PLAYER_IS_DEAD(local_player);
	}
}

//Returns the currently not-represented mouse precision as a fraction of a yaw and pitch frogblasts.
float lostMousePrecisionX() { return shouldSmoothMouselook()?(lost_x_at_last_sample*(float)FIXED_ONE)/512.0:0.0; }
float lostMousePrecisionY() { return shouldSmoothMouselook()?(lost_y_at_last_sample*(float)FIXED_ONE)/2048.0:0.0; }

bool shouldSmoothMouselook(){
    return smooth_mouselook;
}

/*
 *  Return mouse state
 */

void test_mouse(short type, uint32 *flags, _fixed *delta_yaw, _fixed *delta_pitch, _fixed *delta_velocity)
{
	if (mouse_active) {
		*delta_yaw = snapshot_delta_yaw;
		*delta_pitch = snapshot_delta_pitch;
		*delta_velocity = 0;  // Mouse-driven player velocity is unimplemented

        lost_x_at_last_sample = lost_x;
        lost_y_at_last_sample = lost_y;
		snapshot_delta_yaw = snapshot_delta_pitch = 0;
	} else {
		*delta_yaw = 0;
		*delta_pitch = 0;
		*delta_velocity = 0;
	}
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

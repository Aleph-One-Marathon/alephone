/*
 *  mouse_sdl.cpp - Mouse handling, SDL specific implementation
 */

#include "cseries.h"

#include "mouse.h"
#include "player.h"
#include "shell.h"


// Global variables
static int center_x, center_y;		// X/Y center of screen


/*
 *  Initialize in-game mouse handling
 */

void enter_mouse(short type)
{
	if (type != _keyboard_or_game_pad) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_Surface *s = SDL_GetVideoSurface();
		center_x = s->w / 2;
		center_y = s->h / 2;
		SDL_WarpMouse(center_x, center_y);
	}
}


/*
 *  Shutdown in-game mouse handling
 */

void exit_mouse(short type)
{
	if (type != _keyboard_or_game_pad)
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}


/*
 *  Return mouse state
 */

void test_mouse(short type, long *flags, fixed *delta_yaw, fixed *delta_pitch, fixed *delta_velocity)
{
	if (type != _keyboard_or_game_pad) {
		static uint32 last_tick_count = 0;
		uint32 tick_count = SDL_GetTicks();
		int32 ticks_elapsed = tick_count - last_tick_count;

		int x, y;
		uint8 buttons = SDL_GetMouseState(&x, &y);
		SDL_WarpMouse(center_x, center_y);

		if (buttons & SDL_BUTTON_LMASK)		// Left button: primary weapon trigger
			*flags |= _left_trigger_state;
		if (buttons & SDL_BUTTON_RMASK)		// Right button: secondary weapon trigger
			*flags |= _right_trigger_state;

		fixed vx = 0, vy = 0;

		if (ticks_elapsed) {

			// Calculate axis deltas
			vx = ((x - center_x) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;
			vy = -((y - center_y) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;

			// Pin and do nonlinearity
			vx = PIN(vx, -FIXED_ONE/2, FIXED_ONE/2); vx >>= 1; vx *= (vx<0) ? -vx : vx; vx >>= 14;
			vy = PIN(vy, -FIXED_ONE/2, FIXED_ONE/2); vy >>= 1; vy *= (vy<0) ? -vy : vy; vy >>= 13;
		}

		// X axis = yaw
		*delta_yaw = vx;

		// Y axis = pitch or move, depending on mouse input type
		if (type == _mouse_yaw_pitch) {
			*delta_pitch = vy;
			*delta_velocity = 0;
		} else {
			*delta_pitch = 0;
			*delta_velocity = vy;
		}

		last_tick_count = tick_count;
	}
}

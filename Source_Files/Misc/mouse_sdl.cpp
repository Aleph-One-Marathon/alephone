/*
 *  mouse_sdl.cpp - Mouse handling, SDL specific implementation
 */

#include "cseries.h"

#include "mouse.h"
#include "player.h"
#include "shell.h"


// Global variables
static bool mouse_active = false;
static int center_x, center_y;		// X/Y center of screen
static fixed snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity;


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
		SDL_Surface *s = SDL_GetVideoSurface();
		center_x = s->w / 2;
		center_y = s->h / 2;
		SDL_WarpMouse(center_x, center_y);

		snapshot_delta_yaw = snapshot_delta_pitch = snapshot_delta_velocity = 0;

		mouse_active = true;
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
		fixed vx = ((x - center_x) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;
		fixed vy = -((y - center_y) << FIXED_FRACTIONAL_BITS) / ticks_elapsed;

		// Pin and do nonlinearity
		vx = PIN(vx, -FIXED_ONE/2, FIXED_ONE/2); vx >>= 1; vx *= (vx<0) ? -vx : vx; vx >>= 14;
		vy = PIN(vy, -FIXED_ONE/2, FIXED_ONE/2); vy >>= 1; vy *= (vy<0) ? -vy : vy; vy >>= 13;

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

void test_mouse(short type, long *flags, fixed *delta_yaw, fixed *delta_pitch, fixed *delta_velocity)
{
	if (mouse_active) {
		uint8 buttons = SDL_GetMouseState(NULL, NULL);
		if (buttons & SDL_BUTTON_LMASK)		// Left button: primary weapon trigger
			*flags |= _left_trigger_state;
		if (buttons & SDL_BUTTON_RMASK)		// Right button: secondary weapon trigger
			*flags |= _right_trigger_state;

		*delta_yaw = snapshot_delta_yaw;
		*delta_pitch = snapshot_delta_pitch;
		*delta_velocity = snapshot_delta_velocity;
	}
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

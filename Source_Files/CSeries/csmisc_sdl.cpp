/*
 *  csmisc_sdl.cpp - Miscellaneous routines, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"


/*
 *  Return tick counter
 */

uint32 machine_tick_count(void)
{
	return SDL_GetTicks();
}


/*
 *  Wait for mouse click or keypress
 */

boolean wait_for_click_or_keypress(uint32 ticks)
{
	uint32 start = SDL_GetTicks();
	SDL_Event event;
	while (SDL_GetTicks() - start < ticks) {
		SDL_PollEvent(&event);
		if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN)
			return true;
		SDL_Delay(10);
	}
	return false;
}


/*
 *  Disable screen saver
 */

void kill_screen_saver(void)
{
	// nothing to do
}

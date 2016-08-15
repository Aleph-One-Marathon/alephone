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
 *  csmisc_sdl.cpp - Miscellaneous routines, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include <chrono>
#include <thread>
#include "Logging.h"

/* by using static variable initialization time as the epoch, we ensure that
   Aleph One can run for ~49 days without timing issues cropping up */
/* TODO: Every single place machine tick counts are used, switch to uint64 */
static auto epoch = std::chrono::high_resolution_clock::now();

/* a knob to play the game in "slow motion" to debug timing sensitive features.
   this is not a preferences option because of the cheating potential, and
   because of the awesome breakage that will occur at very large values */
static constexpr int TIME_SKEW = 1;

/*
 *  Return tick counter
 */

uint32 machine_tick_count(void)
{
  auto now = std::chrono::high_resolution_clock::now();
  if(now < epoch) {
    logWarning("Time went backwards!");
    epoch = now;
  }
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (now - epoch).count()/TIME_SKEW;
}

/*
 *  Delay a certain number of ticks
 */

void sleep_for_machine_ticks(uint32 ticks)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ticks*TIME_SKEW));
}

/*
 *  Delay until a certain tick count
 */

void sleep_until_machine_tick_count(uint32 ticks)
{
	std::this_thread::sleep_until(std::chrono::high_resolution_clock::time_point(std::chrono::milliseconds(ticks*TIME_SKEW)));
}

/*
 *  Give up a small amount of processor time
 */
void yield(void)
{
	std::this_thread::yield();
}

/*
 *  Wait for mouse click or keypress
 */

bool wait_for_click_or_keypress(uint32 ticks)
{
	uint32 start = machine_tick_count();
	SDL_Event event;
	while (machine_tick_count() - start < ticks) {
		SDL_WaitEventTimeout(&event, ticks);
		switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_KEYDOWN:
			case SDL_CONTROLLERBUTTONDOWN:
				return true;
		}
	}
	return false;
}

/*
 *  network_microphone_sdl.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2002 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  The code in this file is licensed to you under the GNU GPL.  As the copyright holder,
 *  however, I reserve the right to use this code as I see fit, without being bound by the
 *  GPL's terms.  This exemption is not intended to apply to modified versions of this file -
 *  if I were to use a modified version, I would be a licensee of whomever modified it, and
 *  thus must observe the GPL terms.
 *
 *  Interface to various platform-specific network microphone implementations.
 *
 *  See network_microphone_sdl_dummy.cpp if you need something to link against in lieu
 *  of an actual implementation.  See also network_speaker_sdl.* for the playback routines.
 *
 *  Created by woody March 3-8, 2002.
 */

#ifndef	NETWORK_MICROPHONE_SDL_H
#define	NETWORK_MICROPHONE_SDL_H

// "true" does not guarantee that the user has a microphone, or even that sound capture will work...
// but "false" means you have no hope whatsoever.  :)
bool    is_network_microphone_implemented();

// Setup - don't call twice without intervening close...()
void    open_network_microphone();

// Activate/deactivate a network microphone that's been open()ed
void    set_network_microphone_state(bool inActive);

// Call this from time to time to let audio get processed
void    network_microphone_idle_proc();

// Cleanup - multiple calls should be safe.
void    close_network_microphone();

#endif	// NETWORK_MICROPHONE_SDL_H

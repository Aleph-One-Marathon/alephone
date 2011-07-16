#ifndef __NETWORK_SOUND_H
#define __NETWORK_SOUND_H

/*
NETWORK_SOUND.H

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

Sunday, August 14, 1994 3:36:17 AM- go nuts

Feb 1, 2003 (Woody Zenfell):
        Merged SDL-style and Mac-style network audio interfaces.  Both use this now.
        This is the main interface for external code wanting to use the network audio support.
*/

#include "cseries.h"

/* ---------- constants */

/* ---------- prototypes: NETWORK_SPEAKER.C */

// Called by main thread to initialize network speaker system
OSErr open_network_speaker();

// Called by main thread between game updates
void network_speaker_idle_proc();

// Called by main thread to shut down network speaker system
void close_network_speaker();

// Called by received_network_audio_proc, but also available to others
void queue_network_speaker_data(byte* inData, short inLength);

// Called by network routines to store incoming network audio for playback
void received_network_audio_proc(void *buffer, short buffer_size, short player_index);

void quiet_network_speaker(void);

void mute_player_mic(short player_index);
void clear_player_mic_mutes();

/* ---------- prototypes: NETWORK_MICROPHONE.C */

// "true" does not guarantee that the user has a microphone, or even that sound capture will work...
// but "false" means you have no hope whatsoever.  :)
bool    is_network_microphone_implemented();

// This may answer the question a bit more accurately.
bool	has_sound_input_capability(void);

// Setup - don't call twice without intervening close...()
OSErr   open_network_microphone();

// Activate/deactivate a network microphone that's been open()ed
void    set_network_microphone_state(bool inActive);

// Call this from time to time to let audio get processed
void    network_microphone_idle_proc();

// Cleanup - multiple calls should be safe.
void    close_network_microphone();

#endif

#ifndef __NETWORK_SOUND_H
#define __NETWORK_SOUND_H

/*
NETWORK_SOUND.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Sunday, August 14, 1994 3:36:17 AM- go nuts
*/

/* ---------- constants */
#define NETWORK_SOUND_CHUNK_BUFFER_SIZE 512

/* ---------- prototypes: NETWORK_SPEAKER.C */

OSErr open_network_speaker(short block_size, short connection_threshold);
void close_network_speaker(void);
void quiet_network_speaker(void);

void queue_network_speaker_data(byte *buffer, short count);
void network_speaker_idle_proc(void);

/* ---------- prototypes: NETWORK_MICROPHONE.C */

OSErr open_network_microphone(short network_distribution_type);
void close_network_microphone(void);

bool has_sound_input_capability(void);

/* This function is defined in interface.h */
// void handle_microphone_key(bool triggered);

#endif

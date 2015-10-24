/*
 *  network_speaker_shared.cpp
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2002 and beyond by Woody Zenfell, III
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

 *  The code in this file is licensed to you under the GNU GPL.  As the copyright holder,
 *  however, I reserve the right to use this code as I see fit, without being bound by the
 *  GPL's terms.  This exemption is not intended to apply to modified versions of this file -
 *  if I were to use a modified version, I would be a licensee of whomever modified it, and
 *  thus must observe the GPL terms.
 *
 *  Network speaker-related code usable by multiple platforms.
 *
 *  Created by woody Feb 1, 2003, largely from stuff in network_speaker_sdl.cpp.
 *
 *  May 28, 2003 (Gregory Smith):
 *	Speex audio decompression 
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"
#include "network_sound.h"
#include "network_data_formats.h"
#include "network_audio_shared.h"
#include "player.h"
#include "shell.h" // screen_print

#ifdef SPEEX
#include "speex/speex.h"
#include "network_speex.h"
#endif

#include <set>

static std::set<short> sIgnoredPlayers;

// This is what the network distribution system calls when audio is received.
void
received_network_audio_proc(void *buffer, short buffer_size, short player_index) {

	if (sIgnoredPlayers.find(player_index) != sIgnoredPlayers.end()) return;

	network_audio_header_NET* theHeader_NET = (network_audio_header_NET*) buffer;

	network_audio_header    theHeader;

	netcpy(&theHeader, theHeader_NET);

#ifdef SPEEX
	byte* theSoundData = ((byte*)buffer) + sizeof(network_audio_header_NET);
#endif

	// 0 if using uncompressed audio, 1 if using speex
	if(!(theHeader.mFlags & kNetworkAudioForTeammatesOnlyFlag) || (local_player->team == get_player_data(player_index)->team)) 
	{
#ifdef SPEEX
		if (theHeader.mReserved == 1) 
		{
			
			// decode the data
			const int max_frames = 2048 / 160;
			static int16 frames[max_frames][160];
			int nbytes;

			int numFrames = 0;
			while (theSoundData < static_cast<byte*>(buffer) + buffer_size && 
			       numFrames < max_frames)
			{
				// decode a frame
				nbytes = *theSoundData++;

				speex_bits_read_from(&gDecoderBits, (char *) theSoundData, nbytes);
				speex_decode_int(gDecoderState, &gDecoderBits, frames[numFrames]);

				numFrames++;
				theSoundData += nbytes;
			}

			queue_network_speaker_data((byte *) frames[0], 160 * 2 * numFrames);
		}
#endif
	}
}

void mute_player_mic(short player_index)
{
	if (sIgnoredPlayers.find(player_index) != sIgnoredPlayers.end())
	{
		screen_printf("removing player %i's mic from the ignore list", player_index);
		sIgnoredPlayers.erase(player_index);
	}
	else
	{
		screen_printf("adding player %i's mic to the ignore list", player_index);
		sIgnoredPlayers.insert(player_index);
	}
}

void clear_player_mic_mutes()
{
	sIgnoredPlayers.clear();
}

#endif // !defined(DISABLE_NETWORKING)

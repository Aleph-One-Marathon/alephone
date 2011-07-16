/*
 *  network_speaker_sdl.h
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
 *  Realtime audio (network microphone) playback support for SDL platforms.
 *
 *  Created by woody Mar 3-8, 2002.
 *
 *  Feb 1, 2003 (Woody Zenfell):
 *     This file now describes only the interface between the SDL network speaker receiving
 *     code and the SDL sound code.  The main interface to the actual speaker stuff will be
 *     in network_audio.h.
 */

#ifndef NETWORK_SPEAKER_SDL_H
#define NETWORK_SPEAKER_SDL_H

#include    "cseries.h"

// Flags for NetworkSpeakerSoundBuffer::mFlags below
enum {
    kSoundDataIsDisposable  = 0x01  // dequeuer is expected to call release_network_speaker_buffer(mData)
};

// These are used to link the network_speaker routines to the sound_sdl routines.
struct NetworkSpeakerSoundBufferDescriptor {
    byte*   mData;
    uint32  mLength;
    uint32  mFlags;
};

// To insulate callers from details of flag storage
__inline__ bool
is_sound_data_disposable(NetworkSpeakerSoundBufferDescriptor* inBuffer) {
    return (inBuffer->mFlags & kSoundDataIsDisposable) ? true : false;
}


// Called by sound playback routines to get incoming network audio
// (also called by main thread in close_network_speaker())
// Calling this invalidates the pointer returned the previous call.
NetworkSpeakerSoundBufferDescriptor* dequeue_network_speaker_data();

// Called by sound playback routines to return storage-buffers to the freequeue
void release_network_speaker_buffer(byte* inBuffer);

#endif // NETWORK_SPEAKER_SDL_H

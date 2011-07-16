/*
 *  network_audio_shared.h
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
 *  Stuff shared internally between the network microphone and network speaker code.
 *
 *  Created by woody Feb 1, 2003, largely from stuff in network_speaker_sdl.h.
 */

#ifndef NETWORK_AUDIO_SHARED_H
#define NETWORK_AUDIO_SHARED_H

#include    "cseries.h"

// In-memory header for sound data
struct network_audio_header {
    uint32  mReserved;  // Should be 0 for now, later maybe use a FourCharCode for format?  shrug
    uint32  mFlags;
};

// For network_audio_header::mFlags
enum {
    kNetworkAudioForTeammatesOnlyFlag = 0x01
};


// Useful information about the network audio
// Note: at present, the microphone routines ignore these settings and target
// 11025 unsigned 8-bit mono.  If you want to change formats you'll need to edit those
// routines too (hopefully to make them more general ;) ).  Also you'll want to somehow
// differentiate your format from this one (use a Flag, or value in Reserved, or an
// entirely new distribution type, or something).
const bool  kNetworkAudioIsStereo       = false;
const bool  kNetworkAudioIs16Bit        = true;
const bool  kNetworkAudioIsSigned8Bit   = false;
const int   kNetworkAudioSampleRate     = 8000;
const int   kNetworkAudioBytesPerFrame  = (kNetworkAudioIs16Bit ? 2 : 1) * (kNetworkAudioIsStereo ? 2 : 1);

#endif // NETWORK_AUDIO_SHARED_H

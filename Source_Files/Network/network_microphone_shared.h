/*
 *  network_microphone_shared.h
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
 *  The stuff described here should be of interest only to various network-microphone implementors.
 *  Code outside the netmic implementation files should not need to #include this.
 *
 *  Feb. 1, 2003 (Woody Zenfell): created.
 */

#ifndef NETWORK_MICROPHONE_SHARED_H
#define NETWORK_MICROPHONE_SHARED_H

#include "cstypes.h"

// Netmic code should call this once the format is known (and any time the capture format changes).
// copy_and_send_() uses the values specified by the most recent call to this routine.
// It is an error to call copy_and_send() without calling this first.
// Returns whether the capture format is usable.
bool announce_microphone_capture_format(uint32 inSamplesPerSecond, bool inStereo, bool in16Bit);

// Netmic code should call this routine with one or two chunks of available captured data.
// This routine will not actually consume the data unless inForceSend is set or there is enough
// data available to merit sending a packet.
// Returns the number of bytes consumed.
int32 copy_and_send_audio_data(uint8* inFirstChunkReadPosition, int32 inFirstChunkBytesRemaining,
                         uint8* inSecondChunkReadPosition, int32 inSecondChunkBytesRemaining,
                         bool inForceSend);

// Netmic code can compare the amount of data it has against the return from this to decide whether
// it's worth doing any preprocessing that might be necessary before calling copy_and_send().
// Value is only valid for the most recently-specified capture format (with announce_...())
// Calling this without specifying a capture format is an error.
int32 get_capture_byte_count_per_packet();

#endif // NETWORK_MICROPHONE_SHARED_H

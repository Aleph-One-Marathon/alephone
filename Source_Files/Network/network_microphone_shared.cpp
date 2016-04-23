/*
 *  network_microphone_shared.cpp
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
 *  Utility/support routines for various platforms' network microphone implementations.
 *
 *  Created by woody Jan 31, 2003, largely from code originally in network_microphone_sdl_win32.cpp.
 *
 *  May 28, 2003 (Gregory Smith):
 *	Speex audio compression
 */

#if !defined(DISABLE_NETWORKING)

#include    "cseries.h"
#include    "network_speaker_sdl.h"
#include    "network_data_formats.h"
#include    "network_distribution_types.h"

#include	<algorithm>	// for STL pair<> type

#ifdef SPEEX
#include "speex/speex.h"
#include "preferences.h"
#include "network_speex.h"
#endif

#include "map.h" // _force_unique_teams!

using namespace std;

#ifdef DEBUG
// For testing: don't send audio on the network - pass it directly to network_speaker.
//#define MICROPHONE_LOCAL_LOOPBACK
#endif

enum {
	kNetworkAudioSamplesPerPacket = 800,
};

static uint32 sSamplesPerSecond = 0;
static uint32 sStereo = false;
static uint32 s16Bit = false;
static int32 sCaptureBytesPerPacket = 0;
static uint32 sNumberOfBytesPerSample = 0;

static _fixed rate = 0;
static _fixed counter = 0;

bool announce_microphone_capture_format(uint32 inSamplesPerSecond, bool inStereo, bool in16Bit) 
{
	sSamplesPerSecond = inSamplesPerSecond;
	sStereo = inStereo;
	s16Bit = in16Bit;
	
	rate = (_fixed) (FIXED_ONE * (float) inSamplesPerSecond / (float) kNetworkAudioSampleRate);
	counter = 0;

	sNumberOfBytesPerSample = (inStereo ? 2 : 1) * (in16Bit ? 2 : 1);
	sCaptureBytesPerPacket = (uint32) ((rate * kNetworkAudioSamplesPerPacket) >> 16) * sNumberOfBytesPerSample;

	return true;
}


int32
get_capture_byte_count_per_packet() {
    // Catch folks who call us without specifying a rate first
    assert(sSamplesPerSecond > 0);
    
    return sCaptureBytesPerPacket;
}

template<bool stereo, bool sixteenBit>
inline int16 getSample(void *data)
{
	if (sixteenBit)
	{
		if (stereo)
		{
			return (((int16 *) data)[0] >> 1) + (((int16 *)data)[1] >> 1);
		}
		else
		{
			return ((int16 *) data)[0];
		}
	}
	else
	{
		if (stereo)
		{
			return ((((uint8 *) data)[0] / 2 + ((uint8 *) data)[1] / 2) - 128) << 8;
		}
		else
		{
			return (((uint8 *) data)[0] - 128) << 8;
		}
	}
}

#ifdef SPEEX
template <bool stereo, bool sixteenBit>
int32 copy_and_speex_encode_template(uint8* outStorage, void* inStorage, int32 inCount)
{
	static int16 frame[160];
	static int storedSamples = 0;
	int bytesWritten = 0;

	while (inCount > 0)
	{
		int16 left_sample = getSample<stereo, sixteenBit>(inStorage);

		if (inCount > sNumberOfBytesPerSample)
		{
			uint8* data = (uint8 *) inStorage + sNumberOfBytesPerSample;
			int16 right_sample = getSample<stereo, sixteenBit>(data);

			int32 sample = left_sample + (((right_sample - left_sample) * (counter & 0xffff)) >> 16);
			frame[storedSamples++] = (int16) sample;
		}
		else
		{
			frame[storedSamples++] = left_sample;
		}
			
		// advance data
		counter += rate;
		if (counter >= 0x10000) 
		{
			int count = counter >> 16;
			counter &= 0xffff;
			inStorage = (uint8 *) inStorage + sNumberOfBytesPerSample * count;
			inCount -= sNumberOfBytesPerSample * count;
		}

		if (storedSamples >= 160)
		{
			// encode the frame
			speex_bits_reset(&gEncoderBits);
			speex_encode_int(gEncoderState, frame, &gEncoderBits);
			uint8 nbytes = speex_bits_write(&gEncoderBits, reinterpret_cast<char *>(outStorage) + 1, 200);
			bytesWritten += nbytes + 1;
			// first put the size of this frame in storage
			*(outStorage) = nbytes;
			outStorage += nbytes + 1;

			storedSamples = 0;
		}
		
	}

	return bytesWritten;
}

int32 copy_and_speex_encode(uint8* outStorage, void *inStorage, int32 inCount)
{
	if (sStereo) 
	{
		if (s16Bit)
		{
			return copy_and_speex_encode_template<true, true>(outStorage, inStorage, inCount);
		}
		else
		{
			return copy_and_speex_encode_template<true, false>(outStorage, inStorage, inCount);
		}
	}
	else
	{
		if (s16Bit)
		{
			return copy_and_speex_encode_template<false, true>(outStorage, inStorage, inCount);
		}
		else
		{
			return copy_and_speex_encode_template<false, false>(outStorage, inStorage, inCount);
		}
	}
}

static void
send_audio_data(void* inData, short inSize) {
#ifdef MICROPHONE_LOCAL_LOOPBACK
#include "network_sound.h"
    received_network_audio_proc(inData, inSize, 0);
#else
    NetDistributeInformation(kNewNetworkAudioDistributionTypeID, inData, inSize, false, !(GET_GAME_OPTIONS() & _force_unique_teams));
#endif
}

#endif

int32
copy_and_send_audio_data(uint8* inFirstChunkReadPosition, int32 inFirstChunkBytesRemaining,
                         uint8* inSecondChunkReadPosition, int32 inSecondChunkBytesRemaining,
                         bool inForceSend) {
                         
    // Make sure the capture format has been announced to us
    assert(sSamplesPerSecond > 0);

    // caller better not be splitting chunks up mid-sample
    if (inFirstChunkBytesRemaining % sNumberOfBytesPerSample)
    {
	    inFirstChunkBytesRemaining -= (inFirstChunkBytesRemaining % sNumberOfBytesPerSample);
	    assert(inSecondChunkBytesRemaining == 0);
    }

    if (inSecondChunkBytesRemaining % sNumberOfBytesPerSample)
    {
	    inSecondChunkBytesRemaining -= (inSecondChunkBytesRemaining % sNumberOfBytesPerSample);
    }

#ifdef SPEEX
    // Let runtime system worry about allocating and freeing the buffer (and don't do it on the stack).
    // assume Speex will not encode kNetworkAudioSamplesPerPacket samples to be larger than kNetworkAudioSamplesPerPacket * kNetworkAudioBytesPerFrame!
    static uint8 sOutgoingPacketBuffer[kNetworkAudioSamplesPerPacket * kNetworkAudioBytesPerFrame + SIZEOF_network_audio_header];

    network_audio_header    theHeader;
    theHeader.mReserved = 1;
	theHeader.mFlags    = 0;

	network_audio_header_NET*   theHeader_NET = (network_audio_header_NET*) sOutgoingPacketBuffer;
	
	netcpy(theHeader_NET, &theHeader);
	
	uint8*   theOutgoingAudioData = &sOutgoingPacketBuffer[SIZEOF_network_audio_header];
	
	// Do the copying and sending
	pair<int32, int32>  theBytesConsumed;
	int32 theTotalCaptureBytesConsumed = 0;
	
	// Keep sending if we have data and either we're squeezing out the last drop or we have a packet's-worth.
	while(inFirstChunkBytesRemaining >= static_cast<int32>(sNumberOfBytesPerSample) &&
	      (inForceSend || inFirstChunkBytesRemaining + inSecondChunkBytesRemaining >= (int32)sCaptureBytesPerPacket)) {
		
		int captureBytesToCopy = std::min(inFirstChunkBytesRemaining, sCaptureBytesPerPacket);
		int bytesCopied = copy_and_speex_encode(theOutgoingAudioData, inFirstChunkReadPosition, captureBytesToCopy);
		
		theTotalCaptureBytesConsumed += captureBytesToCopy;
		
		// If there's space left in the packet and we have a second chunk, start on it.
		if(captureBytesToCopy < sCaptureBytesPerPacket && inSecondChunkBytesRemaining > 0) {
			int secondCaptureBytesToCopy = std::min(sCaptureBytesPerPacket - captureBytesToCopy, inSecondChunkBytesRemaining);
			
			int secondBytesCopied = copy_and_speex_encode(&theOutgoingAudioData[bytesCopied], inSecondChunkReadPosition, secondCaptureBytesToCopy);
			theTotalCaptureBytesConsumed += secondCaptureBytesToCopy;
			
			send_audio_data((void *) sOutgoingPacketBuffer,
					SIZEOF_network_audio_header + bytesCopied + secondBytesCopied);
			
			// Update the second chunk position and length
			inSecondChunkReadPosition += secondCaptureBytesToCopy;
			inSecondChunkBytesRemaining -= secondCaptureBytesToCopy;
		}
		// Else, either we've filled up a packet or exhausted the buffer (or both).
		else {
			send_audio_data((void *) sOutgoingPacketBuffer, SIZEOF_network_audio_header + bytesCopied);
		}
		
		// Update the first chunk position and length
		inFirstChunkReadPosition += captureBytesToCopy;
		inFirstChunkBytesRemaining -= captureBytesToCopy;
    }
	
	// Now, the first chunk is exhausted.  See if there's any left in the second chunk.  Same rules apply.
	while(inSecondChunkBytesRemaining >= static_cast<int32>(sNumberOfBytesPerSample) &&
	      (inForceSend || inSecondChunkBytesRemaining >= (int32) sCaptureBytesPerPacket)) {
		
		int captureBytesToCopy = std::min(inSecondChunkBytesRemaining, sCaptureBytesPerPacket);
		
		int bytesCopied = copy_and_speex_encode(theOutgoingAudioData, inSecondChunkReadPosition, captureBytesToCopy);
		
		theTotalCaptureBytesConsumed += captureBytesToCopy;
		
		send_audio_data((void *) sOutgoingPacketBuffer, SIZEOF_network_audio_header + bytesCopied);
		
		inSecondChunkReadPosition += captureBytesToCopy;
		inSecondChunkBytesRemaining -= captureBytesToCopy;
	}
	
	return theTotalCaptureBytesConsumed;
#else
	return inFirstChunkBytesRemaining + inSecondChunkBytesRemaining; // eat the entire thing, we only support speex
#endif
}

#endif // !defined(DISABLE_NETWORKING)


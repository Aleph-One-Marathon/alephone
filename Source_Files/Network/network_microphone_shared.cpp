/*
 *  network_microphone_shared.cpp
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
#include "speex.h"
#include "preferences.h"
#include "network_speex.h"
#endif

using namespace std;

#ifdef DEBUG
// For testing: don't send audio on the network - pass it directly to network_speaker.
//#define MICROPHONE_LOCAL_LOOPBACK
#endif


enum {
    kNetworkAudioDataBytesPerPacket = 1024, // this is meaningless in speex
    kNetworkAudioBytesPerSample = 1,
    kNetworkAudioSamplesPerPacket = kNetworkAudioDataBytesPerPacket / kNetworkAudioBytesPerSample
};



// These describe the microphone's capture format characteristics.
static  int	sNumberOfBytesPerSample = 0;
static	bool	sStereo			= false;
static	bool	s16Bit			= false;
static	uint32	sSamplesPerSecond	= 0;	
static	uint32	sCaptureStride		= 0;
static	uint32	sCaptureBytesPerNetworkAudioByte	= 0;
static	uint32	sCaptureBytesPerPacket	= 0;



bool
announce_microphone_capture_format(uint32 inSamplesPerSecond, bool inStereo, bool in16Bit) {
    sSamplesPerSecond		= inSamplesPerSecond;
    sStereo			= inStereo;
    s16Bit			= in16Bit;
    sNumberOfBytesPerSample	= (sStereo ? 2 : 1) * (s16Bit ? 2 : 1);
    sCaptureStride		= (sSamplesPerSecond / kNetworkAudioSampleRate) * (sStereo ? 2 : 1);
    sCaptureBytesPerNetworkAudioByte = (sSamplesPerSecond / kNetworkAudioSampleRate) * sNumberOfBytesPerSample / kNetworkAudioBytesPerSample;
    sCaptureBytesPerPacket	= kNetworkAudioDataBytesPerPacket * sCaptureBytesPerNetworkAudioByte;
    
    if(sCaptureStride <= 0)
        return false;
    
    if(sNumberOfBytesPerSample < kNetworkAudioBytesPerSample)
        return false;
    
    return true;
}



int32
get_capture_byte_count_per_packet() {
    // Catch folks who call us without specifying a rate first
    assert(sSamplesPerSecond > 0);
    
    return sCaptureBytesPerPacket;
}



// Note: a good optimizing compiler should be able to move the decrement of inCount earlier in the loop to
// ensure good branch prediction, if not eliminate it entirely and base the loop on outStorage.
static __inline__ void
copy_16_bit_stereo_samples(uint8* outStorage, int16* inStorage, int32 inCount) {
    while(inCount > 0) {
        *outStorage = 128 + ((inStorage[0] + inStorage[1]) >> 9);
        ++outStorage;
        inStorage += sCaptureStride;
        --inCount;
    }
}

static __inline__ void
copy_8_bit_stereo_samples(uint8* outStorage, uint8* inStorage, int32 inCount) {
    while(inCount > 0) {
        *outStorage = (inStorage[0] + inStorage[1]) / 2;
        ++outStorage;
        inStorage += sCaptureStride;
        --inCount;
    }
}

static __inline__ void
copy_16_bit_mono_samples(uint8* outStorage, int16* inStorage, int32 inCount) {
    while(inCount > 0) {
        *outStorage = 128 + ((*inStorage) >> 8);
        ++outStorage;
        inStorage += sCaptureStride;
        --inCount;
    }
}

static __inline__ void
copy_8_bit_mono_samples(uint8* outStorage, uint8* inStorage, int32 inCount) {
    while(inCount > 0) {
        *outStorage = *inStorage;
        ++outStorage;
        inStorage += sCaptureStride;
        --inCount;
    }
}

#ifdef SPEEX
int32 copy_and_speex_encode(uint8* outStorage, void* inStorage, int32 inCount, int32 inAmountOfNetworkStorage) {

    static float storedFrame[160];
    static int storedSamples = 0;
    int bytesWritten = 0;
    float frame[160];
	if (inCount + storedSamples < 160) {
		// not enough to encode a frame; add the data to storedFrame
	    while (inCount > 0) {
		    if (s16Bit) {
			    if (sStereo) {
				    // downmix to mono
					storedFrame[storedSamples] = (((int16 *) inStorage)[0] + ((int16 *) inStorage)[1]) / 2.0;
			    } else {
			       storedFrame[storedSamples] = *((int16  *) inStorage);
			    }
			    inStorage = static_cast<int16*>(inStorage) + sCaptureStride;
		   } else {
		       if (sStereo) {
		          storedFrame[storedSamples] = ((int16) ((((uint8 *)inStorage)[0] + ((uint8 *)inStorage)[1]) / 2) - 128) << 8;
		      } else {
		          storedFrame[storedSamples] = ((int16) (*((uint8 *)inStorage) - 128)) << 8;            
		     }
		     inStorage = static_cast<uint8*>(inStorage) + sCaptureStride;
		   }
		   inCount--;
		   storedSamples++;
		}
		return 0;
	}


    // first, use the old stored samples
    while (inCount + storedSamples >= 160) {
        // build a frame	
        for (int i = 0; i < 160; i++) {
            if (storedSamples > 0) {
                frame[i] = storedFrame[i];
                storedSamples--;
            } else {
                if (s16Bit) {
                    if (sStereo) {
                        // downmix to mono
                        frame[i] = (((int16 *) inStorage)[0] + ((int16 *) inStorage)[1]) / 2.0;
                    } else {
                        frame[i] = *((int16  *) inStorage);
                    }
                    inStorage = static_cast<int16*>(inStorage) + sCaptureStride;
                } else {
                    if (sStereo) {
                        frame[i] = ((int16) ((((uint8 *)inStorage)[0] + ((uint8 *)inStorage)[1]) / 2) - 128) << 8;
                    } else {
                        frame[i] = ((int16) (*((uint8 *)inStorage) - 128)) << 8;            
                    }
                    inStorage = static_cast<uint8*>(inStorage) + sCaptureStride;
                }
                inCount--;
            }
        }

        // encode the frame
        speex_bits_reset(&gEncoderBits);
        speex_encode(gEncoderState, frame, &gEncoderBits);
        uint8 nbytes = speex_bits_write(&gEncoderBits, reinterpret_cast<char*>(outStorage) + 1, 200);
        bytesWritten += nbytes + 1;
        // first put the size of this frame in storage
        *(outStorage) = nbytes;
        outStorage += nbytes + 1;
    }
    // fill the storedFrame for next time
    assert(storedSamples == 0);
    while (inCount > 0) {
        if (s16Bit) {
            if (sStereo) {
                // downmix to mono
                storedFrame[storedSamples] = (((int16 *) inStorage)[0] + ((int16 *) inStorage)[1]) / 2.0;
            } else {
                storedFrame[storedSamples] = *((int16  *) inStorage);
            }
            inStorage = static_cast<int16*>(inStorage) + sCaptureStride;
        } else {
            if (sStereo) {
                storedFrame[storedSamples] = ((int16) ((((uint8 *)inStorage)[0] + ((uint8 *)inStorage)[1]) / 2) - 128) << 8;
            } else {
                storedFrame[storedSamples] = ((int16) (*((uint8 *)inStorage) - 128)) << 8;            
            }
            inStorage = static_cast<uint8*>(inStorage) + sCaptureStride;
        }
        inCount--;
        storedSamples++;
    }
   
    return bytesWritten;
}
#endif //def SPEEX

// Returns pair (used network storage bytes, used capture storage bytes)
// assumes inAmountOfCaptureStorage > sCaptureBytesPerNetworkAudioByte
static pair<int32, int32>
copy_data_in_capture_format_to_network_format(uint8* inNetworkStorage, int inAmountOfNetworkStorage,
                                              void* inCaptureStorage, int inAmountOfCaptureStorage) {

    int32	theNetworkAudioBytesCaptured		= inAmountOfCaptureStorage / sCaptureBytesPerNetworkAudioByte;
    int32	theNetworkAudioBytesToCopy		= MIN(inAmountOfNetworkStorage, theNetworkAudioBytesCaptured);
    int32	theNumberOfNetworkAudioSamplesToCopy	= theNetworkAudioBytesToCopy / kNetworkAudioBytesPerSample;
    
#ifdef SPEEX
    if (network_preferences->use_speex_encoder) {
        theNumberOfNetworkAudioSamplesToCopy = copy_and_speex_encode(inNetworkStorage, inCaptureStorage, theNumberOfNetworkAudioSamplesToCopy, inAmountOfNetworkStorage);
        return pair<int32, int32>(theNumberOfNetworkAudioSamplesToCopy, theNetworkAudioBytesToCopy * sCaptureBytesPerNetworkAudioByte);
    } else 
#endif
    {
        // Actually perform the copying (mixing stereo->mono, 16->8 bit conversion, downsampling, etc.)
        if(s16Bit) {
            if(sStereo)
                copy_16_bit_stereo_samples(inNetworkStorage, (int16*) inCaptureStorage, theNumberOfNetworkAudioSamplesToCopy);
            else
                copy_16_bit_mono_samples(inNetworkStorage, (int16*) inCaptureStorage, theNumberOfNetworkAudioSamplesToCopy);
        }
        else {
            if(sStereo)
                copy_8_bit_stereo_samples(inNetworkStorage, (uint8*) inCaptureStorage, theNumberOfNetworkAudioSamplesToCopy);
            else
                copy_8_bit_mono_samples(inNetworkStorage, (uint8*) inCaptureStorage, theNumberOfNetworkAudioSamplesToCopy);
        }
    
        // Tell the caller how many bytes of each were used.
        return pair<int32, int32>(theNumberOfNetworkAudioSamplesToCopy, theNetworkAudioBytesToCopy * sCaptureBytesPerNetworkAudioByte);
    }
}


static void
send_audio_data(void* inData, short inSize) {
#ifdef MICROPHONE_LOCAL_LOOPBACK
#include "network_sound.h"
    received_network_audio_proc(inData, inSize, 0);
#else
    NetDistributeInformation(kNewNetworkAudioDistributionTypeID, inData, inSize, false);
#endif
}



int32
copy_and_send_audio_data(uint8* inFirstChunkReadPosition, int32 inFirstChunkBytesRemaining,
                         uint8* inSecondChunkReadPosition, int32 inSecondChunkBytesRemaining,
                         bool inForceSend) {
                         
    // Make sure the capture format has been announced to us
    assert(sSamplesPerSecond > 0);

    // Let runtime system worry about allocating and freeing the buffer (and don't do it on the stack).
    static  uint8           sOutgoingPacketBuffer[kNetworkAudioDataBytesPerPacket + SIZEOF_network_audio_header];

    network_audio_header    theHeader;
#ifdef SPEEX
    if (network_preferences->use_speex_encoder) {
        theHeader.mReserved = 1;
    } else
#endif
    theHeader.mReserved = 0;
    theHeader.mFlags    = 0;

    network_audio_header_NET*   theHeader_NET = (network_audio_header_NET*) sOutgoingPacketBuffer;

    netcpy(theHeader_NET, &theHeader);

    uint8*   theOutgoingAudioData = &sOutgoingPacketBuffer[SIZEOF_network_audio_header];

    // Do the copying and sending
    pair<int32, int32>  theBytesConsumed;
    int32 theTotalCaptureBytesConsumed = 0;

    // Keep sending if we have data and either we're squeezing out the last drop or we have a packet's-worth.
    while(inFirstChunkBytesRemaining >= static_cast<int32>(sCaptureBytesPerNetworkAudioByte) &&
        (inForceSend || inFirstChunkBytesRemaining + inSecondChunkBytesRemaining >= (int32)sCaptureBytesPerPacket)) {

        theBytesConsumed = copy_data_in_capture_format_to_network_format(theOutgoingAudioData,
            kNetworkAudioDataBytesPerPacket, inFirstChunkReadPosition, inFirstChunkBytesRemaining);
            
        theTotalCaptureBytesConsumed += theBytesConsumed.second;

        // If there's space left in the packet and we have a second chunk, start on it.
        if(theBytesConsumed.first < kNetworkAudioDataBytesPerPacket && inSecondChunkBytesRemaining > 0) {
            pair<int32, int32>  theSecondBytesConsumed;

            theSecondBytesConsumed = copy_data_in_capture_format_to_network_format(
                &(theOutgoingAudioData[theBytesConsumed.first]), kNetworkAudioDataBytesPerPacket - theBytesConsumed.first,
                inSecondChunkReadPosition, inSecondChunkBytesRemaining);
                
            theTotalCaptureBytesConsumed += theSecondBytesConsumed.second;
            
            send_audio_data((void*) sOutgoingPacketBuffer,
                SIZEOF_network_audio_header + theBytesConsumed.first + theSecondBytesConsumed.first);
            
            // Update the second chunk position and length
            inSecondChunkReadPosition   += theSecondBytesConsumed.second;
            inSecondChunkBytesRemaining -= theSecondBytesConsumed.second;
        }
        // Else, either we've filled up a packet or exhausted the buffer (or both).
        else {
            send_audio_data((void*) sOutgoingPacketBuffer, SIZEOF_network_audio_header + theBytesConsumed.first);
        }

        // Update the first chunk position and length
        inFirstChunkReadPosition    += theBytesConsumed.second;
        inFirstChunkBytesRemaining  -= theBytesConsumed.second;
    }

    // Now, the first chunk is exhausted.  See if there's any left in the second chunk.  Same rules apply.
    while(inSecondChunkBytesRemaining >= static_cast<int32>(sCaptureBytesPerNetworkAudioByte) &&
        (inForceSend || inSecondChunkBytesRemaining >= kNetworkAudioDataBytesPerPacket)) {
        
        theBytesConsumed = copy_data_in_capture_format_to_network_format(theOutgoingAudioData, kNetworkAudioDataBytesPerPacket,
            inSecondChunkReadPosition, inSecondChunkBytesRemaining);
            
        theTotalCaptureBytesConsumed += theBytesConsumed.second;

        send_audio_data((void*) sOutgoingPacketBuffer, SIZEOF_network_audio_header + theBytesConsumed.first);

        inSecondChunkReadPosition   += theBytesConsumed.second;
        inSecondChunkBytesRemaining -= theBytesConsumed.second;
    }
    
    return theTotalCaptureBytesConsumed;
}

#endif // !defined(DISABLE_NETWORKING)


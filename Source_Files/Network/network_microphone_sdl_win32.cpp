/*
 *  network_microphone_sdl_win32.cpp
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
 *  This is a DirectSoundCapture (Win32 DirectX)-based network microphone implementation.
 *  Other platforms will need their own implementations.  A dummy implementation is
 *  provided in network_microphone_sdl_dummy.cpp for you to link against if you don't have
 *  a "real" implementation.
 *
 *  Please factor out parts useful on other platforms rather than nearly-duplicating them.
 *
 *  Created by woody Mar 3-8, 2002.
 */

#include    "cseries.h"
#include    "network_speaker_sdl.h"
#include    "network_data_formats.h"
#include    "network_distribution_types.h"
#include    <dsound.h>

#ifdef DEBUG
// For testing: don't send audio on the network - pass it directly to network_speaker.
//#define MICROPHONE_LOCAL_LOOPBACK
#endif


enum {
    kNetworkAudioDataBytesPerPacket = 1024   // How many bytes of audio data in each packet?
};



static  LPDIRECTSOUNDCAPTURE        sDirectSoundCapture     = NULL;
static  LPDIRECTSOUNDCAPTUREBUFFER  sCaptureBuffer          = NULL;
static  int                         sCaptureBufferSize      = 0;
static  bool                        sCaptureSystemReady     = false;
static  int                         sNextReadStartPosition  = 0;
static  int                         sCaptureFormatIndex     = 0;
static  int                         sNumberOfBytesPerSample = 0;
static  bool                        sTransmittingAudio      = false;



struct  CaptureFormat {
    uint32  mIdentifier;
    uint32  mRate;
    bool    mStereo;
    bool    m16Bit;
};

// These are searched in the listed order for a format the hardware supports.
// List ONLY those with sample rates >= the network audio sample rate, because
// the format-translation routines can only downsample.
CaptureFormat   sFormatPreferences[] = {
    { WAVE_FORMAT_1M08, 11025, false, false },
    { WAVE_FORMAT_1S08, 11025,  true, false },
    { WAVE_FORMAT_1M16, 11025, false,  true },
    { WAVE_FORMAT_1S16, 11025,  true,  true },
    { WAVE_FORMAT_2M08, 22050, false, false },
    { WAVE_FORMAT_2S08, 22050,  true, false },
    { WAVE_FORMAT_2M16, 22050, false,  true },
    { WAVE_FORMAT_2S16, 22050,  true,  true },
    { WAVE_FORMAT_4M08, 44100, false, false },
    { WAVE_FORMAT_4S08, 44100,  true, false },
    { WAVE_FORMAT_4M16, 44100, false,  true },
    { WAVE_FORMAT_4S16, 44100,  true,  true }
};

const int   NUM_CAPTURE_FORMAT_PREFERENCES = sizeof(sFormatPreferences) / sizeof(sFormatPreferences[0]);



// Note: a good optimizing compiler should be able to move the decrement of inCount earlier in the loop to
// ensure good branch prediction, if not eliminate it entirely and base the loop on outStorage.
static __inline__ void
copy_16_bit_stereo_samples(uint8* outStorage, int16* inStorage, int32 inCount, int32 inPitch) {
    while(inCount > 0) {
        *outStorage = 128 + ((inStorage[0] + inStorage[1]) >> 9);
        ++outStorage;
        inStorage += inPitch;
        --inCount;
    }
}

static __inline__ void
copy_8_bit_stereo_samples(uint8* outStorage, uint8* inStorage, int32 inCount, int32 inPitch) {
    while(inCount > 0) {
        *outStorage = (inStorage[0] + inStorage[1]) / 2;
        ++outStorage;
        inStorage += inPitch;
        --inCount;
    }
}

static __inline__ void
copy_16_bit_mono_samples(uint8* outStorage, int16* inStorage, int32 inCount, int32 inPitch) {
    while(inCount > 0) {
        *outStorage = 128 + ((*inStorage) >> 8);
        ++outStorage;
        inStorage += inPitch;
        --inCount;
    }
}

static __inline__ void
copy_8_bit_mono_samples(uint8* outStorage, uint8* inStorage, int32 inCount, int32 inPitch) {
    while(inCount > 0) {
        *outStorage = *inStorage;
        ++outStorage;
        inStorage += inPitch;
        --inCount;
    }
}



// Returns pair (used network storage bytes, used capture storage bytes)
static pair<int32, int32>
copy_data_in_capture_format_to_network_format(uint8* inNetworkStorage, int inAmountOfNetworkStorage,
                                              void* inCaptureStorage, int inAmountOfCaptureStorage) {

    int32   theNumberOfCapturedSamples      = inAmountOfCaptureStorage / sNumberOfBytesPerSample;
    int32   theNumberOfSamplesToCopy        = MIN(inAmountOfNetworkStorage, theNumberOfCapturedSamples);
    int32   theSamplePitch                  = sFormatPreferences[sCaptureFormatIndex].mRate / kNetworkAudioSampleRate;

    // Not designed for upsampling
    assert(theSamplePitch > 0);

    // Sample pitch tells us how many samples to advance to get to next relevant one.
    if(sFormatPreferences[sCaptureFormatIndex].mStereo)
        theSamplePitch *= 2;

    // Actually perform the copying (mixing stereo->mono, 16->8 bit conversion, downsampling, etc.)
    if(sFormatPreferences[sCaptureFormatIndex].m16Bit) {
        if(sFormatPreferences[sCaptureFormatIndex].mStereo)
            copy_16_bit_stereo_samples(inNetworkStorage, (int16*) inCaptureStorage, theNumberOfSamplesToCopy, theSamplePitch);
        else
            copy_16_bit_mono_samples(inNetworkStorage, (int16*) inCaptureStorage, theNumberOfSamplesToCopy, theSamplePitch);
    }
    else {
        if(sFormatPreferences[sCaptureFormatIndex].mStereo)
            copy_8_bit_stereo_samples(inNetworkStorage, (uint8*) inCaptureStorage, theNumberOfSamplesToCopy, theSamplePitch);
        else
            copy_8_bit_mono_samples(inNetworkStorage, (uint8*) inCaptureStorage, theNumberOfSamplesToCopy, theSamplePitch);
    }

    // Tell the caller how many bytes of each were used.
    return pair<int32, int32>(theNumberOfSamplesToCopy, theNumberOfSamplesToCopy * sNumberOfBytesPerSample);
}



static void
send_audio_data(void* inData, short inSize) {
#ifdef MICROPHONE_LOCAL_LOOPBACK
    received_network_audio_proc(inData, inSize, 0);
#else
    NetDistributeInformation(kNewNetworkAudioDistributionTypeID, inData, inSize, false);
#endif
}



static void
copy_and_send_audio_data(uint8* inFirstChunkReadPosition, int32 inFirstChunkBytesRemaining,
                         uint8* inSecondChunkReadPosition, int32 inSecondChunkBytesRemaining,
                         bool inForceSend) {

    // Let runtime system worry about allocating and freeing the buffer (and don't do it on the stack).
    static  uint8           sOutgoingPacketBuffer[kNetworkAudioDataBytesPerPacket + SIZEOF_network_audio_header];

    network_audio_header    theHeader;
    theHeader.mReserved = 0;
    theHeader.mFlags    = 0;

    network_audio_header_NET*   theHeader_NET = (network_audio_header_NET*) sOutgoingPacketBuffer;

    netcpy(theHeader_NET, &theHeader);

    uint8*   theOutgoingAudioData = &sOutgoingPacketBuffer[SIZEOF_network_audio_header];

    // Do the copying and sending
    pair<int32, int32>  theBytesConsumed;

    // Keep sending if we have data and either we're squeezing out the last drop or we have a packet's-worth.
    while(inFirstChunkBytesRemaining > 0 &&
        (inForceSend || inFirstChunkBytesRemaining + inSecondChunkBytesRemaining >= kNetworkAudioDataBytesPerPacket)) {

        theBytesConsumed = copy_data_in_capture_format_to_network_format(theOutgoingAudioData,
            kNetworkAudioDataBytesPerPacket, inFirstChunkReadPosition, inFirstChunkBytesRemaining);

        // If there's space left in the packet and we have a second chunk, start on it.
        if(theBytesConsumed.first < kNetworkAudioDataBytesPerPacket && inSecondChunkBytesRemaining > 0) {
            pair<int32, int32>  theSecondBytesConsumed;

            theSecondBytesConsumed = copy_data_in_capture_format_to_network_format(
                &(theOutgoingAudioData[theBytesConsumed.first]), kNetworkAudioDataBytesPerPacket - theBytesConsumed.first,
                inSecondChunkReadPosition, inSecondChunkBytesRemaining);
            
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
    while(inSecondChunkBytesRemaining > 0 &&
        (inForceSend || inSecondChunkBytesRemaining >= kNetworkAudioDataBytesPerPacket)) {
        
        theBytesConsumed = copy_data_in_capture_format_to_network_format(theOutgoingAudioData, kNetworkAudioDataBytesPerPacket,
            inSecondChunkReadPosition, inSecondChunkBytesRemaining);

        send_audio_data((void*) sOutgoingPacketBuffer, SIZEOF_network_audio_header + theBytesConsumed.first);

        inSecondChunkReadPosition   += theBytesConsumed.second;
        inSecondChunkBytesRemaining -= theBytesConsumed.second;
    }
}



static void
transmit_captured_data(bool inForceSend) {
    // Find out how much data we can read
    unsigned long  theReadPosition;
    sCaptureBuffer->GetCurrentPosition(&theReadPosition, NULL);

    int32   theAmountOfDataAvailable = (sCaptureBufferSize + theReadPosition - sNextReadStartPosition) % sCaptureBufferSize;

    // If we don't have a full packet's worth yet, don't send unless we're squeezing out the end.
    if(theAmountOfDataAvailable >= kNetworkAudioDataBytesPerPacket || inForceSend) {

        // Lock capture buffer so we can copy audio out
        void*           theFirstChunkStart;
        unsigned long   theFirstChunkSize;
        void*           theSecondChunkStart;
        unsigned long   theSecondChunkSize;
    
        sCaptureBuffer->Lock(sNextReadStartPosition, theAmountOfDataAvailable, &theFirstChunkStart, &theFirstChunkSize,
            &theSecondChunkStart, &theSecondChunkSize, 0);

        // This ought to be the same as theAmountOfDataAvailable, I'd think, but just in case...
        int32   theAmountOfDataLocked   = theFirstChunkSize + theSecondChunkSize;

        // We might not actually use all of it, since we try to send in whole-packet increments.
        int32   theAmountOfDataConsumed = inForceSend ? theAmountOfDataLocked :
                    (theAmountOfDataLocked - (theAmountOfDataLocked % kNetworkAudioDataBytesPerPacket));

        sNextReadStartPosition = (sNextReadStartPosition + theAmountOfDataConsumed) % sCaptureBufferSize;
    
        copy_and_send_audio_data((uint8*) theFirstChunkStart, theFirstChunkSize,
            (uint8*) theSecondChunkStart, theSecondChunkSize, inForceSend);

        sCaptureBuffer->Unlock(theFirstChunkStart, theFirstChunkSize, theSecondChunkStart, theSecondChunkSize);
    }
}



void
open_network_microphone() {
    // We will probably do weird things if people open us twice without closing in between
    assert(!sCaptureSystemReady);
    assert(sDirectSoundCapture == NULL);
    assert(sCaptureBuffer == NULL);

    HRESULT theResult = DirectSoundCaptureCreate(NULL, &sDirectSoundCapture, NULL);

    if(FAILED(theResult)) {
        fprintf(stderr, "DirectSoundCaptureCreate failed: %d\n", theResult);
    }
    else {
        // See what capture formats the device supports
        DSCCAPS theCaptureCapabilities;
        ZeroMemory(&theCaptureCapabilities, sizeof(theCaptureCapabilities));
        theCaptureCapabilities.dwSize   = sizeof(theCaptureCapabilities);
        sDirectSoundCapture->GetCaps(&theCaptureCapabilities);

        // Find the most-preferred (earliest-listed) format the device supports
        sCaptureFormatIndex = NONE;
        for(int i = 0; i < NUM_CAPTURE_FORMAT_PREFERENCES; i++) {
            if(theCaptureCapabilities.dwFormats & sFormatPreferences[i].mIdentifier) {
                // Found a format the capture system supports
                sCaptureFormatIndex = i;
                break;
            }
        }

        if(sCaptureFormatIndex == NONE) {
            fprintf(stderr, "No valid audio capture formats supported\n");
        }
        else {
            CaptureFormat&  theChosenFormat = sFormatPreferences[sCaptureFormatIndex];

            WAVEFORMATEX    theWaveFormat;
            theWaveFormat.wFormatTag        = WAVE_FORMAT_PCM;
            theWaveFormat.nChannels         = theChosenFormat.mStereo ? 2 : 1;
            theWaveFormat.nSamplesPerSec    = theChosenFormat.mRate;
            theWaveFormat.wBitsPerSample    = theChosenFormat.m16Bit ? 16 : 8;
            sNumberOfBytesPerSample         = theWaveFormat.nChannels * theWaveFormat.wBitsPerSample / 8;
            theWaveFormat.nBlockAlign       = sNumberOfBytesPerSample;
            theWaveFormat.nAvgBytesPerSec   = theWaveFormat.nBlockAlign * theWaveFormat.nSamplesPerSec;
            theWaveFormat.cbSize            = 0;

            // Let's try a half-second buffer.
            sCaptureBufferSize              = theWaveFormat.nAvgBytesPerSec / 2;

            DSCBUFFERDESC   theRecordingBufferDescription;
            ZeroMemory(&theRecordingBufferDescription, sizeof(theRecordingBufferDescription));
            theRecordingBufferDescription.dwSize        = sizeof(theRecordingBufferDescription);
            theRecordingBufferDescription.dwBufferBytes = sCaptureBufferSize;
            theRecordingBufferDescription.lpwfxFormat   = &theWaveFormat;

            theResult = sDirectSoundCapture->CreateCaptureBuffer(&theRecordingBufferDescription, &sCaptureBuffer, NULL);

            if(FAILED(theResult)) {
                fprintf(stderr, "CreateCaptureBuffer failed: %d\n", theResult);
            }
            else {
                // Initialization successful
                sCaptureSystemReady = true;
            }
        }
    }

    if(!sCaptureSystemReady)
        fprintf(stderr, "Could not set up DirectSoundCapture - network microphone disabled\n");
}



static void
start_transmitting_audio() {
    assert(sCaptureSystemReady);
    assert(!sTransmittingAudio);

    HRESULT theResult   = sCaptureBuffer->Start(DSCBSTART_LOOPING);

    sTransmittingAudio  = (theResult == DS_OK);
}



static void
stop_transmitting_audio() {
    assert(sCaptureSystemReady);
    assert(sTransmittingAudio);

    HRESULT theResult   = sCaptureBuffer->Stop();

    sTransmittingAudio  = !(theResult == DS_OK);

    // Flush out the last chunk of stuff
    transmit_captured_data(true);
}



void
close_network_microphone() {
    if(sCaptureSystemReady && sTransmittingAudio)
        stop_transmitting_audio();

    if(sCaptureBuffer != NULL) {
        sCaptureBuffer->Release();
        sCaptureBuffer = NULL;
    }

    if(sDirectSoundCapture != NULL) {
        sDirectSoundCapture->Release();
        sDirectSoundCapture = NULL;
    }

    sCaptureSystemReady = false;
}



void
set_network_microphone_state(bool inActive) {
    if(sCaptureSystemReady) {
        if(inActive && !sTransmittingAudio)
            start_transmitting_audio();
        else if(!inActive && sTransmittingAudio)
            stop_transmitting_audio();
    }
}



bool
is_network_microphone_implemented() {
    return true;
}



void
network_microphone_idle_proc() {
    if(sCaptureSystemReady && sTransmittingAudio)
        transmit_captured_data(false);
}

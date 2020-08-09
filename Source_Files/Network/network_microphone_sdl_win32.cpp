/*
 *  network_microphone_sdl_win32.cpp
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
 *  This is a DirectSoundCapture (Win32 DirectX)-based network microphone implementation.
 *  Other platforms will need their own implementations.  A dummy implementation is
 *  provided in network_microphone_sdl_dummy.cpp for you to link against if you don't have
 *  a "real" implementation.
 *
 *  Please factor out parts useful on other platforms rather than nearly-duplicating them.
 *
 *  Created by woody Mar 3-8, 2002.
 *
 *  Feb. 1, 2003 (Woody Zenfell):
 *     factored out parts useful on other platforms into network_microphone_shared.cpp
 */

#if !defined(DISABLE_NETWORKING)

#include    "cseries.h"
#include    "network_speaker_sdl.h"
#include    "network_microphone_shared.h"
#include    "Logging.h"

#ifdef SPEEX
#include "preferences.h"
#include "network_speex.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>



static  LPDIRECTSOUNDCAPTURE        sDirectSoundCapture     = NULL;
static  LPDIRECTSOUNDCAPTUREBUFFER  sCaptureBuffer          = NULL;
static  int                         sCaptureBufferSize      = 0;
static  bool                        sCaptureSystemReady     = false;
static  int                         sNextReadStartPosition  = 0;
static  int                         sCaptureFormatIndex     = 0;
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
    { WAVE_FORMAT_1M16, 11025, false,  true },
    { WAVE_FORMAT_1S16, 11025,  true,  true },
    { WAVE_FORMAT_1M08, 11025, false, false },
    { WAVE_FORMAT_1S08, 11025,  true, false },
    { WAVE_FORMAT_2M16, 22050, false,  true },
    { WAVE_FORMAT_2S16, 22050,  true,  true },
    { WAVE_FORMAT_2M08, 22050, false, false },
    { WAVE_FORMAT_2S08, 22050,  true, false },
    { WAVE_FORMAT_4M16, 44100, false,  true },
    { WAVE_FORMAT_4S16, 44100,  true,  true },
    { WAVE_FORMAT_4M08, 44100, false, false },
    { WAVE_FORMAT_4S08, 44100,  true, false }
};

const int   NUM_CAPTURE_FORMAT_PREFERENCES = sizeof(sFormatPreferences) / sizeof(sFormatPreferences[0]);



static void
transmit_captured_data(bool inForceSend) {
    // Find out how much data we can read
    unsigned long  theReadPosition;
    sCaptureBuffer->GetCurrentPosition(&theReadPosition, NULL);

    int32   theAmountOfDataAvailable = (sCaptureBufferSize + theReadPosition - sNextReadStartPosition) % sCaptureBufferSize;

    // If we don't have a full packet's worth yet, don't send unless we're squeezing out the end.
    if(theAmountOfDataAvailable >= get_capture_byte_count_per_packet() || inForceSend) {
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
        int32 theAmountOfDataConsumed = copy_and_send_audio_data((uint8*) theFirstChunkStart, theFirstChunkSize,
            (uint8*) theSecondChunkStart, theSecondChunkSize, inForceSend);

        sNextReadStartPosition = (sNextReadStartPosition + theAmountOfDataConsumed) % sCaptureBufferSize;
    
        sCaptureBuffer->Unlock(theFirstChunkStart, theFirstChunkSize, theSecondChunkStart, theSecondChunkSize);
    }
}



OSErr
open_network_microphone() {
    logContext("setting up microphone");
    
    // We will probably do weird things if people open us twice without closing in between
    assert(!sCaptureSystemReady);
    assert(sDirectSoundCapture == NULL);
    assert(sCaptureBuffer == NULL);

    HRESULT theResult = DirectSoundCaptureCreate(NULL, &sDirectSoundCapture, NULL);

    if(FAILED(theResult)) {
        logAnomaly("DirectSoundCaptureCreate failed: %d", theResult);
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
            logAnomaly("No valid audio capture formats supported");
        }
        else {
            CaptureFormat&  theChosenFormat = sFormatPreferences[sCaptureFormatIndex];

            WAVEFORMATEX    theWaveFormat;
            theWaveFormat.wFormatTag        = WAVE_FORMAT_PCM;
            theWaveFormat.nChannels         = theChosenFormat.mStereo ? 2 : 1;
            theWaveFormat.nSamplesPerSec    = theChosenFormat.mRate;
            theWaveFormat.wBitsPerSample    = theChosenFormat.m16Bit ? 16 : 8;
            theWaveFormat.nBlockAlign       = theWaveFormat.nChannels * theWaveFormat.wBitsPerSample / 8;
            theWaveFormat.nAvgBytesPerSec   = theWaveFormat.nBlockAlign * theWaveFormat.nSamplesPerSec;
            theWaveFormat.cbSize            = 0;

            // Let's try a half-second buffer.
	    sCaptureBufferSize = (theWaveFormat.nSamplesPerSec / 2) * theWaveFormat.nBlockAlign;

            DSCBUFFERDESC   theRecordingBufferDescription;
            ZeroMemory(&theRecordingBufferDescription, sizeof(theRecordingBufferDescription));
            theRecordingBufferDescription.dwSize        = sizeof(theRecordingBufferDescription);
            theRecordingBufferDescription.dwBufferBytes = sCaptureBufferSize;
            theRecordingBufferDescription.lpwfxFormat   = &theWaveFormat;

            theResult = sDirectSoundCapture->CreateCaptureBuffer(&theRecordingBufferDescription, &sCaptureBuffer, NULL);

            if(FAILED(theResult)) {
                logAnomaly("CreateCaptureBuffer failed: %d", theResult);
            }
            else {
                if(!announce_microphone_capture_format(theChosenFormat.mRate, theChosenFormat.mStereo, theChosenFormat.m16Bit)) {
                    logAnomaly("network microphone support code rejected audio format: %d samp/sec, %s, %d bits/samp",
                                theChosenFormat.mRate, theChosenFormat.mStereo ? "stereo" : "mono", theChosenFormat.m16Bit ? 16 : 8);
                }
                else {
                    // Initialization successful
                    sCaptureSystemReady = true;
                }
            }
        }
    }

#ifdef SPEEX
	if (network_preferences->use_speex_encoder) {
		init_speex_encoder();
	}
#endif

    if(!sCaptureSystemReady)
        logAnomaly("Could not set up DirectSoundCapture - network microphone disabled");
    
    // Here we _lie_, for now, to conform to the same interface the Mac code uses.
    return 0;
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

#ifdef SPEEX
	if (network_preferences->use_speex_encoder) {
		destroy_speex_encoder();
	}
#endif

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



bool
has_sound_input_capability() {
    // We really probably should actually _check_.
    return true;
}



void
network_microphone_idle_proc() {
    if(sCaptureSystemReady && sTransmittingAudio)
        transmit_captured_data(false);
}

#endif // !defined(DISABLE_NETWORKING)

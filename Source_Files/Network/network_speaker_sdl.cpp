/*
 *  network_speaker_sdl.cpp
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
 *  Realtime network audio playback support for SDL platforms.
 *
 *  Created by woody Mar 3-8, 2002.
 *
 *  14 January 2003 (Woody Zenfell): reworked memory management so all new/delete are called
 *      from the main thread: buffers are released by the audio system and returned to us for reuse.
 */

#if !defined(DISABLE_NETWORKING)

#include    "network_sound.h"
#include    "network_speaker_sdl.h"

#include    "network_distribution_types.h"
#include    "CircularQueue.h"
#include    "world.h"   // local_random()
#include "Mixer.h"

#ifdef SPEEX
#include "network_speex.h"
#endif //def SPEEX

enum {
    kSoundBufferQueueSize = 32,     // should never get anywhere near here, but at 12 bytes/struct these are cheap.
    kNoiseBufferSize = 1280 * 2,        // how big a buffer we should use for noise (at 11025 this is about 1/9th of a second)
    kMaxDryDequeues = 1,            // how many consecutive empty-buffers before we stop playing?
    kNumPumpPrimes = 1,             // how many noise-buffers should we start with while buffering incoming data?
    kNumSoundDataBuffers = 8,		// how many actual audio storage buffers should we have?
    kSoundDataBufferSize = 2048 * 2		// how big will each audio storage buffer be?
};

// "Send queue" of buffers from us to audio code (with descriptors)
static  CircularQueue<NetworkSpeakerSoundBufferDescriptor>    sSoundBuffers(kSoundBufferQueueSize);

// "Return queue" of buffers from audio code to us for reuse
static	CircularQueue<byte*>				sSoundDataBuffers(kNumSoundDataBuffers + 2);  // +2: breathing room

// We can provide static noise instead of a "real" buffer once in a while if we need to.
// Also, we provide kNumPumpPrimes of static noise before getting to the "meat" as well.
static  byte*                       		sNoiseBufferStorage = NULL;
static  NetworkSpeakerSoundBufferDescriptor	sNoiseBufferDesc;
static  int                         		sDryDequeues = 0;
static  bool                        		sSpeakerIsOn = false;


OSErr
open_network_speaker() {
    // Allocate storage for noise data - assume if pointer not NULL, already have storage.
    if(sNoiseBufferStorage == NULL) {
        assert(kNoiseBufferSize % 2 == 0);
        uint16* theBuffer = new uint16[kNoiseBufferSize / 2];

        // Fill in noise data (use whole width of local_random())
        for(int i = 0; i < kNoiseBufferSize / 2; i++)
            theBuffer[i] = local_random() / 4;

        sNoiseBufferStorage = (byte*) theBuffer;
    }

    // Fill out the noise-buffer descriptor
    sNoiseBufferDesc.mData      = sNoiseBufferStorage;
    sNoiseBufferDesc.mLength    = kNoiseBufferSize;
    sNoiseBufferDesc.mFlags     = 0;

    // Reset the buffer descriptor queue
    sSoundBuffers.reset();
    
    // Reset the data buffer queue
    sSoundDataBuffers.reset();
    
    // Allocate storage for audio data buffers
    for(int i = 0; i < kNumSoundDataBuffers; i++) {
        byte* theBuffer = new byte[kSoundDataBufferSize];
        sSoundDataBuffers.enqueue(theBuffer);
    }

    // Reset a couple others to sane values
    sDryDequeues    = 0;
    sSpeakerIsOn    = false;
    
#ifdef SPEEX
        init_speex_decoder();
#endif
    
    return 0;
}

void
queue_network_speaker_data(byte* inData, short inLength) {
    if(inLength > 0) {
        if(sSoundDataBuffers.getCountOfElements() > 0) {
            // Fill out a descriptor for a new chunk of storage
            NetworkSpeakerSoundBufferDescriptor theBufferDesc;
            theBufferDesc.mData     = sSoundDataBuffers.peek();
            sSoundDataBuffers.dequeue();
            theBufferDesc.mLength   = inLength;
            theBufferDesc.mFlags    = kSoundDataIsDisposable;
    
            // and copy the data
            memcpy(theBufferDesc.mData, inData, inLength);
    
            // If we're just turning on, prime the queue with a few buffers of noise.
            if(!sSpeakerIsOn) {
                for(int i = 0; i < kNumPumpPrimes; i++) {
                    sSoundBuffers.enqueue(sNoiseBufferDesc);
                }
    
                sSpeakerIsOn = true;
            }
    
            // Enqueue the actual sound data.
            sSoundBuffers.enqueue(theBufferDesc);
        }
        else {
            fdprintf("No sound data buffer space available - audio discarded");
        }
    }
}


void
network_speaker_idle_proc() {
    if(sSpeakerIsOn)
	    Mixer::instance()->EnsureNetworkAudioPlaying();
}


NetworkSpeakerSoundBufferDescriptor*
dequeue_network_speaker_data() {
    // We need this to stick around between calls
    static NetworkSpeakerSoundBufferDescriptor    sBufferDesc;

    // If there is actual sound data, reset the "ran dry" count and return a pointer to the buffer descriptor
    if(sSoundBuffers.getCountOfElements() > 0) {
        sDryDequeues = 0;
        sBufferDesc = sSoundBuffers.peek();
        sSoundBuffers.dequeue();
        return &sBufferDesc;
    }
    // If there's no data available, inc the "ran dry" count and return either a noise buffer or NULL.
    else {
        sDryDequeues++;
        if(sDryDequeues > kMaxDryDequeues) {
            sSpeakerIsOn = false;
            return NULL;
        }
        else
            return &sNoiseBufferDesc;
    }
}


void
close_network_speaker() {
    // Tell the audio system not to get our data anymore
	Mixer::instance()->StopNetworkAudio();

    // Bleed the queue dry of any leftover data
    NetworkSpeakerSoundBufferDescriptor*  theDesc;
    while((theDesc = dequeue_network_speaker_data()) != NULL) {
        if(is_sound_data_disposable(theDesc))
            release_network_speaker_buffer(theDesc->mData);
    }
    
    // Free the sound data buffers
    while(sSoundDataBuffers.getCountOfElements() > 0) {
        byte* theBuffer = sSoundDataBuffers.peek();
        delete [] theBuffer;
        sSoundDataBuffers.dequeue();
    }

    // Free the noise buffer and restore some values
    if(sNoiseBufferStorage != NULL) {
        delete [] sNoiseBufferStorage;
        sNoiseBufferStorage = NULL;
    }
    sDryDequeues    = 0;
    sSpeakerIsOn    = false;
    
    #ifdef SPEEX
    destroy_speex_decoder();
    #endif
}



void
release_network_speaker_buffer(byte* inBuffer) {
    sSoundDataBuffers.enqueue(inBuffer);
}

#endif // !defined(DISABLE_NETWORKING)


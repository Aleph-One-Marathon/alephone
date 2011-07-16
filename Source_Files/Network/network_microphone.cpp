/*
NETWORK_MICROPHONE.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

Sunday, August 14, 1994 1:08:47 AM- go nuts

Tuesday, December 6, 1994 10:38:50 PM  (Jason)
	choose closest sample rate on target device to rate22khz.

Feb 1, 2003 (Woody Zenfell):
        Resurrected for A1.  Reworked to be more tolerant of different supported
        capture rates and formats, thanks to network_microphone_shared.cpp.
        Made interface match that of the SDL network microphone stuff.

 May 28, 2003 (Gregory Smith):
	Speex audio compression
*/

#if !defined(DISABLE_NETWORKING)

#include <string.h>
#include <stdlib.h>

// ZZZ: trying to get this Mac OS code to compile in the SDL context for Mac OS X SDL version.
#ifdef SDL
#define SDL_RFORK_HACK
#endif

#include "cseries.h"

/*
#if !defined(TARGET_API_MAC_CARBON) && !defined(SDL)
#include <SoundInput.h>
#include <AIFF.h>
#include <Gestalt.h>
#else
#include <Carbon/Carbon.h>
#endif
*/

#include "shell.h"
#include "network_sound.h"
#include "interface.h"
#include "network_data_formats.h"
#include "network_microphone_shared.h"

#ifdef SPEEX
#include "network_speex.h"
#define MAC_OPTIMAL_MIC_BUFFER_SIZE 8192 * 2
#endif SPEEX

#ifdef env68k
#pragma segment sound
#endif

/* Notes: */
/* Need to implement delay for maximum recording time.. */
/* Possible scale data up at interrupt time.. */

/* ---------- constants */

/* ---------- structures */
struct sound_device_settings 
{
	short num_channels;
	UnsignedFixed sample_rate;
	short sample_size;
	OSType compression_type;
};

struct net_microphone_data 
{
	long refnum;
	int32 device_internal_buffer_size;
	struct sound_device_settings initial_settings;
	SICompletionUPP completion_proc;
	Ptr buffer;
	struct SPB param_block;
	bool recording;
};

/* -------- constants */

struct sound_device_settings game_mic_settings=
{
	1, /* number of channels */
        rate11025hz, /* sample rate */
	8, /* sample size */
        0 /* compression type (0 = no compression) */
};

/* -------- Globals.. */
static bool net_microphone_installed= false;
struct net_microphone_data net_microphone;

/* ------------- local prototypes */

static OSErr set_device_settings(long refnum, struct sound_device_settings *settings);
static OSErr get_device_settings(long refnum, struct sound_device_settings *settings);
static pascal void sound_recording_completed(SPBPtr pb);
static OSErr start_sound_recording(void);

static OSErr closest_supported_sample_rate(long refNum, UnsignedFixed *sampleRate);

/* ------------- Code */

bool is_network_microphone_implemented()
{
        return true;
}

void network_microphone_idle_proc()
{
        // nothing to do
}

bool has_sound_input_capability(
	void)
{
	long gestalt_flags;
	bool is_capable= false;
	OSErr err;

	err= Gestalt(gestaltSoundAttr, &gestalt_flags);
	if(!err)
	{
		if(gestalt_flags & gestaltHasSoundInputDevice)
		{
			is_capable= true;
		}
	}

        return is_capable;
}

OSErr open_network_microphone()
{
	OSErr err;

	/* do things we only do once */
	{
		static bool initialization= false;
		
		if (!initialization)
		{
			atexit(close_network_microphone);
			
			initialization= true;
		}
	}
        
        #ifdef SPEEX
	init_speex_encoder();
        #endif SPEEX

	if (!net_microphone_installed)
	{
		if (has_sound_input_capability())
		{
			net_microphone.recording= false;
			net_microphone.completion_proc= NewSICompletionUPP(sound_recording_completed);
			
			if (net_microphone.completion_proc)
			{
				err= SPBOpenDevice("\p", siWritePermission, &net_microphone.refnum);
				if(!err)
				{
					err= get_device_settings(net_microphone.refnum, &net_microphone.initial_settings);
					if(!err)
					{
						/* Get the internal buffer size.. */
						err= SPBGetDeviceInfo(net_microphone.refnum, siDeviceBufferInfo,
							 &net_microphone.device_internal_buffer_size);
						if(!err)
						{
							/* Set to our settings.. */
							closest_supported_sample_rate(net_microphone.refnum, &game_mic_settings.sample_rate);
							err= set_device_settings(net_microphone.refnum, (struct sound_device_settings *) &game_mic_settings);

                                                        // ZZZ: some settings may have failed to 'take'.  That's ok, we'll double-check the
                                                        // settings to get an accurate reading, and deal with it.
                                                        if(err == notEnoughHardwareErr)
                                                                err = noErr;
                                                                
							if(err == noErr)
							{
                                                                get_device_settings(net_microphone.refnum, &game_mic_settings);
                                                                if(!announce_microphone_capture_format(game_mic_settings.sample_rate / 65536,
                                                                        game_mic_settings.num_channels == 2, game_mic_settings.sample_size == 16))
                                                                {
                                                                        // network microphone support layer rejected our capture format!
                                                                        err = notEnoughHardwareErr;
                                                                }
                                                                else {
#ifdef SPEEX
net_microphone.buffer = new char[MAC_OPTIMAL_MIC_BUFFER_SIZE];
#endif
                                                                        if(!net_microphone.buffer)
                                                                        {
                                                                                err = notEnoughMemoryErr;
                                                                        }
                                                                        else {
                                                                                net_microphone_installed= true;
                                                                        }
                                                                }
							}
						}
					}
					
					/* Had a problem.  Close it down. */
					if(err) SPBCloseDevice(net_microphone.refnum);
				}
			}
			else
			{
				err= MemError();
			}
		}
		else
		{
			err= noHardware; 
		}
	}

	return err;
}

void close_network_microphone(void)
{
	OSErr err;
        
	if(net_microphone_installed)
	{
		/* Is the completion routine called immediately, or can the application quit before */
		/*  it is called??? */
		if(net_microphone.recording)
		{
			err= SPBStopRecording(net_microphone.refnum);
		}
	
		err= SPBCloseDevice(net_microphone.refnum);

		/* Get rid of the routine descriptor */
		DisposeSICompletionUPP(net_microphone.completion_proc);
                
                if(net_microphone.buffer != NULL) {
                    delete [] net_microphone.buffer;
                    net_microphone.buffer = NULL;
                }

		net_microphone_installed= false;
	}
        
        #ifdef SPEEX
            destroy_speex_encoder();
        #endif SPEEX

}

void set_network_microphone_state(bool triggered)
{
	OSErr err= noErr;
	
	if(net_microphone_installed)
	{
		if(triggered)
		{
			if(!net_microphone.recording)
			{
				err= start_sound_recording();
				
				if(!err)
				{
					/* Tell us we are starting to record.. */
					net_microphone.recording= true;
				}
			} else {
				/* We are recording.... restart the recording if we are done. */
                                if(net_microphone.param_block.error < 0)
                                {
                                        dprintf("Error in completion: %d", net_microphone.param_block.error);
                                        net_microphone.recording= false;
				}
			}
		} else {
			if(net_microphone.recording)
			{
				/* They just stopped.  Finish up.. */
				err= SPBStopRecording(net_microphone.refnum);
				net_microphone.recording= false;
			}
		}
	}

	if(err) dprintf("Err in handle microphone: %d", err);	
}

/* -------- private functions */

static OSErr get_device_settings(
	long refnum,
	struct sound_device_settings *settings)
{
	OSErr error;

	error= SPBGetDeviceInfo(refnum, siNumberChannels, &settings->num_channels);
	if (error==noErr)
	{
		error= SPBGetDeviceInfo(refnum, siSampleRate, &settings->sample_rate);
		if (error==noErr)
		{
			error= SPBGetDeviceInfo(refnum, siSampleSize, &settings->sample_size);
			if (error==noErr)
			{
				error= SPBGetDeviceInfo(refnum, siCompressionType, &settings->compression_type);
			}
		}
	}
	
	return error;
}

// ZZZ change: set as much as we can; if we attempt any invalid settings, return notEnoughHardwareErr.
static OSErr set_device_settings(
	long refnum,
	struct sound_device_settings *settings)
{
	OSErr error;
        bool notEnoughHardware = false;
	
	error= SPBSetDeviceInfo(refnum, siNumberChannels, &settings->num_channels);
        if(error == notEnoughHardwareErr) {
            notEnoughHardware = true;
            error = noErr;
        }
	if (error==noErr)
	{
		error= SPBSetDeviceInfo(refnum, siSampleRate, &settings->sample_rate);
                if(error == notEnoughHardwareErr) {
                    notEnoughHardware = true;
                    error = noErr;
                }
		if (error==noErr)
		{
			error= SPBSetDeviceInfo(refnum, siSampleSize, &settings->sample_size);
                        if(error == notEnoughHardwareErr) {
                            notEnoughHardware = true;
                            error = noErr;
                        }
			if (error==noErr)
			{
				error= SPBSetDeviceInfo(refnum, siCompressionType, &settings->compression_type);
                                if(error == notEnoughHardwareErr) {
                                    notEnoughHardware = true;
                                    error = noErr;
                                }
			}
		}
	}
	
	return (error == noErr && notEnoughHardware) ? notEnoughHardwareErr : error;
}


// ZZZ: some changes here to account for packing problems
struct siSampleRateAvailableData
{
	short sampleRateCount;
	Handle sampleRates;
};

enum { SIZEOF_siSampleRateAvailableData = 2 + 4 };

static OSErr closest_supported_sample_rate(
	long refNum,
	UnsignedFixed *sampleRate)
{
	struct siSampleRateAvailableData data;
        byte sampleRateBuffer[SIZEOF_siSampleRateAvailableData];
	OSErr error;
	
	error= SPBGetDeviceInfo(refNum, siSampleRateAvailable, sampleRateBuffer);
	if (error==noErr)
	{
                data.sampleRateCount = *((short*)(&(sampleRateBuffer[0])));
                data.sampleRates = *((Handle*)(&(sampleRateBuffer[2])));
		UnsignedFixed *sampleRates= (UnsignedFixed *) *data.sampleRates;
		
		if (!data.sampleRateCount)
		{
			/* clip to continuous range */
			if (*sampleRate<sampleRates[0]) *sampleRate= sampleRates[0];
			if (*sampleRate>sampleRates[1]) *sampleRate= sampleRates[1];
		}
		else
		{
			UnsignedFixed closestRate;
			uint32 closest_delta= 0;
			short i;

			/* find closest */
			for (i= 0; i<data.sampleRateCount; ++i)
			{
				uint32 delta= ((uint32)sampleRates[i]>(uint32)*sampleRate) ?
					((uint32)sampleRates[i]-(uint32)*sampleRate) : 
					((uint32)*sampleRate-(uint32)sampleRates[i]);
				
				if (!i || delta<closest_delta) closestRate= sampleRates[i], closest_delta= delta;
			}
			
			*sampleRate= closestRate;
		}
		
		DisposeHandle(data.sampleRates);
	}
	
	return error;
}

static OSErr start_sound_recording(
	void)
{
	/* This is a first time start recording.. */
	obj_clear(net_microphone.param_block);

	net_microphone.param_block.inRefNum= net_microphone.refnum;
#ifdef SPEEX
            net_microphone.param_block.count = MAC_OPTIMAL_MIC_BUFFER_SIZE;
#endif
	net_microphone.param_block.milliseconds= 0;
#ifdef SPEEX
            net_microphone.param_block.bufferLength = MAC_OPTIMAL_MIC_BUFFER_SIZE;
#endif
	net_microphone.param_block.bufferPtr= net_microphone.buffer;
	net_microphone.param_block.completionRoutine= net_microphone.completion_proc;
#ifdef env68k								
	net_microphone.param_block.userLong= (long) get_a5();;
#endif

	return SPBRecord(&net_microphone.param_block, true);
}

/* -------- Completion callback function.. */

static pascal void sound_recording_completed(
	SPBPtr pb)
{
#ifdef env68k
	long old_a5= set_a5(pb->userLong); /* set our a5 world */
#endif

        // Whatever we've got, convert and send it out
        copy_and_send_audio_data((byte*)net_microphone.buffer, pb->count, NULL, 0, true);

	switch (pb->error)
	{
		case noErr:
			/* Reset the sounds.. */
#ifdef SPEEX
                            pb->count = MAC_OPTIMAL_MIC_BUFFER_SIZE;
#endif
			pb->milliseconds= 0;
#ifdef SPEEX
                            pb->bufferLength = MAC_OPTIMAL_MIC_BUFFER_SIZE;
#endif SPEEX
			pb->bufferPtr= net_microphone.buffer;
			pb->completionRoutine= net_microphone.completion_proc;

			/* Respawn the recording! */
			pb->error= SPBRecord(pb, true);
			break;
			
		case abortErr:
			break;
			
		default:
			break;
	}

#ifdef env68k
	set_a5(old_a5); /* restore our a5 world */
#endif
}

#endif // !defined(DISABLE_NETWORKING)


/*
NETWORK_MICROPHONE.C
Sunday, August 14, 1994 1:08:47 AM- go nuts

Tuesday, December 6, 1994 10:38:50 PM  (Jason)
	choose closest sample rate on target device to rate22khz.
*/

#include <string.h>
#include <stdlib.h>

#include "macintosh_cseries.h"

#include <SoundInput.h>
#include <AIFF.h>
#include <Gestalt.h>

#include "macintosh_network.h"
#include "shell.h"
#include "network_sound.h"
#include "interface.h"

#ifdef env68k
#pragma segment sound
#endif

/* Notes: */
/* Need to implement delay for maximum recording time.. */
/* Possible scale data up at interrupt time.. */

//#define TESTING

/* ---------- constants */

/* ---------- structures */
struct sound_device_settings 
{
	short num_channels;
	Fixed sample_rate;
	short sample_size;
	OSType compression_type;
};

struct net_microphone_data 
{
	long refnum;
	long device_internal_buffer_size;
	struct sound_device_settings initial_settings;
	SICompletionUPP completion_proc;
	Ptr buffer;
	struct SPB param_block;
	boolean recording;
	short network_distribution_type;
};

/* -------- constants */

struct sound_device_settings game_mic_settings=
{
	1, /* number of channels */
	rate22khz, /* sample rate */
	8, /* sample size */
	MACE6Type /* MACE 6:1 */
};

/* -------- Globals.. */
static boolean net_microphone_installed= FALSE;
struct net_microphone_data net_microphone;

/* ------------- local prototypes */

static OSErr set_device_settings(long refnum, struct sound_device_settings *settings);
static OSErr get_device_settings(long refnum, struct sound_device_settings *settings);
static pascal void sound_recording_completed(SPBPtr pb);
static OSErr start_sound_recording(void);

static OSErr closest_supported_sample_rate(long refNum, Fixed *sampleRate);

/* ------------- Code */

boolean has_sound_input_capability(
	void)
{
	long gestalt_flags;
	boolean is_capable= FALSE;
	OSErr err;

	err= Gestalt(gestaltSoundAttr, &gestalt_flags);
	if(!err)
	{
		if(gestalt_flags & gestaltHasSoundInputDevice)
		{
			is_capable= TRUE;
		}
	}

	return is_capable;
}

OSErr open_network_microphone(
	short network_distribution_type)
{
	OSErr err;

	/* do things we only do once */
	{
		static boolean initialization= FALSE;
		
		if (!initialization)
		{
			atexit(close_network_microphone);
			
			initialization= TRUE;
		}
	}

	if (!net_microphone_installed)
	{
		if (has_sound_input_capability())
		{
			net_microphone.recording= FALSE;
			net_microphone.network_distribution_type= network_distribution_type;
			net_microphone.buffer= NewPtr(NETWORK_SOUND_CHUNK_BUFFER_SIZE);
			net_microphone.completion_proc= NewSICompletionProc(sound_recording_completed);
			
			if (net_microphone.buffer && net_microphone.completion_proc)
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
							if(!err)
							{
								net_microphone_installed= TRUE;
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
		DisposeRoutineDescriptor(net_microphone.completion_proc);

		net_microphone_installed= FALSE;
	}
}

void handle_microphone(boolean triggered)
{
	OSErr err= noErr;
	boolean success= FALSE;
	
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
					net_microphone.recording= TRUE;
				}
			} else {
				/* We are recording.... restart the recording if we are done. */
				switch(net_microphone.param_block.error)
				{
					case asyncUncompleted:
					case noErr:
						/* Start up the next one.. */
						/* No error.. */
						break;
						
					default:
						dprintf("Error in completion: %d", net_microphone.param_block.error);
						net_microphone.recording= FALSE;
						break;
				}
			}
		} else {
			if(net_microphone.recording)
			{
				/* They just stopped.  Finish up.. */
				err= SPBStopRecording(net_microphone.refnum);
				net_microphone.recording= FALSE;
			}
		}
	}

	if(err) dprintf("Err in handle microphone: %d", err);	
	
	return;
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

static OSErr set_device_settings(
	long refnum,
	struct sound_device_settings *settings)
{
	OSErr error;
	
	error= SPBSetDeviceInfo(refnum, siNumberChannels, &settings->num_channels);
	if (error==noErr)
	{
		error= SPBSetDeviceInfo(refnum, siSampleRate, &settings->sample_rate);
		if (error==noErr)
		{
			error= SPBSetDeviceInfo(refnum, siSampleSize, &settings->sample_size);
			if (error==noErr)
			{
				error= SPBSetDeviceInfo(refnum, siCompressionType, &settings->compression_type);
			}
		}
	}
	
	return error;
}

struct siSampleRateAvailableData
{
	short sampleRateCount;
	Handle sampleRates;
};

static OSErr closest_supported_sample_rate(
	long refNum,
	Fixed *sampleRate)
{
	struct siSampleRateAvailableData data;
	OSErr error;
	
	error= SPBGetDeviceInfo(refNum, siSampleRateAvailable, &data);
	if (error==noErr)
	{
		Fixed *sampleRates= (Fixed *) *data.sampleRates;
		
		if (!data.sampleRateCount)
		{
			/* clip to continuous range */
			if (*sampleRate<sampleRates[0]) *sampleRate= sampleRates[0];
			if (*sampleRate>sampleRates[1]) *sampleRate= sampleRates[1];
		}
		else
		{
			Fixed closestRate;
			unsigned long closest_delta= 0;
			short i;

			/* find closest */
			for (i= 0; i<data.sampleRateCount; ++i)
			{
				unsigned long delta= ((unsigned long)sampleRates[i]>(unsigned long)*sampleRate) ?
					((unsigned long)sampleRates[i]-(unsigned long)*sampleRate) : 
					((unsigned long)*sampleRate-(unsigned long)sampleRates[i]);
				
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
	memset(&net_microphone.param_block, 0, sizeof(struct SPB));

	net_microphone.param_block.inRefNum= net_microphone.refnum;
	net_microphone.param_block.count= NETWORK_SOUND_CHUNK_BUFFER_SIZE;
	net_microphone.param_block.milliseconds= 0;
	net_microphone.param_block.bufferLength= NETWORK_SOUND_CHUNK_BUFFER_SIZE;
	net_microphone.param_block.bufferPtr= net_microphone.buffer;
	net_microphone.param_block.completionRoutine= net_microphone.completion_proc;
#ifdef env68k								
	net_microphone.param_block.userLong= (long) get_a5();;
#endif

	return SPBRecord(&net_microphone.param_block, TRUE);
}

/* -------- Completion callback function.. */

static pascal void sound_recording_completed(
	SPBPtr pb)
{
#ifdef env68k
	long old_a5= set_a5(pb->userLong); /* set our a5 world */
#endif

	/* Send the data, or play it if we are just testing.. */
#ifdef TESTING
	queue_network_speaker_data(net_microphone.buffer, pb->count);
#else
	NetDistributeInformation(net_microphone.network_distribution_type, net_microphone.buffer, pb->count, FALSE);
#endif

	switch (pb->error)
	{
		case noErr:
			/* Reset the sounds.. */
			pb->count= NETWORK_SOUND_CHUNK_BUFFER_SIZE;
			pb->milliseconds= 0;
			pb->bufferLength= NETWORK_SOUND_CHUNK_BUFFER_SIZE;
			pb->bufferPtr= net_microphone.buffer;
			pb->completionRoutine= net_microphone.completion_proc;

			/* Respawn the recording! */
			pb->error= SPBRecord(pb, TRUE);
			break;
			
		case abortErr:
			break;
			
		default:
			break;
	}
	
#ifdef env68k
	set_a5(old_a5); /* restore our a5 world */
#endif

	return;
}

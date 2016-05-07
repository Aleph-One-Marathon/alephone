/*
 *  network_microphone_core_audio.cpp

	Copyright (C) 2007 and beyond by Gregory Smith
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

 */

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>

#include "cstypes.h"
#include "network_microphone_shared.h"

#include <vector>

#ifdef SPEEX
extern void init_speex_encoder();
extern void destroy_speex_encoder();
#endif

static AudioUnit fAudioUnit;
static AudioDeviceID fInputDeviceID;
static AudioStreamBasicDescription fOutputFormat, fDeviceFormat;
static Uint32 fAudioSamples;
static AudioBufferList *fAudioBuffer = NULL;

static bool initialized = false;

static std::vector<uint8> captureBuffer;
static Uint32 captureBufferSize = 0;

static OSStatus audio_input_proc(void *, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *)
{
	// render to a buffer
	OSStatus err = noErr;
	err = AudioUnitRender(fAudioUnit, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, fAudioBuffer);
	if (err != noErr)
	{
		fprintf(stderr, "AudioUnitRender() failed with error %i\n", err);
	} 
	else
	{
		// copy to the capture buffer
		memcpy(&captureBuffer[captureBufferSize], fAudioBuffer->mBuffers[0].mData, inNumberFrames * 2);
		captureBufferSize += inNumberFrames * 2;
		if (captureBufferSize >= get_capture_byte_count_per_packet())
		{
			copy_and_send_audio_data(&captureBuffer.front(), captureBufferSize, NULL, 0, true);
			captureBufferSize = 0;
		}
	}

	return err;
}

OSErr open_network_microphone() 
{
	OSStatus err = noErr;

	// find an AudioOutputUnit (for input)
	AudioComponent component;
	AudioComponentDescription description;
	
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_HALOutput;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
	if ((component = AudioComponentFindNext(NULL, &description)))
	{
		err = AudioComponentInstanceNew(component, &fAudioUnit);
		if (err != noErr)
		{
			fAudioUnit = NULL;
			return err;
		}
	}

	// configure the AudioOutputUnit 
	UInt32 param = 1;
	
	// enable input on the AUHAL
	err = AudioUnitSetProperty(fAudioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &param, sizeof(UInt32));
	if (err == noErr)
	{
		// disable output on the AUHAL
		param = 0;
		err = AudioUnitSetProperty(fAudioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &param, sizeof(UInt32));
	}

	// Select the default input device
	param = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress inDevAddress = { 
		kAudioHardwarePropertyDefaultInputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster };
	err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &inDevAddress, 0, NULL, &param, &fInputDeviceID);
	if (err != noErr)
	{
		fprintf(stderr, "failed to get default input device\n");
		return err;
	}

	const Float64 sampleRates[] = {
		8000.0,
		48000.0,
		44100.0,
		22050.0,
		11025.0
	};

	Float64 sampleRate;

	AudioObjectPropertyAddress sampleRateAddress = {
		kAudioDevicePropertyNominalSampleRate,
		kAudioDevicePropertyScopeOutput,
		kAudioObjectPropertyElementMaster };
	for (int i = 0; i < sizeof(sampleRates) / sizeof(Float64); i++)
	{
		sampleRate = sampleRates[i];
		err = AudioObjectSetPropertyData(fInputDeviceID, &sampleRateAddress, 0, NULL, sizeof(Float64), &sampleRate);
		if (err == noErr)
			break;
	}

	if (err != noErr)
	{
		fprintf(stderr, "failed to set AU sample rate (%i)\n", err);
		return err;
	}

	// Set the current device to the default input device
	err = AudioUnitSetProperty(fAudioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &fInputDeviceID, sizeof(AudioDeviceID));
	if (err != noErr)
	{
		fprintf(stderr, "failed to set AU input device\n");
		return err;
	}

	// setup render callback
	AURenderCallbackStruct callback;
	callback.inputProc = audio_input_proc;
	err = AudioUnitSetProperty(fAudioUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &callback, sizeof(AURenderCallbackStruct));
	
	// get hardware device format
	param = sizeof(AudioStreamBasicDescription);
	err = AudioUnitGetProperty(fAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &fDeviceFormat, &param);
	if (err != noErr)
	{
		fprintf(stderr, "failed to get input device ASBD\n");		return err;
	}

	// change the format to our liking
	fOutputFormat.mChannelsPerFrame = 1;
	fOutputFormat.mSampleRate = fDeviceFormat.mSampleRate;
	fOutputFormat.mFormatID = kAudioFormatLinearPCM;
	fOutputFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
#ifdef __ppc__
	fOutputFormat.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
	fOutputFormat.mBitsPerChannel = 16;
	fOutputFormat.mBytesPerFrame = 2;
	fOutputFormat.mFramesPerPacket = 1;
	fOutputFormat.mBytesPerPacket = fOutputFormat.mBytesPerFrame;

	err = AudioUnitSetProperty(fAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &fOutputFormat, sizeof(AudioStreamBasicDescription));
	if (err != noErr)
	{
		fprintf(stderr, "failed to set input device ASBD\n");
		return err;
	}

	// Get the number of frames in the IO buffer(s)
	param = sizeof(UInt32);
	err = AudioUnitGetProperty(fAudioUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &fAudioSamples, &param);
	if (err != noErr)
	{
		fprintf(stderr, "failed to get audio sample size\n");
		return err;
	}

	err = AudioUnitInitialize(fAudioUnit);
	if (err != noErr)
	{
		fprintf(stderr, "failed to initialize AU %i\n", err);
		return err;
	}

	// Allocate audio buffer
	fAudioBuffer = (AudioBufferList *) calloc (1, sizeof(AudioBufferList) + sizeof(AudioBuffer));
	fAudioBuffer->mNumberBuffers = 1;
	fAudioBuffer->mBuffers[0].mNumberChannels = 1;
	fAudioBuffer->mBuffers[0].mDataByteSize = fAudioSamples * fOutputFormat.mBytesPerFrame;
	fAudioBuffer->mBuffers[0].mData = malloc(fAudioSamples * fOutputFormat.mBytesPerFrame);

	if (!announce_microphone_capture_format(static_cast<uint32>(fOutputFormat.mSampleRate), fOutputFormat.mChannelsPerFrame == 2, fOutputFormat.mBytesPerFrame == 2))
	{
		fprintf(stderr, "network microphone support code rejected audio format (rate=%f)\n", fOutputFormat.mSampleRate);
		return -1;
	}

	captureBuffer.resize(get_capture_byte_count_per_packet() + fAudioBuffer->mBuffers[0].mDataByteSize);

#ifdef SPEEX
	init_speex_encoder();
#endif

	initialized = true;

	return noErr;
}

static bool mic_active = false;

void set_network_microphone_state(bool inActive)
{
	if (!initialized) return;

	if (inActive && !mic_active)
	{
		AudioOutputUnitStart(fAudioUnit);
		mic_active = true;
	} 
	else if (!inActive && mic_active)
	{
		AudioOutputUnitStop(fAudioUnit);
		mic_active = false;
	}
}

void close_network_microphone()
{
	initialized = false;
	if (fAudioBuffer)
		free(fAudioBuffer->mBuffers[0].mData);
	free(fAudioBuffer);
	fAudioBuffer = NULL;

#ifdef SPEEX
	destroy_speex_encoder();
#endif
}

bool is_network_microphone_implemented() {
	return true;
}

void network_microphone_idle_proc()
{
	// do nothing
}


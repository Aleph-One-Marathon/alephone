/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

*/

#include "Mixer.h"
#include "interface.h" // for strERRORS

Mixer* Mixer::m_instance = 0;

extern bool option_nosound;

Mixer::Header::Header() : 
	sixteen_bit(false),
	stereo(false),
	signed_8bit(false),
	bytes_per_frame(0),
	data(0),
	length(0),
	loop(0),
	loop_length(0),
	rate(FIXED_ONE),
	little_endian(false)
{
}

Mixer::Header::Header(const SoundHeader& header) :
	sixteen_bit(header.sixteen_bit),
	stereo(header.stereo),
	signed_8bit(header.signed_8bit),
	bytes_per_frame(header.bytes_per_frame),
	little_endian(header.little_endian),
	data(header.Data()),
	length(header.Length()),
	loop(header.Data() + header.loop_start),
	loop_length(header.loop_end - header.loop_start),
	rate(header.rate)
{
}

void Mixer::Start(uint16 rate, bool sixteen_bit, bool stereo, int num_channels, int volume, uint16 samples)
{
	sound_channel_count = num_channels;
	main_volume = volume;
	desired.freq = rate;
	desired.format = sixteen_bit ? AUDIO_S16SYS : AUDIO_S8;
	desired.channels = stereo ? 2 : 1;
	desired.samples = samples;
	desired.callback = MixerCallback;
	desired.userdata = reinterpret_cast<void *>(this);

	if (option_nosound || SDL_OpenAudio(&desired, &obtained) < 0) 
	{
		if (!option_nosound)
			// opening audio failed
			alert_user(infoError, strERRORS, badSoundChannels, -1);
		sound_channel_count = 0;
		channels.clear();
	}
	else 
	{
		// initialize the channels
		channels.resize(num_channels + EXTRA_CHANNELS);
		SDL_PauseAudio(false);
	}
}

void Mixer::Stop()
{
	SDL_CloseAudio();
	channels.clear();
	sound_channel_count = 0;
}

void Mixer::BufferSound(int channel, const Header& header, _fixed pitch)
{
	SDL_LockAudio();
	if (channels[channel].active)
	{
		// queue the header
		channels[channel].BufferSoundHeader(header, pitch);
	} else {
		// load it directly
		channels[channel].active = true;
		channels[channel].LoadSoundHeader(header, pitch);
	}
	SDL_UnlockAudio();
}

void Mixer::MixerCallback(void *usr, uint8 *stream, int len)
{
	reinterpret_cast<Mixer *>(usr)->Callback(stream, len);
}

void Mixer::Callback(uint8 *stream, int len)
{
	bool stereo = (obtained.channels == 2);
	if ((obtained.format & 0xff) == 16) 
	{
		if (stereo)
			Mix<int16, true, true>(reinterpret_cast<int16 *>(stream), len / 4);
		else
			Mix<int16, false, true>(reinterpret_cast<int16 *>(stream), len / 2);
	} else if (obtained.format & 0x1000) {
		if (stereo)
			Mix<int16, true, true>(reinterpret_cast<int16 *>(stream), len / 2);
		else
			Mix<int16, false, true>(reinterpret_cast<int16 *>(stream), len);
	} else {
		if (stereo)
			Mix<int8, true, false>(reinterpret_cast<int8 *>(stream), len / 2);
		else
			Mix<int8, false, false>(reinterpret_cast<int8 *>(stream), len);
	}
}

void Mixer::StartMusicChannel(bool sixteen_bit, bool stereo, bool signed_8bit, int bytes_per_frame, _fixed rate, bool little_endian)
{
	Channel *c = &channels[sound_channel_count + MUSIC_CHANNEL];
	c->sixteen_bit = sixteen_bit;
	c->stereo = stereo;
	c->signed_8bit = signed_8bit;
	c->little_endian = little_endian;
	c->bytes_per_frame = bytes_per_frame;
	c->counter = 0;
	c->rate = rate;
	c->left_volume = c->right_volume = 0x100;
	c->active = true;
	c->loop_length = 0;
}

void Mixer::UpdateMusicChannel(uint8* data, int len)
{
	Channel *c = &channels[sound_channel_count + MUSIC_CHANNEL];
	c->data = data;
	c->length = len;
}

// ZZZ: realtime microphone stuff
// Is the locking necessary?  If checking c->active is the first thing the interrupt proc does,
// but setting c->active is the last thing we do, there's no way we can get mixed up, right?
void Mixer::EnsureNetworkAudioPlaying()
{
#if !defined(DISABLE_NETWORKING)
	if (!channels.size()) return;
	Channel *c = &channels[sound_channel_count + NETWORK_AUDIO_CHANNEL];
	if (!c->active)
	{
		sNetworkAudioBufferDesc = dequeue_network_speaker_data();

		if (sNetworkAudioBufferDesc)
		{
			SDL_LockAudio();
			c->stereo = kNetworkAudioIsStereo;
			c->sixteen_bit = kNetworkAudioIs16Bit;
			c->signed_8bit = kNetworkAudioIsSigned8Bit;
			c->bytes_per_frame = kNetworkAudioBytesPerFrame;
			c->data = sNetworkAudioBufferDesc->mData;
			c->length = sNetworkAudioBufferDesc->mLength;
			c->loop_length = 0;
			c->rate = (kNetworkAudioSampleRate << 16) / obtained.freq;
#ifdef ALEPHONE_LITTLE_ENDIAN
			c->little_endian = true;
#else
			c->little_endian = false;
#endif
			c->left_volume = 0x100;
			c->right_volume = 0x100;
			c->counter = 0;
			c->active = true;

			SDL_UnlockAudio();
		}
	}
#endif
}

// I can see locking here because we're invalidating some storage, and we want to
// make sure the play routine does not trail on a little bit using that storage.
void Mixer::StopNetworkAudio()
{
#if !defined(DISABLE_NETWORKING)
	if (!channels.size()) return;
	SDL_LockAudio();
	channels[sound_channel_count + NETWORK_AUDIO_CHANNEL].active = false;
	if (sNetworkAudioBufferDesc)
	{
		if (is_sound_data_disposable(sNetworkAudioBufferDesc))
			release_network_speaker_buffer(sNetworkAudioBufferDesc->mData);
		sNetworkAudioBufferDesc = 0;
	}
	SDL_UnlockAudio();
#endif
}

void Mixer::PlaySoundResource(LoadedResource &rsrc)
{
	if (!channels.size()) return;
	Channel *c = &channels[sound_channel_count + RESOURCE_CHANNEL];
	// Open stream to resource
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int)rsrc.GetLength());
	if (p == NULL)
		return;

	// Get resource format
	uint16 format = SDL_ReadBE16(p);
	if (format != 1 && format != 2) {
		fprintf(stderr, "Unknown sound resource format %d\n", format);
		SDL_RWclose(p);
		return;
	}

	// Skip sound data types or reference count
	if (format == 1) {
		uint16 num_data_formats = SDL_ReadBE16(p);
		SDL_RWseek(p, num_data_formats * 6, SEEK_CUR);
	} else if (format == 2)
		SDL_RWseek(p, 2, SEEK_CUR);

	// Lock sound subsystem
	SDL_LockAudio();

	// Scan sound commands for bufferCmd
	uint16 num_cmds = SDL_ReadBE16(p);
	for (int i=0; i<num_cmds; i++) {
		uint16 cmd = SDL_ReadBE16(p);
		uint16 param1 = SDL_ReadBE16(p);
		uint32 param2 = SDL_ReadBE32(p);
		//printf("cmd %04x %04x %08x\n", cmd, param1, param2);

		if (cmd == 0x8051) {

			SoundHeader header;
			header.Load((uint8 *) rsrc.GetPointer() + param2);
			c->active = true;
			c->LoadSoundHeader(header, FIXED_ONE);
			c->left_volume = c->right_volume = 0x100;
		}
	}

	// Unlock sound subsystem
	SDL_UnlockAudio();
	SDL_RWclose(p);
}

void Mixer::StopSoundResource()
{
	if (!channels.size()) return;
	SDL_LockAudio();
	channels[sound_channel_count + RESOURCE_CHANNEL].active = false;
	SDL_UnlockAudio();
}

Mixer::Channel::Channel() :
	active(false),
	sixteen_bit(false),
	stereo(false),
	signed_8bit(false),
	bytes_per_frame(0),
	little_endian(false),
	data(0),
	length(0),
	loop(0),
	loop_length(0),
	rate(0),
	counter(0),
	left_volume(0x100),
	right_volume(0x100),
	next_header(0),
	next_pitch(0)
{
}

void Mixer::Channel::LoadSoundHeader(const Header& header, _fixed pitch)
{
	sixteen_bit = header.sixteen_bit;
	stereo = header.stereo;
	signed_8bit = header.signed_8bit;
	bytes_per_frame = header.bytes_per_frame;
	little_endian = header.little_endian;
	data = header.data;
	length = header.length;
	loop = header.loop;
	loop_length = (header.loop_length >= 4) ? header.loop_length : 0;
	rate = (pitch >> 8) * ((header.rate >> 8) / instance()->obtained.freq);
	counter = 0;
}

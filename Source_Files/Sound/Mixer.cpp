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

Mixer* Mixer::m_instance = 0;

void Mixer::Start(_fixed pitch, bool sixteen_bit, bool stereo, int num_channels, int volume)
{
	sound_channel_count = num_channels;
	main_volume = volume;
	desired.freq = (pitch >> 16) * 22050;
	desired.format = sixteen_bit ? AUDIO_S16SYS : AUDIO_S8;
	desired.channels = stereo ? 2 : 1;
	desired.samples = 2048;
	desired.callback = MixerCallback;
	desired.userdata = reinterpret_cast<void *>(this);

	if (SDL_OpenAudio(&desired, &obtained) < 0) 
	{
		// opening audio failed
//		alert_user(infoError, strERRORS, badSoundChannels, -1);
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
}

void Mixer::BufferSound(int channel, uint8* header, _fixed pitch)
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

void Mixer::StartMusicChannel(bool sixteen_bit, bool stereo, bool signed_8bit, int bytes_per_frame, _fixed rate)
{
	Channel *c = &channels[sound_channel_count + MUSIC_CHANNEL];
	c->sixteen_bit = sixteen_bit;
	c->stereo = stereo;
	c->signed_8bit = signed_8bit;
	c->bytes_per_frame = bytes_per_frame;
	c->counter = 0;
	c->rate = rate;
	c->left_volume = c->right_volume = 0x100;
	c->active = true;
	c->loop_length = 0;
	c->pleft = c->pright = 0;
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
			c->left_volume = 0x100;
			c->right_volume = 0x100;
			c->counter = 0;
			c->active = true;
			c->PrimeInterpolator();

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

			// bufferCmd, load sound header and start channel
			c->active = true;
			c->LoadSoundHeader((uint8 *)rsrc.GetPointer() + param2, FIXED_ONE);
			c->left_volume = c->right_volume = 0x100;
		}
	}

	// Unlock sound subsystem
	SDL_UnlockAudio();
	SDL_RWclose(p);
}

void Mixer::StopSoundResource()
{
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

void Mixer::Channel::LoadSoundHeader(uint8 *data, _fixed pitch)
{
	// Open stream to header
	SDL_RWops *p = SDL_RWFromMem(data, 64);
	if (p == NULL) {
		active = false;
		return;
	}

	// Get sound header type, skip unused sample pointer
	uint8 header_type = data[20];
	SDL_RWseek(p, 4, SEEK_CUR);

	// Parse sound header
	bytes_per_frame = 1;
	signed_8bit = false;
	if (header_type == 0x00) {			// Standard sound header
		//printf("standard sound header\n");
		this->data = data + 22;
		sixteen_bit = stereo = false;
		length = SDL_ReadBE32(p);
		rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / instance()->obtained.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		loop = this->data + loop_start;
		loop_length = SDL_ReadBE32(p) - loop_start;
	} else if (header_type == 0xff || header_type == 0xfe) {	// Extended/compressed sound header
		//printf("extended/compressed sound header\n");
		this->data = data + 64;
		stereo = SDL_ReadBE32(p) == 2;
		if (stereo)
			bytes_per_frame *= 2;
		rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / instance()->obtained.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		loop = this->data + loop_start;
		loop_length = SDL_ReadBE32(p) - loop_start;
		SDL_RWseek(p, 2, SEEK_CUR);
		length = SDL_ReadBE32(p) * bytes_per_frame;
		if (header_type == 0xfe) {
			SDL_RWseek(p, 14, SEEK_CUR);
			uint32 format = SDL_ReadBE32(p);
			SDL_RWseek(p, 12, SEEK_CUR);
			int16 comp_id = SDL_ReadBE16(p);
			if (format != FOUR_CHARS_TO_INT('t', 'w', 'o', 's') || comp_id != -1) {
				fprintf(stderr, "Unsupported compressed sound header format '%c%c%c%c', ID %d\n", data[40], data[41], data[42], data[43], comp_id);
				active = false;
				SDL_RWclose(p);
				return;
			}
			SDL_RWseek(p, 4, SEEK_CUR);
		} else {
			SDL_RWseek(p, 22, SEEK_CUR);
		}
		sixteen_bit = (SDL_ReadBE16(p) == 16);
		if (sixteen_bit) {
			bytes_per_frame *= 2;
			length *= 2;
		}
	} else {							// Unknown header type
		fprintf(stderr, "Unknown sound header type %02x\n", header_type);
		active = false;
		SDL_RWclose(p);
		return;
	}

	// Correct loop count
	if (loop_length < 4)
		loop_length = 0;

	//printf(" data %p, length %d, loop %p, loop_length %d, rate %08x, stereo %d, 16 bit %d\n", c->data, c->length, c->loop, c->loop_length, c->rate, c->stereo, c->sixteen_bit);

	// Reset sample counter
	counter = 0;

	PrimeInterpolator();

	SDL_RWclose(p);	
}

void Mixer::Channel::PrimeInterpolator()
{
	int count = rate >> 16;
	if (length <= bytes_per_frame * count) 
	{
		// who would do this to us!?
		pleft = pright = 0;
		return;
	}

	data += bytes_per_frame * (count - 1);
	if (stereo) {
		if (sixteen_bit) {
			pleft = (int16)SDL_SwapBE16(0[(int16 *)data]);
			pright = (int16)SDL_SwapBE16(1[(int16 *)data]);
		} else if (signed_8bit) {
			pleft = (int32)(int8)(0[data]) * 256;
			pright = (int32)(int8)(1[data]) * 256;
		} else {
			pleft = (int32)(int8)(0[data] ^ 0x80) * 256;
			pright = (int32)(int8)(1[data] ^ 0x80) * 256;
		}
	} else {
		if (sixteen_bit)
			pleft = pright = (int16)SDL_SwapBE16(*(int16 *)data);
		else if (signed_8bit)
			pleft = pright = (int32)(int8)(*(data)) << 8;
		else
			pleft = pright = (int32)(int8)(*(data) ^ 0x80) << 8;
	}
	data += bytes_per_frame;
	length -= bytes_per_frame * count;
	counter += (rate & 0xffff);
}

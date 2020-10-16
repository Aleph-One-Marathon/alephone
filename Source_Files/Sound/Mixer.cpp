/*

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

*/

#include "Mixer.h"
#include "interface.h" // for strERRORS
#include "shell_options.h"

void Mixer::Start(uint16 rate, bool sixteen_bit, bool stereo, int num_channels, float db, uint16 samples)
{
	sound_channel_count = num_channels;
	main_volume = from_db(db);
	desired.freq = rate;
#if defined(__MACH__) && defined(__APPLE__)
	desired.format = sixteen_bit ? AUDIO_S16SYS : AUDIO_U8;
#else
	desired.format = sixteen_bit ? AUDIO_S16SYS : AUDIO_S8;
#endif
	desired.channels = stereo ? 2 : 1;
	desired.samples = samples;
	desired.callback = MixerCallback;
	desired.userdata = reinterpret_cast<void *>(this);

	if (shell_options.nosound || SDL_OpenAudio(&desired, &obtained) < 0) 
	{
		if (!shell_options.nosound)
			// opening audio failed
			alert_user(infoError, strERRORS, badSoundChannels, -1);
		sound_channel_count = 0;
		channels.clear();
	}
	else 
	{
		// initialize the channels
		channels.resize(num_channels + EXTRA_CHANNELS);
		for (int i = 0; i < num_channels + EXTRA_CHANNELS; ++i)
		{
			channels[i].sound_manager_index = i;
			channels[i].source = Channel::SOURCE_SOUND_HEADERS;
		}

		channels[sound_channel_count + MUSIC_CHANNEL].source = Channel::SOURCE_MUSIC;
		channels[sound_channel_count + RESOURCE_CHANNEL].source = Channel::SOURCE_RESOURCE;
		channels[sound_channel_count + NETWORK_AUDIO_CHANNEL].source = Channel::SOURCE_NETWORK_AUDIO;

		SDL_PauseAudio(false);
	}
}

void Mixer::Stop()
{
	SDL_CloseAudio();
	channels.clear();
	sound_channel_count = 0;
}

void Mixer::BufferSound(int channel, const SoundInfo& header, boost::shared_ptr<SoundData> data, _fixed pitch)
{
	SDL_LockAudio();
	if (channels[channel].active)
	{
		// queue the header
		channels[channel].BufferSoundHeader(header, data, pitch);
	} else {
		// load it directly
		channels[channel].active = true;
		channels[channel].LoadSoundHeader(header, data, pitch);
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
	bool is_sixteen_bit = ((obtained.format & 0xff) == 16);
	bool is_signed = obtained.format & 0x8000;
	int samples = len / (stereo ? 2 : 1) / (is_sixteen_bit ? 2 : 1);

	Mix(stream, samples, stereo, is_sixteen_bit, is_signed);
}

void Mixer::StartMusicChannel(bool sixteen_bit, bool stereo, bool signed_8bit, int bytes_per_frame, _fixed rate, bool little_endian)
{
	Channel *c = &channels[sound_channel_count + MUSIC_CHANNEL];
	c->info.sixteen_bit = sixteen_bit;
	c->info.stereo = stereo;
	c->info.signed_8bit = signed_8bit;
	c->info.little_endian = little_endian;
	c->info.bytes_per_frame = bytes_per_frame;
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
			c->info.stereo = kNetworkAudioIsStereo;
			c->info.sixteen_bit = kNetworkAudioIs16Bit;
			c->info.signed_8bit = kNetworkAudioIsSigned8Bit;
			c->info.bytes_per_frame = kNetworkAudioBytesPerFrame;
			c->data = sNetworkAudioBufferDesc->mData;
			c->length = sNetworkAudioBufferDesc->mLength;
			c->loop_length = 0;
			c->rate = (kNetworkAudioSampleRate << 16) / obtained.freq;
			c->info.little_endian = PlatformIsLittleEndian();
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

void Mixer::PlaySoundResource(LoadedResource &rsrc, _fixed pitch)
{
	if (!channels.size()) return;

	Channel *c = &channels[sound_channel_count + RESOURCE_CHANNEL];

	SoundHeader header;
	if (header.Load(rsrc))
	{
		boost::shared_ptr<SoundData> data = header.LoadData(rsrc);
		if (data.get())
		{
			SDL_LockAudio();
			c->active = true;
			c->LoadSoundHeader(header, data, pitch);
			c->left_volume = c->right_volume = 0x100;
			SDL_UnlockAudio();
		}
	}
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
	data(0),
	length(0),
	loop(0),
	loop_length(0),
	rate(0),
	counter(0),
	left_volume(0x100),
	right_volume(0x100),
	next_pitch(0)
{
}

void Mixer::Channel::LoadSoundHeader(const SoundInfo& header, boost::shared_ptr<SoundData> data, _fixed pitch)
{
	if (!header.length) {
		active = false;
		return;
	}

	info = header;
	sound_data = data;
	this->data = &((*sound_data)[0]);
	length = header.length;
	loop = this->data + (header.loop_start);
	if (header.loop_end - header.loop_start >= 4) 
	{
		loop_length = header.loop_end - header.loop_start;
	} 
	else
	{
		loop_length = 0;
	}
	rate = (pitch >> 8) * ((header.rate >> 8) / instance()->obtained.freq);
	counter = 0;
}

void Mixer::Channel::GetMoreData()
{
	if (loop_length)
	{
		data = loop;
		length = loop_length;
	}
	else if (source == SOURCE_MUSIC)
	{
		if (!Music::instance()->FillBuffer())
		{
			active = false;
		}
	}
	else if (source == SOURCE_RESOURCE)
	{
		active = false;
	}
	else if (source == SOURCE_NETWORK_AUDIO)
	{
#if !defined(DISABLE_NETWORKING)
		// this is pretty hacky, could be refactored better
		Mixer* mixer = Mixer::instance();

		// ZZZ: if we're supposed to dispose of the storage, so be it
		if (is_sound_data_disposable(mixer->sNetworkAudioBufferDesc))
		{
			release_network_speaker_buffer(mixer->sNetworkAudioBufferDesc->mData);
		}
		
		// Get the next buffer of data
		mixer->sNetworkAudioBufferDesc = dequeue_network_speaker_data();
		
		// If we have a buffer to play, set it up; else deactivate the channel.
		if (mixer->sNetworkAudioBufferDesc != NULL) {
			data = mixer->sNetworkAudioBufferDesc->mData;
			length = mixer->sNetworkAudioBufferDesc->mLength;
		}
		else
		{
			active = false;
		}
#endif // !defined(DISABLE_NETWORKING)
	}
	else
	{
		// No loop, another sound header queued?
		SoundManager::instance()->IncrementChannelCallbackCount(sound_manager_index); // fix this
		if (next_header.length) {
			
			// Yes, load sound header and continue
			LoadSoundHeader(next_header, next_data, next_pitch);
			next_header.length = 0;
			next_data.reset();
		} else {
			
			// No, turn off channel
			active = false;
		}		
	}
}

template<bool little_endian>
static inline int16 Convert(int16 i)
{
	if (little_endian)
		return SDL_SwapLE16(i);
	else
		return SDL_SwapBE16(i);
}

template<bool little_endian>
static inline int16 Convert(int8 i)
{
	return i * 256;
}

template<bool little_endian>
static inline int16 Convert(uint8 i)
{
	return (int8)(i ^ 0x80) * 256;
}

static inline int32 lerp(int32 x0, int32 x1, _fixed rate)
{
	int32 v = x0 + ((1LL*x1 - x0) * (rate & 0xffff)) / 65536;
	return v;
}

template<class T, bool stereo, bool le_or_signed>
void Mixer::Resample_(Channel* c, int16* left, int16* right, int& samples)
{
	while (samples--)
	{

		if (c->active)
		{
			int32 left0, right0;
			const T* data = reinterpret_cast<const T*>(c->data);

			left0 = Convert<le_or_signed>(*data++);
			if (stereo)
			{
				right0 = Convert<le_or_signed>(*data++);
			}
		
			if ((c->counter & 0xffff) && c->length > c->info.bytes_per_frame)
			{
				int32 left1 = Convert<le_or_signed>(*data++);
				if (stereo) 
				{
					int32 right1 = Convert<le_or_signed>(*data++);
					*left++ = lerp(left0, left1, c->counter);
					*right++ = lerp(right0, right1, c->counter);
				}
				else
				{
					*left++ = *right++ = lerp(left0, left1, c->counter);
				}
			}
			else
			{
				if (stereo)
				{
					*left++ = left0;
					*right++ = right0;
				} 
				else
				{
					*left++ = *right++ = left0;
				}
			}

			c->counter += c->rate;
			if (c->counter >= 0x10000)
			{
				int count = c->counter >> 16;
				c->counter &= 0xffff;
				c->data += c->info.bytes_per_frame * count;
				c->length -= c->info.bytes_per_frame * count;
			
				if (c->length <= 0) 
				{
					c->GetMoreData();
					return;  // sample format may have changed
				}
			}
		} 
		else
		{
			*left++ = *right++ = 0;
		}
	}
}

void Mixer::Resample(Channel* c, int16* left, int16* right, int samples)
{
	int left_to_process = samples;
	while (left_to_process > 0)
	{
		ResampleInner(c, left + samples - left_to_process, right + samples - left_to_process, left_to_process);
	}
}

void Mixer::ResampleInner(Channel* c, int16* left, int16* right, int& samples)
{
	if (c->info.stereo)
	{
		if (c->info.sixteen_bit) 
		{
			if (c->info.little_endian)
			{
				Resample_<int16, true, true>(c, left, right, samples);
			}
			else
			{
				Resample_<int16, true, false>(c, left, right, samples);
			}
		}
		else
		{
			if (c->info.signed_8bit)
			{
				Resample_<int8, true, true>(c, left, right, samples);
			} 
			else
			{
				Resample_<uint8, true, false>(c, left, right, samples);
			}
		}
	} 
	else
	{
		if (c->info.sixteen_bit) 
		{
			if (c->info.little_endian)
			{
				Resample_<int16, false, true>(c, left, right, samples);
			}
			else
			{
				Resample_<int16, false, false>(c, left, right, samples);
			}
		}
		else
		{
			if (c->info.signed_8bit)
			{
				Resample_<int8, false, true>(c, left, right, samples);
			} 
			else
			{
				Resample_<uint8, false, false>(c, left, right, samples);
			}
		}		
	}
}

static inline void apply_volume_and_clip(int32* v, float main_volume, int samples)
{
	while (samples--)
	{
		*v = static_cast<int32>(*v * main_volume);
		if (*v > INT16_MAX)
		{
			*v = INT16_MAX;
		}
		else if (*v < INT16_MIN)
		{
			*v = INT16_MIN;
		}

		++v;
	}
}

void Output(int16* output, int32* left, int32* right, int samples, bool)
{
	while (samples--)
	{
		*output++ = *left++;
		*output++ = *right++;
	}
}

void Output(int16* output, int32* left, int samples, bool)
{
	while (samples--)
	{
		*output++ = *left++;
	}
}

void Output(int8* output, int32* left, int32* right, int samples, bool is_signed)
{
	if (is_signed)
	{
		while (samples--)
		{
			*output++ = *left++ >> 8;
			*output++ = *right++ >> 8;
		}
	}
	else
	{
		while (samples--)
		{
			*output++ = (*left++ >> 8) ^ 0x80;
			*output++ = (*right++ >> 8) ^ 0x80;
		}
	}
}

void Output(int8* output, int32* left, int samples, bool is_signed)
{
	if (is_signed)
	{
		while (samples--)
		{
			*output++ = *left++ >> 8;
		}
	}
	else
	{
		while (samples--)
		{
			*output++ = (*left++ >> 8) ^ 0x80;
		}
	}
}

void Mixer::Mix(uint8* p, int len, bool stereo, bool is_sixteen_bit, bool is_signed)
{
	const int FRAME_SIZE = 512;
	int16 channel_left[FRAME_SIZE];
	int16 channel_right[FRAME_SIZE];

	int32 output_left[FRAME_SIZE];
	int32 output_right[FRAME_SIZE];

	while (len)
	{
		std::fill_n(output_left, FRAME_SIZE, 0);
		std::fill_n(output_right, FRAME_SIZE, 0);

		int samples = std::min(len, FRAME_SIZE);
		
		int channel_count = channels.size();
		for (int channel = 0; channel < channel_count; ++channel)
		{
			Channel* c = &channels[channel];
			Resample(c, channel_left, channel_right, samples);

			int16 left_volume = c->left_volume;
			int16 right_volume = c->right_volume;
			if (IsNetworkAudioPlaying() && c->source != Channel::SOURCE_NETWORK_AUDIO)
			{
				left_volume = right_volume = SoundManager::instance()->GetNetmicVolumeAdjustment();
			}

			for (int i = 0; i < samples; ++i)
			{
				output_left[i] += (channel_left[i] * left_volume) >> 8;
				output_right[i] += (channel_right[i] * right_volume) >> 8;
			}
		}

		if (game_is_networked &&
		    SoundManager::instance()->parameters.mute_while_transmitting &&
			dynamic_world->speaking_player_index != -1 && 
		    dynamic_world->speaking_player_index == local_player_index)
		{
			// mute sound!
			for (int i = 0; i < samples; ++i)
			{
				*p++ = 0;
				if (stereo) 
					*p++ = 0;
			}
		}
		else
		{
			// Mix left+right for mono
			if (!stereo)
			{
				for (int i = 0; i < samples; ++i)
				{
					output_left[i] = (output_left[i] + output_right[i]) / 2;
				}
			}

			apply_volume_and_clip(output_left, main_volume, samples);
			if (stereo)
			{
				apply_volume_and_clip(output_right, main_volume, samples);
			}

			if (stereo)
			{
				if (is_sixteen_bit)
				{
					Output(reinterpret_cast<int16*>(p), output_left, output_right, samples, is_signed);
					p += samples * 4;
				}
				else
				{
					Output(reinterpret_cast<int8*>(p), output_left, output_right, samples, is_signed);
					p += samples * 2;
				}
			}
			else
			{
				if (is_sixteen_bit)
				{
					Output(reinterpret_cast<int16*>(p), output_left, samples, is_signed);
					p += samples * 2;
				}
				else
				{
					Output(reinterpret_cast<int8*>(p), output_left, samples, is_signed);
					p += samples;
				}
			}
		}
		
		len -= samples;
	}
}

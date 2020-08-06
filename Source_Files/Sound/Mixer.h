#ifndef __MIXER_H
#define __MIXER_H

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

#include <cmath>

#include <SDL_endian.h>
#include "cseries.h"
#include "network_speaker_sdl.h"
#include "network_audio_shared.h"
#include "map.h" // to find if netmic is transmitting :(
#include "Music.h"
#include "SoundManager.h"

extern short local_player_index;
extern bool game_is_networked;

class Mixer
{
public:
	static Mixer *instance() { 
		static Mixer *m_instance = nullptr;
		if (!m_instance) 
			m_instance = new Mixer(); 
		return m_instance; 
	}

	static float from_db(float db) {
		if (db <= SoundManager::MINIMUM_VOLUME_DB) {
			return 0.f;
		} else {
			return std::pow(10.f, db / 20.f);
		}
	}
	
	void Start(uint16 rate, bool sixteen_bit, bool stereo, int num_channels, float db, uint16 samples);
	void Stop();

	void SetVolume(float db) { main_volume = from_db(db); }

	void BufferSound(int channel, const SoundInfo& header, boost::shared_ptr<SoundData> data, _fixed pitch);

	// returns the number of normal/ambient channels
	int SoundChannelCount() { return sound_channel_count; }

	void QuietChannel(int channel) {
		SDL_LockAudio();
		channels[channel].Quiet();
		SDL_UnlockAudio();
	}
	
	void SetChannelVolumes(int channel, int16 left, int16 right) { 
		channels[channel].left_volume = left; 
		channels[channel].right_volume = right; 
	}

	bool ChannelBusy(int channel) { return channels[channel].active; }

	// activates the channel
	void StartMusicChannel(bool sixteen_bit, bool stereo, bool signed_8bit, int bytes_per_frame, _fixed rate, bool little_endian);
	void UpdateMusicChannel(uint8* data, int len);
	bool MusicPlaying() { return channels[sound_channel_count + MUSIC_CHANNEL].active; }
	void StopMusicChannel() { SDL_LockAudio(); channels[sound_channel_count + MUSIC_CHANNEL].active = false; SDL_UnlockAudio(); }
	void SetMusicChannelVolume(int16 volume) { channels[sound_channel_count + MUSIC_CHANNEL].left_volume = channels[sound_channel_count + MUSIC_CHANNEL].right_volume = volume; }

	SDL_AudioSpec desired, obtained;

	void EnsureNetworkAudioPlaying();
	void StopNetworkAudio();

	void PlaySoundResource(LoadedResource &rsrc, _fixed pitch = _normal_frequency);
	void StopSoundResource();

private:
        Mixer() : sNetworkAudioBufferDesc(0) { };
	
	
	struct Channel {
		SoundInfo info;
		bool active;			// Flag: currently playing sound
		
		boost::shared_ptr<SoundData> sound_data;
		const uint8 *data;              // Current pointer to sound data
		int32 length;			// Length in bytes remaining to be played
		const uint8 *loop;		// Pointer to loop start
		int32 loop_length;		// Loop length in bytes (0 = don't loop)
		
		_fixed rate;                    // Sample rate (relative to output sample rate)
		_fixed counter;			// Counter for up/downsampling
		
		int16 left_volume;		// Volume (0x100 = nominal)
		int16 right_volume;
		
		SoundInfo next_header;            // Info for next sound
		boost::shared_ptr<SoundData> next_data;
		_fixed next_pitch;		// Pitch of next queued sound header

		Channel();
		void LoadSoundHeader(const SoundInfo& header, boost::shared_ptr<SoundData> data, _fixed pitch);
		void BufferSoundHeader(const SoundInfo& header, boost::shared_ptr<SoundData> data, _fixed pitch) {
			next_header = header;
			next_data = data;
			next_pitch = pitch;
		}

		void Quiet() { active = false; sound_data.reset(); next_data.reset(); }

		enum Source {
			SOURCE_SOUND_HEADERS,
			SOURCE_MUSIC,
			SOURCE_RESOURCE,
			SOURCE_NETWORK_AUDIO,
		} source;

		int sound_manager_index;

		void GetMoreData();
	};

	friend class Channel; // hack for sNetworkAudioBufferDesc in GetMoreData

	std::vector<Channel> channels;

	enum
	{
		MUSIC_CHANNEL,
		RESOURCE_CHANNEL,
		NETWORK_AUDIO_CHANNEL,
		EXTRA_CHANNELS
	};

	float main_volume;
	int sound_channel_count;

	void Resample(Channel* c, int16* left, int16* right, int samples);
	void ResampleInner(Channel* c, int16* left, int16* right, int& samples);
	template<class T, bool stereo, bool le_or_signed>
	static void Resample_(Channel* c, int16* left, int16* right, int& samples);

	static void MixerCallback(void *user, uint8 *stream, int len);
	void Callback(uint8 *stream, int len);
	
	friend class Movie; // to let Movie recorder call our callback

	NetworkSpeakerSoundBufferDescriptor* sNetworkAudioBufferDesc;

	inline bool IsNetworkAudioPlaying() { return channels[sound_channel_count + NETWORK_AUDIO_CHANNEL].active; }

	void Mix(uint8* p, int len, bool stereo, bool is_sixteen_bit, bool is_signed);
};
#endif


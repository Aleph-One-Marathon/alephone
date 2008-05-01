#ifndef __MIXER_H
#define __MIXER_H

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
	static Mixer *instance() { if (!m_instance) m_instance = new Mixer(); return m_instance; }
	void Start(uint16 rate, bool sixteen_bit, bool stereo, int num_channels, int volume, uint16 samples);
	void Stop();

	void SetVolume(short volume) { main_volume = volume; }

	struct Header
	{
		bool sixteen_bit;
		bool stereo;
		bool signed_8bit;
		int bytes_per_frame;
		
		const uint8* data;
		int32 length;

		const uint8* loop;
		int32 loop_length;

		uint32 /* unsigned fixed */ rate;
		bool little_endian;

		Header();
		Header(const SoundHeader& header);
	};

	void BufferSound(int channel, const Header& header, _fixed pitch);

	// returns the number of normal/ambient channels
	int SoundChannelCount() { return sound_channel_count; }

	void QuietChannel(int channel) { channels[channel].active = false; }
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

	void PlaySoundResource(LoadedResource &rsrc);
	void StopSoundResource();

private:
        Mixer() : sNetworkAudioBufferDesc(0) { };
	
	static Mixer *m_instance;
	
	struct Channel {
		bool active;			// Flag: currently playing sound
		
		bool sixteen_bit;		// Flag: 16-bit sound data (8-bit otherwise)
		bool stereo;			// Flag: stereo sound data (mono otherwise)
		bool signed_8bit;		// Flag: 8-bit sound data is signed (unsigned otherwise, 16-bit data is always signed)
		int bytes_per_frame;	        // Bytes per sample frame (1, 2 or 4)
		bool little_endian;             // 16-bit samples are little-endian
		
		const uint8 *data;              // Current pointer to sound data
		int32 length;			// Length in bytes remaining to be played
		const uint8 *loop;		// Pointer to loop start
		int32 loop_length;		// Loop length in bytes (0 = don't loop)
		
		_fixed rate;                    // Sample rate (relative to output sample rate)
		_fixed counter;			// Counter for up/downsampling
		
		int16 left_volume;		// Volume (0x100 = nominal)
		int16 right_volume;
		
		Header *next_header;            // Pointer to next queued sound header (NULL = none)
		_fixed next_pitch;		// Pitch of next queued sound header

		Channel();
		void LoadSoundHeader(const Header& header, _fixed pitch);
		void BufferSoundHeader(const Header& header, _fixed pitch) {
			delete next_header;
			next_header = new Header(header);
			next_pitch = pitch;
		}

		void Quiet() { active = false; };
	};

	std::vector<Channel> channels;

	enum
	{
		MUSIC_CHANNEL,
		RESOURCE_CHANNEL,
		NETWORK_AUDIO_CHANNEL,
		EXTRA_CHANNELS
	};

	int16 main_volume;
	int sound_channel_count;

	static void MixerCallback(void *user, uint8 *stream, int len);
	void Callback(uint8 *stream, int len);

	NetworkSpeakerSoundBufferDescriptor* sNetworkAudioBufferDesc;

	inline bool IsNetworkAudioPlaying() { return channels[sound_channel_count + NETWORK_AUDIO_CHANNEL].active; }


	// this has to live here :(
	template <class T, bool stereo, bool is_signed> 
	inline void Mix(T *p, int len) {
		while (len--) {
			int32 left = 0, right = 0;	// 16-bit internally
			
			// Mix all channels
			for (int i=0; i<channels.size(); i++) {
				Channel *c = &channels[i];
				// Channel active?
				if (c->active) {
					// Yes, read sound data
					int32 dleft, dright;
					if (c->stereo) {
						if (c->sixteen_bit) {
							if (c->little_endian)
							{
								dleft = (int16)SDL_SwapLE16(0[(int16 *)c->data]);
								dright = (int16)SDL_SwapLE16(1[(int16 *)c->data]);
							} 
							else
							{
								dleft = (int16)SDL_SwapBE16(0[(int16 *)c->data]);
								dright = (int16)SDL_SwapBE16(1[(int16 *)c->data]);
							}
						} else if (c->signed_8bit) {
							dleft = (int32)(int8)(0[c->data]) * 256;
							dright = (int32)(int8)(1[c->data]) * 256;
						} else {
							dleft = (int32)(int8)(0[c->data] ^ 0x80) * 256;
							dright = (int32)(int8)(1[c->data] ^ 0x80) * 256;
						}
					} else {
						if (c->sixteen_bit)
							if (c->little_endian)
								dleft = dright = (int16)SDL_SwapLE16(*(int16 *)c->data);
							else
								dleft = dright = (int16)SDL_SwapBE16(*(int16 *)c->data);
						else if (c->signed_8bit)
							dleft = dright = (int32)(int8)(*(c->data)) << 8;
						else
							dleft = dright = (int32)(int8)(*(c->data) ^ 0x80) << 8;
					}
					if ((c->counter & 0xffff) && c->length > c->bytes_per_frame)
					{
						const uint8 *rdata = c->data + c->bytes_per_frame;
						int32 rleft, rright;

						if (c->stereo) {
							if (c->sixteen_bit) {
								if (c->little_endian)
								{
									rleft = (int16)SDL_SwapLE16(0[(int16 *)rdata]);
									rright = (int16)SDL_SwapLE16(1[(int16 *)rdata]);
								}
								else
								{
									rleft = (int16)SDL_SwapBE16(0[(int16 *)rdata]);
									rright = (int16)SDL_SwapBE16(1[(int16 *)rdata]);
								}
							} else if (c->signed_8bit) {
								rleft = (int32)(int8)(0[rdata]) * 256;
								rright = (int32)(int8)(1[rdata]) * 256;
							} else {
								rleft = (int32)(int8)(0[rdata] ^ 0x80) * 256;
								rright = (int32)(int8)(1[rdata] ^ 0x80) * 256;
							}
						} else {
							if (c->sixteen_bit)
							{
								if (c->little_endian)
									rleft = rright = (int16)SDL_SwapLE16(*(int16 *)rdata);
								else
									rleft = rright = (int16)SDL_SwapBE16(*(int16 *)rdata);
							}
							else if (c->signed_8bit)
								rleft = rright = (int32)(int8)(*(rdata)) << 8;
							else
								rleft = rright = (int32)(int8)(*(rdata) ^ 0x80) << 8;
						}

						int32 ileft, iright;
						ileft = dleft + (((rleft - dleft) * (c->counter & 0xffff)) >> 16);
						iright = dright + (((rright - dright) * (c->counter & 0xffff)) >> 16);

						if (IsNetworkAudioPlaying() && i != (sound_channel_count + NETWORK_AUDIO_CHANNEL))
						{
							ileft = (ileft * SoundManager::instance()->GetNetmicVolumeAdjustment()) >> 8;
							iright = (iright * SoundManager::instance()->GetNetmicVolumeAdjustment()) >> 8;
						}

						// Mix into output
						left += (ileft * c->left_volume) >> 8;
						right += (iright * c->right_volume) >> 8;
					}
					else
					{
						if (IsNetworkAudioPlaying() && i != (sound_channel_count + NETWORK_AUDIO_CHANNEL))
						{
							dleft = (dleft * SoundManager::instance()->GetNetmicVolumeAdjustment()) >> 8;
							dright = (dright * SoundManager::instance()->GetNetmicVolumeAdjustment()) >> 8;
						}

						// Mix into output
						left += (dleft * c->left_volume) >> 8;
						right += (dright * c->right_volume) >> 8;
					}
					
					// Advance sound data pointer
					c->counter += c->rate;
					if (c->counter >= 0x10000) {
						int count = c->counter >> 16;
						c->counter &= 0xffff;
						c->data += c->bytes_per_frame * count;
						c->length -= c->bytes_per_frame * count;
						
						// Sound finished? Then enter loop or load next sound header
						if (c->length <= 0) {
							
							// Yes, loop present?
							if (c->loop_length) {
								
								// Yes, enter loop
								c->data = c->loop;
								c->length = c->loop_length;
								
							} else if (i == sound_channel_count + MUSIC_CHANNEL) {
								// More music data?
#ifdef __MACOS__
								if (!Music::instance()->InterruptFillBuffer())
#else
									if (!Music::instance()->FillBuffer())
#endif
									{
										// Music finished, turn it off
										c->active = false;
									}
							} else if (i == sound_channel_count + RESOURCE_CHANNEL) {
								
								// Resource channel, turn it off
								c->active = false;
								
							}
							else if (i == sound_channel_count + NETWORK_AUDIO_CHANNEL) {
#if !defined(DISABLE_NETWORKING)
								// ZZZ: if we're supposed to dispose of the storage, so be it
								if (is_sound_data_disposable(sNetworkAudioBufferDesc))
									release_network_speaker_buffer(sNetworkAudioBufferDesc->mData);
								
								// Get the next buffer of data
								sNetworkAudioBufferDesc = dequeue_network_speaker_data();
								
								// If we have a buffer to play, set it up; else deactivate the channel.
								if (sNetworkAudioBufferDesc != NULL) {
									c->data     = sNetworkAudioBufferDesc->mData;
									c->length   = sNetworkAudioBufferDesc->mLength;
								}
								else
								{
									c->active   = false;
								}
#endif // !defined(DISABLE_NETWORKING)
							} else {
								
								// No loop, another sound header queued?
								SoundManager::instance()->IncrementChannelCallbackCount(i); // fix this
								if (c->next_header) {
									
									// Yes, load sound header and continue
									c->LoadSoundHeader(*(c->next_header), c->next_pitch);
									delete c->next_header;
									c->next_header = 0;									
								} else {
									
									// No, turn off channel
									c->active = false;
								}
							}
						}
					}
				}
			}
				
				// Mix left+right for mono
				if (!stereo)
					left = (left + right) / 2;
				
				// Finalize left channel
				left = (left * main_volume) >> 8; // Apply main volume setting
				if (game_is_networked && SoundManager::instance()->parameters.mute_while_transmitting && 
				    dynamic_world->speaking_player_index == local_player_index)
					left = 0;
				if (left > 32767) // Clip output value
					left = 32767;
				else if (left < -32768)
					left = -32768;
				if (sizeof(T) == 1)
					if (is_signed)
						left >>= 8;
					else
						left = (left >> 8) ^ 0x80;
				*p++ = left; // Write to output buffer
				
				// Finalize right channel
				if (stereo) {
					right = (right * main_volume) >> 8; // Apply main volume setting
					if (game_is_networked && SoundManager::instance()->parameters.mute_while_transmitting &&
					    dynamic_world->speaking_player_index == local_player_index)
						right = 0;
					if (right > 32767) // Clip output value
						right = 32767;
					else if (right < -32768)
						right = -32768;
					if (sizeof(T) == 1)
						if (is_signed)
							// Downscale for 8-bit output
							right >>= 8;
						else
							right = (right >> 8) ^ 0x80;
					*p++ = right; // Write to output buffer
				}
		}
	}
};
#endif


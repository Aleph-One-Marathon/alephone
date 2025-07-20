/*
	Copyright (C) 2023 Benoit Hauquier and the "Aleph One" developers.

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

#ifndef __OPENAL_MANAGER_H
#define __OPENAL_MANAGER_H

#include "MusicPlayer.h"
#include "SoundPlayer.h"
#include "StreamPlayer.h"
#include <queue>

#if defined (_MSC_VER) && !defined (M_PI)
#define _USE_MATH_DEFINES
#endif 

#include <math.h>

constexpr float abortAmplitudeThreshold = MAXIMUM_SOUND_VOLUME / 6.f / 256;
constexpr float angleConvert = 360 / float(FULL_CIRCLE);
constexpr float degreToRadian = M_PI / 180.f;

struct AudioParameters {
	uint32_t rate;
	uint32_t sample_frame_size;
	ChannelType channel_type;
	bool balance_rewind;
	bool hrtf;
	bool sounds_3d;
	float master_volume;
	float music_volume;
};

class OpenALManager {
public:

	enum class OptionalExtension
	{
		Spatialization,
		DirectChannelRemix
	};

	enum class HrtfSupport
	{
		Supported,
		Unsupported,
		Required
	};

	static OpenALManager* Get() { return instance; }
	static bool Init(const AudioParameters& parameters);
	static void Shutdown();
	void Pause(bool paused);
	void Start();
	void Stop();
	std::shared_ptr<SoundPlayer> PlaySound(const Sound& sound, const SoundParameters& parameters);
	std::shared_ptr<MusicPlayer> PlayMusic(std::vector<MusicPlayer::Preset>& presets, uint32_t starting_preset_index, uint32_t starting_segment_index, const MusicParameters& parameters);
	std::shared_ptr<StreamPlayer> PlayStream(CallBackStreamPlayer callback, uint32_t rate, bool stereo, AudioFormat audioFormat, void* userdata);
	std::unique_ptr<AudioPlayer::AudioSource> PickAvailableSource(const AudioPlayer& audioPlayer);
	void UpdateListener(world_location3d listener) { listener_location.Set(listener); }
	const world_location3d& GetListener() const { return listener_location.Get(); }
	void SetMasterVolume(float volume);
	void SetMusicVolume(float volume);
	float GetMasterVolume() const { return master_volume.load(); }
	float GetMusicVolume() const { return music_volume.load(); }
	void ToggleDeviceMode(bool recording_device);
	uint32_t GetFrequency() const { return audio_parameters.rate; }
	uint32_t GetElapsedPauseTime() const { return elapsed_pause_time; }
	void GetPlayBackAudio(uint8* data, int length);
	HrtfSupport GetHrtfSupport() const;
	bool IsHrtfEnabled() const;
	bool IsBalanceRewindSound() const { return audio_parameters.balance_rewind; }
	bool IsPaused() const { return paused_audio; }
	ALCint GetRenderingFormat() const { return openal_rendering_format; }
	ALuint GetLowPassFilter(float highFrequencyGain) const;
	bool IsExtensionSupported(OptionalExtension extension) const { return extension_support.at(extension); }
private:
	static OpenALManager* instance;
	ALCdevice* p_ALCDevice = nullptr;
	ALCcontext* p_ALCContext = nullptr;
	OpenALManager(const AudioParameters& parameters);
	~OpenALManager();
	std::atomic<float> master_volume;
	std::atomic<float> music_volume;
	bool process_audio_active = false;
	bool paused_audio = false;
	uint64_t elapsed_pause_time = 0;
	AtomicStructure<world_location3d> listener_location = {};
	void StopAllPlayers();
	void UpdateParameters(const AudioParameters& parameters);
	void UpdateListener();
	void CleanEverything();
	bool GenerateSources();
	bool GenerateEffects();
	bool OpenDevice();
	bool CloseDevice();
	void ProcessAudioQueue();
	void ResyncPlayers(bool music_players_only = false);
	bool is_using_recording_device = false;
	std::queue<std::unique_ptr<AudioPlayer::AudioSource>> sources_pool;
	std::deque<std::shared_ptr<AudioPlayer>> audio_players_queue; //for audio thread only
	boost::lockfree::spsc_queue<std::shared_ptr<AudioPlayer>, boost::lockfree::capacity<256>> audio_players_shared; //pipeline main => audio thread
	int GetBestOpenALSupportedFormat();
	void RetrieveSource(const std::shared_ptr<AudioPlayer>& player);

	/* Loopback device functions */
	static LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT;
	static LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT;
	static LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT;

	static LPALGETSTRINGISOFT alGetStringiSOFT;

	/* Filter object functions */
	static LPALGENFILTERS alGenFilters;
	static LPALDELETEFILTERS alDeleteFilters;
	static LPALFILTERI alFilteri;
	static LPALFILTERF alFilterf;

	std::unordered_map<OptionalExtension, bool> extension_support;
	bool LoadOptionalExtensions();

	static void MixerCallback(void* usr, uint8* stream, int len);
	SDL_AudioSpec sdl_audio_specs_obtained;
	AudioParameters audio_parameters;
	ALCint openal_rendering_format = 0;
	ALuint low_pass_filter;

	/* format type we supports for mixing / rendering
	* those are used from the first to the last of the list
	  and we stop when our device support the corresponding format 
	  Short is first, because there is no real purpose to use other format now */
	const std::vector<ALCint> format_type = {
		ALC_SHORT_SOFT,
		ALC_FLOAT_SOFT,
		ALC_INT_SOFT,
		ALC_UNSIGNED_BYTE_SOFT
	};

	const std::unordered_map<ALCint, SDL_AudioFormat> mapping_openal_sdl_format = {
		{ALC_FLOAT_SOFT, AUDIO_F32SYS},
		{ALC_INT_SOFT, AUDIO_S32SYS},
		{ALC_SHORT_SOFT, AUDIO_S16SYS},
		{ALC_UNSIGNED_BYTE_SOFT, AUDIO_U8}
	};

	const std::unordered_map<SDL_AudioFormat, ALCint> mapping_sdl_openal_format = {
		{AUDIO_F32SYS, ALC_FLOAT_SOFT},
		{AUDIO_S32SYS, ALC_INT_SOFT},
		{AUDIO_S16SYS, ALC_SHORT_SOFT},
		{AUDIO_U8, ALC_UNSIGNED_BYTE_SOFT}
	};

	const std::unordered_map<ChannelType, ALCint> mapping_sdl_openal_channel = {
		{ChannelType::_mono, ALC_MONO_SOFT},
		{ChannelType::_stereo, ALC_STEREO_SOFT},
		{ChannelType::_quad, ALC_QUAD_SOFT},
		{ChannelType::_5_1, ALC_5POINT1_SOFT},
		{ChannelType::_6_1, ALC_6POINT1_SOFT},
		{ChannelType::_7_1, ALC_7POINT1_SOFT}
	};

	const std::unordered_map<ALCint, ChannelType> mapping_openal_sdl_channel = {
		{ALC_MONO_SOFT, ChannelType::_mono},
		{ALC_STEREO_SOFT, ChannelType::_stereo},
		{ALC_QUAD_SOFT, ChannelType::_quad},
		{ALC_5POINT1_SOFT, ChannelType::_5_1},
		{ALC_6POINT1_SOFT, ChannelType::_6_1},
		{ALC_7POINT1_SOFT, ChannelType::_7_1}
	};

	const std::unordered_map<OptionalExtension, std::string> mapping_extensions_names = {
		{OptionalExtension::Spatialization, "AL_SOFT_source_spatialize"},
		{OptionalExtension::DirectChannelRemix, "AL_SOFT_direct_channels_remix"}
	};
};

#endif

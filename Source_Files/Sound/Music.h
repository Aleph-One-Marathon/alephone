#ifndef __MUSIC_H
#define __MUSIC_H

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

	Handles both intro and level music

*/

#include "Random.h"
#include "MusicPlayer.h"

class Music
{
public:
	static Music *instance() { 
		static Music *m_instance = nullptr;
		if (!m_instance) 
			m_instance = new Music(); 
		return m_instance; 
	}

	static constexpr int reserved_music_slots = 2;
	enum MusicSlot {
		Intro = 0,
		Level = 1
	};

	class Slot {
	private:
		std::shared_ptr<MusicPlayer> musicPlayer;
		std::vector<std::shared_ptr<StreamDecoder>> dynamic_music_tracks;
		std::vector<MusicPlayer::Preset> dynamic_music_presets;
		uint64_t music_fade_start = 0;
		uint32_t music_fade_duration = 0;
		float music_fade_limit_volume;
		float music_fade_start_volume;
		bool music_fade_stop_no_volume;
		MusicPlayer::FadeType music_fade_type;
		MusicParameters parameters;
	public:
		void Fade(float limitVolume, short duration, MusicPlayer::FadeType fadeType, bool stopOnNoVolume = true);
		bool Playing() const { return musicPlayer && musicPlayer->IsActive(); }
		void Pause();
		void Close();
		bool Open(FileSpecifier* file);
		void Play(uint32_t preset_index = 0, uint32_t segment_index = 0);
		bool SetParameters(const MusicParameters& parameters);
		float GetLimitFadeVolume() const { return music_fade_limit_volume; }
		bool IsFading() const { return music_fade_start; }
		bool StopPlayerAfterFadeOut() const { return music_fade_stop_no_volume; }
		void StopFade() { music_fade_start = 0; }
		bool SetVolume(float volume) { return SetParameters({ volume, parameters.loop }); }
		bool SetLoop(bool loop) { return SetParameters({ parameters.volume, loop }); }
		const MusicParameters& GetParameters() const { return parameters; }
		std::pair<bool, float> ComputeFadingVolume() const;
		std::optional<uint32_t> LoadTrack(FileSpecifier* file);
		std::optional<uint32_t> AddPreset();
		std::optional<uint32_t> AddSegmentToPreset(uint32_t preset_index, uint32_t track_index);
		bool IsSegmentIndexValid(uint32_t preset_index, uint32_t segment_index) const;
		bool SetSegmentMapping(uint32_t preset_index, uint32_t segment_index, uint32_t transition_preset_index, const MusicPlayer::Segment::Mapping& transition_segment_mapping);
		bool SetPresetTransition(uint32_t preset_index);
	};

	bool SetupIntroMusic(FileSpecifier& file) { return music_slots[MusicSlot::Intro].Open(&file); }
	void RestartIntroMusic();
	Slot* GetSlot(uint32_t index) { return index < music_slots.size() ? &music_slots[index] : nullptr; }
	void Fade(float limitVolume, short duration, MusicPlayer::FadeType fadeType, bool stopOnNoVolume = true);
	void Pause();
	bool Playing();
	std::optional<uint32_t> Add(const MusicParameters& parameters, FileSpecifier* file = nullptr);
	void Idle();
	void StopLevelMusic();
	void StopInGameMusic();
	void ClearLevelPlaylist();
	void PushBackLevelMusic(const FileSpecifier& file);
	void SetPlaylistParameters(bool randomOrder);
	void SeedLevelMusic();
	void SetClassicLevelMusic(short song_index);
private:
	std::vector<Slot> music_slots;

	Music();
	FileSpecifier* GetLevelMusic();
	bool LoadLevelMusic();

	// level music
	short marathon_1_song_index;
	std::vector<FileSpecifier> playlist;
	size_t song_number;
	bool random_order;
	GM_Random randomizer;
};

#endif

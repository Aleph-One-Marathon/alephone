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

#include "Music.h"
#include "SoundManager.h"
#include "interface.h"
#include "OpenALManager.h"

Music::Music() :
	marathon_1_song_index(NONE),
	song_number(0),
	random_order(false),
	music_slots(reserved_music_slots)
{}

bool Music::Slot::Open(FileSpecifier* file)
{
	Close();

	if (!file)
		return false;

	auto track_id = LoadTrack(file);
	if (!track_id.has_value()) return false;

	auto preset_id = AddPreset();
	if (!preset_id.has_value()) return false;

	return AddSegmentToPreset(preset_id.value(), track_id.value()).has_value();
}

void Music::RestartIntroMusic()
{
	auto& introSlot = music_slots[MusicSlot::Intro];
	if (!introSlot.Playing() && introSlot.SetParameters({}))
	{
		introSlot.Play();
	}
}

void Music::Pause()
{
	for (auto& slot : music_slots)
	{
		slot.Pause();
	}

	music_slots.resize(reserved_music_slots);
}

void Music::Fade(float limitVolume, short duration, MusicPlayer::FadeType fadeType, bool stopOnNoVolume)
{
	for (auto& slot : music_slots)
	{
		slot.Fade(limitVolume, duration, fadeType, stopOnNoVolume);
	}
}

void Music::Slot::Fade(float limitVolume, short duration, MusicPlayer::FadeType fadeType, bool stopOnNoVolume)
{
	if (!Playing()) return;

	music_fade_type = fadeType;
	music_fade_start_volume = parameters.volume;
	music_fade_limit_volume = limitVolume;
	music_fade_start = SoundManager::GetCurrentAudioTick();
	music_fade_duration = duration;
	music_fade_stop_no_volume = stopOnNoVolume;
}

std::optional<uint32_t> Music::Add(const MusicParameters& parameters, FileSpecifier* file)
{
	Slot slot;
	bool success = (!file || slot.Open(file)) && slot.SetParameters(parameters);
	if (!success) return std::nullopt;
	music_slots.push_back(std::move(slot));
	return static_cast<uint32_t>(music_slots.size() - 1);
}

bool Music::Playing()
{
	for (auto& slot : music_slots)
	{
		if (slot.Playing())
			return true;
	}

	return false;
}

void Music::Idle()
{
	if (!SoundManager::instance()->IsInitialized() || !SoundManager::instance()->IsActive() || OpenALManager::Get()->IsPaused()) return;

	if (get_game_state() == _game_in_progress && !music_slots[MusicSlot::Level].Playing() && LoadLevelMusic()) 
	{
		music_slots[MusicSlot::Level].Play();
	}

	for (int i = 0; i < music_slots.size(); i++)
	{
		auto& slot = music_slots.at(i);

		if (slot.IsFading())
		{
			auto volumeResult = slot.ComputeFadingVolume();
			bool fadeIn = volumeResult.first;
			float vol = fadeIn ? std::min(volumeResult.second, slot.GetLimitFadeVolume()) : std::max(volumeResult.second, slot.GetLimitFadeVolume());
			slot.SetVolume(vol);
			if (vol == slot.GetLimitFadeVolume()) slot.StopFade();
			if (vol <= 0 && slot.StopPlayerAfterFadeOut()) slot.Pause();
		}
	}
}

std::pair<bool, float> Music::Slot::ComputeFadingVolume() const
{
	const bool fadeIn = music_fade_limit_volume > music_fade_start_volume;
	const auto elapsed = SoundManager::GetCurrentAudioTick() - music_fade_start;
	const float factor = std::clamp(elapsed / (float)music_fade_duration, 0.f, 1.f);

	if (factor == 1.f) //Ensure fade completion despite float precision
	{
		return { fadeIn, music_fade_limit_volume };
	}

	float volume = music_fade_start_volume;
	switch (music_fade_type) {
	case MusicPlayer::FadeType::Linear:
	{
		volume += (music_fade_limit_volume - music_fade_start_volume) * factor;
		break;
	}
	case MusicPlayer::FadeType::Sinusoidal:
	{
		const auto t = factor * M_PI_2;
		const auto shape = fadeIn ? std::sin(t) : 1.0 - std::cos(t);
		volume += (music_fade_limit_volume - music_fade_start_volume) * shape;
		break;
	}
	case MusicPlayer::FadeType::None:
	{
		volume = music_fade_limit_volume;
		break;
	}
	default:
		assert(false);
		break;
	}

	return { fadeIn, volume };
}

void Music::Slot::Pause()
{
	if (Playing())
	{
		musicPlayer->AskStop();
	}

	StopFade();
}

void Music::Slot::Close()
{
	Pause();
	musicPlayer.reset();
	dynamic_music_presets.clear();
	dynamic_music_tracks.clear();
}

bool Music::Slot::SetParameters(const MusicParameters& parameters)
{
	this->parameters = parameters;
	this->parameters.volume = std::max(std::min(parameters.volume, 1.f), 0.f);
	if (musicPlayer) musicPlayer->UpdateParameters(this->parameters);
	return true;
}

void Music::Slot::Play(uint32_t preset_index, uint32_t segment_index)
{
	if (!OpenALManager::Get() || Playing() || !IsSegmentIndexValid(preset_index, segment_index)) return;
	musicPlayer = OpenALManager::Get()->PlayMusic(dynamic_music_presets, preset_index, segment_index, parameters);
}

std::optional<uint32_t> Music::Slot::LoadTrack(FileSpecifier* file)
{
	std::shared_ptr<StreamDecoder> segment_decoder = file ? StreamDecoder::Get(*file) : nullptr;
	if (!segment_decoder) return std::nullopt;
	dynamic_music_tracks.emplace_back(segment_decoder);
	return static_cast<uint32_t>(dynamic_music_tracks.size() - 1);
}

std::optional<uint32_t> Music::Slot::AddPreset()
{
	dynamic_music_presets.emplace_back();
	return static_cast<uint32_t>(dynamic_music_presets.size() - 1);
}

std::optional<uint32_t> Music::Slot::AddSegmentToPreset(uint32_t preset_index, uint32_t track_index)
{
	if (preset_index >= dynamic_music_presets.size() || track_index >= dynamic_music_tracks.size())
		return std::nullopt;

	dynamic_music_presets[preset_index].AddSegment(dynamic_music_tracks[track_index]);
	return static_cast<uint32_t>(dynamic_music_presets[preset_index].GetSegments().size() - 1);
}

bool Music::Slot::SetSegmentMapping(uint32_t preset_index, uint32_t segment_index, uint32_t transition_preset_index, const MusicPlayer::Segment::Mapping& transition_segment_mapping)
{
	if (!IsSegmentIndexValid(preset_index, segment_index) || !IsSegmentIndexValid(transition_preset_index, transition_segment_mapping.segment_id))
		return false;

	auto segment = dynamic_music_presets[preset_index].GetSegment(segment_index);
	if (!segment) return false;

	segment->SetSegmentMapping(transition_preset_index, transition_segment_mapping);
	return true;
}

bool Music::Slot::IsSegmentIndexValid(uint32_t preset_index, uint32_t segment_index) const
{
	return preset_index < dynamic_music_presets.size() && segment_index < dynamic_music_presets[preset_index].GetSegments().size();
}

bool Music::Slot::SetPresetTransition(uint32_t preset_index)
{
	if (!musicPlayer || !musicPlayer->IsActive()) return false;
	return musicPlayer->RequestPresetTransition(preset_index);
}

bool Music::LoadLevelMusic()
{
	FileSpecifier* level_song_file = GetLevelMusic();
	auto& slot = music_slots[MusicSlot::Level];
	return slot.Open(level_song_file) && slot.SetParameters({ 1.f, playlist.size() == 1 });
}

void Music::SetPlaylistParameters(bool randomOrder)
{
	random_order = randomOrder;
}

void Music::SeedLevelMusic()
{
	song_number = 0;
	randomizer.z ^= SoundManager::GetCurrentAudioTick();
	randomizer.SetTable();
}

void Music::SetClassicLevelMusic(short song_index)
{
	if (playlist.size() || song_index < 0)
		return;

	FileSpecifier file;
	sprintf(temporary, "Music/%02d.ogg", song_index);
	file.SetNameWithPath(temporary);
	if (!file.Exists())
	{
		sprintf(temporary, "Music/%02d.mp3", song_index);
		file.SetNameWithPath(temporary);
	}
	if (!file.Exists())
		return;

	PushBackLevelMusic(file);
	marathon_1_song_index = song_index;
}

void Music::ClearLevelPlaylist()
{
	playlist.clear();
	marathon_1_song_index = NONE;
}

void Music::StopInGameMusic()
{
	for (int i = MusicSlot::Level; i < music_slots.size(); i++)
	{
		music_slots[i].Close();
	}
}

void Music::StopLevelMusic()
{
	music_slots[MusicSlot::Level].Close();
}

void Music::PushBackLevelMusic(const FileSpecifier& file)
{
	if (std::find(playlist.begin(), playlist.end(), file) != playlist.end())
	{
		return;
	}

	playlist.push_back(file);

	if (playlist.size() > 1)
	{
		music_slots[MusicSlot::Level].SetLoop(false);
	}
}

FileSpecifier* Music::GetLevelMusic()
{
	// No songs to play
	if (playlist.empty()) return nullptr;

	size_t NumSongs = playlist.size();
	if (NumSongs == 1) return &playlist[0];

	if (random_order)
		song_number = randomizer.KISS() % NumSongs;

	// Get the song number to within range if playing sequentially;
	// if the song number gets too big, then it's reset back to the first one
	if (song_number >= NumSongs) song_number = 0;

	return &playlist[song_number++];
}

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
	music_slots(reserved_music_slots),
	music_context(MusicContext::Default)
{}

void Music::SetContext(MusicContext context)
{
	if (context == music_context) return;

	if (context == MusicContext::Default && music_context == MusicContext::RevertSameLevel)
	{
		StopAutoIdle();
	}

	else if (context == MusicContext::RevertSameLevel && music_context == MusicContext::Default)
	{
		StartAutoIdle();
	}

	music_context = context;
}

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
	std::vector<Music::Slot> new_slots;
	new_slots.reserve(music_slots.size());

	for (auto i = 0; i < music_slots.size(); ++i)
	{
		Music::Slot& slot = music_slots[i];

		if (slot.CanStop())
			slot.Pause();

		if (i < reserved_music_slots || !slot.CanStop())
			new_slots.push_back(std::move(slot)); //indexes won't be preserved on reload for lua but order will
	}

	music_slots = std::move(new_slots);
}

void Music::Fade(float limitVolume, short duration, FadeType fadeType, bool stopOnNoVolume)
{
	for (auto& slot : music_slots)
	{
		slot.Fade(limitVolume, duration, fadeType, stopOnNoVolume);
	}
}

void Music::Slot::Fade(float limitVolume, short duration, FadeType fadeType, bool stopOnNoVolume)
{
	if (!Playing()) return;
	if (stopOnNoVolume && limitVolume <= 0.f && !CanStop()) return;

	auto currentVolume = musicPlayer->GetParameters().volume;
	if (currentVolume == limitVolume) return;

	music_fade_type = fadeType;
	music_fade_start_volume = currentVolume;
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
		if (slot.CanStop() && slot.Playing())
			return true;
	}

	return false;
}

void Music::Idle()
{
	if (!SoundManager::instance()->IsInitialized() || !SoundManager::instance()->IsActive() || OpenALManager::Get()->IsPaused()) return;

	//TODO: Replace me with music context and remove interface.h include
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

Uint32 Music::AutoIdle(Uint32 interval, void* param)
{
	Music::instance()->Idle(); 
	return auto_idle_timer_interval_ms;
}

void Music::StartAutoIdle()
{
	if (auto_idle_timer_id) return;
	auto_idle_timer_id = SDL_AddTimer(auto_idle_timer_interval_ms, AutoIdle, this);
}

void Music::StopAutoIdle()
{
	if (!auto_idle_timer_id) return;
	SDL_RemoveTimer(auto_idle_timer_id);
	auto_idle_timer_id = 0;
}

std::pair<bool, float> Music::Slot::ComputeFadingVolume() const
{
	const bool fadeIn = music_fade_limit_volume > music_fade_start_volume;
	const auto elapsed = SoundManager::GetCurrentAudioTick() - music_fade_start;
	const float factor = std::clamp(elapsed / (float)music_fade_duration, 0.f, 1.f);

	float volume = music_fade_start_volume;
	switch (music_fade_type) {
	case FadeType::Linear:
	{
		volume += (music_fade_limit_volume - music_fade_start_volume) * factor;
		break;
	}
	case FadeType::Sinusoidal:
	{
		volume += (music_fade_limit_volume - music_fade_start_volume) * std::sin(factor * M_PI_2);
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

bool Music::Slot::SetNextSegment(uint32_t preset_index, uint32_t segment_index, uint32_t transition_preset_index, uint32_t transition_segment_index)
{
	if (!IsSegmentIndexValid(preset_index, segment_index) || !IsSegmentIndexValid(transition_preset_index, transition_segment_index))
		return false;

	auto segment = dynamic_music_presets[preset_index].GetSegment(segment_index);
	if (!segment) return false;

	segment->SetNextSegment(transition_preset_index, transition_segment_index);
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

bool Music::Slot::CanStop() const
{
	return !parameters.persist_on_revert || Music::instance()->GetContext() != MusicContext::RevertSameLevel;
}

bool Music::LoadLevelMusic()
{
	FileSpecifier* level_song_file = GetLevelMusic();
	auto& slot = music_slots[MusicSlot::Level];
	return slot.Open(level_song_file) && slot.SetParameters({ 1.f, playlist.size() == 1, slot.GetParameters().persist_on_revert });
}

void Music::SetPlaylistParameters(bool randomOrder, bool persistOnRevert)
{
	random_order = randomOrder;
	music_slots[MusicSlot::Level].SetPersistOnRevert(persistOnRevert);
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
		auto& slot = music_slots[i];
		if (slot.CanStop()) slot.Close();
	}
}

void Music::StopLevelMusic()
{
	auto& slot = music_slots[MusicSlot::Level];
	if (slot.CanStop()) slot.Close();
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

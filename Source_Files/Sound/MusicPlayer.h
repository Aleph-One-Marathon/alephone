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

#ifndef __MUSIC_PLAYER_H
#define __MUSIC_PLAYER_H

#include "AudioPlayer.h"

struct MusicParameters {
	float volume = 1.f;
	bool loop = true;
};

class MusicPlayer : public AudioPlayer {
public:

	class Segment {
	private:
		std::shared_ptr<StreamDecoder> decoder;
		std::unordered_map<uint32_t, uint32_t> presets_mapping; //preset index - next segment index
	public:
		Segment(std::shared_ptr<StreamDecoder> decoder) { this->decoder = decoder; }
		std::shared_ptr<StreamDecoder> GetDecoder() const { return decoder; }
		std::optional<uint32_t> GetNextSegmentIndex(uint32_t preset_index) const { return presets_mapping.find(preset_index) != presets_mapping.end() ? std::make_optional(presets_mapping.find(preset_index)->second) : std::nullopt; }
		void SetNextSegment(uint32_t preset_index, uint32_t segment_index) { presets_mapping[preset_index] = segment_index; }
	};

	class Preset {
	private:
		std::vector<Segment> segments;
	public:
		void AddSegment(const Segment& segment) { segments.push_back(segment); }
		Segment* GetSegment(uint32_t index) { return index >= 0 && index < segments.size() ? &segments[index] : nullptr; }
		const std::vector<Segment>& GetSegments() const { return segments; }
	};

	MusicPlayer(std::vector<Preset>& presets, uint32_t starting_preset_index, uint32_t starting_segment_index, const MusicParameters& parameters); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 5.f; } //Doesn't really matter, just be above maximum volume (1) to be prioritized over sounds
	void UpdateParameters(const MusicParameters& musicParameters) { parameters.Store(musicParameters); }
	MusicParameters GetParameters() const { return parameters.Get(); }
	bool RequestPresetTransition(uint32_t preset_index);

private:
	uint32_t GetNextData(uint8* data, uint32_t length) override;
	SetupALResult SetUpALSourceIdle() override;
	bool LoadParametersUpdates() override { return parameters.Update(); }
	AtomicStructure<MusicParameters> parameters;
	std::shared_ptr<StreamDecoder> current_decoder;
	std::vector<Preset> music_presets;
	uint32_t current_preset_index;
	uint32_t current_segment_index;
	std::atomic_uint32_t requested_preset_index;

	friend class OpenALManager;
};

#endif
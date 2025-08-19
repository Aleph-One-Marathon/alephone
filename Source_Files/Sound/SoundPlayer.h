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

#ifndef __SOUND_PLAYER_H
#define __SOUND_PLAYER_H

#include "AudioPlayer.h"
#include "SoundFile.h"
#include "sound_definitions.h"

struct SoundStereo {

	bool is_panning;
	float gain_global;
	float gain_left;
	float gain_right;

	bool operator==(const SoundStereo& other) const {
		return std::tie(is_panning, gain_global, gain_right, gain_left) == std::tie(other.is_panning, other.gain_global, other.gain_right, other.gain_left);
	}

	bool operator!=(const SoundStereo& other) const {
		return !(*(this) == other);
	}
};

struct SoundParameters {
	short identifier = NONE; //Identifier of the sound
	short source_identifier = NONE; //Identifier of the source emitting the sound
	float pitch = 1.f;
	bool is_2d = true; //if false it will use source_location3d to position sound (3D sounds)
	bool soft_rewind = false; //if true the sound can only rewind after it's done playing
	bool soft_start = false; //if true the sound will use transitions to fade in from silence to proper computed volume
	uint16_t obstruction_flags = 0;
	uint16_t flags = 0;
	sound_behavior behavior = _sound_is_normal;
	world_location3d source_location3d = {};
	world_location3d* dynamic_source_location3d = nullptr;
	SoundStereo stereo_parameters = {}; //2D panning
};

struct SoundBehavior {
	float distance_reference;
	float distance_max;
	float rolloff_factor;
	float max_gain;
	float high_frequency_gain;

	bool operator==(const SoundBehavior& other) const {
		return std::tie(distance_reference, distance_max, rolloff_factor, max_gain, high_frequency_gain) == 
			std::tie(other.distance_reference, other.distance_max, other.rolloff_factor, other.max_gain, other.high_frequency_gain);
	}

	bool operator!=(const SoundBehavior& other) const {
		return !(*(this) == other);
	}
};

struct Sound {
	SoundInfo header;
	std::shared_ptr<SoundData> data;
};

class SoundPlayer : public AudioPlayer {
public:
	SoundPlayer(const Sound& sound, const SoundParameters& parameters); //Must not be used outside OpenALManager (public for make_shared)
	void UpdateParameters(const SoundParameters& parameters) { this->parameters.Store(parameters); }
	void UpdateRewindParameters(const SoundParameters& parameters) { this->rewind_parameters.Store(parameters); }
	short GetIdentifier() const { return parameters.Get().identifier; }
	short GetSourceIdentifier() const { return parameters.Get().source_identifier; }
	SoundParameters GetParameters() const { return parameters.Get(); }
	static float Simulate(const SoundParameters& soundParameters);
	float GetPriority() const override { return Simulate(parameters.Get()); }
	void AskSoftStop() { soft_stop_signal = true; } //not supported by 3d sounds because no need to
	void AskRewind(const SoundParameters& soundParameters, const Sound& sound);
	bool CanRewind(uint64_t baseTick) const;
	bool CanFastRewind(const SoundParameters& soundParameters) const;
	bool HasActiveRewind() const { return rewind_signal.load() && !soft_stop_signal.load(); }
private:

	struct SoundTransition {
		uint64_t start_transition_tick = 0;
		float current_volume = 0.f;
		SoundBehavior current_sound_behavior;
		bool allow_transition = false;
	};

	void Rewind() override;
	void Init(const SoundParameters& parameters);
	uint32_t GetNextData(uint8* data, uint32_t length) override;
	SetupALResult SetUpALSourceIdle() override;
	SetupALResult SetUpALSource3D();
	bool SetUpALSourceInit() override;
	bool LoadParametersUpdates() override;
	void ResetTransition();
	float ComputeParameterForTransition(float targetParameter, float currentParameter, uint64_t currentTick) const;
	float ComputeVolumeForTransition(float targetVolume);
	bool MustDisableHrtf() const;
	std::tuple<AudioFormat, uint32_t, bool> GetAudioFormat() const override;
	uint32_t ProcessData(uint8_t* outputData, uint32_t remainingSoundDataLength, uint32_t remainingBufferLength);
	SoundBehavior ComputeVolumeForTransition(const SoundBehavior& targetSoundBehavior);
	AtomicStructure<Sound> sound;
	AtomicStructure<SoundParameters> parameters;
	AtomicStructure<SoundParameters> rewind_parameters;
	SoundTransition sound_transition;
	uint32_t data_length;
	uint32_t current_index_data;
	uint64_t start_tick;

	std::atomic_bool soft_stop_signal = { false };

	static constexpr uint32_t rewind_time = 83U;
	static constexpr uint32_t fast_rewind_time = 35U;
	static constexpr float smooth_volume_transition_threshold = 0.1f;
	static constexpr uint32_t smooth_volume_transition_time_ms = 300U;

	static constexpr SoundBehavior sound_behavior_parameters[] = {
		{0.5f, 5.f, 1.f, 1.f, 1.f},
		{2.5f, 15.f, 1.7f, 1.f, 1.f},
		{3.f, 20.f, 1.2f, 1.f, 1.f}
	};

	static constexpr SoundBehavior sound_obstructed_or_muffled_behavior_parameters[] = {
		{0.5f, 3.f, 2.f, 0.3f, 0.15f},
		{2.5f, 9.f, 3.4f, 0.5f, 0.2f},
		{3.f, 12.f, 2.4f, 0.75f, 0.3f}
	};

	static constexpr SoundBehavior sound_obstructed_and_muffled_behavior_parameters[] = {
		{0.2f, 2.f, 3.f, 0.15f, 0.1f},
		{1.5f, 6.f, 4.2f, 0.25f, 0.15f},
		{2.f, 9.f, 3.6f, 0.375f, 0.2f}
	};

	friend class OpenALManager;
};

#endif
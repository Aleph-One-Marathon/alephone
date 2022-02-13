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
};

struct SoundParameters {
	short identifier = NONE; //Identifier of the sound
	short source_identifier = NONE; //Identifier of the source emitting the sound
	float pitch = 1;
	bool loop = false; //for now it will only be used by sound (musics work differently but should use this tbh)
	bool local = true; //if false it will use source_location3d to position sound (3D sounds)
	bool filterable = true; //for now volume only / if true, can be modified by global filters, otherwise is immune to that
	uint16_t obstruction_flags = 0;
	sound_behavior behavior = _sound_is_normal;
	world_location3d source_location3d = {};
	SoundStereo stereo_parameters = {}; //2D panning
	uint16_t flags = 0;
};

struct SoundBehavior {
	float distance_reference;
	float distance_max;
	float rolloff_factor;
	float max_gain;
};

class SoundPlayer : public AudioPlayer {
public:
	SoundPlayer(const SoundInfo& header, const SoundData& sound_data, SoundParameters parameters); //Must not be used outside OpenALManager (public for make_shared)
	void UpdateParameters(SoundParameters parameters);
	short GetIdentifier() const { return parameters.identifier; }
	short GetSourceIdentifier() const { return parameters.source_identifier; }
	SoundParameters GetParameters() const { return parameters; }
	static float Simulate(SoundParameters soundParameters);
	float GetPriority() const { return Simulate(parameters); }
private:
	void Rewind();
	int GetNextData(uint8* data, int length);
	int LoopManager(uint8* data, int length);
	bool SetUpALSourceIdle() const;
	bool SetUpALSourceInit() const;
	void SetUpALSource3D() const;
	SoundParameters parameters;
	SoundInfo header;
	SoundData sound_data;
	uint32_t data_length;
	uint32_t current_index_data = 0;
	uint32_t start_tick;

	static constexpr int rewind_time = 83;

	static constexpr SoundBehavior sound_behavior_parameters[] = {
		{0.5, 5, 1, 1},
		{1, 10, 1, 1},
		{2, 15, 1, 1}
	};

	static constexpr SoundBehavior sound_obstruct_behavior_parameters[] = {
		{0, 0, 1, 0}, //This line is never used and wouldn't work
		{0.5, 7, 1, 0.5},
		{0.5, 10, 1, 4.f / 3}
	};

	friend class OpenALManager;
};

#endif
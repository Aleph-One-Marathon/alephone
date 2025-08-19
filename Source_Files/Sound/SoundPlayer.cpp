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

#include "SoundPlayer.h"
#include "OpenALManager.h"
#include "SoundManager.h"

constexpr SoundBehavior SoundPlayer::sound_behavior_parameters[];
constexpr SoundBehavior SoundPlayer::sound_obstructed_or_muffled_behavior_parameters[];
constexpr SoundBehavior SoundPlayer::sound_obstructed_and_muffled_behavior_parameters[];
SoundPlayer::SoundPlayer(const Sound& sound, const SoundParameters& parameters)
	: AudioPlayer(sound.header.rate >> 16, sound.header.stereo, sound.header.audio_format) {  //since header.rate is on 16.16 format
	this->sound = sound;
	Init(parameters);
}

void SoundPlayer::Init(const SoundParameters& parameters) {
	const auto& sound = this->sound.Get();
	AudioPlayer::Init(sound.header.rate >> 16, sound.header.stereo, sound.header.audio_format);
	this->parameters.Set(parameters);
	data_length = sound.header.length;
	start_tick = SoundManager::GetCurrentAudioTick();
	current_index_data = 0;
}

//Simulate what the volume of our sound would be if we play it
//If the volume is 0 then we just don't play the sound and drop it
float SoundPlayer::Simulate(const SoundParameters& soundParameters) {
	if (soundParameters.is_2d && !soundParameters.stereo_parameters.is_panning) return 1.f; //ofc we play all 2d sounds without stereo panning
	if (soundParameters.stereo_parameters.is_panning) return soundParameters.stereo_parameters.gain_global;

	const auto& listener = OpenALManager::Get()->GetListener(); //if we don't have a listener on a 3d sound, there is a problem
	float distance = std::sqrt(
		std::pow((float)(soundParameters.source_location3d.point.x - listener.point.x) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.y - listener.point.y) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.z - listener.point.z) / WORLD_ONE, 2)
	);

	const bool obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) || (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const bool double_obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) && (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const bool muffled = soundParameters.obstruction_flags & _sound_was_media_muffled;

	const auto& behaviorParameters = double_obstruction || (obstruction && muffled) ? sound_obstructed_and_muffled_behavior_parameters[soundParameters.behavior] :
		obstruction || muffled ? sound_obstructed_or_muffled_behavior_parameters[soundParameters.behavior] :
		sound_behavior_parameters[soundParameters.behavior];

	if (distance > behaviorParameters.distance_max) {
		return 0;
	}

	//This is the AL_INVERSE_DISTANCE_CLAMPED function we simulate
	distance = std::max(distance, behaviorParameters.distance_reference);
	const float volume = behaviorParameters.distance_reference / (behaviorParameters.distance_reference + behaviorParameters.rolloff_factor * (distance - behaviorParameters.distance_reference));

	return volume;
}

bool SoundPlayer::CanRewind(uint64_t baseTick) const {
	const auto& rewindParameters = rewind_parameters.Get();
	if (rewindParameters.soft_rewind) return true;
	const auto rewindTime = OpenALManager::Get()->IsBalanceRewindSound() || !CanFastRewind(rewindParameters) ? rewind_time : fast_rewind_time;
	return baseTick + rewindTime < SoundManager::GetCurrentAudioTick();
}

bool SoundPlayer::CanFastRewind(const SoundParameters& soundParameters) const {
	return (soundParameters.source_identifier != NONE || (soundParameters.is_2d && !soundParameters.stereo_parameters.is_panning))
		&& soundParameters.source_identifier == GetSourceIdentifier();
}

void SoundPlayer::AskRewind(const SoundParameters& soundParameters, const Sound& newSound) {
	soft_stop_signal = false;
	UpdateRewindParameters(soundParameters);
	sound.Store(newSound);
	AudioPlayer::AskRewind();
}

void SoundPlayer::Rewind() {

	if (!CanRewind(start_tick) && (current_index_data > 0 || IsPlaying())) { //if a sound hasn't started playing yet, it's ok to replace it
		rewind_signal = false;
		return;
	}

	const auto& rewindParameters = rewind_parameters.Get();

	if (!rewindParameters.soft_rewind) {
		AudioPlayer::Rewind();
	}
	else {

		if (current_index_data < sound.Get().header.length)
			return;
		else
			rewind_signal = false;
	}

	const auto& oldParameters = parameters.Get();

	sound.Update();
	Init(rewindParameters);

	if (oldParameters.source_identifier != rewindParameters.source_identifier) ResetTransition();
	if (!rewindParameters.soft_rewind) SetUpALSourceInit();
}

bool SoundPlayer::LoadParametersUpdates() {

	const bool softStop = soft_stop_signal.load();
	SoundParameters soundParameters, bestParameters, bestRewindParameters;
	float lastPriority = 0.f, rewindLastPriority = 0.f;

	if (softStop && sound_transition.allow_transition) return true;

	while (parameters.Consume(soundParameters)) {

		const float priority = Simulate(soundParameters);

		if (priority <= lastPriority) continue;

		bestParameters = soundParameters;
		lastPriority = priority;
	}

	if (lastPriority > 0) parameters.Set(bestParameters);

	while (rewind_parameters.Consume(soundParameters)) {

		const float priority = Simulate(soundParameters);

		if (priority <= rewindLastPriority) continue;

		bestRewindParameters = soundParameters;
		rewindLastPriority = priority;
	}

	if (rewindLastPriority > 0) rewind_parameters.Set(bestRewindParameters);

	return lastPriority > 0 || rewindLastPriority > 0 || softStop;
}

//This is called everytime we process a player in the queue with this source
SetupALResult SoundPlayer::SetUpALSourceIdle() {

	const auto& soundParameters = parameters.Get();
	alSourcef(audio_source->source_id, AL_PITCH, soundParameters.pitch);
	SetupALResult result = {true, true};
	bool softStopDone = false;

	if (soundParameters.is_2d) {

		const bool softStopSignal = soft_stop_signal.load();
		const bool softStartBegin = !sound_transition.allow_transition && soundParameters.soft_start;
		float volume = softStopSignal || softStartBegin ? 0.f : 1.f;

		if (soundParameters.stereo_parameters.is_panning) {
			auto pan = (acosf(std::min(soundParameters.stereo_parameters.gain_left, 1.f)) + asinf(std::min(soundParameters.stereo_parameters.gain_right, 1.f))) / ((float)M_PI); // average angle in [0,1]
			pan = 2 * pan - 1; // convert to [-1, 1]
			pan *= 0.5f; // 0.5 = sin(30') for a +/- 30 degree arc
			alSource3f(audio_source->source_id, AL_POSITION, pan, 0, -sqrtf(1.0f - pan * pan));
			volume *= soundParameters.stereo_parameters.gain_global;
		}
		else {
			alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
		}

		float finalVolume = ComputeVolumeForTransition(volume);
		result.second = finalVolume == volume && !softStartBegin;

		if (softStopSignal && finalVolume == 0)
			softStopDone = true;
		else {
			const bool actualStereo = std::get<bool>(GetAudioFormat());
			if (actualStereo && !stereo) finalVolume *= 0.5f;
			finalVolume *= OpenALManager::Get()->GetMasterVolume();
		}

		alSourcef(audio_source->source_id, AL_GAIN, finalVolume);
		alSourcef(audio_source->source_id, AL_MAX_GAIN, finalVolume);
	}
	else { //3d sounds
		const auto positionX = (float)(soundParameters.source_location3d.point.x) / WORLD_ONE;
		const auto positionY = (float)(soundParameters.source_location3d.point.y) / WORLD_ONE;
		const auto positionZ = (float)(soundParameters.source_location3d.point.z) / WORLD_ONE;

		/*This part is valid when we have yaw and pitch of the source (it seems we don't)
		* This would allow to play with OpenAL cone attenuation. For instance, a sound of a monster firing in
		* our direction would have more gain than if it's firing while facing back to us */
#if 0
		const auto yaw = soundParameters.source_location3d.yaw * angleConvert;
		const auto pitch = soundParameters.source_location3d.pitch * angleConvert;

		const ALfloat u = std::cos(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		const ALfloat v = std::sin(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		const ALfloat w = std::sin(degreToRadian * pitch);

		alSource3f(audio_source->source_id, AL_DIRECTION, u, w, v);
#endif
		alSource3f(audio_source->source_id, AL_POSITION, positionX, positionZ, positionY);
		result = SetUpALSource3D();
	}

	result.first &= alGetError() == AL_NO_ERROR && !softStopDone;
	return result;
}

//This is called once, when we assign the source to the player
bool SoundPlayer::SetUpALSourceInit() {

	alSourcei(audio_source->source_id, AL_GAIN, 0);
	alSourcei(audio_source->source_id, AL_MAX_GAIN, 0);
	alSourcei(audio_source->source_id, AL_MIN_GAIN, 0);
	alSourcei(audio_source->source_id, AL_DIRECT_FILTER, AL_FILTER_NULL);

	if (parameters.Get().is_2d) {
		alSourcei(audio_source->source_id, AL_DISTANCE_MODEL, AL_NONE);
		alSourcei(audio_source->source_id, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
		alSourcei(audio_source->source_id, AL_ROLLOFF_FACTOR, 0);
		alSource3i(audio_source->source_id, AL_DIRECTION, 0, 0, 0);
		alSourcei(audio_source->source_id, AL_REFERENCE_DISTANCE, 0);
		alSourcei(audio_source->source_id, AL_MAX_DISTANCE, 0);
	}
	else {
		alSourcei(audio_source->source_id, AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE_CLAMPED);
		alSourcei(audio_source->source_id, AL_SOURCE_RELATIVE, AL_FALSE);
	}

#ifdef AL_SOFT_source_spatialize
	if (OpenALManager::Get()->IsExtensionSupported(OpenALManager::OptionalExtension::Spatialization))
		alSourcei(audio_source->source_id, AL_SOURCE_SPATIALIZE_SOFT, AL_TRUE);
#endif

#ifdef AL_SOFT_direct_channels_remix
	if (OpenALManager::Get()->IsExtensionSupported(OpenALManager::OptionalExtension::DirectChannelRemix))
		alSourcei(audio_source->source_id, AL_DIRECT_CHANNELS_SOFT, MustDisableHrtf() ? AL_REMIX_UNMATCHED_SOFT : AL_FALSE);
#endif

	return alGetError() == AL_NO_ERROR;
}

void SoundPlayer::ResetTransition() {
	sound_transition.allow_transition = false;
	sound_transition.start_transition_tick = 0;
}

bool SoundPlayer::MustDisableHrtf() const {
	return OpenALManager::Get()->IsHrtfEnabled() && parameters.Get().is_2d && OpenALManager::Get()->IsExtensionSupported(OpenALManager::OptionalExtension::DirectChannelRemix);
}

float SoundPlayer::ComputeParameterForTransition(float targetParameter, float currentParameter, uint64_t currentTick) const {
	const float computedParameter = std::max((targetParameter - currentParameter) * std::min((currentTick - sound_transition.start_transition_tick) / (float)smooth_volume_transition_time_ms, 1.f) + currentParameter, 0.f);
	return targetParameter > currentParameter ? std::min(targetParameter, computedParameter) : std::max(targetParameter, computedParameter);
}

SoundBehavior SoundPlayer::ComputeVolumeForTransition(const SoundBehavior& targetSoundBehavior) {

	auto computedSoundBehavior = targetSoundBehavior;

	if (sound_transition.allow_transition && targetSoundBehavior != sound_transition.current_sound_behavior) {

		const auto currentTick = SoundManager::GetCurrentAudioTick();

		if (sound_transition.start_transition_tick == 0) {
			sound_transition.start_transition_tick = currentTick;
		}

		computedSoundBehavior.distance_max = ComputeParameterForTransition(targetSoundBehavior.distance_max, sound_transition.current_sound_behavior.distance_max, currentTick);
		computedSoundBehavior.distance_reference = ComputeParameterForTransition(targetSoundBehavior.distance_reference, sound_transition.current_sound_behavior.distance_reference, currentTick);
		computedSoundBehavior.max_gain = ComputeParameterForTransition(targetSoundBehavior.max_gain, sound_transition.current_sound_behavior.max_gain, currentTick);
		computedSoundBehavior.rolloff_factor = ComputeParameterForTransition(targetSoundBehavior.rolloff_factor, sound_transition.current_sound_behavior.rolloff_factor, currentTick);
		computedSoundBehavior.high_frequency_gain = ComputeParameterForTransition(targetSoundBehavior.high_frequency_gain, sound_transition.current_sound_behavior.high_frequency_gain, currentTick);
		sound_transition.start_transition_tick = currentTick;
	}

	sound_transition.allow_transition = true;
	sound_transition.current_sound_behavior = computedSoundBehavior;

	if (sound_transition.current_sound_behavior == targetSoundBehavior) {
		sound_transition.start_transition_tick = 0;
	}

	return computedSoundBehavior;
}

float SoundPlayer::ComputeVolumeForTransition(float targetVolume) {

	float volume = targetVolume;

	if (sound_transition.allow_transition && std::abs(targetVolume - sound_transition.current_volume) > smooth_volume_transition_threshold) {

		const auto currentTick = SoundManager::GetCurrentAudioTick();

		if (sound_transition.start_transition_tick == 0) {
			sound_transition.start_transition_tick = currentTick;
		}

		volume = ComputeParameterForTransition(targetVolume, sound_transition.current_volume, currentTick);
		sound_transition.start_transition_tick = currentTick;
	}
	
	sound_transition.allow_transition = true;
	sound_transition.current_volume = volume;

	if (sound_transition.current_volume == targetVolume) {
		sound_transition.start_transition_tick = 0;
	}

	return volume;
}

//Distance units are WORLD_ONE and are a copy of sound_behavior_definition for most part
SetupALResult SoundPlayer::SetUpALSource3D() {

	const auto& soundParameters = parameters.Get();
	const bool obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) || (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const bool double_obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) && (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const bool muffled = soundParameters.obstruction_flags & _sound_was_media_muffled;
	float volume = OpenALManager::Get()->GetMasterVolume();

#if 0 //previous rulesets for obstructions

	if (muffled) volume /= 2;

	//Exception to the rule
	if (soundParameters.behavior == _sound_is_quiet && obstruction) {
		alSourcef(audio_source->source_id, AL_GAIN, 0);
		return;
	}

	//One more rule for this case
	if (soundParameters.behavior == _sound_is_loud && !obstruction) {
		alSourcef(audio_source->source_id, AL_MIN_GAIN, volume / 8);
	}

#endif // 0

	const auto& behaviorParameters = double_obstruction || (obstruction && muffled) ? sound_obstructed_and_muffled_behavior_parameters[soundParameters.behavior] :
										obstruction || muffled ? sound_obstructed_or_muffled_behavior_parameters[soundParameters.behavior] :
																sound_behavior_parameters[soundParameters.behavior];

	const auto finalBehaviorParameters = ComputeVolumeForTransition(behaviorParameters);

	alSourcef(audio_source->source_id, AL_REFERENCE_DISTANCE, finalBehaviorParameters.distance_reference);
	alSourcef(audio_source->source_id, AL_MAX_DISTANCE, finalBehaviorParameters.distance_max);
	alSourcef(audio_source->source_id, AL_ROLLOFF_FACTOR, finalBehaviorParameters.rolloff_factor);
	alSourcef(audio_source->source_id, AL_MAX_GAIN, finalBehaviorParameters.max_gain * volume);
	alSourcef(audio_source->source_id, AL_GAIN, finalBehaviorParameters.max_gain * volume);
	alSourcei(audio_source->source_id, AL_DIRECT_FILTER, OpenALManager::Get()->GetLowPassFilter(finalBehaviorParameters.high_frequency_gain));
	return SetupALResult(alGetError() == AL_NO_ERROR, finalBehaviorParameters == behaviorParameters);
}

uint32_t SoundPlayer::ProcessData(uint8_t* outputData, uint32_t remainingSoundDataLength, uint32_t remainingBufferLength) {

	const auto& sound = this->sound.Get();

	if (sound.header.stereo || !MustDisableHrtf())
	{
		const auto length = std::min(remainingSoundDataLength, remainingBufferLength);
		std::copy(sound.data->data() + current_index_data, sound.data->data() + current_index_data + length, outputData);
		current_index_data += length;
		return length;
	}

	//all of this is just for hrtf
	const auto input = (sound.data->data() + current_index_data);

	switch (sound.header.audio_format) {
	case AudioFormat::_8_bit:
	{
		int index = 0;
		while (index < remainingSoundDataLength && index < remainingBufferLength / 2)
		{
			auto byte = input[index];
			outputData[2 * index] = byte;
			outputData[2 * index + 1] = byte;
			index++;
		}

		current_index_data += index;
		return index * 2;
	}
	case AudioFormat::_16_bit:
	{
		int index = 0;
		int16_t* output = reinterpret_cast<int16_t*>(outputData);

		while (index < remainingSoundDataLength / 2 && index < remainingBufferLength / 4)
		{
			int16_t sample;
			std::memcpy(&sample, input + index * 2, sizeof(int16_t));
			output[2 * index] = sample;
			output[2 * index + 1] = sample;
			index++;
		}

		current_index_data += index * 2;
		return index * 4;
	}
	case AudioFormat::_32_float: //we don't support this format for sounds
	default:
		assert(false);
		return 0;
	}
}

uint32_t SoundPlayer::GetNextData(uint8* data, uint32_t length) {
	const auto remainingDataLength = data_length - current_index_data;
	return ProcessData(data, remainingDataLength, length);
}

std::tuple<AudioFormat, uint32_t, bool> SoundPlayer::GetAudioFormat() const {

	if (sound.Get().header.stereo || !MustDisableHrtf())
		return AudioPlayer::GetAudioFormat();

	return std::make_tuple(format, rate, true);
}
#include "SoundPlayer.h"
#include "OpenALManager.h"

constexpr SoundBehavior SoundPlayer::sound_behavior_parameters[];
constexpr SoundBehavior SoundPlayer::sound_obstruct_behavior_parameters[];
SoundPlayer::SoundPlayer(const Sound sound, SoundParameters parameters)
	: AudioPlayer(sound.header.rate >> 16, sound.header.stereo, sound.header.audio_format) {  //since header.rate is on 16.16 format
	parameters.loop = parameters.loop || sound.header.loop_end - sound.header.loop_start >= 4;
	this->sound = sound;
	this->parameters = parameters;
	data_length = sound.header.length;
	start_tick = GetCurrentTick();
}

//Simulate what the volume of our sound would be if we play it
//If the volume is 0 then we just don't play the sound and drop it
float SoundPlayer::Simulate(const SoundParameters soundParameters) {
	if (soundParameters.local && !soundParameters.stereo_parameters.is_panning) return 1; //ofc we play all local sounds without stereo panning
	if (soundParameters.stereo_parameters.is_panning) return soundParameters.stereo_parameters.gain_global;

	const auto listener = OpenALManager::Get()->GetListener(); //if we don't have a listener on a non local sound, there is a problem
	float distance = std::sqrt(
		std::pow((float)(soundParameters.source_location3d.point.x - listener.point.x) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.y - listener.point.y) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.z - listener.point.z) / WORLD_ONE, 2)
	);

	const bool obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) || (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const auto behaviorParameters = obstruction ? sound_obstruct_behavior_parameters[soundParameters.behavior] : sound_behavior_parameters[soundParameters.behavior];

	if (distance > behaviorParameters.distance_max) {
		return 0;
	}

	//This is the AL_INVERSE_DISTANCE_CLAMPED function we simulate
	distance = std::max(distance, behaviorParameters.distance_reference);
	float volume = behaviorParameters.distance_reference / (behaviorParameters.distance_reference + behaviorParameters.rolloff_factor * (distance - behaviorParameters.distance_reference));

	return volume;
}

void SoundPlayer::AskRewind(SoundParameters soundParameters, const Sound& newSound) {
	soundParameters._is_for_rewind = true;
	UpdateParameters(soundParameters);

	if (parameters.Get().permutation != soundParameters.permutation) {
		sound.Store(newSound);
	}

	AudioPlayer::AskRewind();
}

void SoundPlayer::Rewind() {
	if ((OpenALManager::Get()->IsBalanceRewindSound() || rewind_parameters.source_identifier != parameters.Get().source_identifier) && !CanRewindSound(start_tick))
		rewind_signal = false;
	else {
		sound.Update();
		parameters.Set(rewind_parameters);
		AudioPlayer::Rewind();
		current_index_data = 0;
		data_length = sound.Get().header.length;
		start_tick = GetCurrentTick();
		sound_transition.allow_transition = false;
	}
}

int SoundPlayer::LoopManager(uint8* data, int length) {
	if (parameters.Get().loop) {

		auto header = sound.Get().header;
		int loopLength = header.loop_end - header.loop_start;

		if (loopLength >= 4) {
			data_length = loopLength;
			current_index_data = header.loop_start;
		}
		else {
			current_index_data = 0;
		}

		return GetNextData(data, length);
	}

	return 0;
}

bool SoundPlayer::LoadParametersUpdates() {

	SoundParameters soundParameters, bestParameters, bestRewindParameters;
	float lastPriority = 0, rewindLastPriority = 0;

	while (parameters.Consume(soundParameters)) {

		float priority = Simulate(soundParameters);

		if (soundParameters._is_for_rewind) {

			if (priority <= rewindLastPriority) continue;

			bestRewindParameters = soundParameters;
			rewindLastPriority = priority;

		} else {

			if (priority <= lastPriority) continue;

			bestParameters = soundParameters;
			lastPriority = priority;
		}
	}

	if (lastPriority > 0) {
		parameters.Set(bestParameters);
		sound_transition.allow_transition = true;
		sound_transition.last_update_tick = GetCurrentTick();
	}

	if (rewindLastPriority > 0) rewind_parameters = bestRewindParameters;

	return lastPriority > 0 || rewindLastPriority > 0;
}

//This is called everytime we process a player in the queue with this source
std::pair<bool, bool> SoundPlayer::SetUpALSourceIdle() {

	auto& soundParameters = parameters.Get();
	alSourcef(audio_source->source_id, AL_PITCH, soundParameters.pitch);
	bool updatesDone = true;

	if (soundParameters.local) {

		float vol = volume;
		if (soundParameters.stereo_parameters.is_panning) {
			auto pan = (acosf(std::min(soundParameters.stereo_parameters.gain_left, 1.f)) + asinf(std::min(soundParameters.stereo_parameters.gain_right, 1.f))) / ((float)M_PI); // average angle in [0,1]
			pan = 2 * pan - 1; // convert to [-1, 1]
			pan *= 0.5f; // 0.5 = sin(30') for a +/- 30 degree arc
			alSource3f(audio_source->source_id, AL_POSITION, pan, 0, -sqrtf(1.0f - pan * pan));
			vol *= soundParameters.stereo_parameters.gain_global;
		}
		else {
			alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
		}

		ComputeVolumeForTransition(vol);
		auto finalVolume = sound_transition.current_volume;
		updatesDone = sound_transition.current_volume == vol;

		printf("%f\n", sound_transition.current_volume);

		finalVolume *= OpenALManager::Get()->GetMasterVolume();
		alSourcef(audio_source->source_id, AL_GAIN, finalVolume);
		alSourcef(audio_source->source_id, AL_MAX_GAIN, finalVolume);
	}
	else { //3d sounds
		auto positionX = (float)(soundParameters.source_location3d.point.x) / WORLD_ONE;
		auto positionY = (float)(soundParameters.source_location3d.point.y) / WORLD_ONE;
		auto positionZ = (float)(soundParameters.source_location3d.point.z) / WORLD_ONE;

		/*This part is valid when we have yaw and pitch of the source (it seems we don't)
		* This would allow to play with OpenAL cone attenuation. For instance, a sound of a monster firing in
		* our direction would have more gain than if it's firing while facing back to us */
#if 0
		auto yaw = soundParameters.source_location3d.yaw * angleConvert;
		auto pitch = soundParameters.source_location3d.pitch * angleConvert;

		ALfloat u = std::cos(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		ALfloat	v = std::sin(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		ALfloat	w = std::sin(degreToRadian * pitch);

		alSource3f(audio_source->source_id, AL_DIRECTION, u, w, v);
#endif
		alSource3f(audio_source->source_id, AL_POSITION, positionX, positionZ, positionY);
		SetUpALSource3D();
	}

	return std::pair<bool, bool>(alGetError() == AL_NO_ERROR, updatesDone);
}

//This is called once, when we assign the source to the player
bool SoundPlayer::SetUpALSourceInit() {

	alSourcei(audio_source->source_id, AL_GAIN, 0);
	alSourcei(audio_source->source_id, AL_MAX_GAIN, 0);
	alSourcei(audio_source->source_id, AL_MIN_GAIN, 0);
	alSourcei(audio_source->source_id, AL_DIRECT_FILTER, AL_FILTER_NULL);

	if (parameters.Get().local) {
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

	return alGetError() == AL_NO_ERROR;
}

void SoundPlayer::ComputeVolumeForTransition(float targetVolume) {

	float volume;
	auto currentTick = GetCurrentTick();

	if (sound_transition.allow_transition && std::abs(targetVolume - sound_transition.current_volume) > smooth_volume_transition_threshold) {
		volume = std::max((targetVolume - sound_transition.current_volume) * std::min((currentTick - sound_transition.last_update_tick) / (float)smooth_volume_transition_time_ms, 1.f) + sound_transition.current_volume, 0.f);
		volume = targetVolume > sound_transition.current_volume ? std::min(targetVolume, volume) : std::max(targetVolume, volume);
	}
	else {
		volume = targetVolume;
	}
	
	sound_transition.last_update_tick = currentTick;
	sound_transition.current_volume = volume;
}

//Distance units are WORLD_ONE and are a copy of sound_behavior_definition for most part
void SoundPlayer::SetUpALSource3D() const {

	auto& sound_parameters = parameters.Get();
	bool obstruction = (sound_parameters.obstruction_flags & _sound_was_obstructed) || (sound_parameters.obstruction_flags & _sound_was_media_obstructed);
	bool muffled = sound_parameters.obstruction_flags & _sound_was_media_muffled;
	float calculated_volume = volume * OpenALManager::Get()->GetMasterVolume();

#if 0 //previous rulesets for obstructions

	if (muffled) calculated_volume /= 2;

	//Exception to the rule
	if (sound_parameters.behavior == _sound_is_quiet && obstruction) {
		alSourcef(audio_source->source_id, AL_GAIN, 0);
		return;
	}

	//One more rule for this case
	if (sound_parameters.behavior == _sound_is_loud && !obstruction) {
		alSourcef(audio_source->source_id, AL_MIN_GAIN, calculated_volume / 8);
	}

#endif // 0

	SoundBehavior behaviorParameters = obstruction ? sound_obstruct_behavior_parameters[sound_parameters.behavior] : sound_behavior_parameters[sound_parameters.behavior];
	alSourcef(audio_source->source_id, AL_REFERENCE_DISTANCE, behaviorParameters.distance_reference);
	alSourcef(audio_source->source_id, AL_MAX_DISTANCE, behaviorParameters.distance_max);
	alSourcef(audio_source->source_id, AL_ROLLOFF_FACTOR, behaviorParameters.rolloff_factor);
	alSourcef(audio_source->source_id, AL_MAX_GAIN, behaviorParameters.max_gain * calculated_volume);
	alSourcef(audio_source->source_id, AL_GAIN, behaviorParameters.max_gain * calculated_volume);
	alSourcei(audio_source->source_id, AL_DIRECT_FILTER, muffled || obstruction ? OpenALManager::Get()->GetObstructionFilter() : AL_FILTER_NULL);
}

int SoundPlayer::GetNextData(uint8* data, int length) {
	int remainingDataLength = data_length - current_index_data;
	if (remainingDataLength <= 0) return LoopManager(data, length);
	int returnedDataLength = std::min(remainingDataLength, length);
	auto& sound_data = sound.Get().data;
	std::copy(sound_data.data() + current_index_data, sound_data.data() + current_index_data + returnedDataLength, data);
	current_index_data += returnedDataLength;
	if (returnedDataLength < length && parameters.Get().loop) return returnedDataLength + LoopManager(data + returnedDataLength, length - returnedDataLength);
	return returnedDataLength;
}
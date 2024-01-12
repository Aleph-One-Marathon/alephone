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
	auto& sound = this->sound.Get();
	AudioPlayer::Init(sound.header.rate >> 16, sound.header.stereo, sound.header.audio_format);
	auto soundParameters = parameters;
	soundParameters.loop = parameters.loop || sound.header.loop_end - sound.header.loop_start >= 4;
	this->parameters.Set(soundParameters);
	data_length = sound.header.length;
	start_tick = SoundManager::GetCurrentAudioTick();
	sound_transition.allow_transition = false;
	sound_transition.start_transition_tick = 0;
	current_index_data = 0;
}

//Simulate what the volume of our sound would be if we play it
//If the volume is 0 then we just don't play the sound and drop it
float SoundPlayer::Simulate(const SoundParameters& soundParameters) {
	if (soundParameters.is_2d && !soundParameters.stereo_parameters.is_panning) return 1; //ofc we play all 2d sounds without stereo panning
	if (soundParameters.stereo_parameters.is_panning) return soundParameters.stereo_parameters.gain_global;

	const auto& listener = OpenALManager::Get()->GetListener(); //if we don't have a listener on a 3d sound, there is a problem
	float distance = std::sqrt(
		std::pow((float)(soundParameters.source_location3d.point.x - listener.point.x) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.y - listener.point.y) / WORLD_ONE, 2) +
		std::pow((float)(soundParameters.source_location3d.point.z - listener.point.z) / WORLD_ONE, 2)
	);

	const bool obstruction = (soundParameters.obstruction_flags & _sound_was_obstructed) || (soundParameters.obstruction_flags & _sound_was_media_obstructed);
	const bool muffled = soundParameters.obstruction_flags & _sound_was_media_muffled;

	const auto& behaviorParameters = obstruction && muffled ? sound_obstructed_and_muffled_behavior_parameters[soundParameters.behavior] :
		obstruction || muffled ? sound_obstructed_or_muffled_behavior_parameters[soundParameters.behavior] :
		sound_behavior_parameters[soundParameters.behavior];

	if (distance > behaviorParameters.distance_max) {
		return 0;
	}

	//This is the AL_INVERSE_DISTANCE_CLAMPED function we simulate
	distance = std::max(distance, behaviorParameters.distance_reference);
	float volume = behaviorParameters.distance_reference / (behaviorParameters.distance_reference + behaviorParameters.rolloff_factor * (distance - behaviorParameters.distance_reference));

	return volume;
}

bool SoundPlayer::CanRewind(int baseTick) const {
	const auto& rewindParameters = rewind_parameters.Get();
	if (rewindParameters.soft_rewind) return true;
	auto rewindTime = OpenALManager::Get()->IsBalanceRewindSound() || !CanFastRewind(rewindParameters) ? rewind_time : fast_rewind_time;
	return baseTick + rewindTime < SoundManager::GetCurrentAudioTick();
}

bool SoundPlayer::CanFastRewind(const SoundParameters& soundParameters) const {
	return (soundParameters.source_identifier != NONE || (soundParameters.is_2d && !soundParameters.stereo_parameters.is_panning))
		&& soundParameters.source_identifier == GetSourceIdentifier();
}

void SoundPlayer::AskRewind(const SoundParameters& soundParameters, const Sound& newSound) {
	UpdateRewindParameters(soundParameters);

	if (parameters.Get().permutation != soundParameters.permutation) {
		sound.Store(newSound);
	}

	AudioPlayer::AskRewind();
}

void SoundPlayer::Rewind() {

	if (!CanRewind(start_tick)) {
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

	sound.Update();
	Init(rewindParameters);
	if (!rewindParameters.soft_rewind) SetUpALSourceInit();
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

	bool softStop = soft_stop_signal.load();
	SoundParameters soundParameters, bestParameters, bestRewindParameters;
	float lastPriority = 0, rewindLastPriority = 0;

	if (softStop && sound_transition.allow_transition) return true;

	while (parameters.Consume(soundParameters)) {

		float priority = Simulate(soundParameters);

		if (priority <= lastPriority) continue;

		bestParameters = soundParameters;
		lastPriority = priority;
	}

	if (lastPriority > 0) parameters.Set(bestParameters);

	while (rewind_parameters.Consume(soundParameters)) {

		float priority = Simulate(soundParameters);

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

		bool softStopSignal = soft_stop_signal.load();
		float volume = softStopSignal ? 0 : 1;

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
		result.second = finalVolume == volume;

		if (softStopSignal && finalVolume == 0) softStopDone = true;

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

	return alGetError() == AL_NO_ERROR;
}

float SoundPlayer::ComputeParameterForTransition(float targetParameter, float currentParameter, int currentTick) const {
	float computedParameter = std::max((targetParameter - currentParameter) * std::min((currentTick - sound_transition.start_transition_tick) / (float)smooth_volume_transition_time_ms, 1.f) + currentParameter, 0.f);
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
	const bool muffled = soundParameters.obstruction_flags & _sound_was_media_muffled;
	float volume = OpenALManager::Get()->GetMasterVolume();

#if 0 //previous rulesets for obstructions

	if (muffled) volume /= 2;

	//Exception to the rule
	if (sound_parameters.behavior == _sound_is_quiet && obstruction) {
		alSourcef(audio_source->source_id, AL_GAIN, 0);
		return;
	}

	//One more rule for this case
	if (sound_parameters.behavior == _sound_is_loud && !obstruction) {
		alSourcef(audio_source->source_id, AL_MIN_GAIN, volume / 8);
	}

#endif // 0

	const auto& behaviorParameters = obstruction && muffled ? sound_obstructed_and_muffled_behavior_parameters[soundParameters.behavior] :
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

int SoundPlayer::GetNextData(uint8* data, int length) {
	int remainingDataLength = data_length - current_index_data;
	if (remainingDataLength <= 0) return LoopManager(data, length);
	int returnedDataLength = std::min(remainingDataLength, length);
	auto& sound_data = sound.Get().data;
	std::copy(sound_data->data() + current_index_data, sound_data->data() + current_index_data + returnedDataLength, data);
	current_index_data += returnedDataLength;
	if (returnedDataLength < length && parameters.Get().loop) return returnedDataLength + LoopManager(data + returnedDataLength, length - returnedDataLength);
	return returnedDataLength;
}
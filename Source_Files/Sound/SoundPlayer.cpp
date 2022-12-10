#include "SoundPlayer.h"
#include "OpenALManager.h"
#include "Movie.h"

constexpr SoundBehavior SoundPlayer::sound_behavior_parameters[];
constexpr SoundBehavior SoundPlayer::sound_obstruct_behavior_parameters[];
SoundPlayer::SoundPlayer(const Sound sound, SoundParameters parameters)
	: AudioPlayer(sound.header.rate >> 16, sound.header.stereo, sound.header.sixteen_bit) {  //since header.rate is on 16.16 format
	parameters.loop = parameters.loop || sound.header.loop_end - sound.header.loop_start >= 4;
	this->sound = sound;
	this->parameters = parameters;
	data_length = sound.header.length;
	filterable = parameters.filterable;
	SetStartTick();
}

//Simulate what the volume of our sound would be if we play it
//If the volume is 0 then we just don't play the sound and drop it
float SoundPlayer::Simulate(SoundParameters soundParameters) {
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

	//This is the AL_LINEAR_DISTANCE_CLAMPED function we simulate
	distance = std::max(distance, behaviorParameters.distance_reference);
	distance = std::min(distance, behaviorParameters.distance_max);
	float volume = 1 - behaviorParameters.rolloff_factor * (distance - behaviorParameters.distance_reference) / (behaviorParameters.distance_max - behaviorParameters.distance_reference);

	return volume;
}

void SoundPlayer::UpdateParameters(SoundParameters parameters) {
	this->parameters.Store(parameters);
}

void SoundPlayer::Replace(const Sound sound) {
	this->sound.Store(sound);
}

void SoundPlayer::Rewind() {
	if (OpenALManager::Get()->IsBalanceRewindSound() && !CanRewindSound(start_tick))
		rewind_state = false;
	else {
		AudioPlayer::Rewind();
		current_index_data = 0;
		SetStartTick();
	}
}

bool SoundPlayer::CanRewindSound(int baseTick) const {
	if (Movie::instance()->IsRecording())
		return baseTick + rewind_time < Movie::instance()->GetCurrentAudioTimeStamp();
	else
		return baseTick + rewind_time < machine_tick_count();
}

void SoundPlayer::SetStartTick() {
	start_tick = Movie::instance()->IsRecording() ? Movie::instance()->GetCurrentAudioTimeStamp() : machine_tick_count();
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

bool SoundPlayer::Update() {

	SoundParameters sound_parameters, best_parameters;
	float priority = 0, last_priority = 0;
	while (parameters.Consume(sound_parameters)) {

		priority = Simulate(sound_parameters);

		if (priority > last_priority) {
			best_parameters = sound_parameters;
			last_priority = priority;
		}
	}

	if (last_priority > 0) {
		parameters.Set(best_parameters);
	}

	sound.Update();
	return true;
}

//This is called everytime we process a player in the queue with this source
bool SoundPlayer::SetUpALSourceIdle() const {

	auto& sound_parameters = parameters.Get();
	alSourcef(audio_source->source_id, AL_PITCH, sound_parameters.pitch);

	if (sound_parameters.local) {

		float vol = volume ? volume * OpenALManager::Get()->GetComputedVolume(filterable) : OpenALManager::Get()->GetComputedVolume(filterable);
		if (sound_parameters.stereo_parameters.is_panning) {
			auto pan = (acosf(std::min(sound_parameters.stereo_parameters.gain_left, 1.f)) + asinf(std::min(sound_parameters.stereo_parameters.gain_right, 1.f))) / ((float)M_PI); // average angle in [0,1]
			pan = 2 * pan - 1; // convert to [-1, 1]
			pan *= 0.5f; // 0.5 = sin(30') for a +/- 30 degree arc
			alSource3f(audio_source->source_id, AL_POSITION, pan, 0, -sqrtf(1.0f - pan * pan));
			vol *= sound_parameters.stereo_parameters.gain_global;
		}
		else {
			alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
		}

		alSourcef(audio_source->source_id, AL_GAIN, vol);
		alSourcef(audio_source->source_id, AL_MAX_GAIN, vol);
	}
	else {
		auto positionX = (float)(sound_parameters.source_location3d.point.x) / WORLD_ONE;
		auto positionY = (float)(sound_parameters.source_location3d.point.y) / WORLD_ONE;
		auto positionZ = (float)(sound_parameters.source_location3d.point.z) / WORLD_ONE;

		/*This part is valid when we have yaw and pitch of the source (it seems we don't)
		* This would allow to play with OpenAL cone attenuation. For instance, a sound of a monster firing in
		* our direction would have more gain than if it's firing while facing back to us */
#if 0
		auto yaw = parameters.source_location3d.yaw * angleConvert;
		auto pitch = parameters.source_location3d.pitch * angleConvert;

		ALfloat u = std::cos(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		ALfloat	v = std::sin(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
		ALfloat	w = std::sin(degreToRadian * pitch);

		alSource3f(audio_source->source_id, AL_DIRECTION, u, w, v);
#endif
		alSource3f(audio_source->source_id, AL_POSITION, positionX, positionZ, positionY);
		SetUpALSource3D();
	}

	return true;
}

//This is called once, when we assign the source to the player
bool SoundPlayer::SetUpALSourceInit() const {
	alSourcei(audio_source->source_id, AL_MIN_GAIN, 0);

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
		alSourcei(audio_source->source_id, AL_DISTANCE_MODEL, AL_LINEAR_DISTANCE_CLAMPED);
		alSourcei(audio_source->source_id, AL_SOURCE_RELATIVE, AL_FALSE);
	}

	return true;
}

//The model used (attenuation function) is set in openalmanager constructor
//We use the AL_LINEAR_DISTANCE_CLAMPED function which isn't really realistic but this is how actual
//sounds work in marathon. If we want more realistic sounds, we should move to the default function AL_INVERSE_DISTANCE_CLAMPED
//Obstructions can also be done using openal filters but I won't use it here
//Distance units are WORLD_ONE and are a copy of sound_behavior_definition for most part
void SoundPlayer::SetUpALSource3D() const {

	auto& sound_parameters = parameters.Get();
	bool obstruction = (sound_parameters.obstruction_flags & _sound_was_obstructed) || (sound_parameters.obstruction_flags & _sound_was_media_obstructed);
	bool muffled = sound_parameters.obstruction_flags & _sound_was_media_muffled;
	float calculated_volume = volume ? volume * OpenALManager::Get()->GetComputedVolume(filterable) : OpenALManager::Get()->GetComputedVolume(filterable);

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

	SoundBehavior behaviorParameters = obstruction ? sound_obstruct_behavior_parameters[sound_parameters.behavior] : sound_behavior_parameters[sound_parameters.behavior];
	alSourcef(audio_source->source_id, AL_REFERENCE_DISTANCE, behaviorParameters.distance_reference);
	alSourcef(audio_source->source_id, AL_MAX_DISTANCE, behaviorParameters.distance_max);
	alSourcef(audio_source->source_id, AL_ROLLOFF_FACTOR, behaviorParameters.rolloff_factor);
	alSourcef(audio_source->source_id, AL_MAX_GAIN, behaviorParameters.max_gain * calculated_volume);
	alSourcef(audio_source->source_id, AL_GAIN, behaviorParameters.max_gain * calculated_volume);
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

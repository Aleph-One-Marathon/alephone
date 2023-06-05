#include "AudioPlayer.h"
#include "OpenALManager.h"
#include "Movie.h"
#include <array>

AudioPlayer::AudioPlayer(int rate, bool stereo, AudioFormat audioFormat) {
	this->rate = rate;
	format = mapping_audio_format_openal.at({ audioFormat, stereo });
}

bool AudioPlayer::AssignSource() {
	if (audio_source) return true;
	audio_source = OpenALManager::Get()->PickAvailableSource(GetPriority());
	return audio_source && SetUpALSourceInit();
}

void AudioPlayer::ResetSource() {
	if (!audio_source) return;

	alSourceStop(audio_source->source_id);

	//let's be sure all of our buffers are detached from the source
	alSourcei(audio_source->source_id, AL_BUFFER, 0);

	for (auto& buffer : audio_source->buffers) {
		buffer.second = false;
	}

	SetUpALSourceInit();
}

int AudioPlayer::GetCurrentTick() const { 
	return Movie::instance()->IsRecording() ? Movie::instance()->GetCurrentAudioTimeStamp() : machine_tick_count(); 
}

//Get back the source of the player
std::unique_ptr<AudioPlayer::AudioSource> AudioPlayer::RetrieveSource() {
	ResetSource();
	return std::move(audio_source);
}

void AudioPlayer::UnqueueBuffers() {
	ALint nbBuffersProcessed;
	alGetSourcei(audio_source->source_id, AL_BUFFERS_PROCESSED, &nbBuffersProcessed);

	while (nbBuffersProcessed-- > 0) {
		ALuint bufferId;
		alSourceUnqueueBuffers(audio_source->source_id, 1, &bufferId);
		audio_source->buffers[bufferId] = false;
	}
}

void AudioPlayer::FillBuffers() {

	UnqueueBuffers(); //First we unqueue buffers that can be

	for (auto& buffer : audio_source->buffers) { //now we process our buffers that are ready

		if (buffer.second) continue;

		std::array<uint8, buffer_samples> data = {};
		size_t bufferOffset = 0;

		while (buffer_samples > bufferOffset) {
			int actualDataLength = GetNextData(data.data() + bufferOffset, buffer_samples - bufferOffset);
			if (actualDataLength <= 0) break;
			bufferOffset += actualDataLength;
		}

		if (bufferOffset <= 0) return;
		alBufferData(buffer.first, format, data.data(), bufferOffset, rate);
		alSourceQueueBuffers(audio_source->source_id, 1, &buffer.first);
		buffer.second = true;
	}
}

//we stop the source to get rid of the playing sounds
void AudioPlayer::Rewind() {
	ResetSource();
	rewind_signal = false;
}

bool AudioPlayer::Play() {

	FillBuffers();
	ALint state;

	//Get relevant source info 
	alGetSourcei(audio_source->source_id, AL_SOURCE_STATE, &state);

	//Make sure the source hasn't underrun 
	if (state != AL_PLAYING && state != AL_PAUSED)
	{
		ALint queued;

		//If no buffers are queued, playback is finished 
		alGetSourcei(audio_source->source_id, AL_BUFFERS_QUEUED, &queued);
		if (queued == 0) return false; //End playing

		alSourcePlay(audio_source->source_id);
	}

	return true; //still has to play some data
}

bool AudioPlayer::Update() {
	bool needsUpdate = LoadParametersUpdates() || !is_sync_with_al_parameters;
	if (rewind_signal) Rewind();
	if (!needsUpdate) return true;
	is_sync_with_al_parameters = true;
	auto updateStatus = SetUpALSourceIdle();
	is_sync_with_al_parameters.store(is_sync_with_al_parameters && updateStatus.second);
	return updateStatus.first;
}

void AudioPlayer::SetVolume(float volume) {
	if (volume == this->volume) return;
	this->volume = volume;
	is_sync_with_al_parameters = false;
}

SetupALResult AudioPlayer::SetUpALSourceIdle() {
	float audio_volume = volume.load();
	float master_volume = OpenALManager::Get()->GetMasterVolume();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, master_volume);
	alSourcef(audio_source->source_id, AL_GAIN, audio_volume * master_volume);
	return SetupALResult(alGetError() == AL_NO_ERROR, true);
}

bool AudioPlayer::SetUpALSourceInit() {
	alSourcei(audio_source->source_id, AL_MIN_GAIN, 0);
	alSourcei(audio_source->source_id, AL_PITCH, 1);
	alSourcei(audio_source->source_id, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
	alSourcei(audio_source->source_id, AL_ROLLOFF_FACTOR, 0);
	alSource3i(audio_source->source_id, AL_DIRECTION, 0, 0, 0);
	alSourcei(audio_source->source_id, AL_DISTANCE_MODEL, AL_NONE);
	alSourcei(audio_source->source_id, AL_REFERENCE_DISTANCE, 0);
	alSourcei(audio_source->source_id, AL_MAX_DISTANCE, 0);
	alSourcei(audio_source->source_id, AL_DIRECT_FILTER, AL_FILTER_NULL);
	return alGetError() == AL_NO_ERROR;
}
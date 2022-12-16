#include "AudioPlayer.h"
#include "OpenALManager.h"
#include "Movie.h"

AudioPlayer::AudioPlayer(int rate, bool stereo, bool sixteen_bit) {
	this->rate = rate;
	format = GetCorrespondingFormat(stereo, sixteen_bit);
}

bool AudioPlayer::AssignSource() {
	if (audio_source) return true;
	audio_source = OpenALManager::Get()->PickAvailableSource(GetPriority());
	return this->audio_source && SetUpALSourceInit();
}

void AudioPlayer::ResetSource() {
	if (audio_source) {
		alSourceStop(audio_source->source_id);

		//let's be sure all of our buffers are detached from the source
		alSourcei(audio_source->source_id, AL_BUFFER, 0);

		for (auto& buffer : audio_source->buffers) {
			buffer.second = false;
		}
	}
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
		ALuint bufid;
		alSourceUnqueueBuffers(audio_source->source_id, 1, &bufid);
		audio_source->buffers[bufid] = false;
	}
}

void AudioPlayer::FillBuffers() {

	UnqueueBuffers(); //First we unqueue buffers that can be

	for (auto& buffer : audio_source->buffers) {
		if (!buffer.second) { //now we process our buffers that are ready
			std::vector<uint8> data(buffer_samples);
			int dataLength = GetNextData(data.data(), buffer_samples);
			if (dataLength <= 0) return;
			alBufferData(buffer.first, format, data.data(), dataLength, rate);
			alSourceQueueBuffers(audio_source->source_id, 1, &buffer.first);
			buffer.second = true;
		}
	}
}

//we stop the source to get rid of the playing sounds
void AudioPlayer::Rewind() {
	ResetSource();
	rewind_state = false;
}

bool AudioPlayer::Play() {

	if (rewind_state) Rewind(); //We have to restart the sound here

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

//Figure out the OpenAL format for formats we are currently supporting
int AudioPlayer::GetCorrespondingFormat(bool stereo, bool isSixteenBit) const {
	return stereo ? isSixteenBit ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8 :
		            isSixteenBit ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
}

bool AudioPlayer::SetUpALSourceIdle() const {
	float audio_volume = volume.load();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, OpenALManager::Get()->GetComputedVolume(filterable));
	alSourcef(audio_source->source_id, AL_GAIN, audio_volume * OpenALManager::Get()->GetComputedVolume(filterable));
	return true;
}

bool AudioPlayer::SetUpALSourceInit() const {
	alSourcei(audio_source->source_id, AL_MIN_GAIN, 0);
	alSourcei(audio_source->source_id, AL_PITCH, 1);
	alSourcei(audio_source->source_id, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3i(audio_source->source_id, AL_POSITION, 0, 0, 0);
	alSourcei(audio_source->source_id, AL_ROLLOFF_FACTOR, 0);
	alSource3i(audio_source->source_id, AL_DIRECTION, 0, 0, 0);
	alSourcei(audio_source->source_id, AL_DISTANCE_MODEL, AL_NONE);
	alSourcei(audio_source->source_id, AL_REFERENCE_DISTANCE, 0);
	alSourcei(audio_source->source_id, AL_MAX_DISTANCE, 0);
	return true;
}
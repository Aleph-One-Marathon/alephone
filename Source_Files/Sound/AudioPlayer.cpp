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

#include "AudioPlayer.h"
#include "OpenALManager.h"
#include <array>

AudioPlayer::AudioPlayer(uint32_t rate, bool stereo, AudioFormat audioFormat) : queued_rate(rate), queued_format(audioFormat), queued_stereo(stereo) {
	Init(rate, stereo, audioFormat);
}

void AudioPlayer::Init(uint32_t rate, bool stereo, AudioFormat audioFormat) {
	this->rate = rate;
	this->stereo = stereo;
	this->format = audioFormat;
}

bool AudioPlayer::AssignSource() {
	if (audio_source) return true;
	audio_source = OpenALManager::Get()->PickAvailableSource(*this);
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

	//OpenAL does not support queueing multiple buffers with different format for a same source so we wait
	if (HasBufferFormatChanged()) {
		ALint nbBuffersQueued;
		alGetSourcei(audio_source->source_id, AL_BUFFERS_QUEUED, &nbBuffersQueued);
		if (nbBuffersQueued > 0) return;

		auto [wantedFormat, wantedRate, wantedStereo] = GetAudioFormat();
		queued_format = wantedFormat;
		queued_rate = wantedRate;
		queued_stereo = wantedStereo;
	}

	for (auto& buffer : audio_source->buffers) { //now we process our buffers that are ready

		if (buffer.second) continue;

		std::array<uint8, buffer_samples> data = {};
		uint32_t bufferOffset = 0;

		while (buffer_samples > bufferOffset && !HasBufferFormatChanged()) {
			auto actualDataLength = GetNextData(data.data() + bufferOffset, buffer_samples - bufferOffset);
			if (!actualDataLength) break;
			bufferOffset += actualDataLength;
		}

		if (!bufferOffset) return;
		alBufferData(buffer.first, mapping_audio_format_openal.at({ queued_format, queued_stereo }), data.data(), bufferOffset, queued_rate);
		alSourceQueueBuffers(audio_source->source_id, 1, &buffer.first);
		buffer.second = true;
	}
}

bool AudioPlayer::HasBufferFormatChanged() const { 
	auto [wantedFormat, wantedRate, wantedStereo] = GetAudioFormat();
	return queued_rate != wantedRate || queued_format != wantedFormat || queued_stereo != wantedStereo;
}

//we stop the source to get rid of the playing sounds
void AudioPlayer::Rewind() {
	ResetSource();
	rewind_signal = false;
}

bool AudioPlayer::IsPlaying() const {
	if (!audio_source) return false;
	ALint state;
	alGetSourcei(audio_source->source_id, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING || state == AL_PAUSED; //underrun source is considered as playing
}

bool AudioPlayer::Play() {

	FillBuffers();

	if (!IsPlaying()) {

		ALint queued;

		//If no buffers are queued, playback is finished 
		alGetSourcei(audio_source->source_id, AL_BUFFERS_QUEUED, &queued);
		if (queued == 0) return false; //End playing

		alSourcePlay(audio_source->source_id);
	}

	return alGetError() == AL_NO_ERROR; //still has to play some data
}

bool AudioPlayer::Update() {
	bool needsUpdate = LoadParametersUpdates() || !is_sync_with_al_parameters;
	if (rewind_signal) Rewind();
	if (!needsUpdate) return true;
	auto updateStatus = SetUpALSourceIdle();
	is_sync_with_al_parameters = updateStatus.second;
	return updateStatus.first;
}

SetupALResult AudioPlayer::SetUpALSourceIdle() {
	float master_volume = OpenALManager::Get()->GetMasterVolume();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, master_volume);
	alSourcef(audio_source->source_id, AL_GAIN, master_volume);
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

#ifdef AL_SOFT_source_spatialize
	if (OpenALManager::Get()->IsExtensionSupported(OpenALManager::OptionalExtension::Spatialization))
		alSourcei(audio_source->source_id, AL_SOURCE_SPATIALIZE_SOFT, AL_AUTO_SOFT);
#endif

#ifdef AL_SOFT_direct_channels_remix
	if (OpenALManager::Get()->IsExtensionSupported(OpenALManager::OptionalExtension::DirectChannelRemix))
		alSourcei(audio_source->source_id, AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
#endif

	return alGetError() == AL_NO_ERROR;
}
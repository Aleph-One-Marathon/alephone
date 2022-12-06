#include "MusicPlayer.h"
#include "OpenALManager.h"

std::atomic<float> MusicPlayer::default_volume = { 1 };
MusicPlayer::MusicPlayer(StreamDecoder* decoder) : AudioPlayer(decoder->Rate(), decoder->IsStereo(), decoder->IsSixteenBit()) {
	this->decoder = decoder;
}

int MusicPlayer::GetNextData(uint8* data, int length) {
	return decoder->Decode(data, length);
}

bool MusicPlayer::SetUpALSourceIdle() const {
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_volume * OpenALManager::Get()->GetComputedVolume(filterable));
	alSourcef(audio_source->source_id, AL_GAIN, volume ? volume * OpenALManager::Get()->GetComputedVolume(filterable) : default_volume * OpenALManager::Get()->GetComputedVolume(filterable));
	return true;
}
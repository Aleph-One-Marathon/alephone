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
	float default_music_volume = default_volume.load();
	float music_volume = volume.load();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_music_volume * OpenALManager::Get()->GetComputedVolume(filterable));
	alSourcef(audio_source->source_id, AL_GAIN, music_volume ? music_volume * OpenALManager::Get()->GetComputedVolume(filterable) : default_music_volume * OpenALManager::Get()->GetComputedVolume(filterable));
	return true;
}
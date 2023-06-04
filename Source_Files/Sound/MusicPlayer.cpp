#include "MusicPlayer.h"
#include "OpenALManager.h"

std::atomic<float> MusicPlayer::default_volume = { 1 };
MusicPlayer::MusicPlayer(StreamDecoder* decoder) : AudioPlayer(decoder->Rate(), decoder->IsStereo(), decoder->GetAudioFormat()) {
	this->decoder = decoder;
}

int MusicPlayer::GetNextData(uint8* data, int length) {
	return decoder->Decode(data, length);
}

std::pair<bool, bool> MusicPlayer::SetUpALSourceIdle() {
	float master_volume = OpenALManager::Get()->GetMasterVolume();
	float default_music_volume = default_volume.load();
	float music_volume = volume.load();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_music_volume * master_volume);
	alSourcef(audio_source->source_id, AL_GAIN, default_music_volume * music_volume * master_volume);
	return std::pair<bool, bool>(alGetError() == AL_NO_ERROR, true);
}
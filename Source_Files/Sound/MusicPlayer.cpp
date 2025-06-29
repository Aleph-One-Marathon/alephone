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

#include "MusicPlayer.h"
#include "OpenALManager.h"

MusicPlayer::MusicPlayer(std::shared_ptr<StreamDecoder> decoder, MusicParameters parameters) : AudioPlayer(decoder->Rate(), decoder->IsStereo(), decoder->GetAudioFormat()) {
	this->decoder = decoder;
	this->parameters = parameters;
	this->decoder->Rewind();
}

uint32_t MusicPlayer::GetNextData(uint8* data, uint32_t length) {
	const auto dataSize = decoder->Decode(data, length);
	if (dataSize == length || !parameters.Get().loop) return dataSize;
	decoder->Rewind();
	return dataSize + GetNextData(data + dataSize, length - dataSize);
}

SetupALResult MusicPlayer::SetUpALSourceIdle() {
	const float default_music_volume = OpenALManager::Get()->GetMusicVolume() * OpenALManager::Get()->GetMasterVolume();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_music_volume);
	alSourcef(audio_source->source_id, AL_GAIN, default_music_volume * parameters.Get().volume);
	return SetupALResult(alGetError() == AL_NO_ERROR, true);
}
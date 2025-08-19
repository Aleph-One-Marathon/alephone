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

MusicPlayer::MusicPlayer(std::vector<Preset>& presets, uint32_t starting_preset_index, uint32_t starting_segment_index, const MusicParameters& parameters)
	: AudioPlayer(presets[starting_preset_index].GetSegment(starting_segment_index)->GetDecoder()->Rate(),
		presets[starting_preset_index].GetSegment(starting_segment_index)->GetDecoder()->IsStereo(),
		presets[starting_preset_index].GetSegment(starting_segment_index)->GetDecoder()->GetAudioFormat()) {

	this->parameters = parameters;
	current_decoder = presets[starting_preset_index].GetSegment(starting_segment_index)->GetDecoder();
	current_decoder->Rewind();
	music_presets = presets;
	current_preset_index = starting_preset_index;
	current_segment_index = starting_segment_index;
	requested_preset_index = starting_preset_index;
}

SetupALResult MusicPlayer::SetUpALSourceIdle() {
	const float default_music_volume = OpenALManager::Get()->GetMusicVolume() * OpenALManager::Get()->GetMasterVolume();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_music_volume);
	alSourcef(audio_source->source_id, AL_GAIN, default_music_volume * parameters.Get().volume);
	return SetupALResult(alGetError() == AL_NO_ERROR, true);
}

uint32_t MusicPlayer::GetNextData(uint8* data, uint32_t length) {
	const auto dataSize = current_decoder->Decode(data, length);
	if (dataSize == length) return dataSize;

	const auto nextPresetIndex = requested_preset_index.load();
	const auto nextSegmentIndex = music_presets[current_preset_index].GetSegment(current_segment_index)->GetNextSegmentIndex(nextPresetIndex);

	if (nextSegmentIndex.has_value()) {
		current_segment_index = nextSegmentIndex.value();
		current_preset_index = nextPresetIndex;
	}
	else if (!parameters.Get().loop) {
		return dataSize;
	}

	current_decoder->Rewind();
	current_decoder = music_presets[current_preset_index].GetSegment(current_segment_index)->GetDecoder();
	Init(current_decoder->Rate(), current_decoder->IsStereo(), current_decoder->GetAudioFormat());
	return HasBufferFormatChanged() ? dataSize : dataSize + GetNextData(data + dataSize, length - dataSize);
}

bool MusicPlayer::RequestPresetTransition(uint32_t preset_index) {
	if (preset_index >= music_presets.size()) return false;
	requested_preset_index.store(preset_index);
	return true;
}
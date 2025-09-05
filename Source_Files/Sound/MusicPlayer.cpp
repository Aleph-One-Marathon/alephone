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
	transition_preset_index = starting_preset_index;
	current_segment_index = starting_segment_index;
	requested_preset_index = starting_preset_index;
}

SetupALResult MusicPlayer::SetUpALSourceIdle() {
	const float default_music_volume = OpenALManager::Get()->GetMusicVolume() * OpenALManager::Get()->GetMasterVolume();
	alSourcef(audio_source->source_id, AL_MAX_GAIN, default_music_volume);
	alSourcef(audio_source->source_id, AL_GAIN, default_music_volume * parameters.Get().volume);
	return SetupALResult(alGetError() == AL_NO_ERROR, true);
}

bool MusicPlayer::ProcessTransitionIn(uint8* data, uint32_t length, uint32_t next_preset_index, std::pair<Segment::Mapping, SegmentTransitionOffsets> segment_mapping_offsets) {

	const auto mapping = segment_mapping_offsets.first;
	const auto transitionOffsets = segment_mapping_offsets.second;

	const auto segment = music_presets[next_preset_index].GetSegment(mapping.segment_id);
	const auto decoder = segment->GetDecoder();
	const auto currentPosition = decoder->Position();

	if (mapping.transition_in.fade_type != FadeType::None && transitionOffsets.second.has_value()) {

		const auto fadeInTransitionOffset = transitionOffsets.second.value();

		if (currentPosition < fadeInTransitionOffset) {
			const auto startFadeOffset = currentPosition - length;
			const auto fadedDataLength = std::min(length, fadeInTransitionOffset - startFadeOffset);
			ApplyFade(mapping.transition_in.fade_type, true, fadeInTransitionOffset, startFadeOffset, data, fadedDataLength);
			return startFadeOffset + fadedDataLength < fadeInTransitionOffset;
		}
	}

	return false;
}

bool MusicPlayer::ProcessTransition(uint8* data, uint32_t length, uint32_t next_preset_index, std::optional<Segment::Mapping> segment_mapping) {

	if (segment_mapping.has_value()) { 	//out

		const auto& mapping = segment_mapping.value();
		const auto transitionOffsets = ComputeTransitionOffsets(next_preset_index, mapping);
		const auto currentPosition = current_decoder->Position();
		const auto bufferStart = currentPosition - length;

		if (mapping.transition_out.fade_type != FadeType::None && transitionOffsets.first.has_value()) {

			const auto fadeOutTransitionOffset = transitionOffsets.first.value();

			if (currentPosition > fadeOutTransitionOffset) {

				const auto fadeTotalLength = current_decoder->Size() - fadeOutTransitionOffset;
				const auto fadeOffsetInBuffer = (fadeOutTransitionOffset > bufferStart) ? fadeOutTransitionOffset - bufferStart : 0;
				const auto startFadeOffset = (bufferStart + fadeOffsetInBuffer) - fadeOutTransitionOffset;
				const auto fadedDataLength = currentPosition - (bufferStart + fadeOffsetInBuffer);

				ApplyFade(mapping.transition_out.fade_type, false, fadeTotalLength, startFadeOffset, data + fadeOffsetInBuffer, fadedDataLength);
				bool transitionDone = startFadeOffset + fadedDataLength >= fadeTotalLength;

				if (mapping.crossfade) {

					const auto nextSegment = music_presets[next_preset_index].GetSegment(mapping.segment_id);
					const auto nextDecoder = nextSegment->GetDecoder();
					const bool useSameDecoder = current_decoder == nextDecoder;

					if (useSameDecoder) {
						nextDecoder->Position(crossfade_same_decoder_current_position);
					}

					std::vector<uint8> crossfade_fadeInData(length);
					nextDecoder->Decode(crossfade_fadeInData.data(), length); //we prevent crossfades if the fade in track is shorter than the fade out duration so we are always fine here
					transitionDone &= !ProcessTransitionIn(crossfade_fadeInData.data(), length, next_preset_index, { mapping, transitionOffsets });
					CrossFadeMix(data, crossfade_fadeInData.data(), length);

					if (useSameDecoder) {
						crossfade_same_decoder_current_position = nextDecoder->Position();
						current_decoder->Position(currentPosition);
					}
				}

				return !transitionDone;
			}
		}
	}

	if (current_transition_mapping_offsets.first.has_value() && current_transition_mapping_offsets.second.has_value()) { //in
		return ProcessTransitionIn(data, length, current_preset_index, { current_transition_mapping_offsets.first.value(), current_transition_mapping_offsets.second.value() });
	}

	return false;
}

void MusicPlayer::ApplyFade(FadeType fade_type, bool fade_in, uint32_t fade_total_length, uint32_t current_position, uint8* data, uint32_t faded_data_length) {

	if (fade_type == FadeType::None) return;

	assert(faded_data_length % sizeof(float) == 0);
	assert(fade_total_length % sizeof(float) == 0);
	assert(current_position % sizeof(float) == 0);

	auto buffer = reinterpret_cast<float*>(data);
	const uint32_t samplesToFade = faded_data_length / sizeof(float);
	const uint32_t fadeTotalSamples = fade_total_length / sizeof(float);
	const uint32_t offsetSamples = current_position / sizeof(float);

	for (uint32_t i = 0; i < samplesToFade; ++i) {

		const float t = static_cast<float>(offsetSamples + i) / static_cast<float>(fadeTotalSamples);
		float factor;

		switch (fade_type) {

		case FadeType::Linear:
			factor = fade_in ? t : 1.f - t;
			break;

		case FadeType::Sinusoidal:
			factor = fade_in ? std::sin(t * M_PI_2) : std::cos(t * M_PI_2);
			break;

		default:
			assert(false);
			break;
		}

		buffer[i] *= factor;
	}
}

void MusicPlayer::CrossFadeMix(uint8* data_out, uint8* data_in, uint32_t length) {

	assert(length % sizeof(float) == 0);

	const uint32_t totalSamples = length / sizeof(float);
	const auto in = reinterpret_cast<float*>(data_in);
	auto out = reinterpret_cast<float*>(data_out);

	for (uint32_t i = 0; i < totalSamples; ++i) {
		out[i] = std::clamp(out[i] + in[i], -1.0f, 1.0f);
	}
}

uint32_t MusicPlayer::GetTransitionPresetIndex() const {

	if (music_presets.size() < 2 || transition_is_active) return transition_preset_index;

	const auto requestedPresetIndex = requested_preset_index.load();
	if (requestedPresetIndex == current_preset_index) return current_preset_index;
	if (requestedPresetIndex == transition_preset_index) return transition_preset_index;

	const auto segmentMapping = music_presets[current_preset_index].GetSegment(current_segment_index)->GetNextSegmentMapping(requestedPresetIndex);
	if (!segmentMapping.has_value()) return transition_preset_index;

	const auto transitionOffsets = ComputeTransitionOffsets(requestedPresetIndex, segmentMapping.value());
	return !transitionOffsets.first.has_value() || transitionOffsets.first.value() > current_decoder->Position() ? requestedPresetIndex : transition_preset_index;
}

uint32_t MusicPlayer::GetNextData(uint8* data, uint32_t length) {

	if (!length) return 0;

	transition_preset_index = GetTransitionPresetIndex();
	const auto nextSegmentMapping = music_presets[current_preset_index].GetSegment(current_segment_index)->GetNextSegmentMapping(transition_preset_index);
	const auto dataSize = current_decoder->Decode(data, length);
	transition_is_active = ProcessTransition(data, dataSize, transition_preset_index, nextSegmentMapping);

	if (dataSize == length || (!nextSegmentMapping.has_value() && !parameters.Get().loop)) return dataSize;

	SwitchSegment(nextSegmentMapping, nextSegmentMapping.has_value() ? std::make_optional(ComputeTransitionOffsets(transition_preset_index, nextSegmentMapping.value())) : std::nullopt);
	return HasBufferFormatChanged() ? dataSize : dataSize + GetNextData(data + dataSize, length - dataSize);
}

void MusicPlayer::SwitchSegment(std::optional<Segment::Mapping> segment_mapping, std::optional<SegmentTransitionOffsets> transition_offsets) {

	current_decoder->Position(crossfade_same_decoder_current_position);
	current_transition_mapping_offsets = { segment_mapping, transition_offsets };
	crossfade_same_decoder_current_position = 0U;

	if (!segment_mapping.has_value()) return;

	current_segment_index = segment_mapping.value().segment_id;
	current_preset_index = transition_preset_index;
	current_decoder = music_presets[current_preset_index].GetSegment(current_segment_index)->GetDecoder();
	Init(current_decoder->Rate(), current_decoder->IsStereo(), current_decoder->GetAudioFormat());
}

MusicPlayer::SegmentTransitionOffsets MusicPlayer::ComputeTransitionOffsets(uint32_t preset_index, const Segment::Mapping& segment_mapping) const {

	SegmentTransitionOffsets results;

	const auto nextSegment = music_presets[preset_index].GetSegment(segment_mapping.segment_id);
	if (!nextSegment) return results;

	const auto [currentFormat, currentRate, currentIsStereo] = GetAudioFormat();
	const auto nextDecoder = nextSegment->GetDecoder();

	if (segment_mapping.crossfade && (currentFormat != nextDecoder->GetAudioFormat() || currentRate != nextDecoder->Rate() || currentIsStereo != nextDecoder->IsStereo()))
		return results;

	if (segment_mapping.transition_out.fade_type != FadeType::None && segment_mapping.transition_out.time_seconds) {

		const auto currentSegmentDuration = current_decoder->Duration();
		const auto nextSegmentDuration = nextDecoder->Duration();

		if (currentSegmentDuration > segment_mapping.transition_out.time_seconds && (!segment_mapping.crossfade || nextSegmentDuration > segment_mapping.transition_out.time_seconds)) {
			const auto startTransitionPositionSeconds = currentSegmentDuration - segment_mapping.transition_out.time_seconds;
			const auto startTransitionPositionByte = static_cast<uint32_t>(startTransitionPositionSeconds * currentRate) * current_decoder->BytesPerFrame();
			results.first = startTransitionPositionByte;
		}
	}

	if (segment_mapping.transition_in.fade_type != FadeType::None && segment_mapping.transition_in.time_seconds) {

		const auto nextSegmentDuration = nextDecoder->Duration();

		if (nextSegmentDuration > segment_mapping.transition_in.time_seconds) {
			const auto endTransitionPositionSeconds = segment_mapping.transition_in.time_seconds;
			const auto endTransitionPositionByte = static_cast<uint32_t>(endTransitionPositionSeconds * nextDecoder->Rate()) * nextDecoder->BytesPerFrame();
			results.second = endTransitionPositionByte;
		}
	}

	return results;
}

bool MusicPlayer::RequestPresetTransition(uint32_t preset_index) {
	if (preset_index >= music_presets.size()) return false;
	requested_preset_index.store(preset_index);
	return true;
}
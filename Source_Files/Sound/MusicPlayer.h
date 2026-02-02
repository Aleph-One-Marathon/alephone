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

#ifndef __MUSIC_PLAYER_H
#define __MUSIC_PLAYER_H

#include "AudioPlayer.h"

struct MusicParameters {
	float volume = 1.f;
	bool loop = true;
};

class MusicPlayer : public AudioPlayer {
public:

	enum class FadeType {
		None,
		Linear,
		Sinusoidal
	};

	class Segment {
	public:

		struct Transition {
			FadeType fade_type;
			float time_seconds;
		};

		struct Edge {
			uint32_t target_segment_id;
			Transition transition_out;
			Transition transition_in;
			bool crossfade;
		
			Edge(uint32_t segment_id, Transition transition_out, Transition transition_in, bool crossfade) :
				target_segment_id(segment_id), transition_out(transition_out), transition_in(transition_in), crossfade(crossfade) {}
		};

		Segment(std::shared_ptr<StreamDecoder> decoder) : decoder(decoder) {}
		std::shared_ptr<StreamDecoder> GetDecoder() const { return decoder; }
		std::optional<Edge> GetSegmentEdge(uint32_t sequence_index) const { return sequence_edges.find(sequence_index) != sequence_edges.end() ? std::make_optional(sequence_edges.find(sequence_index)->second) : std::nullopt; }
		void SetSegmentEdge(uint32_t sequence_index, const Edge& segment_edge) { sequence_edges.insert_or_assign(sequence_index, segment_edge); }
	private:
		std::shared_ptr<StreamDecoder> decoder;
		std::unordered_map<uint32_t, Edge> sequence_edges; //sequence index - segment edge
	};

	class Sequence {
	private:
		std::vector<Segment> segments;
	public:
		void AddSegment(std::shared_ptr<StreamDecoder> decoder) { segments.emplace_back(decoder); }
		const Segment* GetSegment(uint32_t index) const { return index < segments.size() ? &segments[index] : nullptr; }
		Segment* GetSegment(uint32_t index) { return index < segments.size() ? &segments[index] : nullptr; }
		const std::vector<Segment>& GetSegments() const { return segments; }
	};

	MusicPlayer(std::vector<Sequence>& sequences, uint32_t starting_sequence_index, uint32_t starting_segment_index, const MusicParameters& parameters); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 5.f; } //Doesn't really matter, just be above maximum volume (1) to be prioritized over sounds
	void UpdateParameters(const MusicParameters& musicParameters) { parameters.Store(musicParameters); }
	MusicParameters GetParameters() const { return parameters.Get(); }
	bool RequestSequenceTransition(uint32_t sequence_index);
private:

	typedef std::pair<std::optional<uint32_t>, std::optional<uint32_t>> SegmentTransitionOffsets; //out-in

	uint32_t GetNextData(uint8* data, uint32_t length) override;
	bool ProcessTransition(uint8* data, uint32_t length, uint32_t next_sequence_index, std::optional<Segment::Edge> segment_edge);
	bool ProcessTransitionIn(uint8* data, uint32_t length, uint32_t next_sequence_index, std::pair<Segment::Edge, SegmentTransitionOffsets> segment_edge_offsets);
	void ApplyFade(FadeType fade_type, bool fade_in, uint32_t fade_length, uint32_t current_position, uint8* data, uint32_t faded_data_length);
	void CrossFadeMix(uint8* data_out, uint8* data_in, uint32_t length);
	void SwitchSegment(std::optional<Segment::Edge> segment_edge, std::optional<SegmentTransitionOffsets> transition_offsets);
	uint32_t GetTransitionSequenceIndex() const;
	SetupALResult SetUpALSourceIdle() override;
	bool LoadParametersUpdates() override { return parameters.Update(); }
	SegmentTransitionOffsets ComputeTransitionOffsets(uint32_t sequence_index, const Segment::Edge& segment_edge) const;
	AtomicStructure<MusicParameters> parameters;
	std::shared_ptr<StreamDecoder> current_decoder;
	std::vector<Sequence> music_sequences;
	uint32_t current_sequence_index;
	uint32_t transition_sequence_index;
	uint32_t current_segment_index;
	std::pair<std::optional<Segment::Edge>, std::optional<SegmentTransitionOffsets>> current_transition_edge_offsets;
	std::atomic_uint32_t requested_sequence_index;
	bool transition_is_active = false;
	uint32_t crossfade_same_decoder_current_position = 0U;

	friend class OpenALManager;
};

#endif
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
	MusicPlayer(std::shared_ptr<StreamDecoder> decoder, MusicParameters parameters); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 5.f; } //Doesn't really matter, just be above maximum volume (1) to be prioritized over sounds
	void UpdateParameters(MusicParameters musicParameters) { parameters.Store(musicParameters); }
	MusicParameters GetParameters() const { return parameters.Get(); }
private:
	std::shared_ptr<StreamDecoder> decoder;
	AtomicStructure<MusicParameters> parameters;
	uint32_t GetNextData(uint8* data, uint32_t length) override;
	SetupALResult SetUpALSourceIdle() override;
	bool LoadParametersUpdates() override { return parameters.Update(); }

	friend class OpenALManager;
};

#endif
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

#ifndef __STREAM_PLAYER_H
#define __STREAM_PLAYER_H

#include "AudioPlayer.h"

typedef int (*CallBackStreamPlayer)(uint8* data, int length);

class StreamPlayer : public AudioPlayer {
public:
	//Length must be <= buffer_samples variable
	StreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, AudioFormat audioFormat); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 10; } //As long as it's only used for intro video, it doesn't really matter
private:
	int GetNextData(uint8* data, int length) override;
	CallBackStreamPlayer CallBackFunction;
	int data_length;

	friend class OpenALManager;
};

#endif
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

#include "StreamPlayer.h"

StreamPlayer::StreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, AudioFormat audioFormat)
	: AudioPlayer(rate, stereo, audioFormat) {
	data_length = length;
	CallBackFunction = callback;
	assert(data_length <= buffer_samples && "StreamPlayer not supported length");
}

int StreamPlayer::GetNextData(uint8* data, int length) {
	return CallBackFunction(data, std::min(length, data_length));
}
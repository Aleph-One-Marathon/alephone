#ifndef __SNDFILEDECODER_H
#define __SNDFILEDECODER_H

/*

	Copyright (C) 2007 Gregory Smith.
	and the "Aleph One" developers.
 
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

	Decodes sound files with libsndfile

*/

#include "Decoder.h"
#include "sndfile.h"

class SndfileDecoder : public Decoder
{
public:
	bool Open(FileSpecifier& File);
	int32 Decode(uint8* buffer, int32 max_length);
	void Rewind();
	void Close();

	AudioFormat GetAudioFormat() { return AudioFormat::_32_float; }
	bool IsStereo() { return (sfinfo.channels == 2); }
	int BytesPerFrame() { return 4 * (IsStereo() ? 2 : 1); }
	uint32_t Rate() { return sfinfo.samplerate; }
	bool IsLittleEndian() { return PlatformIsLittleEndian(); }
	float Duration() { return Frames() / Rate(); }
	uint32_t Position();
	void Position(uint32_t position);
	uint32_t Size() { return Frames() * BytesPerFrame(); }

	int32 Frames() { return sfinfo.frames; }

	SndfileDecoder();
	~SndfileDecoder();
private:
	SNDFILE* sndfile;
	SF_INFO sfinfo;
	SDL_RWops* rwops;
};

#endif

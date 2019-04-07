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

#ifdef HAVE_SNDFILE
#include "sndfile.h"

class SndfileDecoder : public Decoder
{
public:
	bool Open(FileSpecifier& File);
	int32 Decode(uint8* buffer, int32 max_length);
	void Rewind();
	void Close();

	bool IsSixteenBit() { return true; }
	bool IsStereo() { return (sfinfo.channels == 2); }
	bool IsSigned() { return true; }
	int BytesPerFrame() { return 2 * (IsStereo() ? 2 : 1); }
	float Rate() { return (float) sfinfo.samplerate; }
	bool IsLittleEndian() { return PlatformIsLittleEndian(); }

	int32 Frames() { return sfinfo.frames; }

	SndfileDecoder();
	~SndfileDecoder();
private:
	SNDFILE* sndfile;
	SF_INFO sfinfo;
	SDL_RWops* rwops;
};

#endif

#endif

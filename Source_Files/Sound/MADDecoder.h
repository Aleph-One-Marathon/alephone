#ifndef __MADDECODER_H
#define __MADDECODER_H

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

	Handles mp3 files with libmad

*/

#include "cseries.h"
#include "Decoder.h"
#ifdef HAVE_MAD
#include <mad.h>

class MADDecoder : public StreamDecoder
{
public:
	bool Open(FileSpecifier &File);
	int32 Decode(uint8* buffer, int32 max_length);
	void Rewind();
	void Close();

	bool IsSixteenBit() { return true; }
	bool IsStereo() { return stereo; }
	bool IsSigned() { return true; }
	int BytesPerFrame() { return bytes_per_frame; }
	float Rate() { return rate; }
	bool IsLittleEndian() { return PlatformIsLittleEndian(); }

	MADDecoder();
	~MADDecoder();

private:
	bool stereo;
	int bytes_per_frame;
	float rate;
	int sample;

	OpenedFile file;
	bool file_done;

	mad_stream Stream;
	mad_frame Frame;
	mad_synth Synth;

	static const int INPUT_BUFFER_SIZE = 5 * 8192;

	uint8 InputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD];

	bool DecodeFrame();
};

#endif

#endif

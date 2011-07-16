#ifndef __BASICIFFDECODER_H
#define __BASICIFFDECODER_H

/*

	Copyright (C) 2001-2007 and beyond by Bungie Studios, Inc.
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

	Handles uncompressed AIFF and WAV files

*/

#include "Decoder.h"

class BasicIFFDecoder : public Decoder
{
public:
	bool Open(FileSpecifier &File);
	int32 Decode(uint8* buffer, int32 max_length);
	bool Done();
	void Rewind();
	void Close();

	bool IsSixteenBit() { return sixteen_bit; }
	bool IsStereo() { return stereo; }
	bool IsSigned() { return signed_8bit; }
	int BytesPerFrame() { return bytes_per_frame; }
	float Rate() { return rate; }
	bool IsLittleEndian() { return little_endian; }

	int32 Frames() { return length / bytes_per_frame; }

	BasicIFFDecoder();
	~BasicIFFDecoder() { }

private:
	bool sixteen_bit;
	bool stereo;
	bool signed_8bit;
	int bytes_per_frame;
	float rate;
	bool little_endian;

	int32 length;

	OpenedFile file;
	int32 data_offset;

};

#endif

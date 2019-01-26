#ifndef __VORBISDECODER_H
#define __VORBISDECODER_H

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

	Handles ogg/vorbis files

*/

#include "cseries.h"
#include "Decoder.h"
#ifdef HAVE_VORBISFILE
#include <vorbis/vorbisfile.h>

class VorbisDecoder : public StreamDecoder
{
public:
	bool Open(FileSpecifier& File);
	int32 Decode(uint8* buffer, int32 max_length);
	void Rewind();
	void Close();
	
	bool IsSixteenBit() { return true; }
	bool IsStereo() { return stereo; }
	bool IsSigned() { return true; }
	int BytesPerFrame() { return 2 * (IsStereo() ? 2 : 1); }
	float Rate() { return rate; }
	bool IsLittleEndian() { return PlatformIsLittleEndian(); }

	VorbisDecoder();
	~VorbisDecoder();

private:
	bool stereo;
	float rate;

	OggVorbis_File ov_file;
	ov_callbacks callbacks;
};

#endif

#endif


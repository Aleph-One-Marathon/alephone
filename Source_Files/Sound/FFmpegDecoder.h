#ifndef __FFMPEGDECODER_H
#define __FFMPEGDECODER_H

/*

 Copyright (C) 2012 and beyond by Jeremiah Morris
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

 Decodes sound files with lbav/ffmpeg

*/

#include "Decoder.h"

#ifdef HAVE_FFMPEG

class FFmpegDecoder : public StreamDecoder
{
public:
	bool Open(FileSpecifier& File);
	int32 Decode(uint8* buffer, int32 max_length);
	void Rewind();
	void Close();

	bool IsSixteenBit() { return true; }
	bool IsStereo() { return (channels == 2); }
	bool IsSigned() { return true; }
	int BytesPerFrame() { return 2 * (IsStereo() ? 2 : 1); }
	float Rate() { return rate; }
	bool IsLittleEndian() { return PlatformIsLittleEndian(); }

	FFmpegDecoder();
	~FFmpegDecoder();
private:
	struct ffmpeg_vars *av;

	bool GetAudio();
	
	float rate;
	int channels;
};

#endif

#endif

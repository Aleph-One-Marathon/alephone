#ifndef __DECODER_H
#define __DECODER_H

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

	Decodes music and external sounds

*/

#include "cseries.h"
#include "FileHandler.h"

class StreamDecoder
{
public:
	virtual bool Open(FileSpecifier &File) = 0;
	virtual int32 Decode(uint8* buffer, int32 max_length) = 0;
	virtual void Rewind() = 0;
	virtual void Close() = 0;

	virtual bool IsSixteenBit() = 0;
	virtual bool IsStereo() = 0;
	virtual bool IsSigned() = 0;
	virtual int BytesPerFrame() = 0;
	virtual float Rate() = 0;
	virtual bool IsLittleEndian() = 0;

	StreamDecoder() { }
	virtual ~StreamDecoder() { }

	static StreamDecoder* Get(FileSpecifier &File); // can return 0
};

class Decoder : public StreamDecoder
{
public:
	Decoder() : StreamDecoder() { }
	~Decoder() { }

	// total number of frames in the file
	virtual int32 Frames() = 0; 

	static Decoder* Get(FileSpecifier &File); // can return 0
};

#endif

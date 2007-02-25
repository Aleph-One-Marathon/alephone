/*

	Copyright (C) 2007 Gregory Smith
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Handles sound files with libsndfile

*/

#include "SndfileDecoder.h"

#ifdef HAVE_SNDFILE

SndfileDecoder::SndfileDecoder() :
	sndfile(0)
{
}

SndfileDecoder::~SndfileDecoder()
{
	Close();
}

bool SndfileDecoder::Open(FileSpecifier& File)
{
	if (sndfile)
	{
		sf_close(sndfile);
		sndfile = 0;
	}

	sfinfo.format = 0;
	sndfile = sf_open(File.GetPath(), SFM_READ, &sfinfo);

	return sndfile;
}

int32 SndfileDecoder::Decode(uint8* buffer, int32 max_length)
{
	if (!sndfile) return 0;

	int32 bytes_read = sf_read_short(sndfile, (int16*) buffer, max_length / 2) * 2;
#ifdef ALEPHONE_LITTLE_ENDIAN
	// sndfile puts it in native byte order, so swap it back
	int16 *buffer_16 = (int16*) buffer;
	for (int i = 0; i < bytes_read / 2; i++)
	{
		buffer_16[i] = SDL_Swap16(buffer_16[i]);
	}
#endif

	return bytes_read;
		
}

void SndfileDecoder::Rewind()
{
	if (!sndfile) return;
	sf_seek(sndfile, 0, SEEK_SET);
}	

void SndfileDecoder::Close()
{
	if (sndfile)
	{
		sf_close(sndfile);
		sndfile = 0;
	}
}
#endif

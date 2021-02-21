/*

	Copyright (C) 2007 Gregory Smith
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

#include "Decoder.h"
#include "BasicIFFDecoder.h"
#include "MADDecoder.h"
#include "SndfileDecoder.h"
#include "VorbisDecoder.h"
#include "FFmpegDecoder.h"
#include <memory>

using std::unique_ptr;

StreamDecoder *StreamDecoder::Get(FileSpecifier& File)
{
#ifdef HAVE_FFMPEG
	{
		unique_ptr<FFmpegDecoder> ffmpegDecoder(new FFmpegDecoder);
		if (ffmpegDecoder->Open(File))
			return ffmpegDecoder.release();
	}
#endif

#ifdef HAVE_SNDFILE
	{ 
		unique_ptr<SndfileDecoder> sndfileDecoder(new SndfileDecoder);
		if (sndfileDecoder->Open(File))
			return sndfileDecoder.release();
	}
#else
	{
		unique_ptr<BasicIFFDecoder> iffDecoder(new BasicIFFDecoder);
		if (iffDecoder->Open(File))
			return iffDecoder.release();
	}
#endif

#ifdef HAVE_VORBISFILE
	{
		unique_ptr<VorbisDecoder> vorbisDecoder(new VorbisDecoder);
		if (vorbisDecoder->Open(File))
			return vorbisDecoder.release();
	}
#endif

#ifdef HAVE_MAD
	{
		unique_ptr<MADDecoder> madDecoder(new MADDecoder);
		if (madDecoder->Open(File))
			return madDecoder.release();
	}
#endif

	return 0;
}

Decoder *Decoder::Get(FileSpecifier &File)
{
#ifdef HAVE_SNDFILE
	{
		unique_ptr<SndfileDecoder> sndfileDecoder(new SndfileDecoder);
		if (sndfileDecoder->Open(File))
			return sndfileDecoder.release();
	}
#else
	{
		unique_ptr<BasicIFFDecoder> iffDecoder(new BasicIFFDecoder);
		if (iffDecoder->Open(File))
			return iffDecoder.release();
	}
#endif
    
    return 0;
}

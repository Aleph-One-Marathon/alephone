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

	Handles mp3 files with libmad

*/

#include "MADDecoder.h"
#ifdef HAVE_MAD
#include <mad.h>

MADDecoder::MADDecoder() :
	stereo(false),
	bytes_per_frame(0),
	rate(0),
	sample(0)
{
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
}

MADDecoder::~MADDecoder()
{
	Close();
}

bool MADDecoder::Open(FileSpecifier &File)
{
	if (!File.Open(file)) return false;

	file_done = false;

	if (DecodeFrame())
	{
		stereo = (MAD_NCHANNELS(&Frame.header) == 2);
		bytes_per_frame = 2 * (stereo ? 2 : 1);
		rate = Frame.header.samplerate;

		sample = 0;
		return true;
	}
	else
	{
		return false;
	}
}

static inline int16 MadFixedToSshort(mad_fixed_t Fixed)
{
	if (Fixed >= MAD_F_ONE)
		return SHRT_MAX;
	if (Fixed <= -MAD_F_ONE)
		return -SHRT_MAX;

	Fixed = Fixed >> (MAD_F_FRACBITS - 15);
	return ((int16) Fixed);
}

int32 MADDecoder::Decode(uint8* buffer, int32 max_length)
{
	int32 bytes_decoded = 0;
	if (stereo) 
		max_length = max_length - 3;
	else
		max_length = max_length - 1;

	while (bytes_decoded < max_length)
	{
		if (sample < Synth.pcm.length)
		{
			int16 sample_data;
			sample_data = MadFixedToSshort(Synth.pcm.samples[0][sample]);
#ifdef ALEPHONE_LITTLE_ENDIAN
			buffer[bytes_decoded++] = sample_data & 0xff;
			buffer[bytes_decoded++] = sample_data >> 8;
#else
			buffer[bytes_decoded++] = sample_data >> 8;
			buffer[bytes_decoded++] = sample_data & 0xff;

#endif
			if (stereo)
			{
				sample_data = MadFixedToSshort(Synth.pcm.samples[1][sample]);
#ifdef ALEPHONE_LITTLE_ENDIAN
				buffer[bytes_decoded++] = sample_data & 0xff;
				buffer[bytes_decoded++] = sample_data >> 8;
#else
				buffer[bytes_decoded++] = sample_data >> 8;
				buffer[bytes_decoded++] = sample_data & 0xff;
#endif
			}

			sample++;
		}
		else if (!DecodeFrame())
		{
			Synth.pcm.length = 0;
			return bytes_decoded;
		}
	}

	return bytes_decoded;
}

bool MADDecoder::DecodeFrame()
{
	uint8 *GuardPtr = 0;
	if (Stream.buffer == NULL || Stream.error == MAD_ERROR_BUFLEN)
	{
		if (file_done) return false;
		size_t ReadSize, Remaining;
		uint8 *ReadStart;
		// fill the stream buffer
		if (Stream.next_frame != NULL)
		{
			Remaining = Stream.bufend - Stream.next_frame;
			memmove(InputBuffer, Stream.next_frame, Remaining);
			ReadStart = InputBuffer + Remaining;
			ReadSize = INPUT_BUFFER_SIZE - Remaining;
		}
		else
		{
			ReadSize = INPUT_BUFFER_SIZE;
			ReadStart = InputBuffer;
			Remaining = 0;
		}

		bool guard = false;
		{
			int32 position;
			file.GetPosition(position);
			int32 length;
			file.GetLength(length);

			if (ReadSize > length - position)
			{
				ReadSize = length - position;
				guard = true;
			}
		}
			
		if (!file.Read(ReadSize, ReadStart))
		{
			file_done = true;
			return false;
		}

		if (guard)
		{
			GuardPtr = ReadStart + ReadSize;
			memset(GuardPtr, 0, MAD_BUFFER_GUARD);
			ReadSize += MAD_BUFFER_GUARD;
			file_done = true;
		}

		mad_stream_buffer(&Stream, InputBuffer, ReadSize + Remaining);
		Stream.error = MAD_ERROR_NONE;
	}

	// decode the next MPEG frame
	if (mad_frame_decode(&Frame, &Stream))
	{
		if (MAD_RECOVERABLE(Stream.error) || Stream.error == MAD_ERROR_BUFLEN)
		{
			return DecodeFrame();
		}
		else
		{
			return false;
		}
	}

	mad_synth_frame(&Synth, &Frame);
	sample = 0;
	return true;
}

void MADDecoder::Rewind()
{
	mad_stream_finish(&Stream);
	mad_frame_finish(&Frame);
	mad_synth_finish(&Synth);

	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);

	sample = 0;
	file.SetPosition(0);
	file_done = false;

	DecodeFrame();
}

void MADDecoder::Close()
{	
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
}

#endif

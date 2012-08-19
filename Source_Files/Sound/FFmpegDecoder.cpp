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
 
 Decodes sound files with SDL_ffmpeg
 
 */

#include "FFmpegDecoder.h"

#ifdef HAVE_FFMPEG

FFmpegDecoder::FFmpegDecoder() :
	sffile(NULL), stream(NULL), frame(NULL)
{
}

FFmpegDecoder::~FFmpegDecoder()
{
	Close();
}

bool FFmpegDecoder::Open(FileSpecifier& File)
{
	Close();
	
	sffile = SDL_ffmpegOpen(File.GetPath());
	if (!sffile)
		return false;
	
	SDL_ffmpegSelectAudioStream(sffile, 0);
	stream = SDL_ffmpegGetAudioStream(sffile, 0);
	if (!stream)
	{
		Close();
		return false;
	}
	
	SDL_AudioSpec spec = SDL_ffmpegGetAudioSpec(sffile, 0, NULL);
	rate = spec.freq;
	channels = spec.channels;

	frame = SDL_ffmpegCreateAudioFrame(sffile, 2 * spec.channels * spec.freq);
	if (!frame)
	{
		Close();
		return false;
	}
	
	return true;
}

int32 FFmpegDecoder::Decode(uint8* buffer, int32 max_length)
{
	if (!frame)
		return 0;

	int32 total_bytes_read = 0;
	int32 bytes_read = 0;
	while (total_bytes_read < max_length)
	{
		if (frame->size)
		{
			// already grabbed data, use it up
			bytes_read = MIN(frame->size, max_length - total_bytes_read);
			memcpy(&buffer[total_bytes_read], &frame->buffer[frame->capacity - frame->size], bytes_read);
			total_bytes_read += bytes_read;
			frame->size -= bytes_read;
		}
		else if (frame->last)
		{
			// consumed last of the data
			break;
		}
		else
		{
			// time to grab more
			SDL_ffmpegGetAudioFrame(sffile, frame);
			if (!frame->size && !frame->last)
				frame->last = 1;  // error grabbing?				
		}
	}
	memset(&buffer[total_bytes_read], 0, max_length - total_bytes_read);
//	return max_length;
	
	return total_bytes_read;
}

void FFmpegDecoder::Rewind()
{
	SDL_ffmpegSeek(sffile, 0);
	frame->size = 0;
	frame->last = 0;
}	

void FFmpegDecoder::Close()
{
	if (frame)
	{
		SDL_ffmpegFreeAudioFrame(frame);
		frame = NULL;
	}
	stream = NULL;
	if (sffile)
	{
		SDL_ffmpegFree(sffile);
		sffile = NULL;
	}
}

#endif

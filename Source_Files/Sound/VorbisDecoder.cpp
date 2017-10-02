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

	Handles ogg/vorbis files

*/

#include "VorbisDecoder.h"

#ifdef HAVE_VORBISFILE

static size_t sdl_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return SDL_RWread((SDL_RWops*)datasource, ptr, size, nmemb);
}

static int sdl_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    return SDL_RWseek((SDL_RWops*)datasource, (int)offset, whence);
}

static int sdl_close_func(void *datasource)
{
    return SDL_RWclose((SDL_RWops*)datasource);
}

static long sdl_tell_func(void *datasource)
{
    return SDL_RWtell((SDL_RWops*)datasource);
}

VorbisDecoder::VorbisDecoder() :
	stereo(false),
	rate(0),
	ov_file()
{	
	callbacks.read_func = sdl_read_func;
	callbacks.seek_func = sdl_seek_func;
	callbacks.close_func = sdl_close_func;
	callbacks.tell_func = sdl_tell_func;
}

VorbisDecoder::~VorbisDecoder()
{
	Close();
}

bool VorbisDecoder::Open(FileSpecifier &File)
{
	Close();
	
	OpenedFile file;
	if (!File.Open(file))
		return false;
	
	if (ov_open_callbacks(file.GetRWops(), &ov_file, NULL, 0, callbacks) != 0)
		return false;
	
	// Let ov_file own the RWops
	file.TakeRWops();
	
	vorbis_info *vi = ov_info(&ov_file, -1);
	if (!vi)
	{
		Close();
		return false;
	}
	
	stereo = vi->channels == 2;
	rate = vi->rate;
	
	return true;
}

int32 VorbisDecoder::Decode(uint8* buffer, int32 max_length)
{
	int32 total_bytes_read = 0;
	int32 bytes_read = 0;
	int current_section = 0;
	while (total_bytes_read < max_length)
	{
		bytes_read = ov_read(&ov_file, (char *) &buffer[total_bytes_read], max_length - total_bytes_read, IsLittleEndian() ? 0 : 1, 2, 1, &current_section);
		if (bytes_read <= 0)
			return total_bytes_read;
		total_bytes_read += bytes_read;
	}

	return total_bytes_read;
}

void VorbisDecoder::Rewind()
{
	ov_raw_seek(&ov_file, 0);
}

void VorbisDecoder::Close()
{
	ov_clear(&ov_file);
}

#endif

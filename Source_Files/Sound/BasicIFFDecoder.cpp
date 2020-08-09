/*

	Copyright (C) 1991-2007 and beyond by Bungie Studios, Inc.
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

	Handles uncompressed WAV and AIFF files

*/

#if defined(_MSC_VER)
#define NOMINMAX
#include <algorithm>
#endif

#include "BasicIFFDecoder.h"
#include "AStream.h"
#include <vector>
#include <SDL_endian.h>

using std::vector;

BasicIFFDecoder::BasicIFFDecoder() :
	sixteen_bit(false),
	stereo(false),
	signed_8bit(false),
	bytes_per_frame(0),
	rate(0),
	little_endian(false),
	length(0),
	data_offset(0)
	
{
}

bool BasicIFFDecoder::Open(FileSpecifier& File)
{
	if (!File.Open(file)) return false;

	SDL_RWops *music_rw = file.GetRWops();

	uint32 sample_rate = 0;
	
	// Read magic ID
	uint32 magic = SDL_ReadBE32(music_rw);
	if (magic == FOUR_CHARS_TO_INT('F', 'O', 'R', 'M')) {
		
		// Maybe an AIFF file, check further
		uint32 total_size = SDL_ReadBE32(music_rw);
		if (SDL_ReadBE32(music_rw) != FOUR_CHARS_TO_INT('A', 'I', 'F', 'F'))
			return false;
		
		// Seems so, look for COMM and SSND chunks
		bool comm_found = false;
		bool ssnd_found = false;
		do {
			
			// Read chunk ID and size
			uint32 id = SDL_ReadBE32(music_rw);
			uint32 size = SDL_ReadBE32(music_rw);
			int pos = SDL_RWtell(music_rw);
			
			switch (id) {
			case FOUR_CHARS_TO_INT('C', 'O', 'M', 'M'): {
				comm_found = true;

				little_endian = false;
				stereo = (SDL_ReadBE16(music_rw) == 2);
				SDL_RWseek(music_rw, 4, SEEK_CUR);
				sixteen_bit = (SDL_ReadBE16(music_rw) == 16);
				signed_8bit = true;
				
				uint32 srate = SDL_ReadBE32(music_rw);	// This is a 6888x 80-bit floating point number, but we only read the first 4 bytes and try to guess the sample rate
				switch (srate) {
				case 0x400eac44:
					sample_rate = 44100;
					break;
				case 0x400dac44:
					sample_rate = 22050;
					break;
				case 0x400cac44:
				default:
					sample_rate = 11025;
				}
				
				break;
			}
			
			case FOUR_CHARS_TO_INT('S', 'S', 'N', 'D'):
				ssnd_found = true;
			
			length = size;
			SDL_RWseek(music_rw, 8, SEEK_CUR);
			data_offset = SDL_RWtell(music_rw);
			break;
			}
			
			// Skip to next chunk
			if (size & 1)
				size++;
			SDL_RWseek(music_rw, pos + size, SEEK_SET);
			
		} while (uint32(SDL_RWtell(music_rw)) < total_size);
		
		if (!comm_found)
			return false;
		if (!ssnd_found)
			return false;
	} else if (magic == FOUR_CHARS_TO_INT('R', 'I', 'F', 'F')) {
		
		// Maybe a WAV file, check further
		uint32 total_size = SDL_ReadLE32(music_rw);
		if (SDL_ReadBE32(music_rw) != FOUR_CHARS_TO_INT('W', 'A', 'V', 'E'))
			return false;
		
		// Seems so, look for fmt and data chunks
		bool fmt_found = false;
		bool data_found = false;
		do {
			
			// Read chunk ID and size
			uint32 id = SDL_ReadBE32(music_rw);
			uint32 size = SDL_ReadLE32(music_rw);
			int pos = SDL_RWtell(music_rw);
			
			switch (id) {
			case FOUR_CHARS_TO_INT('f', 'm', 't', ' '):
				fmt_found = true;
			little_endian = true;
			if (SDL_ReadLE16(music_rw) != 1) // PCM encoding
				return false;
			stereo = (SDL_ReadLE16(music_rw) == 2);
			sample_rate = SDL_ReadLE32(music_rw);
			SDL_RWseek(music_rw, 4, SEEK_CUR);
			bytes_per_frame = SDL_ReadLE16(music_rw);
			sixteen_bit = (SDL_ReadLE16(music_rw) == 16);
			signed_8bit = false;
			break;
			
			case FOUR_CHARS_TO_INT('d', 'a', 't', 'a'):
				data_found = true;
			
			length = size;
			data_offset = SDL_RWtell(music_rw);
			break;
			}
			
			// Skip to next chunk
			if (size & 1)
				size++;
			SDL_RWseek(music_rw, pos + size, SEEK_SET);
			
		} while (uint32(SDL_RWtell(music_rw)) < total_size);
		
		if (!fmt_found)
			return false;
		if (!data_found)
			return false;
	} else {
		return false;
	}
	
	bytes_per_frame = (stereo ? 2 : 1) * (sixteen_bit ? 2 : 1);
	rate = sample_rate;

	return true;
}

int32 BasicIFFDecoder::Decode(uint8* buffer, int32 max_length)
{
	if (!file.IsOpen()) return 0;
	int32 position;
	file.GetPosition(position);
	int32 length = std::min(max_length, (int32) (data_offset + this->length - position));
	if (length && file.Read(length, buffer))
	{
		return length;
	}
	else
	{
		return 0;
	}
}

bool BasicIFFDecoder::Done()
{
	if (!file.IsOpen()) return true;

	int32 position;
	file.GetPosition(position);
	return (position >= data_offset + length);
}

void BasicIFFDecoder::Rewind()
{
	if (file.IsOpen())
		file.SetPosition(data_offset);
}

void BasicIFFDecoder::Close()
{
	length = 0;
	if (file.IsOpen())
		file.Close();
}

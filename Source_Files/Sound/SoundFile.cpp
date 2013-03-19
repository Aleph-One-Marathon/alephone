/*
SOUNDFILE.CPP

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

*/

#include "SoundFile.h"
#include "Logging.h"
#include "csmisc.h"
#include "Decoder.h"

#include <assert.h>
#include <boost/make_shared.hpp>

SoundHeader::SoundHeader() :
	SoundInfo(),
	data_offset(0)
{
}

bool SoundHeader::UnpackStandardSystem7Header(AIStreamBE &header)
{
	try 
	{
		bytes_per_frame = 1;
		signed_8bit = false;
		sixteen_bit = false;
		stereo = false;
		little_endian = false;
		header.ignore(4); // sample pointer
		header >> length;
		header >> rate;
		header >> loop_start;
		header >> loop_end;
		
		return true;
	} catch (...) {
		return false;
	}
}

bool SoundHeader::UnpackExtendedSystem7Header(AIStreamBE &header)
{
	try 
	{
		signed_8bit = false;
		header.ignore(4); // sample pointer
		int32 num_channels;
		header >> num_channels;
		stereo = (num_channels == 2);
		header >> rate;
		header >> loop_start;
		header >> loop_end;
		uint8 header_type;
		header >> header_type;
		header.ignore(1); // baseFrequency
		int32 num_frames;
		header >> num_frames;
		
		if (header_type == 0xfe)
		{
			header.ignore(10); // AIFF rate
			header.ignore(4); // marker chunk
			uint32 format;
			header >> format;
			header.ignore(4 * 3); // future use, ptr, ptr
			int16 comp_id;
			header >> comp_id;
			if (format != FOUR_CHARS_TO_INT('t','w','o','s') || comp_id != -1) {
				return false;
			}
			signed_8bit = true;
			header.ignore(4);
		} else {
			header.ignore(22);
		}

		int16 sample_size;
		header >> sample_size;

		sixteen_bit = (sample_size == 16);
		bytes_per_frame = (sixteen_bit ? 2 : 1) * (stereo ? 2 : 1);

		length = num_frames * bytes_per_frame;
		little_endian = false;

		if ((loop_end - loop_start >= 4) && ((loop_start % bytes_per_frame) || (loop_end % bytes_per_frame)))
		{
			logWarning3("loop_start=%i and loop_end=%i but bytes_per_frame=%i; interpreting as frame offsets", loop_start, loop_end, bytes_per_frame);
			loop_start *= bytes_per_frame;
			loop_end *= bytes_per_frame;
		}
		
		return true;
	} catch (...) {
		return false;
	}
}

boost::shared_ptr<SoundData> SoundHeader::Load(const uint8* data)
{
	Clear();

	boost::shared_ptr<SoundData> p;
	if (data[20] == 0x00)
	{
		AIStreamBE header(data, 22);
		if (UnpackStandardSystem7Header(header))
		{
			data_offset = 22;
			p = boost::make_shared<SoundData>(data + data_offset, data + data_offset + length);
		}
	}
	else if (data[20] == 0xff || data[20] == 0xfe)
	{
		AIStreamBE header(data, 64);
		if (UnpackExtendedSystem7Header(header))
		{
			data_offset = 64;
			p = boost::make_shared<SoundData>(data + data_offset, data + data_offset + length);
		}
	}

	return p;
}

bool SoundHeader::Load(OpenedFile &SoundFile)
{
	Clear();
	if (!SoundFile.IsOpen()) return false;

	int32 file_position;
	SoundFile.GetPosition(file_position);
	SoundFile.SetPosition(file_position + 20);
	uint8 header_type;
	if (!SoundFile.Read(1, &header_type)) return false;
	SoundFile.SetPosition(file_position);

	if (header_type == 0x0)
	{
		// standard sound header
		vector<uint8> headerBuffer(22);
		if (!SoundFile.Read(headerBuffer.size(), &headerBuffer[0]))
			return false;

		AIStreamBE header(&headerBuffer[0], headerBuffer.size());
		if (!UnpackStandardSystem7Header(header)) return false;
		data_offset = 22;
		return true;
	}
	else if (header_type == 0xff || header_type == 0xfe)
	{
		vector<uint8> headerBuffer(64);
		if (!SoundFile.Read(headerBuffer.size(), &headerBuffer[0]))
			return false;

		AIStreamBE header(&headerBuffer[0], headerBuffer.size());
		if (!UnpackExtendedSystem7Header(header)) return false;
		data_offset = 64;
		return true;
	}

	return false;
}

boost::shared_ptr<SoundData> SoundHeader::LoadData(OpenedFile& SoundFile)
{
	boost::shared_ptr<SoundData> p;

	if (!SoundFile.IsOpen()) return p;

	int32 file_position;
	SoundFile.GetPosition(file_position);
	SoundFile.SetPosition(file_position + data_offset);

	p = boost::make_shared<SoundData>(length);
	if (!SoundFile.Read(length, &((*p)[0])))
	{
		p.reset();
	}

	return p;
}

SoundDefinition::SoundDefinition() :
	sound_code(0), 
	behavior_index(1), 
	flags(0),
	chance(0),
	low_pitch(0),
	high_pitch(0),
	permutations(1),
	permutations_played(0),
	group_offset(0), single_length(0), total_length(0),
	last_played(0)
{
}

bool SoundDefinition::Unpack(OpenedFile &SoundFile)
{
	if (!SoundFile.IsOpen()) return false;

	vector<uint8> headerBuffer(HeaderSize());
	if (!SoundFile.Read(headerBuffer.size(), &headerBuffer[0])) 
		return false;
	
	AIStreamBE header(&headerBuffer[0], headerBuffer.size());
	
	header >> sound_code;

	header >> behavior_index;
	header >> flags;
	
	header >> chance;
	
	header >> low_pitch;
	header >> high_pitch;

	header >> permutations;
	header >> permutations_played;
	header >> group_offset;
	header >> single_length;
	header >> total_length;

	sound_offsets.resize(MAXIMUM_PERMUTATIONS_PER_SOUND);
	for (int i = 0; i < sound_offsets.size(); i++)
	{
		header >> sound_offsets[i];
	}

	header >> last_played;
	
	header.ignore(4 * 2);

	return true;
}

bool SoundDefinition::Load(OpenedFile &SoundFile, bool LoadPermutations)
{
	if (!SoundFile.IsOpen()) return false;

	if (LoadPermutations)
		sounds.resize(permutations);
	else 
		sounds.resize(std::min(permutations, static_cast<int16>(1)));

	for (int i = 0; i < sounds.size(); i++)
	{
		if (!SoundFile.SetPosition(group_offset + sound_offsets[i])
		    || !sounds[i].Load(SoundFile))
		{
			sounds.clear();
			return false;
		}
	}
    
    return true;
}


boost::shared_ptr<SoundData> SoundDefinition::LoadData(OpenedFile& SoundFile, short permutation)
{
	boost::shared_ptr<SoundData> p;
	if (!SoundFile.IsOpen()) 
	{
		return p;
	}

	if (!SoundFile.SetPosition(group_offset + sound_offsets[permutation]))
	{
		return p;
	}

	return sounds[permutation].LoadData(SoundFile);
}

bool M2SoundFile::Open(FileSpecifier& SoundFileSpec)
{
	Close();

	std::auto_ptr<OpenedFile> sound_file(new OpenedFile);

	if (!SoundFileSpec.Open(*sound_file, false)) return false;

	std::vector<uint8> headerBuffer;
	headerBuffer.resize(HeaderSize());

	if (!sound_file->Read(headerBuffer.size(), &headerBuffer[0]))
		return false;

	AIStreamBE header(&headerBuffer[0], headerBuffer.size());
	header >> version;
	header >> tag;
	header >> source_count;
	header >> sound_count;
	header.ignore(v1Unused * 2);

	if ((version != 0 && version != 1) ||
	    tag != FOUR_CHARS_TO_INT('s','n','d','2') ||
	    sound_count < 0 ||
	    source_count < 0)
	{
		return false;
	}

	if (sound_count == 0)
	{
		sound_count = source_count;
		source_count = 1;
	}

	// load the definitions
	sound_definitions.resize(source_count);
	for (int source = 0; source < source_count; source++)
	{
		sound_definitions[source].resize(sound_count);
		for (int i = 0; i < sound_count; i++)
		{
			if (!sound_definitions[source][i].Unpack(*sound_file)) 
			{
				Close();
				return false;
			}
		}
	}

	// load all the headers
	for (int source = 0; source < source_count; ++source) 
	{
		for (int i = 0; i < sound_count; ++i) 
		{
			sound_definitions[source][i].Load(*sound_file, true);
		}
	}

	// keep the sound file opened
	opened_sound_file = sound_file;
	
	return true;
}

void M2SoundFile::Close()
{
	sound_definitions.clear();
}

SoundDefinition* M2SoundFile::GetSoundDefinition(int source, int sound_index)
{
	if (source < sound_definitions.size() && sound_index < sound_definitions[source].size())
		return &sound_definitions[source][sound_index];
	else
		return 0;
}

boost::shared_ptr<SoundData> M2SoundFile::GetSoundData(SoundDefinition* definition, int permutation)
{
	return definition->LoadData(*opened_sound_file, permutation);
}

bool M1SoundFile::Open(FileSpecifier& SoundFile)
{
	return SoundFile.Open(resource_file);
}

void M1SoundFile::Close()
{
	resource_file.Close();
}

SoundDefinition* M1SoundFile::GetSoundDefinition(int, int sound_index)
{
	if (resource_file.Check('s', 'n', 'd', ' ', sound_index))
	{
		std::map<int16, SoundDefinition>::iterator it = definitions.find(sound_index);
		if (it == definitions.end())
		{
			SoundDefinition definition;
			definition.sound_code = sound_index;
			it = definitions.insert(std::pair<int16, SoundDefinition>(sound_index, definition)).first;
		}
		return &(it->second);
	}
	else
	{
		return 0;
	}
}

SoundHeader M1SoundFile::GetSoundHeader(SoundDefinition* definition, int)
{
	SoundHeader header;
	LoadedResource rsrc;
	if (resource_file.Get('s', 'n', 'd', ' ', definition->sound_code, rsrc))
	{
		header.Load(FindData(rsrc));
	}

	return header;
}

boost::shared_ptr<SoundData> M1SoundFile::GetSoundData(SoundDefinition* definition, int)
{
	SoundHeader header;
	LoadedResource rsrc;
	if (resource_file.Get('s', 'n', 'd', ' ', definition->sound_code, rsrc))
	{
		return header.Load(FindData(rsrc));
	}
	else
	{
		return boost::shared_ptr<SoundData>();
	}
}

const uint8* M1SoundFile::FindData(LoadedResource& rsrc)
{
	// Open stream to resource
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int)rsrc.GetLength());
	if (p == NULL)
		return 0;

	// Get resource format
	uint16 format = SDL_ReadBE16(p);
	if (format != 1 && format != 2) {
		fprintf(stderr, "Unknown sound resource format %d\n", format);
		SDL_RWclose(p);
		return 0;
	}

	// Skip sound data types or reference count
	if (format == 1) {
		uint16 num_data_formats = SDL_ReadBE16(p);
		SDL_RWseek(p, num_data_formats * 6, SEEK_CUR);
	} else if (format == 2)
		SDL_RWseek(p, 2, SEEK_CUR);

	// Scan sound commands for bufferCmd
	uint16 num_cmds = SDL_ReadBE16(p);
	for (int i=0; i<num_cmds; i++) {
		uint16 cmd = SDL_ReadBE16(p);
		uint16 param1 = SDL_ReadBE16(p);
		uint32 param2 = SDL_ReadBE32(p);
		//printf("cmd %04x %04x %08x\n", cmd, param1, param2);

		if (cmd == 0x8051) {
			return reinterpret_cast<const uint8*>(rsrc.GetPointer()) + param2;
		}
	}
}

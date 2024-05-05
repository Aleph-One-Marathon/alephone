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
#include "byte_swapping.h"
#include <assert.h>

#include "BStream.h"
#include <boost/iostreams/stream_buffer.hpp>
#include <utility>

namespace io = boost::iostreams;

SoundHeader::SoundHeader() :
	SoundInfo(),
	data_offset(0),
	signed_8bits(false)
{
}

bool SoundHeader::UnpackStandardSystem7Header(BIStreamBE &header)
{
	try 
	{
		bytes_per_frame = 1;
		audio_format = AudioFormat::_8_bit;
		stereo = false;
		little_endian = false;
		header.ignore(4); // sample pointer
		header >> length;
		header >> rate;
		header >> loop_start;
		header >> loop_end;
		
		return true;
	} catch (const basic_bstream::failure& e) {
		return false;
	}
}

bool SoundHeader::UnpackExtendedSystem7Header(BIStreamBE &header)
{
	try 
	{
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
			signed_8bits = true;
			header.ignore(4);
		} else {
			header.ignore(22);
		}

		int16 sample_size;
		header >> sample_size;

		audio_format = sample_size == 16 ? AudioFormat::_16_bit : AudioFormat::_8_bit;
		bytes_per_frame = (audio_format == AudioFormat::_16_bit ? 2 : 1) * (stereo ? 2 : 1);

		length = num_frames * bytes_per_frame;
		little_endian = false;

		if ((loop_end - loop_start >= 4) && ((loop_start % bytes_per_frame) || (loop_end % bytes_per_frame)))
		{
			logWarning("loop_start=%i and loop_end=%i but bytes_per_frame=%i; interpreting as frame offsets", loop_start, loop_end, bytes_per_frame);
			loop_start *= bytes_per_frame;
			loop_end *= bytes_per_frame;
		}
		
		return true;
	} catch (const basic_bstream::failure& e) {
		return false;
	}
}

bool SoundHeader::Load(BIStreamBE& s)
{
	Clear();

	uint8 encoding;
	s.rdbuf()->pubseekoff(20, std::ios_base::cur);
	encoding = s.rdbuf()->sgetc();
	s.rdbuf()->pubseekoff(-20, std::ios_base::cur);

	switch (encoding) 
	{
	case stdSH:
		if (UnpackStandardSystem7Header(s))
		{
			data_offset = 22;
			return true;
		}
		break;
	case extSH:
	case cmpSH:
		if (UnpackExtendedSystem7Header(s))
		{
			data_offset = 64;
			return true;
		}
		break;
	}

	return false;
}

std::shared_ptr<SoundData> SoundHeader::LoadData(BIStreamBE& s)
{
	if (!data_offset || length <= 0)
	{
		return std::shared_ptr<SoundData>();
	}

	s.ignore(data_offset);
	auto p = std::make_shared<SoundData>(length);
	try 
	{
		s.read(reinterpret_cast<char*>(&(*p)[0]), length);

		switch (audio_format)
		{
			case AudioFormat::_8_bit:
				if (signed_8bits) {
					ConvertSignedToUnsignedByte(p->data(), length);
				}
				break;
			case AudioFormat::_16_bit:
				if (little_endian ^ PlatformIsLittleEndian()) {
					byte_swap_memory(p->data(), _2byte, length / 2);
				}
				break;
		}
	}
	catch (const basic_bstream::failure& e)
	{
		p.reset();
	}

	return p;
}

void SoundHeader::ConvertSignedToUnsignedByte(uint8* data, int length) 
{
	for (int i = 0; i < length; i++) {
		data[i] = static_cast<int8>(data[i]) + 128;
	}
}

bool SoundHeader::Load(OpenedFile &SoundFile)
{
	io::stream_buffer<opened_file_device> sb(SoundFile);
	BIStreamBE s(&sb);

	return Load(s);
}

std::shared_ptr<SoundData> SoundHeader::LoadData(OpenedFile& SoundFile)
{
	io::stream_buffer<opened_file_device> sb(SoundFile);
	BIStreamBE s(&sb);
	
	return LoadData(s);
}

bool SoundHeader::Load(LoadedResource& rsrc)
{
	io::stream_buffer<io::array_source> sb(reinterpret_cast<char*>(rsrc.GetPointer()), rsrc.GetLength());
	BIStreamBE s(&sb);

	// Get resource format
	uint16 format;
	s >> format;
	if (format != 1 && format != 2)
	{
		logWarning("Unknown sound resource format %d", format);
		return false;
	}

	// Skip sound data types or reference count
	if (format == 1)
	{
		uint16 num_data_formats;
		s >> num_data_formats;
		s.ignore(num_data_formats * 6);
	}
	else if (format == 2)
	{
		s.ignore(2);
	}

	// Scan sound commands for bufferCmd
	uint16 num_cmds;
	s >> num_cmds;
	for (int i = 0; i < num_cmds; ++i) 
	{
		uint16 cmd, param1;
		uint32 param2;
		
		s >> cmd
		  >> param1
		  >> param2;

		if (cmd == bufferCmd)
		{
			s.rdbuf()->pubseekpos(param2);
			if (Load(s))
			{
				data_offset += param2;
				return true;
			}
		}
	}

	return false;
}

std::shared_ptr<SoundData> SoundHeader::LoadData(LoadedResource& rsrc)
{
	io::stream_buffer<io::array_source> sb(reinterpret_cast<char*>(rsrc.GetPointer()), rsrc.GetLength());
	BIStreamBE s(&sb);

	return LoadData(s);
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


std::shared_ptr<SoundData> SoundDefinition::LoadData(OpenedFile& SoundFile, short permutation)
{
	std::shared_ptr<SoundData> p;
	if (!SoundFile.IsOpen()) 
	{
		return p;
	}

	if (!SoundFile.SetPosition(group_offset + sound_offsets[permutation]))
	{
		return p;
	}

	if (permutation >= sounds.size())
	{
		return p;
	}
	
	return sounds[permutation].LoadData(SoundFile);
}

bool M2SoundFile::Open(FileSpecifier& SoundFileSpec)
{
	Close();

	auto sound_file = std::make_unique<OpenedFile>();

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
	opened_sound_file = std::move(sound_file);
	
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

std::shared_ptr<SoundData> M2SoundFile::GetSoundData(SoundDefinition* definition, int permutation)
{
	return definition->LoadData(*opened_sound_file, permutation);
}

bool M1SoundFile::Open(FileSpecifier& SoundFile)
{
	Close();
	return SoundFile.Open(resource_file);
}

void M1SoundFile::Close()
{
	headers.clear();
	definitions.clear();
	cached_sound_code = -1;
	cached_rsrc.Unload();
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
			definition.behavior_index = 2; // sound_is_loud
			definition.sound_code = sound_index;
			// look for permutations
			definition.permutations = 1;
			while (resource_file.Check('s', 'n', 'd', ' ', sound_index + definition.permutations) && definition.permutations < MAXIMUM_PERMUTATIONS_PER_SOUND)
			{
				++definition.permutations;
			}

			it = definitions.insert(std::pair<int16, SoundDefinition>(sound_index, definition)).first;
		}
		return &(it->second);
	}
	else
	{
		return 0;
	}
}

const int M1SoundFile::MAXIMUM_PERMUTATIONS_PER_SOUND = 5;

SoundHeader M1SoundFile::GetSoundHeader(SoundDefinition* definition, int permutation)
{
	int sound_index = definition->sound_code + std::min(permutation, MAXIMUM_PERMUTATIONS_PER_SOUND);

	SoundHeader header;
	std::map<int16, SoundHeader>::iterator it = headers.find(sound_index);
	if (it == headers.end())
	{
		if (cached_sound_code != sound_index)
		{
			resource_file.Get('s', 'n', 'd', ' ', sound_index, cached_rsrc);
			cached_sound_code = sound_index;
		}

		SoundHeader header;
		header.Load(cached_rsrc);
		it = headers.insert(std::pair<int16, SoundHeader>(sound_index, header)).first;
	}
	
	return it->second;
}

std::shared_ptr<SoundData> M1SoundFile::GetSoundData(SoundDefinition* definition, int permutation)
{
	int sound_index = definition->sound_code + std::min(permutation, MAXIMUM_PERMUTATIONS_PER_SOUND);

	if (cached_sound_code != sound_index)
	{
		resource_file.Get('s', 'n', 'd', ' ', sound_index, cached_rsrc);
		cached_sound_code = sound_index;
	}

	return GetSoundHeader(definition, permutation).LoadData(cached_rsrc);
}

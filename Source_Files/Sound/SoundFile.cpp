/*
SOUNDFILE.CPP

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

*/

#include "SoundFile.h"

bool SoundDefinition::Unpack(OpenedFile &SoundFile)
{
	if (!SoundFile.IsOpen()) return false;

	vector<uint8> headerBuffer(HeaderSize());
	if (!SoundFile.Read(headerBuffer.size(), &headerBuffer.front())) 
		return false;
	
	AIStreamBE header(&headerBuffer.front(), headerBuffer.size());
	
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
		sound_data.resize(total_length);
	else
		sound_data.resize(single_length);

	if (!SoundFile.SetPosition(group_offset))
	{
		sound_data.clear();
		return false;
	}

	if (SoundFile.Read(sound_data.size(), &sound_data.front()))
	{
//		_sm_globals->loaded_sounds_size += sound_data.size();
		return true;
	}
	else
	{
		sound_data.clear();
		return false;
	}

}

bool SoundFile::Open(FileSpecifier& SoundFileSpec)
{
	Close();

	std::auto_ptr<OpenedFile> sound_file(new OpenedFile);

	if (!SoundFileSpec.Open(*sound_file, false)) return false;

	std::vector<uint8> headerBuffer;
	headerBuffer.resize(HeaderSize());

	if (!sound_file->Read(headerBuffer.size(), &headerBuffer.front()))
		return false;

	AIStreamBE header(&headerBuffer.front(), headerBuffer.size());
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

	// keep the sound file opened
	opened_sound_file = sound_file;
	
	return true;
}

void SoundFile::Close()
{
	sound_definitions.clear();
}

SoundDefinition* SoundFile::GetSoundDefinition(int source, int sound_index)
{
	if (source < sound_definitions.size() && sound_index < sound_definitions[source].size())
		return &sound_definitions[source][sound_index];
	else
		return 0;
}

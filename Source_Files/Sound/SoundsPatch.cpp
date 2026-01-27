/*
SOUND_PATCH.CPP

	Copyright (C) 2026 by Gregory Smith
 
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

#include "SoundsPatch.h"

#include <boost/iostreams/stream_buffer.hpp>

#include "FileHandler.h"
#include "ReplacementSounds.h"
#include "SoundFile.h"

namespace io = boost::iostreams;

SoundsPatches sounds_patches;

struct SoundDefinitionPatch {
	bool load(BIStreamBE& stream);

	SoundDefinition definition;
	std::vector<std::shared_ptr<SoundData>> data;
};

class SoundsPatch {
public:
	bool load(BIStreamBE& stream);
	void apply(std::map<std::pair<int, int>, SoundDefinitionPatch>& patches);

private:
	std::map<std::pair<int, int>, SoundDefinitionPatch> definition_patches;
};

bool SoundDefinitionPatch::load(BIStreamBE& stream)
{
	auto offset = stream.tellg();

	data.clear();
	
	for (auto i = 0; i < definition.permutations; ++i)
	{
		// definition group offset is ignored in sound patches
		stream.rdbuf()->pubseekoff(definition.sound_offsets[i], std::ios_base::cur);
		
		auto& sound_header = definition.sounds.emplace_back();
		if (sound_header.Load(stream))
		{
			stream.rdbuf()->pubseekpos(offset);
			stream.rdbuf()->pubseekoff(definition.sound_offsets[i], std::ios_base::cur);

			auto data = sound_header.LoadData(stream);
			if (data)
			{
				this->data.push_back(data);
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
		
		stream.rdbuf()->pubseekpos(offset);
	}

	return true;
}

bool SoundsPatch::load(BIStreamBE& stream)
{
	while (stream.tellg() < stream.maxg())
	{

		uint32_t sndc;
		stream >> sndc;

		if (sndc != FOUR_CHARS_TO_INT('s','n','d','c'))
		{
			return false;
		}
		
		uint16_t source;
		stream >> source;
		
		uint16_t index;
		stream >> index;

		SoundDefinitionPatch patch;
		if (patch.definition.Unpack(stream))
		{
			// Anvil shapes patches to have a redundant list of permutation
			// sizes here
			for (auto i = 0; i < patch.definition.permutations; ++i)
			{
				uint32_t permutation_size;
				stream >> permutation_size;
			}

			auto offset = stream.tellg();
			if (patch.load(stream))
			{
				definition_patches.insert(std::make_pair(std::make_pair(source, index), patch));
			}
			
			stream.rdbuf()->pubseekpos(offset);
			stream.rdbuf()->pubseekoff(patch.definition.total_length, std::ios_base::cur);
		}
		else
		{
			// we have to be able to unpack definitions in order to find the next one
			return false;
		}
	}

	return true;
}

void SoundsPatch::apply(std::map<std::pair<int, int>, SoundDefinitionPatch>& patches)
{
	auto replacements = SoundReplacements::instance();
	
	for (const auto& [key, value] : patches)
	{
		for (auto i = 0; i < value.definition.sounds.size(); ++i)
		{
			replacements->Remove(key.second, i);
		}
	}
	definition_patches.merge(patches);
	std::swap(definition_patches, patches);
}

bool SoundsPatches::add(BIStreamBE& stream)
{
	SoundsPatch sound_patch;
	if (sound_patch.load(stream))
	{
		sound_patch.apply(definition_patches);
		return true;
	}
	else
	{
		return false;
	}
}

bool SoundsPatches::add(FileSpecifier& file_specifier)
{
	OpenedFile file;
	if (file_specifier.Open(file)) {
		io::stream_buffer<opened_file_device> sb{file};
		BIStreamBE stream{&sb};

		return add(stream);
	}
	else
	{
		return false;
	}
}

SoundDefinition* SoundsPatches::get_definition(int source, int sound_index)
{
	auto it = definition_patches.find(std::make_pair(source, sound_index));
	if (it != definition_patches.end())
	{
		return &it->second.definition;
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<SoundData> SoundsPatches::get_sound_data(SoundDefinition* definition, int permutation)
{
	for (const auto& [key, value] : definition_patches)
	{
		if (definition->sound_code == value.definition.sound_code &&
			definition->total_length == value.definition.total_length)
		{
			if (permutation < value.data.size())
			{
				return value.data[permutation];
			}
			break;
		}
	}

	return nullptr;
}

void SoundsPatches::clear()
{
	definition_patches.clear();
}

/*

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

#include "ReplacementSounds.h"
#include "Decoder.h"
#include "SoundManager.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

boost::shared_ptr<SoundData> ExternalSoundHeader::LoadExternal(FileSpecifier& File)
{
	boost::shared_ptr<SoundData> p;
	std::unique_ptr<Decoder> decoder(Decoder::Get(File));
	if (!decoder.get()) return p;

	length = decoder->Frames() * decoder->BytesPerFrame();
	if (!length) return p;

	p = boost::make_shared<SoundData>(length);

	if (decoder->Decode(&(*p)[0], length) != length) 
	{
		p.reset();
		length = 0;
		return p;
	}
	
	sixteen_bit = decoder->IsSixteenBit();
	stereo = decoder->IsStereo();
	signed_8bit = decoder->IsSigned();
	bytes_per_frame = decoder->BytesPerFrame();
	little_endian = decoder->IsLittleEndian();
	loop_start = loop_end = 0;
	rate = (uint32 /* unsigned fixed */) (FIXED_ONE * decoder->Rate());

	return p;
}

SoundOptions* SoundReplacements::GetSoundOptions(short Index, short Slot)
{
	boost::unordered_map<key, SoundOptions>::iterator it = m_hash.find(key(Index, Slot));
	if (it != m_hash.end()) 
	{
		return &it->second;
	} 
	else
	{
		return 0;
	}
}

void SoundReplacements::Add(const SoundOptions& Data, short Index, short Slot)
{
	SoundManager::instance()->UnloadSound(Index);
	m_hash[key(Index, Slot)] = Data;
}

void SoundReplacements::Reset()
{
	for (auto kvp : m_hash)
	{
		SoundManager::instance()->UnloadSound(kvp.first.first);
	}
	m_hash.clear();
}

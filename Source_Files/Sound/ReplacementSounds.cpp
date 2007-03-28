/*

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

#include "ReplacementSounds.h"
#include "Decoder.h"

SoundReplacements *SoundReplacements::m_instance = 0;

bool ExternalSoundHeader::LoadExternal(FileSpecifier& File)
{
	auto_ptr<Decoder> decoder(Decoder::Get(File));
	if (!decoder.get()) return false;

	int32 length = decoder->Frames() * decoder->BytesPerFrame();
	if (!length) return false;

	if (decoder->Decode(Load(length), length) != length) 
	{
		Clear();
		return false;
	}
	
	sixteen_bit = decoder->IsSixteenBit();
	stereo = decoder->IsStereo();
	signed_8bit = decoder->IsSigned();
	bytes_per_frame = decoder->BytesPerFrame();
	little_endian = decoder->IsLittleEndian();
	loop_start = loop_end = 0;
	rate = (uint32 /* unsigned fixed */) (FIXED_ONE * decoder->Rate());

	return true;
	
}

SoundReplacements::SoundReplacements()
{
	SOHash.resize(SoundOptions::HashSize, NONE);
}

SoundOptions* SoundReplacements::GetSoundOptions(short Index, short Slot)
{
	// Set up a *reference* to the appropriate hashtable entry
	int16& HashVal = SOHash[SoundOptions::HashFunc(Index, Slot)];
	
	// Check to see if the sound-option entry is correct;
	// if it is, then we're done.
	if (HashVal != NONE)
	{
		std::vector<SoundOptionsEntry>::iterator SOIter = SOList.begin() + HashVal;
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			return &SOIter->OptionsData;
		}
	}

	// Fallback for the case of a hashtable miss;
	// do a linear search and then update the has entry appropriately
	int16 Indx = 0;
	for (std::vector<SoundOptionsEntry>::iterator SOIter = SOList.begin(); SOIter < SOList.end(); SOIter++, Indx++)
	{
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			HashVal = Indx;
			return &SOIter->OptionsData;
		}
	}

	// None found
	return 0;
}

void SoundReplacements::Add(const SoundOptions& Data, short Index, short Slot)
{
	// Check to see if a frame is already accounted for
	for (vector<SoundOptionsEntry>::iterator SOIter = SOList.begin(); SOIter < SOList.end(); SOIter++)
	{
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			SOIter->OptionsData = Data;
			return;
		}
	}

	// If not, then add a new frame entry
	SoundOptionsEntry DataEntry;
	DataEntry.Index = Index;
	DataEntry.Slot = Slot;
	DataEntry.OptionsData = Data;
	SOList.push_back(DataEntry);
}

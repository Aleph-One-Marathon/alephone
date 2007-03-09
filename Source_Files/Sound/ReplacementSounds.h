#ifndef __REPLACEMENTSOUNDS_H
#define __REPLACEMENTSOUNDS_H

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

	Handles MML-specified external replacement sounds

*/

#include <string>
#include "SoundFile.h"

class ExternalSoundHeader : public SoundHeader
{
public:
	ExternalSoundHeader() : SoundHeader() { }
	~ExternalSoundHeader() { }
	bool LoadExternal(FileSpecifier& File);
};

struct SoundOptions
{
	std::string File;
	ExternalSoundHeader Sound;

	static const int HashSize = 1 << 8;
	static const int HashMask = HashSize - 1;
	static inline uint8 HashFunc(short Index, short Slot) {
		// This function will avoid collisions when accessing sounds with the same index
		// and different slots (permutations)
		return (uint8)(Index ^ (Slot << 4));
	}
};

struct SoundOptionsEntry
{
	short Index;
	short Slot;
	SoundOptions OptionsData;
};

class SoundReplacements
{
public:
	static inline SoundReplacements* instance() { 
		if (!m_instance) m_instance = new SoundReplacements;
		return m_instance;
	}

	SoundOptions *GetSoundOptions(short Index, short Slot);
	void Reset() { SOList.clear(); }
	void Add(const SoundOptions& Data, short Index, short Slot);

private:
	SoundReplacements();
	static SoundReplacements *m_instance;
	
	// list of sound-options records
	// and a hash table for them
	std::vector<SoundOptionsEntry> SOList;
	std::vector<int16> SOHash;
};

#endif

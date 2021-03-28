#ifndef __REPLACEMENTSOUNDS_H
#define __REPLACEMENTSOUNDS_H

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

	Handles MML-specified external replacement sounds

*/

#include <string>
#include "SoundFile.h"

#include <boost/unordered_map.hpp>

class ExternalSoundHeader : public SoundInfo
{
public:
	ExternalSoundHeader() : SoundInfo() { }
	~ExternalSoundHeader() { }
	boost::shared_ptr<SoundData> LoadExternal(FileSpecifier& File);
};

struct SoundOptions
{
	FileSpecifier File;
	ExternalSoundHeader Sound;
};

class SoundReplacements
{
public:
	static inline SoundReplacements* instance() { 
		static SoundReplacements *m_instance = nullptr;
		if (!m_instance) m_instance = new SoundReplacements;
		return m_instance;
	}

	SoundOptions *GetSoundOptions(short Index, short Slot);
	void Reset();
	void Add(const SoundOptions& Data, short Index, short Slot);

private:
	SoundReplacements() { }

	typedef std::pair<short, short> key;

	boost::unordered_map<key, SoundOptions> m_hash;
};

#endif

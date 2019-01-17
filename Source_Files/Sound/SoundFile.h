#ifndef __SOUND_DEFINITION_H
#define __SOUND_DEFINITION_H

/*
SOUND_DEFINITIONS.H

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

#include "AStream.h"
#include "BStream.h"
#include "FileHandler.h"
#include <memory>
#include <vector>
#include <map>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

typedef std::vector<uint8> SoundData;

class SoundInfo 
{
public:
	SoundInfo() : sixteen_bit(false), 
		      stereo(false), 
		      signed_8bit(false), 
		      little_endian(false), 
		      bytes_per_frame(1),
		      loop_start(0),
		      loop_end(0),
		      rate(0),
		      length(0) { }
	
	bool sixteen_bit;
	bool stereo;
	bool signed_8bit;
	bool little_endian;
	int bytes_per_frame;
	int32 loop_start;
	int32 loop_end;
	uint32 rate;
	int32 length;
};

class SoundHeader : public SoundInfo
{
public:
	SoundHeader();
	virtual ~SoundHeader() { };

	bool Load(BIStreamBE& stream);
	boost::shared_ptr<SoundData> LoadData(BIStreamBE& stream);

	bool Load(OpenedFile &SoundFile); // loads a system 7 header from file
	boost::shared_ptr<SoundData> LoadData(OpenedFile& SoundFile);
	
	bool Load(LoadedResource& rsrc); // finds system 7 header in rsrc
	boost::shared_ptr<SoundData> LoadData(LoadedResource& rsrc);

	int32 Length() const
		{ return length; };
	
	void Clear() { length = 0; }

private:
	static const uint8 stdSH = 0x00; // standard sound header
	static const uint8 extSH = 0xFF; // extended sound header
	static const uint8 cmpSH = 0xFE; // compressed sound header
	static const uint16 bufferCmd = 0x8051;

	bool UnpackStandardSystem7Header(BIStreamBE &header);
	bool UnpackExtendedSystem7Header(BIStreamBE &header);
	
	uint32 data_offset;
};

class SoundDefinition
{
public:
	SoundDefinition();
	bool Unpack(OpenedFile &SoundFile);
	bool Load(OpenedFile &SoundFile, bool LoadPermutations);
	boost::shared_ptr<SoundData> LoadData(OpenedFile& SoundFile, short permutation);
	void Unload() { sounds.clear(); }

	static const int MAXIMUM_PERMUTATIONS_PER_SOUND = 5;

private:
	static int HeaderSize() { return 64; }
	
public: // for now
	int16 sound_code;
	int16 behavior_index;
	uint16 flags;

	uint16 chance; // play sound if AbsRandom() >= chance
	
	/* if low_pitch==0 use FIXED_ONE; if high_pitch==0 use low pitch; else choose in [low_pitch, high_pitch] */
	_fixed low_pitch, high_pitch;

	/* filled in later */
	int16 permutations;
	uint16 permutations_played;

	int32 group_offset, single_length, total_length; // magic numbers necessary to load sounds
	std::vector<int32> sound_offsets; // zero-based from group offset

	uint32 last_played; // machine ticks

	std::vector<SoundHeader> sounds;
};

class SoundFile
{
public:
	virtual bool Open(FileSpecifier& SoundFile) = 0;
	virtual void Close() = 0;
	virtual SoundDefinition* GetSoundDefinition(int source, int sound_index) = 0;
	virtual SoundHeader GetSoundHeader(SoundDefinition* definition, int permutation) = 0;
	virtual boost::shared_ptr<SoundData> GetSoundData(SoundDefinition* definition, int permutation) = 0;

	virtual int SourceCount() { return 1; };
	virtual ~SoundFile() = default;
};

class M1SoundFile : public SoundFile
{
public:
	M1SoundFile() : cached_sound_code(-1) { }
	virtual ~M1SoundFile() = default;
	bool Open(FileSpecifier& SoundFile);
	void Close();
	SoundDefinition* GetSoundDefinition(int source, int sound_index);
	SoundHeader GetSoundHeader(SoundDefinition* definition, int permutation);
	boost::shared_ptr<SoundData> GetSoundData(SoundDefinition* definition, int permutation);

private:
	OpenedResourceFile resource_file;
	LoadedResource cached_rsrc;
	int16 cached_sound_code;

	static const int MAXIMUM_PERMUTATIONS_PER_SOUND;

	std::map<int16, SoundDefinition> definitions;
	std::map<int16, SoundHeader> headers;
};

class M2SoundFile : public SoundFile
{
public:
	virtual ~M2SoundFile() = default;
	bool Open(FileSpecifier &SoundFile);
	void Close();
	SoundDefinition* GetSoundDefinition(int source, int sound_index);
	SoundHeader GetSoundHeader(SoundDefinition* definition, int permutation) { 
		return definition->sounds[permutation];
	}
	boost::shared_ptr<SoundData> GetSoundData(SoundDefinition* definition, int permutation);

	int SourceCount() { return source_count; }

private:
	int32 version;
	int32 tag;
	
	int16 source_count;
	int16 sound_count;

	static const int v1Unused = 124;

	std::vector< std::vector<SoundDefinition> > sound_definitions;

	static int HeaderSize() { return 260; }
	std::unique_ptr<OpenedFile> opened_sound_file;
};

#endif

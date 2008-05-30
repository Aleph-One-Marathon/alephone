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
#include "Logging.h"
#include "csmisc.h"
#include "Decoder.h"

#include <assert.h>

SoundHeader::SoundHeader() :
	sixteen_bit(false),
	stereo(false),
	signed_8bit(false),
	bytes_per_frame(false),
	little_endian(false),
	loop_start(0),
	loop_end(0),
	rate(0),
	data(0),
	length(0)
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

bool SoundHeader::Load(const uint8* data)
{
	Clear();
	if (data[20] == 0x00)
	{
		AIStreamBE header(data, 22);
		if (!UnpackStandardSystem7Header(header)) return false;
		this->data = data + 22;
		return true;
	}
	else if (data[20] == 0xff || data[20] == 0xfe)
	{
		AIStreamBE header(data, 64);
		if (!UnpackExtendedSystem7Header(header)) return false;
		this->data = data + 64;
		return true;
	}
	return false;
}

bool SoundHeader::Load(OpenedFile &SoundFile)
{
	Clear();
	if (!SoundFile.IsOpen()) return false;

	long file_position;
	SoundFile.GetPosition(file_position);
	SoundFile.SetPosition(file_position + 20);
	uint8 header_type;
	if (!SoundFile.Read(1, &header_type)) return false;
	SoundFile.SetPosition(file_position);

	if (header_type == 0x0)
	{
		// standard sound header
		vector<uint8> headerBuffer(22);
		if (!SoundFile.Read(headerBuffer.size(), &headerBuffer.front()))
			return false;

		AIStreamBE header(&headerBuffer.front(), headerBuffer.size());
		if (!UnpackStandardSystem7Header(header)) return false;
		stored_data.resize(length);
		if (!SoundFile.Read(stored_data.size(), &stored_data.front())) return false;
		return true;
	}
	else if (header_type == 0xff || header_type == 0xfe)
	{
		vector<uint8> headerBuffer(64);
		if (!SoundFile.Read(headerBuffer.size(), &headerBuffer.front()))
			return false;

		AIStreamBE header(&headerBuffer.front(), headerBuffer.size());
		if (!UnpackExtendedSystem7Header(header)) return false;
		stored_data.resize(length);
		if (!SoundFile.Read(stored_data.size(), &stored_data.front())) return false;
		return true;
	}

	return false;
}

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
}

int32 SoundDefinition::LoadedSize()
{
	int32 size = 0;
	for (int i = 0; i < sounds.size(); i++)
	{
		size += sounds[i].Length();
	}

	return size;
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
	real_sound_count = sound_count;
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

void SoundFile::UnloadCustomSounds() {
  sound_count = real_sound_count;
  for(vector<vector<SoundDefinition> >::iterator it = sound_definitions.begin(); it != sound_definitions.end(); ++it) {
    (*it).resize(sound_count);
  }
}

int SoundFile::NewCustomSoundDefinition() {
  SoundDefinition def;
  def.behavior_index = 2; // _sound_is_normal?
  def.flags = 0;
  def.chance = 0;
  def.low_pitch = def.high_pitch = 0;
  def.permutations = 0;
  def.permutations_played = 0;
  def.group_offset = def.single_length = def.total_length = 0;
  def.last_played = machine_tick_count();
  int index = sound_count;
  def.sound_code = 19000 + index; // necessary...
  ++sound_count;
  for(vector<vector<SoundDefinition> >::iterator it = sound_definitions.begin(); it != sound_definitions.end(); ++it) {
    // this makes me squirm, what if index is wrong?
    (*it).push_back(def);
    // make me feel better
    assert(sound_count == (*it).size());
  }
  return index;
}

// ReplacementSounds.cpp refuses to cooperate because of a weird interaction
// with the existing sound loading code... so I ended up having to copy and
// paste a fair amount of code from there.
static bool TryLoadingExternal(SoundHeader& hdr, const char* path) {
  FileSpecifier file;
  if(!file.SetNameWithPath(path)) return false;
  auto_ptr<Decoder> decoder(Decoder::Get(file));
  if(!decoder.get()) return false;
  int32 length = decoder->Frames() * decoder->BytesPerFrame();
  if(!length) return false;
  // it has to be done this way because of some weirdness that happens if it
  // isn't. Is it normal for this much weirdness to be concentrated in this
  // little code?
  if(decoder->Decode(hdr.Load(length), length) != length) {
    hdr.Clear();
    return false;
  }
  else {
    hdr.sixteen_bit = decoder->IsSixteenBit();
    hdr.stereo = decoder->IsStereo();
    hdr.signed_8bit = decoder->IsSigned();
    hdr.bytes_per_frame = decoder->BytesPerFrame();
    hdr.little_endian = decoder->IsLittleEndian();
    hdr.loop_start = hdr.loop_end = 0;
    hdr.rate = (uint32) (FIXED_ONE * decoder->Rate());
    return true;
  }
}

bool SoundFile::AddCustomSoundSlot(int index, const char* file) {
  if(index < real_sound_count && index >= sound_count) return false;
  SoundHeader sound;
  bool ret = TryLoadingExternal(sound, file);
  if(!ret) return false;
  for(vector<vector<SoundDefinition> >::iterator it = sound_definitions.begin(); it != sound_definitions.end(); ++it) {
    if((*it)[index].sounds.size() >= 5 /*MAXIMUM_PERMUTATIONS_PER_SOUND*/) continue;
    int sound_index = (*it)[index].sounds.size();
    (*it)[index].sounds.resize(sound_index+1);
    (*it)[index].sounds[sound_index] = sound;
    ++(*it)[index].permutations;
  }
  return true;
}

SoundDefinition* SoundFile::GetSoundDefinition(int source, int sound_index)
{
	if (source < sound_definitions.size() && sound_index < sound_definitions[source].size())
		return &sound_definitions[source][sound_index];
	else
		return 0;
}

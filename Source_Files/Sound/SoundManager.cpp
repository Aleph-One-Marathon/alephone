/*
SOUND.C

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

#include <iostream>
#include <functional>

#include "SoundManager.h"
#include "ReplacementSounds.h"
#include "sound_definitions.h"
#include "images.h"
#include "InfoTree.h"
#include "OpenALManager.h"
#include "shell_options.h"
#include "Movie.h"

#undef SLOT_IS_USED
#undef SLOT_IS_FREE
#undef MARK_SLOT_AS_FREE
#undef MARK_SLOT_AS_USED

#define SLOT_IS_USED(o) ((o)->flags&(uint16)0x8000)
#define SLOT_IS_FREE(o) (!SLOT_IS_USED(o))
#define MARK_SLOT_AS_FREE(o) ((o)->flags&=(uint16)~0x8000)
#define MARK_SLOT_AS_USED(o) ((o)->flags|=(uint16)0x8000)

class SoundMemoryManager {
public:
	SoundMemoryManager(std::size_t max_size) : m_size(0), m_max_size(max_size) { }

	void SetMaxSize(std::size_t max_size) { m_max_size = max_size; }

	void Add(std::shared_ptr<SoundData> data, short index, short slot);
	std::shared_ptr<SoundData> Get(short index, short slot) { return m_entries[index].data[slot]; }
	void Update(short index);
	std::function<void (short)> SoundReleased;

	bool IsLoaded(short index) {
		return m_entries.count(index);
	}

	void Clear() { m_entries.clear(); m_size = 0; }
	void Release(short index); // sound must be loaded

private:
	struct Entry {
		Entry() : data(5), last_played(0) { }
		std::vector<std::shared_ptr<SoundData> > data;
		uint64_t last_played;

		std::size_t size() {
			std::size_t n = 0;
			for (std::vector<std::shared_ptr<SoundData> >::iterator it = data.begin(); it != data.end(); ++it) 
			{
				if (it->get()) 
				{
					n += (*it)->size();
				}
			}
			
			return n;
		}
	};

	void ReleaseOldestSound();
	std::map<short, Entry> m_entries;
	std::size_t m_size;
	std::size_t m_max_size;
};

void SoundMemoryManager::Add(std::shared_ptr<SoundData> data, short index, short slot)
{
	m_entries[index].data[slot] = data;
	m_entries[index].last_played = machine_tick_count();

	m_size += data->size();

	while (m_size > m_max_size)
	{
		std::cerr << "Size is too big (" << m_size << ">" << m_max_size << ")" << std::endl;
		ReleaseOldestSound();
	}
}

void SoundMemoryManager::Release(short index)
{
	if (SoundReleased) 
	{
		SoundReleased(index);
	}
	m_size -= m_entries[index].size();
	m_entries.erase(index);
}

void SoundMemoryManager::ReleaseOldestSound()
{
	if (!m_entries.size())
	{
		return;
	}
	
	std::map<short, Entry>::iterator oldest_sound = m_entries.begin();
	std::map<short, Entry>::iterator it = oldest_sound;
	++it;
	for (; it != m_entries.end(); ++it)
	{
		if (it->second.last_played < oldest_sound->second.last_played)
		{
			oldest_sound = it;
		}
	}

	std::cerr << "Dropping sound " << oldest_sound->first << std::endl;
	Release(oldest_sound->first);
}

void SoundMemoryManager::Update(short index)
{
	m_entries[index].last_played = machine_tick_count();
}


static void Shutdown()
{
	SoundManager::instance()->Shutdown();
	OpenALManager::Shutdown();
}

// From FileSpecifier_SDL.cpp
extern void get_default_sounds_spec(FileSpecifier &file);

void SoundManager::Initialize(const Parameters& new_parameters)
{

	FileSpecifier InitialSoundFile;
	get_default_sounds_spec(InitialSoundFile);
	if (OpenSoundFile(InitialSoundFile))
	{
		atexit(::Shutdown);
		parameters.flags = 0;
		initialized = true;
		active = false;
		SetParameters(new_parameters);
	}
}

void SoundManager::SetParameters(const Parameters& parameters)
{
	if (!initialized) return;

	// Stuff in our new parameters
	this->parameters = parameters;
	this->parameters.Verify();
	SetStatus(true);
}

void SoundManager::Shutdown()
{
	instance()->SetStatus(false);
	instance()->CloseSoundFile();
}

bool SoundManager::OpenSoundFile(FileSpecifier& File)
{
	UnloadAllSounds();
	sound_file.reset(new M2SoundFile);
	if (!sound_file->Open(File))
	{
		// try M1 sounds
		sound_file.reset(new M1SoundFile);
		if (!sound_file->Open(File))
		{
			return false;
		}
		set_sounds_images_file(File);
	}

	sound_source = (parameters.flags & _16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;
	if (sound_file->SourceCount() == 1)
		sound_source = _8bit_22k_source;

	return true;
}

void SoundManager::CloseSoundFile()
{
	StopAllSounds();
	sound_file->Close();
}

bool SoundManager::AdjustVolumeUp(short sound_index)
{
	if (active && parameters.volume_db < MAXIMUM_VOLUME_DB)
	{
		parameters.volume_db += 2.f;
		if (parameters.volume_db > MAXIMUM_VOLUME_DB)
		{
			parameters.volume_db = MAXIMUM_VOLUME_DB;
		}
		OpenALManager::Get()->SetMasterVolume(From_db(parameters.volume_db));
		PlaySound(sound_index, 0, NONE);
		return true;
	}
	return false;
}

bool SoundManager::AdjustVolumeDown(short sound_index)
{
	if (active && parameters.volume_db > MINIMUM_VOLUME_DB)
	{
		parameters.volume_db -= 2.f;
		if (parameters.volume_db <= MINIMUM_VOLUME_DB)
		{
			parameters.volume_db = MINIMUM_VOLUME_DB;
		}
		OpenALManager::Get()->SetMasterVolume(From_db(parameters.volume_db));
		PlaySound(sound_index, 0, NONE);
		return true;
	}
	return false;
}

bool SoundManager::LoadSound(short sound_index)
{
	if (!active) return false;

	SoundDefinition *definition = GetSoundDefinition(sound_index);
	if (!definition) return false;

	// Load all the external-file sounds for each index;
	// fill the slots appropriately.
	int NumSlots= (parameters.flags & _more_sounds_flag) ? definition->permutations : 1;

	if (definition->sound_code == NONE) 
	{
		return false;
	}

	if (!(parameters.flags & _ambient_sound_flag) && (definition->flags & _sound_is_ambient))
	{
		return false;
	}
			
	if (sounds->IsLoaded(sound_index))
	{
		sounds->Update(sound_index);
	} 
	else
	{
		for (int i = 0; i < NumSlots; ++i)
		{
			auto p = sound_file->GetSoundData(definition, i);

			SoundOptions *SndOpts = SoundReplacements::instance()->GetSoundOptions(sound_index, i);
			if (SndOpts)
			{
				auto x = SndOpts->Sound.LoadExternal(SndOpts->File);
				if (x.get()) 
				{
					p = x;
				}
			}

			if (p.get())
			{
				sounds->Add(p, sound_index, i);
			}
		}
	}

	return sounds->IsLoaded(sound_index);
}

void SoundManager::LoadSounds(short *sounds, short count)
{
	for (short i = 0; i < count; i++)
	{
		LoadSound(sounds[i]);
	}
}

void SoundManager::StopSound(short identifier, short sound_index)
{
	if (active)
	{
		auto player = GetSoundPlayer(sound_index, identifier, !(parameters.flags & _3d_sounds_flag));
		if (player) player->AskStop();
	}
}

void SoundManager::UnloadSound(short sound_index)
{
	StopSound(NONE, sound_index);
	if (sounds->IsLoaded(sound_index))
	{
		sounds->Release(sound_index);
	}
}

void SoundManager::UnloadAllSounds()
{
	if (active)
	{
		sounds->Clear();
	}
}

//Do we have a player currently streaming with the same sound we want to play ?
//A sound is identified as unique with sound index + source index, NONE is considered as a valid source index (local sounds)
//The flag sound_identifier_only must be used to know if there is a sound playing with a specific identifier without caring of the source
std::shared_ptr<SoundPlayer> SoundManager::GetSoundPlayer(short identifier, short source_identifier, bool sound_identifier_only) const {

	std::vector<std::shared_ptr<SoundPlayer>> matchingPlayers;
	std::copy_if(sound_players.begin(), sound_players.end(), std::back_inserter(matchingPlayers),
		[identifier](const std::shared_ptr<SoundPlayer> player) { return player->IsActive() && (identifier != NONE && player->GetIdentifier() == identifier); });

	auto matchingPlayer = matchingPlayers.size() > 0 ? matchingPlayers[0] : std::shared_ptr<SoundPlayer>();

	if (!sound_identifier_only) {

		auto matchingSourcePlayer = std::find_if(matchingPlayers.begin(), matchingPlayers.end(),
			[source_identifier](const std::shared_ptr<SoundPlayer> player) { return player->GetSourceIdentifier() == source_identifier; });

		if (matchingSourcePlayer == matchingPlayers.end() && matchingPlayers.size() >= MAX_SOUNDS_FOR_SOURCE) {
			matchingPlayer = *std::min_element(matchingPlayers.begin(), matchingPlayers.end(),
				[](const std::shared_ptr<SoundPlayer>& a, const std::shared_ptr<SoundPlayer>& b)
				{  return a->GetPriority() < b->GetPriority(); });
		}
		else {
			matchingPlayer = matchingSourcePlayer != matchingPlayers.end() ? *matchingSourcePlayer : std::shared_ptr<SoundPlayer>();
		}
	}

	return matchingPlayer;
}

std::shared_ptr<SoundPlayer> SoundManager::PlaySound(LoadedResource& rsrc, const SoundParameters& parameters) {
	SoundHeader header;
	if (!active || OpenALManager::Get()->GetMasterVolume() <= 0 || !(header.Load(rsrc)))
		return std::shared_ptr<SoundPlayer>();

	auto data = header.LoadData(rsrc);
	return ManageSound({ header, data }, parameters);
}

std::shared_ptr<SoundPlayer> SoundManager::PlaySound(short sound_index, 
			     world_location3d *source,
			     short identifier, // NONE is no identifier and the sound is immediately orphaned
			     _fixed pitch,
			     bool soft_rewind)
{
	if (sound_index == NONE || !active || OpenALManager::Get()->GetMasterVolume() <= 0 || !LoadSound(sound_index))
		return std::shared_ptr<SoundPlayer>();

	SoundParameters parameters;
	parameters.identifier = sound_index;
	parameters.pitch = pitch;
	parameters.soft_rewind = soft_rewind;
	parameters.is_2d = !source;
	parameters.source_identifier = identifier;

	if (source) {

		parameters.source_location3d = *source;
		parameters.dynamic_source_location3d = (this->parameters.flags & _dynamic_tracking_flag) && identifier != NONE ? source : nullptr;

		if (this->parameters.flags & _3d_sounds_flag) {
			parameters.obstruction_flags = GetSoundObstructionFlags(sound_index, source);
		}
		else {
			SoundVolumes variables;
			CalculateInitialSoundVariables(sound_index, source, variables); //obstruction calcul is done here
			parameters.is_2d = true;
			parameters.stereo_parameters.is_panning = true;
			parameters.stereo_parameters.gain_global = variables.volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_left = variables.left_volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_right = variables.right_volume * 1.f / MAXIMUM_SOUND_VOLUME;
		}
	}

	return BufferSound(parameters);
}
				
std::shared_ptr<SoundPlayer> SoundManager::DirectPlaySound(short sound_index, angle direction, short volume, _fixed pitch)
{
	if (sound_index == NONE || !active || parameters.volume_db <= MINIMUM_VOLUME_DB || !LoadSound(sound_index))
		return std::shared_ptr<SoundPlayer>();

	SoundParameters parameters;
	parameters.identifier = sound_index;
	parameters.pitch = pitch;

	world_location3d* listener = _sound_listener_proc();
	if (direction != NONE && listener)
	{
		SoundVolumes variables;
		variables.volume = volume;
		AngleAndVolumeToStereoVolume(direction - listener->yaw, volume, &variables.right_volume, &variables.left_volume);
		parameters.stereo_parameters.is_panning = true;
		parameters.stereo_parameters.gain_global = volume * 1.f / MAXIMUM_SOUND_VOLUME;
		parameters.stereo_parameters.gain_left = variables.left_volume * 1.f / MAXIMUM_SOUND_VOLUME;
		parameters.stereo_parameters.gain_right = variables.right_volume * 1.f / MAXIMUM_SOUND_VOLUME;
	}

	return BufferSound(parameters);
}

void SoundManager::StopAllSounds() {
	for (auto& soundPlayer : sound_players)
		soundPlayer->AskStop();
}

uint64_t SoundManager::GetCurrentAudioTick() {

	if (Movie::instance()->IsRecording())
		return Movie::instance()->GetCurrentAudioTimeStamp();

	return machine_tick_count() - (OpenALManager::Get() ? OpenALManager::Get()->GetElapsedPauseTime() : 0);
}

//if we want to manage things with our sound players, it's here
void SoundManager::ManagePlayers() {

	CleanInactivePlayers(sound_players);

	for (auto& soundPlayer : sound_players) {

		auto parameters = soundPlayer->GetParameters();
		bool updateParameters = false;

		if (parameters.dynamic_source_location3d) {
			auto source_location3d = parameters.source_location3d;
			parameters.source_location3d = *parameters.dynamic_source_location3d;
			updateParameters = source_location3d != parameters.source_location3d;
		}
		if (!parameters.is_2d) {
			auto obstruction_flags = parameters.obstruction_flags;
			parameters.obstruction_flags = GetSoundObstructionFlags(parameters.identifier, &parameters.source_location3d);
			updateParameters = updateParameters || obstruction_flags != parameters.obstruction_flags;
		} else if (parameters.stereo_parameters.is_panning && parameters.source_identifier != NONE) { //only occurs when 3D sounds is disabled
			auto stereo_parameters = parameters.stereo_parameters;
			SoundVolumes variables;
			CalculateInitialSoundVariables(parameters.identifier, &parameters.source_location3d, variables);
			parameters.stereo_parameters.gain_global = variables.volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_left = variables.left_volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_right = variables.right_volume * 1.f / MAXIMUM_SOUND_VOLUME;
			updateParameters = updateParameters || stereo_parameters != parameters.stereo_parameters;
		}

		if (updateParameters) soundPlayer->UpdateParameters(parameters);
	}
}

void SoundManager::UpdateListener()
{
	if (!active || !(parameters.flags & _3d_sounds_flag)) return;
	auto listener = _sound_listener_proc();
	if (listener && *listener != OpenALManager::Get()->GetListener()) OpenALManager::Get()->UpdateListener(*listener);
}

void SoundManager::Idle()
{
	if (!active || OpenALManager::Get()->IsPaused()) return;

	UpdateListener();
	CauseAmbientSoundSourceUpdate();
	ManagePlayers();
}

void SoundManager::CauseAmbientSoundSourceUpdate()
{
	if (parameters.volume_db > MINIMUM_VOLUME_DB && (parameters.flags & _ambient_sound_flag))
	{
		UpdateAmbientSoundSources();
	}
}

struct ambient_sound_data
{
	uint16 flags;
	short sound_index;
	SoundManager::SoundVolumes variables;
};

static sound_behavior_definition *get_sound_behavior_definition(
	const short sound_behavior_index)
{
	return GetMemberWithBounds(sound_behavior_definitions,sound_behavior_index,NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS);
}

uint16 SoundManager::GetSoundObstructionFlags(short sound_index, world_location3d* source)
{
	uint16 returnedFlags = 0;

	if (!source)
		return returnedFlags;

	SoundDefinition* definition = GetSoundDefinition(sound_index);
	if (!definition) return returnedFlags;
	struct sound_behavior_definition *behavior= get_sound_behavior_definition(definition->behavior_index);
	// LP change: idiot-proofing
	if (!behavior) return returnedFlags;
	
	auto flags = _sound_obstructed_proc(source, static_cast<bool>(parameters.flags & _3d_sounds_flag));

	if ((flags&_sound_was_obstructed) && !(definition->flags&_sound_cannot_be_obstructed))
	{
		returnedFlags |= _sound_was_obstructed;
	}
	if ((flags & _sound_was_media_obstructed) && !(definition->flags & _sound_cannot_be_media_obstructed))
	{
		returnedFlags |= _sound_was_media_obstructed;
	}	
	if ((flags&_sound_was_media_muffled) && !(definition->flags&_sound_cannot_be_media_obstructed))
	{
		returnedFlags |= _sound_was_media_muffled;
	}
	
	return returnedFlags;
}

static short distance_to_volume(
	SoundDefinition* definition,
	world_distance distance,
	uint16 flags)
{
	struct sound_behavior_definition* behavior = get_sound_behavior_definition(definition->behavior_index);
	// LP change: idiot-proofing
	if (!behavior) return 0; // Silence

	struct depth_curve_definition* depth_curve;
	short volume;

	if (((flags & _sound_was_obstructed) && !(definition->flags & _sound_cannot_be_obstructed)) ||
		((flags & _sound_was_media_obstructed) && !(definition->flags & _sound_cannot_be_media_obstructed)))
	{
		depth_curve = &behavior->obstructed_curve;
	}
	else
	{
		depth_curve = &behavior->unobstructed_curve;
	}

	if (distance <= depth_curve->maximum_volume_distance)
	{
		volume = depth_curve->maximum_volume;
	}
	else
	{
		if (distance > depth_curve->minimum_volume_distance)
		{
			volume = depth_curve->minimum_volume;
		}
		else
		{
			volume = depth_curve->minimum_volume - ((depth_curve->minimum_volume - depth_curve->maximum_volume) * (depth_curve->minimum_volume_distance - distance)) /
				(depth_curve->minimum_volume_distance - depth_curve->maximum_volume_distance);
		}
	}

	if ((flags & _sound_was_media_muffled) && !(definition->flags & _sound_cannot_be_media_obstructed))
	{
		volume >>= 1;
	}

	return volume;
}

static ambient_sound_definition *get_ambient_sound_definition(
	const short ambient_sound_index)
{
	return GetMemberWithBounds(ambient_sound_definitions,ambient_sound_index,NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
}

void SoundManager::AddOneAmbientSoundSource(ambient_sound_data *ambient_sounds, world_location3d *source, world_location3d *listener, short ambient_sound_index, short absolute_volume)
{
	if (ambient_sound_index == NONE) return;

	// LP change; make NONE in case this sound definition is invalid
	struct ambient_sound_definition* SoundDef = get_ambient_sound_definition(ambient_sound_index);
	short sound_index = (SoundDef) ? SoundDef->sound_index : NONE;

	if (sound_index == NONE) return;

	SoundDefinition* definition = SoundManager::instance()->GetSoundDefinition(sound_index);

	// LP change: idiot-proofing
	if (!definition || definition->sound_code == NONE) return;

	struct sound_behavior_definition* behavior = get_sound_behavior_definition(definition->behavior_index);
	// LP change: idiot-proofing
	if (!behavior) return; // Silence

	struct ambient_sound_data* ambient;
	short distance = 0;
	short i;

	if (source)
	{
		distance = distance3d(&listener->point, &source->point);
	}

	for (i = 0, ambient = ambient_sounds;
		i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS;
		++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient))
		{
			if (ambient->sound_index == sound_index) break;
		}
		else
		{
			MARK_SLOT_AS_USED(ambient);

			ambient->sound_index = sound_index;
			ambient->variables.volume = ambient->variables.left_volume = ambient->variables.right_volume = 0;
			break;
		}
	}

	if (i == MAXIMUM_PROCESSED_AMBIENT_SOUNDS) return;
	if (source && distance >= behavior->unobstructed_curve.minimum_volume_distance) return;

	short volume, left_volume, right_volume;

	if (source)
	{
		// LP change: made this long-distance friendly
		int32 dx = int32(listener->point.x) - int32(source->point.x);
		int32 dy = int32(listener->point.y) - int32(source->point.y);

		volume = distance_to_volume(definition, distance, _sound_obstructed_proc(source));
		volume = (absolute_volume * volume) >> MAXIMUM_SOUND_VOLUME_BITS;

		if (dx || dy)
		{
			AngleAndVolumeToStereoVolume(arctangent(dx, dy) - listener->yaw, volume, &right_volume, &left_volume);
		}
		else
		{
			left_volume = right_volume = volume;
		}
	}
	else
	{
		volume = left_volume = right_volume = absolute_volume;
	}

	short maximum_volume = MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, volume);
	short maximum_left_volume = MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, left_volume);
	short maximum_right_volume = MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, right_volume);

	ambient->variables.volume = CEILING(ambient->variables.volume + volume, maximum_volume);
	ambient->variables.left_volume = CEILING(ambient->variables.left_volume + left_volume, maximum_left_volume);
	ambient->variables.right_volume = CEILING(ambient->variables.right_volume + right_volume, maximum_right_volume);
}

struct random_sound_definition *get_random_sound_definition(
	const short random_sound_index)
{
	return GetMemberWithBounds(random_sound_definitions,random_sound_index,NUMBER_OF_RANDOM_SOUND_DEFINITIONS);
}

short SoundManager::RandomSoundIndexToSoundIndex(short random_sound_index)
{
	random_sound_definition *definition = get_random_sound_definition(random_sound_index);
	return definition ? definition->sound_index : NONE;
}

SoundManager::Parameters::Parameters() :
	volume_db(DEFAULT_SOUND_LEVEL_DB),
	flags(_more_sounds_flag | _dynamic_tracking_flag | _ambient_sound_flag | _16bit_sound_flag),
	rate(DEFAULT_RATE),
	samples(DEFAULT_SAMPLES),
	music_db(DEFAULT_MUSIC_LEVEL_DB),
	video_export_volume_db(DEFAULT_VIDEO_EXPORT_VOLUME_DB),
	channel_type(ChannelType::_stereo)
{
}

bool SoundManager::Parameters::Verify()
{
	if (volume_db < MINIMUM_VOLUME_DB)
	{
		volume_db = MINIMUM_VOLUME_DB;
	}
	else if (volume_db > MAXIMUM_VOLUME_DB)
	{
		volume_db = MAXIMUM_VOLUME_DB;
	}
	
	return true;
}

SoundManager::SoundManager() : active(false), initialized(false), sounds(new SoundMemoryManager(10 << 20)) 
{ 
	
}

void SoundManager::SetStatus(bool active)
{
	if (!initialized) return;

	if (active) 
	{
		sounds->Clear();
		uint32 total_buffer_size;

		if (parameters.flags & _more_sounds_flag)
			total_buffer_size = MORE_SOUND_BUFFER_SIZE;
		else
			total_buffer_size = MINIMUM_SOUND_BUFFER_SIZE;
		if (parameters.flags & _ambient_sound_flag)
			total_buffer_size += AMBIENT_SOUND_BUFFER_SIZE;
		if (parameters.flags & _16bit_sound_flag)
		{
			total_buffer_size *= 2;
		}

		total_buffer_size *= 16;

		sounds->SetMaxSize(total_buffer_size);
				
		sound_source = (parameters.flags & _16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;

		if (shell_options.nosound) return;

		const AudioParameters audio_parameters = {
			parameters.rate,
			parameters.samples,
            parameters.channel_type,
			!(parameters.flags & _lower_restart_delay),
            static_cast<bool>(parameters.flags & _hrtf_flag),
            static_cast<bool>(parameters.flags & _3d_sounds_flag),
			From_db(parameters.volume_db),
			From_db(parameters.music_db, true)
		};

		if (!OpenALManager::Init(audio_parameters)) return;

		OpenALManager::Get()->Start();
	}
	else
	{
		if (OpenALManager::Get()) OpenALManager::Get()->Stop();
	}

	this->active = active;
}

std::shared_ptr<SoundPlayer> SoundManager::UpdateExistingPlayer(const Sound& sound, const SoundParameters& soundParameters, float simulatedVolume)
{
	//We have to play a sound, but let's find out first if we don't have a player with the source we would need
	if (soundParameters.flags & _sound_does_not_self_abort) return std::shared_ptr<SoundPlayer>();

	auto existingPlayer = GetSoundPlayer(soundParameters.identifier, soundParameters.source_identifier, !(parameters.flags & _3d_sounds_flag) || (soundParameters.flags & _sound_cannot_be_restarted));

	if (existingPlayer) {

		if (soundParameters.soft_rewind || (!(soundParameters.flags & _sound_cannot_be_restarted) &&
			(existingPlayer->CanFastRewind(soundParameters) || simulatedVolume + abortAmplitudeThreshold > SoundPlayer::Simulate(existingPlayer->GetParameters())))) {
			existingPlayer->AskRewind(soundParameters, sound); //we found one, we won't create another player but rewind this one instead
		}
	}

	return existingPlayer;
}

SoundDefinition* SoundManager::GetSoundDefinition(short sound_index)
{
	SoundDefinition* sound_definition = sound_file->GetSoundDefinition(sound_source, sound_index);
	if (sound_source == _16bit_22k_source && sound_definition && sound_definition->permutations == 0)
	{
		sound_definition = sound_file->GetSoundDefinition(_8bit_22k_source, sound_index);
	}

	return sound_definition;
}

std::shared_ptr<SoundPlayer> SoundManager::ManageSound(const Sound& sound, const SoundParameters& parameters)
{
	auto returnedPlayer = std::shared_ptr<SoundPlayer>();
	float simulatedVolume = SoundPlayer::Simulate(parameters);
	if (simulatedVolume <= 0) return returnedPlayer;
	auto existingPlayer = UpdateExistingPlayer(sound, parameters, simulatedVolume);
	if (existingPlayer) return existingPlayer;
	returnedPlayer = OpenALManager::Get()->PlaySound(sound, parameters);
	if (returnedPlayer) sound_players.insert(returnedPlayer);
	return returnedPlayer;
}

std::shared_ptr<SoundPlayer> SoundManager::BufferSound(SoundParameters& parameters)
{
	auto returnedPlayer = std::shared_ptr<SoundPlayer>();
	SoundDefinition* definition = GetSoundDefinition(parameters.identifier);
	if (!definition || !definition->permutations)
		return returnedPlayer;

	int permutation = GetRandomSoundPermutation(parameters.identifier);

	assert(permutation >= 0 && permutation < definition->permutations);

	SoundInfo header;

	SoundOptions* SndOpts = SoundReplacements::instance()->GetSoundOptions(parameters.identifier, permutation);
	if (SndOpts && SndOpts->Sound.length)
	{
		header = SndOpts->Sound;
	}
	else
	{
		header = sound_file->GetSoundHeader(definition, permutation);
	}

	auto sound = sounds->Get(parameters.identifier, permutation);
	if (sound.get())
	{
		parameters.pitch = CalculatePitchModifier(parameters.identifier, parameters.pitch);
		parameters.flags |= definition->flags;
		parameters.behavior = (sound_behavior)definition->behavior_index;

		return ManageSound({ header, sound }, parameters);
	}

	return returnedPlayer;
}

float SoundManager::CalculatePitchModifier(short sound_index, _fixed pitch_modifier)
{
	SoundDefinition *definition = GetSoundDefinition(sound_index);

	if (definition && !(definition->flags & _sound_cannot_change_pitch))
	{
		if (!(definition->flags & _sound_resists_pitch_changes))
		{
			pitch_modifier += ((FIXED_ONE-pitch_modifier)>>1);
		}
	}
	else
	{
		pitch_modifier = FIXED_ONE;
	}

	return pitch_modifier * 1.f / _normal_frequency;
}

void SoundManager::AngleAndVolumeToStereoVolume(angle delta, short volume, short *right_volume, short *left_volume)
{
	if (parameters.channel_type != ChannelType::_mono)
	{
		short fraction = delta & ((1<<(ANGULAR_BITS-2))-1);
		short maximum_volume = volume + (volume >> 1);
		short minimum_volume = volume >> 2;
		short middle_volume = volume - minimum_volume;

		switch (NORMALIZE_ANGLE(delta)>>(ANGULAR_BITS-2))
		{
		case 0: // rear right quarter [v,vmax] [v,vmin]
			*left_volume= middle_volume + ((fraction*(maximum_volume-middle_volume))>>(ANGULAR_BITS-2));
			*right_volume= middle_volume + ((fraction*(minimum_volume-middle_volume))>>(ANGULAR_BITS-2));
			break;
			
		case 1: // front right quarter [vmax,vmid] [vmin,vmid]
			*left_volume= maximum_volume + ((fraction*(volume-maximum_volume))>>(ANGULAR_BITS-2));
			*right_volume= minimum_volume + ((fraction*(volume-minimum_volume))>>(ANGULAR_BITS-2));
			break;
			
		case 2: // front left quarter [vmid,vmin] [vmid,vmax]
			*left_volume= volume + ((fraction*(minimum_volume-volume))>>(ANGULAR_BITS-2));
			*right_volume= volume + ((fraction*(maximum_volume-volume))>>(ANGULAR_BITS-2));
			break;
			
		case 3: // rear left quarter [vmin,v] [vmax,v]
			*left_volume= minimum_volume + ((fraction*(middle_volume-minimum_volume))>>(ANGULAR_BITS-2));
			*right_volume= maximum_volume + ((fraction*(middle_volume-maximum_volume))>>(ANGULAR_BITS-2));
			break;
			
		default:
			assert(false);
			break;

		}
	}
	else
	{
		*left_volume = *right_volume = volume;
	}
}

short SoundManager::GetRandomSoundPermutation(short sound_index)
{
	SoundDefinition *definition = GetSoundDefinition(sound_index);
	if (!definition) return 0;

	short permutation;

	if (!(definition->permutations > 0)) return 0;

	if (parameters.flags & _more_sounds_flag)
	{
		if ((definition->permutations_played & ((1<<definition->permutations)-1))==((1<<definition->permutations)-1)) 
			definition->permutations_played = 0;
		permutation = local_random() % definition->permutations;
		while (definition->permutations_played & (1 << permutation)) 
			if ((permutation += 1) >= definition->permutations) 
				permutation = 0;
		definition->permutations_played |= 1 << permutation;
	}
	else
	{
		permutation = 0;
	}

	return permutation;
}

static void add_one_ambient_sound_source(struct ambient_sound_data *ambient_sounds,
					 world_location3d *source, world_location3d *listener, short sound_index,
					 short absolute_volume)
{
	SoundManager::instance()->AddOneAmbientSoundSource(ambient_sounds, source, listener, sound_index, absolute_volume);
}

void SoundManager::CleanInactivePlayers(std::set<std::shared_ptr<SoundPlayer>>& players) {
	auto iterator = players.begin();
	while (iterator != players.end()) {
		if (!(*iterator)->IsActive())
			iterator = players.erase(iterator);
		else
			iterator++;
	}
}

void SoundManager::UpdateAmbientSoundSources()
{
	CleanInactivePlayers(ambient_sound_players);
	ambient_sound_data ambient_sounds[MAXIMUM_PROCESSED_AMBIENT_SOUNDS];

	// reset all local copies
	for (short i = 0; i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS; i++)
	{
		ambient_sounds[i].flags = 0;
		ambient_sounds[i].sound_index = NONE;
	}

	// accumulate up to MAXIMUM_PROCESSED_AMBIENT_SOUNDS worth of sounds
	_sound_add_ambient_sources_proc(&ambient_sounds, add_one_ambient_sound_source);

	// remove all zero volume sounds
	for (short i = 0; i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS; i++)
	{
		ambient_sound_data* ambient = &ambient_sounds[i];
		if (SLOT_IS_USED(ambient) && !ambient->variables.volume)
			MARK_SLOT_AS_FREE(ambient);
	}

	{
		ambient_sound_data* lowest_priority;
		short count;

		do
		{
			lowest_priority = 0;
			count = 0;

			for (short i = 0; i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS; i++)
			{
				ambient_sound_data* ambient = &ambient_sounds[i];
				if (SLOT_IS_USED(ambient))
				{
					if (!lowest_priority || lowest_priority->variables.volume > ambient->variables.volume + ABORT_AMPLITUDE_THRESHHOLD)
					{
						lowest_priority = ambient;
					}

					count++;
				}
			}

			if (count > MAXIMUM_AMBIENT_SOUND_CHANNELS)
			{
				assert(lowest_priority);
				MARK_SLOT_AS_FREE(lowest_priority);
				count--;
			}
		} while (count > MAXIMUM_AMBIENT_SOUND_CHANNELS);

	}

	for (auto sound = ambient_sound_players.begin(); sound != ambient_sound_players.end(); sound++) { //we are playing a sound that shouldn't be played anymore, stop the looping sound
		bool found = false;
		auto &soundPlayer = *sound;
		for (int i = 0; i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS; i++) {
			if (SLOT_IS_USED(&ambient_sounds[i]) && soundPlayer->GetIdentifier() == ambient_sounds[i].sound_index) {
				found = true;
				break;
			}
		}

		if (!found) {
			soundPlayer->AskSoftStop();
		}
	}

	// update ambient sounds already playing and add new ones if necessary
	for (short i = 0; i < MAXIMUM_PROCESSED_AMBIENT_SOUNDS; i++)
	{
		if (SLOT_IS_FREE(&ambient_sounds[i])) continue;

		auto soundPlayer = GetSoundPlayer(ambient_sounds[i].sound_index, NONE);

		if (soundPlayer && soundPlayer->HasActiveRewind())
		{
			auto parameters = soundPlayer->GetParameters();
			auto stereo_parameters = parameters.stereo_parameters;
			parameters.stereo_parameters.gain_global = ambient_sounds[i].variables.volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_left = ambient_sounds[i].variables.left_volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_right = ambient_sounds[i].variables.right_volume * 1.f / MAXIMUM_SOUND_VOLUME;

			if (stereo_parameters != parameters.stereo_parameters) {
				soundPlayer->UpdateParameters(parameters);
				soundPlayer->UpdateRewindParameters(parameters);
			}
		}
		else if (LoadSound(ambient_sounds[i].sound_index)) {
			SoundParameters parameters;
			parameters.identifier = ambient_sounds[i].sound_index;
			parameters.soft_rewind = true;
			parameters.soft_start = true;
			parameters.pitch = FIXED_ONE;
			parameters.flags = ambient_sounds[i].flags;
			parameters.stereo_parameters.is_panning = true;
			parameters.stereo_parameters.gain_global = ambient_sounds[i].variables.volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_left = ambient_sounds[i].variables.left_volume * 1.f / MAXIMUM_SOUND_VOLUME;
			parameters.stereo_parameters.gain_right = ambient_sounds[i].variables.right_volume * 1.f / MAXIMUM_SOUND_VOLUME;

			auto ambientSound = BufferSound(parameters);
			if (ambientSound) {
				ambient_sound_players.insert(ambientSound);
			}
		}
	}
}


void SoundManager::CalculateSoundVariables(short sound_index, world_location3d* source, SoundVolumes& variables)
{
	SoundDefinition* definition = GetSoundDefinition(sound_index);
	if (!definition) return;

	world_location3d* listener = _sound_listener_proc();

	if (source && listener)
	{
		world_distance distance = distance3d(&source->point, &listener->point);

		// LP change: made this long-distance friendly
		int32 dx = int32(listener->point.x) - int32(source->point.x);
		int32 dy = int32(listener->point.y) - int32(source->point.y);

		// calculate the relative volume due to the given depth curve
		variables.volume = distance_to_volume(definition, distance, _sound_obstructed_proc(source));

		if (dx || dy)
		{

			// set volume, left_volume, right_volume
			AngleAndVolumeToStereoVolume(arctangent(dx, dy) - listener->yaw, variables.volume, &variables.right_volume, &variables.left_volume);
		}
		else
		{
			variables.left_volume = variables.right_volume = variables.volume;
		}
	}

}

void SoundManager::CalculateInitialSoundVariables(short sound_index, world_location3d* source, SoundVolumes& variables)
{
	SoundDefinition* definition = GetSoundDefinition(sound_index);
	if (!definition) return;

	if (!source)
	{
		variables.volume = variables.left_volume = variables.right_volume = MAXIMUM_SOUND_VOLUME;
	}

	// and finally, do all the stuff we regularly do ...
	CalculateSoundVariables(sound_index, source, variables);
}

// List of sounds

// Extra formerly-hardcoded sounds and their accessors; this is done for M1 compatibility:
// Added Ian-Rickard-style interface commands for button actions (map resizing, resolution changes, ...)

static short _Sound_TerminalLogon = _snd_computer_interface_logon;
static short _Sound_TerminalLogoff = _snd_computer_interface_logout;
static short _Sound_TerminalPage = _snd_computer_interface_page;

static short _Sound_TeleportIn = _snd_teleport_in;
static short _Sound_TeleportOut = _snd_teleport_out;

static short _Sound_GotPowerup = _snd_got_powerup;
static short _Sound_GotItem = _snd_got_item;

static short _Sound_Crunched = _snd_body_being_crunched;
static short _Sound_Exploding = _snd_juggernaut_exploding;

static short _Sound_Breathing = _snd_breathing;
static short _Sound_OxygenWarning = _snd_oxygen_warning;

static short _Sound_AdjustVolume = _snd_adjust_volume;

static short _Sound_ButtonSuccess = _snd_computer_interface_page;
static short _Sound_ButtonFailure = _snd_absorbed;
static short _Sound_ButtonInoperative = _snd_cant_toggle_switch;
static short _Sound_OGL_Reset = _snd_juggernaut_exploding;
static short _Sound_Center_Button = _snd_owl;


short Sound_TerminalLogon() {return _Sound_TerminalLogon;}
short Sound_TerminalLogoff() {return _Sound_TerminalLogoff;}
short Sound_TerminalPage() {return _Sound_TerminalPage;}

short Sound_TeleportIn() {return _Sound_TeleportIn;}
short Sound_TeleportOut() {return _Sound_TeleportOut;}

short Sound_GotPowerup() {return _Sound_GotPowerup;}
short Sound_GotItem() {return _Sound_GotItem;}

short Sound_Crunched() {return _Sound_Crunched;}
short Sound_Exploding() {return _Sound_Exploding;}

short Sound_Breathing() {return _Sound_Breathing;}
short Sound_OxygenWarning() {return _Sound_OxygenWarning;}

short Sound_AdjustVolume() {return _Sound_AdjustVolume;}

short Sound_ButtonSuccess() {return _Sound_ButtonSuccess;}
short Sound_ButtonFailure() {return _Sound_ButtonFailure;}
short Sound_ButtonInoperative() {return _Sound_ButtonInoperative;}
short Sound_OGL_Reset() {return _Sound_OGL_Reset;}
short Sound_Center_Button() {return _Sound_Center_Button;}

// XML elements for parsing sound specifications
struct ambient_sound_definition *original_ambient_sound_definitions = NULL;
struct random_sound_definition *original_random_sound_definitions = NULL;

extern int16 dialog_sound_definitions[];
extern int16* original_dialog_sound_definitions;
extern int number_of_dialog_sounds();

void reset_mml_sounds()
{
	if (original_ambient_sound_definitions) {
		for (int i = 0; i < NUMBER_OF_AMBIENT_SOUND_DEFINITIONS; i++)
			ambient_sound_definitions[i] = original_ambient_sound_definitions[i];
		free(original_ambient_sound_definitions);
		original_ambient_sound_definitions = NULL;
	}
	if (original_random_sound_definitions) {
		for (int i = 0; i < NUMBER_OF_RANDOM_SOUND_DEFINITIONS; i++)
			random_sound_definitions[i] = original_random_sound_definitions[i];
		free(original_random_sound_definitions);
		original_random_sound_definitions = NULL;
	}
	if (original_dialog_sound_definitions)
	{
		for (int i = 0; i < number_of_dialog_sounds(); ++i)
		{
			dialog_sound_definitions[i] = original_dialog_sound_definitions[i];
		}
		free(original_dialog_sound_definitions);
		original_dialog_sound_definitions = 0;
	}
	SoundReplacements::instance()->Reset();
}

void parse_mml_sounds(const InfoTree& root)
{
	// back up old values first
	if (!original_ambient_sound_definitions) {
		original_ambient_sound_definitions = (struct ambient_sound_definition *) malloc(sizeof(struct ambient_sound_definition) * NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
		assert(original_ambient_sound_definitions);
		for (int i = 0; i < NUMBER_OF_AMBIENT_SOUND_DEFINITIONS; i++)
			original_ambient_sound_definitions[i] = ambient_sound_definitions[i];
	}
	if (!original_random_sound_definitions) {
		original_random_sound_definitions = (struct random_sound_definition *) malloc(sizeof(struct random_sound_definition) * NUMBER_OF_RANDOM_SOUND_DEFINITIONS);
		assert(original_random_sound_definitions);
		for (int i = 0; i < NUMBER_OF_RANDOM_SOUND_DEFINITIONS; i++)
			original_random_sound_definitions[i] = random_sound_definitions[i];
	}
	if (!original_dialog_sound_definitions)
	{
		original_dialog_sound_definitions = (int16*) malloc(sizeof(int16) * number_of_dialog_sounds());
		for (int i = 0; i < number_of_dialog_sounds(); ++i)
		{
			original_dialog_sound_definitions[i] = dialog_sound_definitions[i];
		}
	}
	
	root.read_attr("terminal_logon", _Sound_TerminalLogon);
	root.read_attr("terminal_logoff", _Sound_TerminalLogoff);
	root.read_attr("terminal_page", _Sound_TerminalPage);
	root.read_attr("teleport_in", _Sound_TeleportIn);
	root.read_attr("teleport_out", _Sound_TeleportOut);
	root.read_attr("got_powerup", _Sound_GotPowerup);
	root.read_attr("got_item", _Sound_GotItem);
	root.read_attr("crunched", _Sound_Crunched);
	root.read_attr("exploding", _Sound_Exploding);
	root.read_attr("breathing", _Sound_Breathing);
	root.read_attr("oxygen_warning", _Sound_OxygenWarning);
	root.read_attr("adjust_volume", _Sound_AdjustVolume);
	root.read_attr("button_success", _Sound_ButtonSuccess);
	root.read_attr("button_failure", _Sound_ButtonFailure);
	root.read_attr("button_inoperative", _Sound_ButtonInoperative);
	root.read_attr("ogl_reset", _Sound_OGL_Reset);
	root.read_attr("center_button", _Sound_Center_Button);
	
	for (const InfoTree &ambient : root.children_named("ambient"))
	{
		int16 index;
		if (!ambient.read_indexed("index", index, NUMBER_OF_AMBIENT_SOUND_DEFINITIONS))
			continue;
		ambient.read_indexed("sound", ambient_sound_definitions[index].sound_index, SHRT_MAX+1, true);
	}
	for (const InfoTree &random : root.children_named("random"))
	{
		int16 index;
		if (!random.read_indexed("index", index, NUMBER_OF_RANDOM_SOUND_DEFINITIONS))
			continue;
		random.read_indexed("sound", random_sound_definitions[index].sound_index, SHRT_MAX+1, true);
	}
	for (const InfoTree &dialog : root.children_named("dialog"))
	{
		int16 index;
		if (!dialog.read_indexed("index", index, number_of_dialog_sounds()))
			continue;
		dialog.read_indexed("sound", dialog_sound_definitions[index], SHRT_MAX+1, true);
	}
	
	// external sounds: set or clear in order
	for (const InfoTree::value_type &v : root)
	{
		std::string name = v.first;
		if (name == "sound_clear")
		{
			SoundReplacements::instance()->Reset();
		}
		else if (name == "sound")
		{
			InfoTree external = v.second;
			int16 index;
			if (!external.read_attr("index", index))
				continue;
			
			int16 slot = 0;
			external.read_indexed("slot", slot, MAXIMUM_PERMUTATIONS_PER_SOUND);
			
			SoundOptions data;
			data.File = FileSpecifier();
			std::string filename;
			if (external.read_attr("file", filename))
				data.File.SetNameWithPath(filename.c_str());

			SoundReplacements::instance()->Add(data, index, slot);
		}
	}
}

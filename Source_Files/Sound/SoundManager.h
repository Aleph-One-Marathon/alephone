#ifndef __SOUNDMANAGER_H
#define __SOUNDMANAGER_H

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

#include "cseries.h"
#include "FileHandler.h"
#include "SoundFile.h"
#include "world.h"
#include "SoundPlayer.h"
#include <set>

struct ambient_sound_data;

class SoundMemoryManager;

class SoundManager
{
public:
	// master and music volumes are now in dB
	static constexpr float DEFAULT_SOUND_LEVEL_DB = -8.f;
	static constexpr float MAXIMUM_VOLUME_DB = 0.f;
	static constexpr float MINIMUM_VOLUME_DB = -40.f;
	static constexpr float DEFAULT_MUSIC_LEVEL_DB = -12.f;
	static constexpr float DEFAULT_VIDEO_EXPORT_VOLUME_DB = -8.f;
	static constexpr int MAX_SOUNDS_FOR_SOURCE = 3;
	
	static inline SoundManager* instance() { 
		static SoundManager *m_instance = 0;
		if (!m_instance) m_instance = new SoundManager; 
		return m_instance; 
	}

	struct Parameters;
	void Initialize(const Parameters&);
	void SetParameters(const Parameters&);
	void Shutdown();

	bool OpenSoundFile(FileSpecifier& File);
	void CloseSoundFile();

	bool AdjustVolumeUp(short sound_index = NONE);
	bool AdjustVolumeDown(short sound_index = NONE);

	bool LoadSound(short sound);
	void LoadSounds(short *sounds, short count);

	void UnloadSound(short sound);
	void UnloadAllSounds();

	std::shared_ptr<SoundPlayer> PlaySound(LoadedResource& rsrc, const SoundParameters& parameters);
	std::shared_ptr<SoundPlayer> PlaySound(short sound_index, world_location3d *source, short identifier, _fixed pitch = _normal_frequency, bool soft_rewind = false);
	std::shared_ptr<SoundPlayer> DirectPlaySound(short sound_index, angle direction, short volume, _fixed pitch);

	void StopSound(short identifier, short sound_index);
	void StopAllSounds();

	void UpdateListener();

	void Idle();

	class Pause
	{
	public:
		Pause() { instance()->SetStatus(false); }
		~Pause() { instance()->SetStatus(true); }
	};

	// ambient sounds
	void CauseAmbientSoundSourceUpdate();
	void AddOneAmbientSoundSource(ambient_sound_data *ambient_sounds, world_location3d *source, world_location3d *listener, short ambient_sound_index, short absolute_volume);

	// random sounds
	short RandomSoundIndexToSoundIndex(short random_sound_index);

	static uint64_t GetCurrentAudioTick();
	static float From_db(float db, bool music = false) { return db <= (SoundManager::MINIMUM_VOLUME_DB / (music ? 2 : 1)) ? 0 : std::pow(10.f, db / 20.f); }

	struct Parameters
	{
		static const int DEFAULT_RATE = 44100;
		static const int DEFAULT_SAMPLES = 1024;
		float volume_db; // db
		uint16 flags; // dynamic_tracking, etc. 
		
		uint16 rate; // in Hz
		uint16 samples; // size of buffer

		float music_db; // music volume in dB

		float video_export_volume_db;

		ChannelType channel_type;

		Parameters();
		bool Verify();
	} parameters;

	struct SoundVolumes
	{
		short volume = 0, left_volume = 0, right_volume = 0;
	};

	bool IsActive() { return active; }
	bool IsInitialized() { return initialized; }

private:
	SoundManager();
	void SetStatus(bool active);
	SoundDefinition* GetSoundDefinition(short sound_index);
	std::shared_ptr<SoundPlayer> BufferSound(SoundParameters& parameters);
	float CalculatePitchModifier(short sound_index, _fixed pitch_modifier);
	void AngleAndVolumeToStereoVolume(angle delta, short volume, short *right_volume, short *left_volume);
	short GetRandomSoundPermutation(short sound_index);
	uint16 GetSoundObstructionFlags(short sound_index, world_location3d* source);
	void UpdateAmbientSoundSources();
	void ManagePlayers();
	std::set<std::shared_ptr<SoundPlayer>> sound_players;
	std::set<std::shared_ptr<SoundPlayer>> ambient_sound_players;
	bool initialized;
	bool active;
	static void CleanInactivePlayers(std::set<std::shared_ptr<SoundPlayer>>& players);
	void CalculateSoundVariables(short sound_index, world_location3d* source, SoundVolumes& variables);
	void CalculateInitialSoundVariables(short sound_index, world_location3d* source, SoundVolumes& variables);
	std::shared_ptr<SoundPlayer> GetSoundPlayer(short identifier, short source_identifier, bool sound_identifier_only = false) const;
	std::shared_ptr<SoundPlayer> UpdateExistingPlayer(const Sound& sound, const SoundParameters& soundParameters, float simulatedVolume);
	std::shared_ptr<SoundPlayer> ManageSound(const Sound& sound, const SoundParameters& parameters);
	short sound_source; // 8-bit, 16-bit
	
	std::unique_ptr<SoundFile> sound_file;
	SoundMemoryManager* sounds;

	// buffer sizes
	static const int MINIMUM_SOUND_BUFFER_SIZE = 300*KILO;
	static const int MORE_SOUND_BUFFER_SIZE = 600*KILO;
	static const int AMBIENT_SOUND_BUFFER_SIZE = 1*MEG;
	static const int MAXIMUM_SOUND_BUFFER_SIZE = 1*MEG;

	// channels
	static const int MAXIMUM_AMBIENT_SOUND_CHANNELS = 4;
	static const int MAXIMUM_PROCESSED_AMBIENT_SOUNDS = 5;

	// volumes
	// MAXIMUM_SOUND_VOLUME is 256, NUMBER_OF_SOUND_VOLUME_LEVELS is 8
	static const int MAXIMUM_OUTPUT_SOUND_VOLUME = 2 * MAXIMUM_SOUND_VOLUME;
	static const int SOUND_VOLUME_DELTA = MAXIMUM_OUTPUT_SOUND_VOLUME / NUMBER_OF_SOUND_VOLUME_LEVELS;
	static const int MAXIMUM_AMBIENT_SOUND_VOLUME = 3 * MAXIMUM_SOUND_VOLUME / 2;
	static const int DEFAULT_VOLUME_WHILE_SPEAKING = MAXIMUM_SOUND_VOLUME / 8;

	// pitch
	static const int MINIMUM_SOUND_PITCH= 1;
	static const int MAXIMUM_SOUND_PITCH= 256*FIXED_ONE;

	// best channel
	static const int ABORT_AMPLITUDE_THRESHHOLD= (MAXIMUM_SOUND_VOLUME/6);
	static const int MINIMUM_RESTART_TICKS= MACHINE_TICKS_PER_SECOND/12;

	// channel flags
	static const int _sound_is_local = 0x0001;
};

/* ---------- types */

typedef void (*add_ambient_sound_source_proc_ptr)(ambient_sound_data *ambient_sounds,
	world_location3d *source, world_location3d *listener, short sound_index,
	short absolute_volume);

/* ---------- external prototypes */

/* _sound_listener_proc() gives the location and facing of the listener at any point in time;
	what are the alternatives to providing this function? */
world_location3d *_sound_listener_proc(void);

/* _sound_obstructed_proc() tells whether the given sound is obstructed or not */
uint16 _sound_obstructed_proc(world_location3d *source, bool distinguish_obstruction_types = false);

void _sound_add_ambient_sources_proc(void *data, add_ambient_sound_source_proc_ptr add_one_ambient_sound_source);

// Accessors for remaining formerly hardcoded sounds:

short Sound_TerminalLogon();
short Sound_TerminalLogoff();
short Sound_TerminalPage();

short Sound_TeleportIn();
short Sound_TeleportOut();

short Sound_GotPowerup();
short Sound_GotItem();

short Sound_Crunched();
short Sound_Exploding();

short Sound_Breathing();
short Sound_OxygenWarning();

short Sound_AdjustVolume();

// LP: Ian-Rickard-style commands for interface buttons

short Sound_ButtonSuccess();
short Sound_ButtonFailure();
short Sound_ButtonInoperative();
short Sound_OGL_Reset();
short Sound_Center_Button();

class InfoTree;
void parse_mml_sounds(const InfoTree& root);
void reset_mml_sounds();

#endif

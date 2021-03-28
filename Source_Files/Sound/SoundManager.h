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

#include "SoundManagerEnums.h"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

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
	void TestVolume(float db, short sound_index);

	bool LoadSound(short sound);
	void LoadSounds(short *sounds, short count);

	void OrphanSound(short identifier);

	void UnloadSound(short sound);
	void UnloadAllSounds();

	void PlaySound(short sound_index, world_location3d *source, short identifier, _fixed pitch = _normal_frequency);
	void PlayLocalSound(short sound_index, _fixed pitch = _normal_frequency) { PlaySound(sound_index, 0, NONE, pitch); }
	void DirectPlaySound(short sound_index, angle direction, short volume, _fixed pitch);
	bool SoundIsPlaying(short sound_index);

	void StopSound(short identifier, short sound_index);
	void StopAllSounds() { StopSound(NONE, NONE); }

	inline int16 GetNetmicVolumeAdjustment() {
		return (parameters.volume_while_speaking);
	}

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

	struct Parameters
	{
		static const int DEFAULT_RATE = 44100;
		static const int DEFAULT_SAMPLES = 1024;
		int16 channel_count; // >= 0
		float volume_db; // db
		uint16 flags; // stereo, dynamic_tracking, etc. 
		
		uint16 rate; // in Hz
		uint16 samples; // size of buffer

		float music_db; // music volume in dB

		int16 volume_while_speaking; // [0, NUMBER_OF_SOUND_VOLUME_LEVELS)
		bool mute_while_transmitting;

		float video_export_volume_db;

		Parameters();
		bool Verify();
	} parameters;

	struct Channel
	{
		uint16 flags;
		
		short sound_index; // sound_index being played in this channel
		short identifier; // unique sound identifier for the sound being played in this channel (object_index)
		struct Variables
		{
			short volume, left_volume, right_volume;
			_fixed original_pitch, pitch;
			
			short priority;
		} variables; // variables of the sound being played

		world_location3d *dynamic_source; // can be NULL for immobile sounds
		world_location3d source; // must be valid

		uint32 start_tick;

		int mixer_channel;
		short callback_count;
	};

	void IncrementChannelCallbackCount(int channel) { channels[channel].callback_count++; } // fix this

	bool IsActive() { return active; }
	bool IsInitialized() { return initialized; }

private:
	SoundManager();
	void SetStatus(bool active);

	SoundDefinition* GetSoundDefinition(short sound_index);
	void BufferSound(Channel &, short sound_index, _fixed pitch, bool ext_play_immed = true);

	Channel *BestChannel(short sound_index, Channel::Variables& variables);
	void FreeChannel(Channel &);

	void UnlockLockedSounds();

	void CalculateSoundVariables(short sound_index, world_location3d *source, Channel::Variables& variables);
	void CalculateInitialSoundVariables(short sound_index, world_location3d *source, Channel::Variables& variables, _fixed pitch);
	void InstantiateSoundVariables(Channel::Variables& variables, Channel& channel, bool first_time);

	_fixed CalculatePitchModifier(short sound_index, _fixed pitch_modifier);
	void AngleAndVolumeToStereoVolume(angle delta, short volume, short *right_volume, short *left_volume);

	short GetRandomSoundPermutation(short sound_index);

	void TrackStereoSounds();
	void UpdateAmbientSoundSources();

	bool initialized;
	bool active;

	short total_channel_count;

	short sound_source; // 8-bit, 16-bit
	
	std::vector<Channel> channels;

	boost::scoped_ptr<SoundFile> sound_file;
	SoundMemoryManager* sounds;

	// buffer sizes
	static const int MINIMUM_SOUND_BUFFER_SIZE = 300*KILO;
	static const int MORE_SOUND_BUFFER_SIZE = 600*KILO;
	static const int AMBIENT_SOUND_BUFFER_SIZE = 1*MEG;
	static const int MAXIMUM_SOUND_BUFFER_SIZE = 1*MEG;

	// channels
	static const int MAXIMUM_SOUND_CHANNELS = 32;
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
uint16 _sound_obstructed_proc(world_location3d *source);

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

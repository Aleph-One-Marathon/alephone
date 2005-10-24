/*
SOUND.C

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

Friday, March 26, 1993 10:16:00 AM

Friday, March 26, 1993 10:16:01 AM
	this is the source file that will make minotaur’s sound manager look like a truckload of dead
	rats in a tampon factory.
Saturday, March 27, 1993 8:13:44 PM
	a few things have been changed, i think most of the coding is done.
Tuesday, April 6, 1993 9:26:54 PM
	is there any way to use SyncCmd’s to better synchronize the left and right channels of
	stereo sound?  why do all sounds end in a click when the sound manager is doing realtime
	sound mixing?
Tuesday, May 18, 1993 9:35:21 PM
	ha ha ha.  after wwdc93, hacking out jim reekes’ new stereo amplitude command, rewriting all
	of this crap so it’ll actually work like you might expect.  fixed a bug where the sample
	rate wasn’t restored after drinking the potion.  we used to have tons of problems in our
	amplitude calculations (that was why it didn’t initially work under the new sound driver).
Sunday, July 4, 1993 7:56:21 AM
	there exists a bug such that for every channel (especially the monster channel) there is
	a possibility that the channel will stop playing sounds but report no errors.
Friday, July 16, 1993 6:04:11 PM
	the last_played field of a sound definition was not being set by play_sound, so the sound
	which was first loaded will be disposed of first.  would that manifest in the ‘pause bug’?
	alex has a version of Suitcase which was marking all the sounds I loaded as purgable.
Friday, July 23, 1993 9:43:39 PM
	the pause bug was SCSI Probe’s COMMAND-SPACE hotkey.  reduced sound space to 250k.  i can’t
	wait to rewrite this and make it real.
Saturday, November 20, 1993 10:32:34 PM
	‘making this real’ means making it a true n-channel sound manager, where n is some function
	of the current cpu.  this means throwing away all that channel code garbage.
Sunday, January 2, 1994 11:21:41 AM
	sound_code was not being set in play_sound; added threshhold to best_channel
Friday, August 19, 1994 8:26:31 PM
	added custom depth-fading curves, which respect obstructions, and removed the MoveHHi
	before every sound is played.  this is bad news, because we might have sounds locked down
	in our heap when we tried to load shapes, other sounds, etc.  we try to improve this
	situation slightly with the addition of the sound_manager_idle_proc (to unlock handles)
	and by asking everyone who really needs memory to call stop_all_sounds, which will unlock
	the sound handles.
Friday, September 2, 1994 1:29:03 PM
	moving to slot-base collection model fixed a gnarly reentrancy bug while releasing sounds.
Tuesday, September 6, 1994 2:53:35 PM
	added _depth_fading_flag and _high_quality_flag (resamples 22k to 11k if false).
Sunday, November 6, 1994 9:17:24 PM  (Jason)
	if the sound manager is never initialized, calling any of it’s procedures doesn’t crash.
Friday, December 2, 1994 5:26:10 PM  (Jason)
	old= CurResFile(); UseResFile(_sm_sound_file_handle); Get1Resource('snd', ...); UseResFile(old);
Saturday, December 10, 1994 4:32:46 PM  (Jason)
	in search of the golden sample rate: changing rate22khz to 0x56220000 on PowerMacs didn’t
	seem to make any difference.
Friday, January 27, 1995 12:49:25 AM  (Jason')
	the marathon 2 rewrite begins.
Friday, June 16, 1995 10:18:42 AM  (Jason)
	background, ambient and scenery sounds.
Friday, July 7, 1995 12:47:22 PM  (Jason)
	_sound_cannot_be_aborted, sounds which are not always played
Thursday, August 17, 1995 10:41:51 AM  (Jason)
	fixed sound pitches.
Tuesday, August 29, 1995 8:56:16 AM  (Jason)
	moved macintosh-specific code to SOUND_MACINTOSH.C.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 23, 2000 (Loren Petrich):
	Increased maximum number of sound channels from 4 to 8;
	also changed the resource-fork menu.

May 27, 2000 (Loren Petrich):
	Fixed directionality bug for long-distance sounds;
	made some distance calculations more long-distance friendly

Jun 3, 2000 (Loren Petrich):
	Idiot-proofed the sound accessors; when a sound index is out of range,
	an accessor returns NULL.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 26, 2000 (Loren Petrich):
	Moved get_default_sounds_spec() to preprocess_map_mac.c

Sep 2, 2000 (Loren Petrich):
	Added unpacking of the sound header and also
	generalized the sound-data size from (static ) NUMBER_OF_SOUND_DEFINITIONS
	to (per-file) number_of_sound_definitions -- also updated _sm_globals when
	reading in a file, since the definition pointer and count will vary.

Sep 23, 2000 (Loren Petrich):
	Added XML support for changing the ambient and random sound definitions,
	in order to support Shebob's Pfh'Joueur

Oct 14, 2000 (Loren Petrich):
	Added music-volume stuff

Apr 8, 2001 (Loren Petrich):
	Added support for loading external sounds

Apr 26, 2001 (Loren Petrich):
	Got that support working

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
*/

/*
sound pitches do not work

there should be no difference between ambient and normal sound channels
shortening radii on low-volume ambient sound sorces would be a good idea
*/

#include "cseries.h"

#include <stdlib.h>

#ifdef mac
#if defined(EXPLICIT_CARBON_HEADER)
    #include <quicktime/Quicktime.h>
#else
// #include <Gestalt.h>
#include <Movies.h>
#endif
#endif

#include "shell.h"

#include "world.h"
#include "interface.h"
#include "mysound.h"
#include "game_errors.h"
#include "FileHandler.h"
#include "Packing.h"

#include <string.h>

#ifdef env68k
#pragma segment sound
#endif

/* ---------- constants */

enum
{
	MAXIMUM_OUTPUT_SOUND_VOLUME= 2*MAXIMUM_SOUND_VOLUME,
	SOUND_VOLUME_DELTA= MAXIMUM_OUTPUT_SOUND_VOLUME/NUMBER_OF_SOUND_VOLUME_LEVELS,
	DEFAULT_SOUND_LEVEL= NUMBER_OF_SOUND_VOLUME_LEVELS/3,
	
	// LP addition:
	DEFAULT_MUSIC_LEVEL = NUMBER_OF_SOUND_VOLUME_LEVELS/2,

	ABORT_AMPLITUDE_THRESHHOLD= (MAXIMUM_SOUND_VOLUME/6),
	MINIMUM_RESTART_TICKS= MACHINE_TICKS_PER_SECOND/12,
	
	// LP change: increase maximum number of sound channels and also
	// ambient ones
	MAXIMUM_SOUND_CHANNELS= 8,
	MAXIMUM_AMBIENT_SOUND_CHANNELS= 4,

	MINIMUM_SOUND_PITCH= 1,
	MAXIMUM_SOUND_PITCH= 256*FIXED_ONE
};

enum /* channel flags */
{
	_sound_is_local= 0x0001 // .source is invalid
};

/* ---------- macros */

/* from marathon map.h */
#define SLOT_IS_USED(o) ((o)->flags&(uint16)0x8000)
#define SLOT_IS_FREE(o) (!SLOT_IS_USED(o))
#define MARK_SLOT_AS_FREE(o) ((o)->flags&=(uint16)~0x8000)
#define MARK_SLOT_AS_USED(o) ((o)->flags|=(uint16)0x8000)

#define AMBIENT_SOUND_CHANNELS (_sm_globals->channels + _sm_parameters->channel_count)

/* ---------- structures */


// Handling of loaded external-file sounds:
struct LoadedSound
{

	// Loaded-sound object
#ifdef mac
	Movie QTSnd;
#endif
	
	// Whether it is queued to be used
	bool Queued;
	
	LoadedSound()
	{
#ifdef mac
		QTSnd = NULL;
		Queued = false;
#endif
	}

	~LoadedSound() {Unload();}

	// These functions return whether the operation succeeded or failed,
	// with the exceptions being the "is" functions.

	bool Load(FileSpecifier& File);
	bool Unload();
	bool IsLoaded();
	
	bool Play();
	bool Stop();
	bool IsPlaying();
	
	// Necessary for classic MacOS; real OSes like MacOS X ought not to need this
	bool Update();
	
	// Control:
	// The pitch and volume values are in fixed-integer units (FIXED_ONE = 1.0)
	bool SetPitch(_fixed Pitch);
	bool SetVolume(_fixed Left, _fixed Right);
};


// Sound-options stuff:
struct SoundOptions
{
	// The filename, including the path, in Unix style
	vector<char> File;
	
	LoadedSound Sound;
};


struct SoundOptionsEntry
{
	short Index;
	short Slot;
	SoundOptions OptionsData;
};


#ifdef mac
// Get the sound-options record for a sound index and a sound slot (or permutation)
// Will return NULL if there is no such record;
// use the native-Marathon soundfile for the sound and use the bare MacOS Sound Manager
static SoundOptions *GetSoundOptions(short Index, short Slot);
#endif


struct sound_variables
{
	short volume, left_volume, right_volume;
	_fixed original_pitch, pitch;
	
	short priority;
};

struct channel_data
{
	uint16 flags;

	short sound_index; /* sound_index being played in this channel */

	short identifier; /* unique sound identifier for the sound being played in this channel (object_index) */
	struct sound_variables variables; /* variables of the sound being played */	
	world_location3d *dynamic_source; /* can be NULL for immobile sounds */
	world_location3d source; /* must be valid */

	unsigned long start_tick;

	LoadedSound *SndPtr;	// For external-file sounds; set whenever a sound is active

#if defined(mac)
	SndChannelPtr channel;
#elif defined(SDL)
	void *channel;
#endif
	short callback_count;
};

struct sound_manager_globals
{
	short total_channel_count;
	long total_buffer_size;

	short sound_source; // 8-bit, 16-bit
	struct sound_definition *base_sound_definitions, *used_sound_definitions;
	
	uint16 available_flags;
	
	long loaded_sounds_size;
	
	struct channel_data channels[MAXIMUM_SOUND_CHANNELS+MAXIMUM_AMBIENT_SOUND_CHANNELS];

#ifdef mac
	long old_sound_volume;
	SndCallBackUPP sound_callback_upp;
#endif
};


/* ---------- globals */

static bool _sm_initialized, _sm_active;
static struct sound_manager_globals *_sm_globals;
static struct sound_manager_parameters *_sm_parameters;

// Moved out of _sm_globals so it could work correctly;
// there is apparently some sort of buffer overrun somwhere.
static OpenedFile SoundFile;

/* include globals */
#include "sound_definitions.h"

// List of sound-options records
// and a hash table for them
static vector<SoundOptionsEntry> SOList;
static vector<int16> SOHash;

// Hash table for sounds
const int SOHashSize = 1 << 8;
const int SOHashMask = SOHashSize - 1;
inline uint8 SOHashFunc(short Index, short Slot)
{
	// This function will avoid collisions when accessing sounds with the same index
	// and different slots (permutations)
	return (uint8)(Index ^ (Slot << 4));
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

/* ---------- machine-specific prototypes */

static void initialize_machine_sound_manager(struct sound_manager_parameters *parameters);

static bool channel_busy(struct channel_data *channel);
static void unlock_sound(short sound_index);
static void dispose_sound(short sound_index);
// Returns not the handle, but the pointer and size
static byte *read_sound_from_file(short sound_index, int32 &size);

static void quiet_channel(struct channel_data *channel);
static void instantiate_sound_variables(struct sound_variables *variables,
	struct channel_data *channel, bool first_time);
	// LP: extra: whether to start playing an external sound immediately
static void buffer_sound(struct channel_data *channel, short sound_index, _fixed pitch, bool ExtPlayImmed=true);

/* ---------- private prototypes */

static void track_stereo_sounds(void);
static void unlock_locked_sounds(void);

static bool _load_sound(short sound);
static short _release_least_useful_sound(void);

static struct channel_data *best_channel(short sound_index, struct sound_variables *variables);
static void free_channel(struct channel_data *channel);

static short get_random_sound_permutation(short sound_index);

static void calculate_initial_sound_variables(short sound_index, world_location3d *source,
	struct sound_variables *variables, _fixed pitch_modifier);
static void calculate_sound_variables(short sound_index, world_location3d *source,
	struct sound_variables *variables);
static _fixed calculate_pitch_modifier(short sound_index, _fixed pitch_modifier);

static void angle_and_volume_to_stereo_volume(angle delta, short volume, short *right_volume, short *left_volume);
static short distance_to_volume(struct sound_definition *definition, world_distance distance,
	uint16 flags);

static void update_ambient_sound_sources(void);

/* ---------- private code */

// LP change: inlined all these accessors; they return NULL for an invalid index value:

struct sound_definition *get_sound_definition(
	const short sound_index)
{
	sound_definition *rv = GetMemberWithBounds(_sm_globals->used_sound_definitions,sound_index,number_of_sound_definitions);
	if (_sm_globals->sound_source == _16bit_22k_source && rv && rv->permutations == 0) rv = GetMemberWithBounds(_sm_globals->base_sound_definitions,sound_index,number_of_sound_definitions);
	return rv;
}

struct ambient_sound_definition *get_ambient_sound_definition(
	const short ambient_sound_index)
{
	return GetMemberWithBounds(ambient_sound_definitions,ambient_sound_index,NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
}

struct random_sound_definition *get_random_sound_definition(
	const short random_sound_index)
{
	return GetMemberWithBounds(random_sound_definitions,random_sound_index,NUMBER_OF_RANDOM_SOUND_DEFINITIONS);
}

struct sound_behavior_definition *get_sound_behavior_definition(
	const short sound_behavior_index)
{
	return GetMemberWithBounds(sound_behavior_definitions,sound_behavior_index,NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS);
}


/* ---------- machine-specific code */

#if defined(mac)
#include "sound_macintosh.h"
#elif defined(SDL)
#include "sound_sdl.h"
//void	calc_buffer(int16* a, int b, bool c);
//void	calc_buffer(int8* a, int b, bool c);
#endif

/* ---------- code */

void initialize_sound_manager(
	struct sound_manager_parameters *parameters)
{
	_sm_globals= new sound_manager_globals;
	_sm_parameters= new sound_manager_parameters;
	// sound_definitions= new sound_definition[NUMBER_OF_SOUND_SOURCES*NUMBER_OF_SOUND_DEFINITIONS];
	assert(_sm_globals && _sm_parameters);
	// assert(_sm_globals && _sm_parameters && sound_definitions);

	obj_clear(*_sm_globals);
	obj_clear(*_sm_parameters);
	// objlist_clear(sound_definitions, NUMBER_OF_SOUND_SOURCES*NUMBER_OF_SOUND_DEFINITIONS);
	
	initialize_machine_sound_manager(parameters);
}

bool open_sound_file(FileSpecifier& File)
{
	// LP: rewrote the whole g*dd*mn thing
	if (!_sm_globals) return false;
	
	if (!File.Open(SoundFile)) return false;
	
	// Read the header:
	
	uint8 HeaderBuffer[SIZEOF_sound_file_header];
	struct sound_file_header header;
	if (!SoundFile.Read(SIZEOF_sound_file_header,HeaderBuffer))
	{
		SoundFile.Close();
		return false;
	}
	
	// Unpack it
	uint8 *S = HeaderBuffer;
	
	StreamToValue(S,header.version);
	StreamToValue(S,header.tag);
	StreamToValue(S,header.source_count);
	StreamToValue(S,header.sound_count);
	
	S += 124*2;

	assert((S - HeaderBuffer) == SIZEOF_sound_file_header);
	
	if ((header.version != 0 && header.version != SOUND_FILE_VERSION) ||
		header.tag != SOUND_FILE_TAG ||
		header.sound_count < 0 ||
		header.source_count < 0)
	{
		SoundFile.Close();
		return false;
	}
	
	// Version 0 data file (M2 demo)
	if (header.sound_count == 0) {
		header.sound_count = header.source_count;
		header.source_count = 1;
	}

	long DefsBufferSize = header.source_count*header.sound_count*SIZEOF_sound_definition;
	uint8 *DefsBuffer = new uint8[DefsBufferSize];
	assert(DefsBuffer);
	if (!SoundFile.Read(DefsBufferSize,DefsBuffer))
	{
		SoundFile.Close();
		return false;
	}
	
	// Unlike the buffer, all sound sources must be present here
	if (sound_definitions) delete []sound_definitions;
	number_of_sound_definitions = header.sound_count;
	int TotalSoundDefs = NUMBER_OF_SOUND_SOURCES*number_of_sound_definitions;
	sound_definitions = new sound_definition[TotalSoundDefs];
	assert(sound_definitions);
	
	// Set the sounds to silence (which unloaded ones will be)
	objlist_clear(sound_definitions,TotalSoundDefs);
	for (int k=0; k<TotalSoundDefs; k++)
		sound_definitions[k].sound_code = NONE;
	
	// Unpack them!
	S = DefsBuffer;
	int Count = PIN(header.source_count,0,NUMBER_OF_SOUND_SOURCES)*number_of_sound_definitions;
	
	sound_definition* ObjPtr = sound_definitions;
	
	for (int k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->sound_code);
		
		StreamToValue(S,ObjPtr->behavior_index);
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->chance);
		
		StreamToValue(S,ObjPtr->low_pitch);
		StreamToValue(S,ObjPtr->high_pitch);
		
		StreamToValue(S,ObjPtr->permutations);
		StreamToValue(S,ObjPtr->permutations_played);
		StreamToValue(S,ObjPtr->group_offset);
		StreamToValue(S,ObjPtr->single_length);
		StreamToValue(S,ObjPtr->total_length);
		StreamToList(S,ObjPtr->sound_offsets,MAXIMUM_PERMUTATIONS_PER_SOUND);
		
		StreamToValue(S,ObjPtr->last_played);
		
		S += 4*2;
	}
	
	assert((S - DefsBuffer) == Count*SIZEOF_sound_definition);
	
	delete []DefsBuffer;
	
	// LP: code copied from sound_macintosh.cpp;
	// keeps the _sm_globals values in sync with what had been most recently read.
	_sm_globals->sound_source= (_sm_parameters->flags&_16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;
	if (header.source_count == 1)
		_sm_globals->sound_source = _8bit_22k_source;
	_sm_globals->base_sound_definitions= sound_definitions;
	_sm_globals->used_sound_definitions= sound_definitions + (_sm_globals->sound_source == _16bit_22k_source)?sound_definitions:0;
		
	// Load MML resources in file
	// Be sure to ignore not-found errors
#if defined(mac)
	short SavedType, SavedError = get_game_error(&SavedType);
	XML_LoadFromResourceFork(File);
	set_game_error(SavedType,SavedError);
#endif
		
	return true;
}

static void close_sound_file(void)
{
	SoundFile.Close();
}

void load_sound(
	short sound)
{
	if (_sm_active)
	{
		if (sound!=NONE)
		{
			_load_sound(sound);
		}
	}
}

void load_sounds(
	short *sounds,
	short count)
{
	short i;
	
	for (i= 0; i<count; ++i)
	{
		load_sound(sounds[i]);
	}
}

void sound_manager_idle_proc(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		unlock_locked_sounds();
		track_stereo_sounds();
		cause_ambient_sound_source_update();
		
#ifdef mac
		// Update QuickTime-played sounds
		for (int ic = 0; ic<MAXIMUM_SOUND_CHANNELS+MAXIMUM_AMBIENT_SOUND_CHANNELS; ++ic)
		{
			channel_data *channel = _sm_globals->channels + ic;
			LoadedSound *SndPtr = channel->SndPtr;
			if (!SndPtr) continue;
			
			// If an external sound had been queued, play it only if the channel is idle;
			// test for that by counting callbacks
			if (SndPtr->Queued && channel->callback_count > 0)
			{
				// SCStatus Status;
				// SndChannelStatus(channel->channel,sizeof(SCStatus),&Status);
				// if (!Status.scChannelBusy)
				SndPtr->Play();
				SndPtr->Queued = false;
			} else {
				if (!SndPtr->Update()) continue;
				if (!SndPtr->IsPlaying())
				{
					// Blank out inactive sound and make a fake callback
					channel->callback_count++;
					channel->SndPtr = NULL;
				}
			}
		}	
#endif
	}
}

void cause_ambient_sound_source_update(
	void)
{
	if (_sm_active && _sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		if (_sm_parameters->flags&_ambient_sound_flag)
		{
			update_ambient_sound_sources();
		}
	}
}

void direct_play_sound(
	short sound_index,
	angle direction, // can be NONE
	short volume,
	_fixed pitch)
{
	/* don’t do anything if we’re not initialized or active, or our sound_code is NONE,
		or our volume is zero, our we have no sound channels */
	if (sound_index!=NONE && _sm_active && sound_index<number_of_sound_definitions &&
		_sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		struct sound_variables variables;
		struct channel_data *channel;

		/* make sure the sound data is in memory */
		if (_load_sound(sound_index))
		{
			/* get the channel, and free it for our new sound */
			if ((channel= best_channel(sound_index, &variables))!=NULL)
			{
				world_location3d *listener= _sound_listener_proc();
				
				variables.priority= 0;
				variables.volume= volume;
				
				if (direction==NONE || !listener)
				{
					variables.left_volume= variables.right_volume= volume;
				}
				else
				{
					angle_and_volume_to_stereo_volume(direction - listener->yaw,
						volume, &variables.right_volume, &variables.left_volume);
				}
		
				/* set the volume and pitch in this channel */
				instantiate_sound_variables(&variables, channel, true);

				/* initialize the channel */
				channel->flags= _sound_is_local; // but possibly being played in stereo
				channel->callback_count= 0; // #MD
				channel->start_tick= machine_tick_count();
				channel->sound_index= sound_index;
				channel->identifier= NONE;
				channel->dynamic_source= (world_location3d *) NULL;
				MARK_SLOT_AS_USED(channel);

				/* start the sound playing */
				buffer_sound(channel, sound_index, pitch);
			}
		}
	}
}

void _play_sound(
	short sound_index,
	world_location3d *source,
	short identifier, /* NONE is no identifier and the sound is immediately orphaned */
	_fixed pitch) /* on top of all existing pitch modifiers */
{
	/* don’t do anything if we’re not initialized or active, or our sound_code is NONE,
		or our volume is zero, our we have no sound channels */
	if (sound_index!=NONE && _sm_active && sound_index<number_of_sound_definitions &&
		_sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		struct sound_variables variables;
		struct channel_data *channel;

		calculate_initial_sound_variables(sound_index, source, &variables, pitch);
		
		/* make sure the sound data is in memory */
		if (_load_sound(sound_index))
		{
			/* get the channel, and free it for our new sound */
			if ((channel= best_channel(sound_index, &variables))!=NULL)
			{
				/* set the volume and pitch in this channel */
				instantiate_sound_variables(&variables, channel, true);
				
				/* initialize the channel */
				channel->flags= 0;
				channel->callback_count= 0; // #MD
				channel->start_tick= machine_tick_count();
				channel->sound_index= sound_index;
				channel->identifier= identifier;
				channel->dynamic_source= (identifier==NONE) ? (world_location3d *) NULL : source;
				MARK_SLOT_AS_USED(channel);
				
				/* start the sound playing */
				buffer_sound(channel, sound_index, pitch);

				/* if we have a valid source, copy it, otherwise remember that we don’t */
				if (source)
				{
					channel->source= *source;
				}
				else
				{
					channel->flags|= _sound_is_local;
				}
			}
		}
	}
}

void unload_all_sounds(
	void)
{
	if (_sm_active)
	{
		stop_all_sounds();
		
		while (_release_least_useful_sound()!=NONE)
			;
		
		for (vector<SoundOptionsEntry>::iterator SOIter = SOList.begin(); SOIter < SOList.end(); SOIter++)
			SOIter->OptionsData.Sound.Unload();
	}
}

void stop_sound(
	short identifier,
	short sound_index)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;

		// if we’re stopping everything...
		if (identifier==NONE && sound_index==NONE)
		{
			// can’t fade to silence here
			
			// stop the ambient sound channels, too
			if (_sm_parameters->flags&_ambient_sound_flag)
			{
				for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
				{
					free_channel(channel);
				}
			}
		}

		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && (channel->identifier==identifier || identifier==NONE) &&
				(channel->sound_index==sound_index || sound_index==NONE))
			{
				free_channel(channel);
			}
		}
	}
	
	return;
 }

// doesn’t check ambient sounds
bool sound_is_playing(
	short sound_index)
{
	bool sound_playing= false;
	
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel->sound_index==sound_index)
			{
				sound_playing= true;
			}
		}
		
		unlock_locked_sounds();
	}
	
	return sound_playing;
}

void orphan_sound(
	short identifier)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (channel->identifier==identifier || identifier==NONE)
			{
				channel->dynamic_source= (world_location3d *) NULL;
				channel->identifier= NONE;
			}
		}
	}
}

uint16 available_sound_manager_flags(
	uint16 flags)
{
	uint16 available_flags= _sm_active ? _sm_globals->available_flags : 0;
	
	if (!(flags&_stereo_flag)) available_flags&= ~(uint16)_dynamic_tracking_flag;
	
	return available_flags;
}

void default_sound_manager_parameters(
	void *prefs)
{
	struct sound_manager_parameters *parameters=(struct sound_manager_parameters *)prefs;

	obj_clear(*parameters);
	
	parameters->channel_count= MAXIMUM_SOUND_CHANNELS;
	parameters->volume= DEFAULT_SOUND_LEVEL;
	// LP: adopted CB's(?) added flags
	parameters->flags= _more_sounds_flag | _stereo_flag | _dynamic_tracking_flag | _ambient_sound_flag | _16bit_sound_flag;
	parameters->pitch= FIXED_ONE;
	// LP addition:
	parameters->music = DEFAULT_MUSIC_LEVEL;
}

bool verify_sound_manager_parameters(
	struct sound_manager_parameters *parameters)
{
	// pin parameters
	parameters->channel_count= PIN(parameters->channel_count, 0, MAXIMUM_SOUND_CHANNELS);
	parameters->volume= PIN(parameters->volume, 0, NUMBER_OF_SOUND_VOLUME_LEVELS);
	parameters->pitch= PIN(parameters->pitch, MINIMUM_SOUND_PITCH, MAXIMUM_SOUND_PITCH);

	// adjust flags	
	parameters->flags&= _sm_globals->available_flags;
	
	return true;
}

short random_sound_index_to_sound_index(
	short random_sound_index)
{
	struct random_sound_definition *definition= get_random_sound_definition(random_sound_index);
	
	// LP change: make NONE in case this sound definition is invalid
	if (definition)
		return definition->sound_index;
	else
		return NONE;
}

/* ---------- private code */


static void unlock_locked_sounds(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		// if we're done playing a locked sound, dispose it
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && !channel_busy(channel))
			{
				free_channel(channel);
			}
		}
	}
}

static void track_stereo_sounds(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0 && (_sm_parameters->flags&_dynamic_tracking_flag))
	{
		short i;
		struct channel_data *channel;
		
		// if we're done playing a locked sound, dispose it
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel_busy(channel) && !(channel->flags&_sound_is_local))
			{
				struct sound_variables variables= channel->variables;
				
				if (channel->dynamic_source) channel->source= *channel->dynamic_source;
				calculate_sound_variables(channel->sound_index, &channel->source, &variables);
				instantiate_sound_variables(&variables, channel, false);
			}
		}
	}
}

static short get_random_sound_permutation(
	short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return 0;
	short permutation;
	
	// LP change: idiot-proofing
	if (!(definition->permutations>0)) return 0;
	// assert(definition->permutations>0);

	if (_sm_parameters->flags&_more_sounds_flag)
	{
		if ((definition->permutations_played&((1<<definition->permutations)-1))==((1<<definition->permutations)-1)) definition->permutations_played= 0;
		permutation= local_random()%definition->permutations;
		while (definition->permutations_played & (1<<permutation)) if ((permutation+= 1)>=definition->permutations) permutation= 0;
		definition->permutations_played|= 1<<permutation;
	}
	else
	{
		permutation= 0;
	}

	definition->last_played= machine_tick_count();

	return permutation;
}

static struct channel_data *best_channel(
	short sound_index,
	struct sound_variables *variables)
{
	short i;
	struct channel_data *channel;
	struct channel_data *best_channel;
	struct sound_definition *definition= get_sound_definition(sound_index);
	if (!definition) return NULL;
	
	best_channel= (struct channel_data *) NULL;
	if (!definition->chance || (local_random()>definition->chance))
	{
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel_busy(channel))
			{
				/* if this channel is at a lower volume than the sound we are trying to play and is at the same
					priority, or the channel is at a lower priority, then we can abort it */
				if ((channel->variables.volume<=variables->volume+ABORT_AMPLITUDE_THRESHHOLD && channel->variables.priority==variables->priority) ||
					channel->variables.priority<variables->priority)
				{
					/* if this channel is already playing our sound, this is our channel (for
						better or for worse) */
					if (channel->sound_index==sound_index)
					{
						if (definition->flags&_sound_cannot_be_restarted)
						{
							best_channel= (struct channel_data *) NULL;
							break;
						}
						
						if (!(definition->flags&_sound_does_not_self_abort))
						{
							best_channel= (channel->start_tick+MINIMUM_RESTART_TICKS<machine_tick_count()) ? channel : (struct channel_data *) NULL;
							break;
						}
					}
					
					/* if we haven’t found an alternative channel or this channel is at a lower
						volume than our previously best channel (which isn’t an unused channel),
						then we’ve found a new best channel */
					if (!best_channel ||
						(SLOT_IS_USED(best_channel) && best_channel->variables.volume>channel->variables.volume) ||
						(SLOT_IS_USED(best_channel) && best_channel->variables.priority<channel->variables.priority))
					{
						best_channel= channel;
					}
				}
				else
				{
					if (channel->sound_index==sound_index && !(definition->flags&_sound_does_not_self_abort))
					{
						/* if we’re already playing this sound at a higher volume, don’t abort it */
						best_channel= (struct channel_data *) NULL;
						break;
					}
				}
			}
			else
			{
				/* unused channel (we won’t get much better than this!) */
				if (SLOT_IS_USED(channel)) free_channel(channel);
				best_channel= channel;
			}
		}
	}

	if (best_channel)
	{
		/* stop whatever sound is playing and unlock the old handle if necessary */
		free_channel(best_channel);
	}
	
	return best_channel;
}

static short _release_least_useful_sound(
	void)
{
	short sound_index, least_used_sound_index= NONE;
	struct sound_definition *least_used_definition= (struct sound_definition *) NULL;

	for (sound_index= 0; sound_index<number_of_sound_definitions; ++sound_index)
	{
		struct sound_definition *definition = get_sound_definition(sound_index);
		// if (definition->handle && (!least_used_definition || least_used_definition->last_played>definition->last_played))
		if (definition->ptr && (!least_used_definition || least_used_definition->last_played>definition->last_played))
		{
			least_used_sound_index= sound_index;
			least_used_definition= definition;
		}
	}
	
	if (least_used_sound_index!=NONE)
	{
		stop_sound(NONE, least_used_sound_index);
		dispose_sound(least_used_sound_index);
	}
	
	return least_used_sound_index;
}

/* if .sound_handle isn’t NULL then we still have a locked handle in our heap; so unlock the
	handle and clear the .sound_handle field */
static void free_channel(
	struct channel_data *empty_channel)
{
	if (SLOT_IS_USED(empty_channel))
	{
		short sound_index= empty_channel->sound_index;
		
		quiet_channel(empty_channel);
	
		assert(sound_index!=NONE);
		empty_channel->sound_index= NONE;
		MARK_SLOT_AS_FREE(empty_channel);
		
		// if anybody else is playing this sound_index, we can’t unlock the handle
		if (!sound_is_playing(sound_index)) unlock_sound(sound_index);
	}
}

bool _load_sound(
	short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return false;
		
	if (definition->sound_code!=NONE &&
		((_sm_parameters->flags&_ambient_sound_flag) || !(definition->flags&_sound_is_ambient)))
	{
		if (!definition->ptr)
		{
			definition->ptr = read_sound_from_file(sound_index, definition->size);
			definition->last_played= machine_tick_count();
			
			while (_sm_globals->loaded_sounds_size>_sm_globals->total_buffer_size) _release_least_useful_sound();
		}
		
		if (definition->ptr)
		{
			definition->permutations_played= 0;
		}
	}
	
	return definition->ptr ? true : false;
}

static void calculate_initial_sound_variables(
	short sound_index,
	world_location3d *source,
	struct sound_variables *variables,
	_fixed pitch_modifier)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;

	(void) (pitch_modifier);

	/* if we have no sound source, play at full volume */
	if (!source)
	{
		variables->volume= variables->left_volume= variables->right_volume= MAXIMUM_SOUND_VOLUME;
		//ghs: is this what the priority should be if there's no source?
		variables->priority = definition->behavior_index;
	}

	/* and finally, do all the stuff we regularly do ... */
	calculate_sound_variables(sound_index, source, variables);
}

static _fixed calculate_pitch_modifier(
	short sound_index,
	_fixed pitch_modifier)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return FIXED_ONE;

	if (!(definition->flags&_sound_cannot_change_pitch))
	{
		if (!(definition->flags&_sound_resists_pitch_changes))
		{
			pitch_modifier+= ((FIXED_ONE-pitch_modifier)>>1);
		}
	}
	else
	{
		pitch_modifier= FIXED_ONE;
	}
	
	return pitch_modifier;
}

static void calculate_sound_variables(
	short sound_index,
	world_location3d *source,
	struct sound_variables *variables)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;

	world_location3d *listener= _sound_listener_proc();

	if (source && listener)
	{
		world_distance distance= distance3d(&source->point, &listener->point);
		// LP change: made this long-distance friendly
		long dx= long(listener->point.x) - long(source->point.x);
		long dy= long(listener->point.y) - long(source->point.y);
		
		/* for now, a sound's priority is it's behavior_index */
		variables->priority= definition->behavior_index;
	
		/* calculate the relative volume due to the given depth curve */
		variables->volume= distance_to_volume(definition, distance, _sound_obstructed_proc(source));

		if (dx || dy)
		{
			/* set volume, left_volume, right_volume */
			angle_and_volume_to_stereo_volume(arctangent(dx, dy) - listener->yaw,
				variables->volume, &variables->right_volume, &variables->left_volume);
		}
		else
		{
			variables->left_volume= variables->right_volume= variables->volume;
		}
	} 
}

static void angle_and_volume_to_stereo_volume(
	angle delta,
	short volume,
	short *right_volume,
	short *left_volume)
{
	if (_sm_parameters->flags&_stereo_flag)
	{
		short fraction= delta&((1<<(ANGULAR_BITS-2))-1);
		short maximum_volume= volume + (volume>>1);
		short minimum_volume= volume>>2;
		short middle_volume= volume-minimum_volume;
		
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
		*left_volume= *right_volume= volume;
	}
}

static short distance_to_volume(
	struct sound_definition *definition,
	world_distance distance,
	uint16 flags)
{
	struct sound_behavior_definition *behavior= get_sound_behavior_definition(definition->behavior_index);
	// LP change: idiot-proofing
	if (!behavior) return 0; // Silence
	
	struct depth_curve_definition *depth_curve;
	short volume;

	if (((flags&_sound_was_obstructed) && !(definition->flags&_sound_cannot_be_obstructed)) ||
		((flags&_sound_was_media_obstructed) && !(definition->flags&_sound_cannot_be_media_obstructed)))
	{
		depth_curve= &behavior->obstructed_curve;
	}
	else
	{
		depth_curve= &behavior->unobstructed_curve;
	}
	
	if (distance<=depth_curve->maximum_volume_distance)
	{
		volume= depth_curve->maximum_volume;
	}
	else
	{
		if (distance>depth_curve->minimum_volume_distance)
		{
			volume= depth_curve->minimum_volume;
		}
		else
		{
			volume= depth_curve->minimum_volume - ((depth_curve->minimum_volume-depth_curve->maximum_volume)*(depth_curve->minimum_volume_distance-distance)) /
				(depth_curve->minimum_volume_distance-depth_curve->maximum_volume_distance);
		}
	}
	
	if ((flags&_sound_was_media_muffled) && !(definition->flags&_sound_cannot_be_media_obstructed))
	{
		volume>>= 1;
	}
	
	return volume;
}

/* ---------- ambient sound sources */

enum
{
	MAXIMUM_PROCESSED_AMBIENT_SOUNDS= 5
};

struct ambient_sound_data
{
	uint16 flags;
	short sound_index;

	struct sound_variables variables;
	
	struct channel_data *channel;
};

static void add_one_ambient_sound_source(struct ambient_sound_data *ambient_sounds,
	world_location3d *source, world_location3d *listener, short sound_index,
	short absolute_volume);

static void update_ambient_sound_sources(
	void)
{
	struct ambient_sound_data ambient_sounds[MAXIMUM_PROCESSED_AMBIENT_SOUNDS];
	struct ambient_sound_data *ambient;
	struct channel_data *channel;
	bool channel_used[MAXIMUM_AMBIENT_SOUND_CHANNELS], sound_handled[MAXIMUM_PROCESSED_AMBIENT_SOUNDS];
	short i, j;

	// reset all local copies	
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		ambient->flags= 0;
		ambient->sound_index= NONE;
		
		sound_handled[i]= false;
	}

	for (i= 0; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i)
	{
		channel_used[i]= false;
	}
	
	// accumulate up to MAXIMUM_PROCESSED_AMBIENT_SOUNDS worth of sounds
	_sound_add_ambient_sources_proc(&ambient_sounds, add_one_ambient_sound_source);

	// remove all zero volume sounds
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient) && !ambient->variables.volume) MARK_SLOT_AS_FREE(ambient);
	}

	{
		struct ambient_sound_data *lowest_priority;
		short count;
		
		do
		{
			lowest_priority= (struct ambient_sound_data *) NULL;
			count= 0;
			
			for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
			{
				if (SLOT_IS_USED(ambient))
				{
					if (!lowest_priority ||
//						lowest_priority->sound_index>ambient->sound_index ||
						lowest_priority->variables.volume>ambient->variables.volume+ABORT_AMPLITUDE_THRESHHOLD)
					{
						lowest_priority= ambient;
					}
					
					count+= 1;
				}
			}
			
			if (count>MAXIMUM_AMBIENT_SOUND_CHANNELS)
			{
				assert(lowest_priority);
				MARK_SLOT_AS_FREE(lowest_priority);
				count-= 1;
			}
		}
		while (count>MAXIMUM_AMBIENT_SOUND_CHANNELS);
	}
	
	// update .variables of those sounds which we are already playing
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient))
		{
			for (j= 0, channel= AMBIENT_SOUND_CHANNELS; j<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++j, ++channel)
			{
				if (SLOT_IS_USED(channel) && channel->sound_index==ambient->sound_index)
				{
					instantiate_sound_variables(&ambient->variables, channel, false);
					
					sound_handled[i]= channel_used[j]= true;
					
					break;
				}
			}
		}
	}
	
	// allocate a channel for a sound we just started playing
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient) && !sound_handled[i])
		{
			for (j= 0, channel= AMBIENT_SOUND_CHANNELS; j<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++j, ++channel)
			{
				if (!channel_used[j])
				{
					// take over this channel for a new sound
					if (SLOT_IS_USED(channel)) free_channel(channel);
					
					channel->flags= 0;
					channel->callback_count= 2; // #MD as if two sounds had just stopped playing
					channel->sound_index= ambient->sound_index;
					// channel->identifier
					// channel->dynamic_source
					// channel->source
					MARK_SLOT_AS_USED(channel);
					
					channel_used[j]= true;
					
					instantiate_sound_variables(&ambient->variables, channel, true);

					break;
				}
			}
		}
	}
	
	// remove those sounds which are no longer being played
	for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
	{
		if (SLOT_IS_USED(channel) && !channel_used[i])
		{
			free_channel(channel);
		}
	}

	// unlock those handles which need to be unlocked and buffer new sounds if necessary
	for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
	{
		if (SLOT_IS_USED(channel))
		{
			if (_load_sound(channel->sound_index))
			{
				// make sure at least two sounds are buffered at all times			
				while (channel->callback_count) // #MD
				{
					// LP: if an external sound is active or queued, don't do anything more
					LoadedSound *SndPtr = channel->SndPtr;
					if (SndPtr) break;
					
					buffer_sound(channel, channel->sound_index, FIXED_ONE, false);
									
					channel->callback_count-= 1;
				}
			}
		}
	}
}

enum
{
	MAXIMUM_AMBIENT_SOUND_VOLUME= 3*MAXIMUM_SOUND_VOLUME/2
};

// NULL source means directionless
static void add_one_ambient_sound_source(
	struct ambient_sound_data *ambient_sounds,
	world_location3d *source,
	world_location3d *listener,
	short ambient_sound_index,
	short absolute_volume)
{
	if (ambient_sound_index!=NONE)
	{
		// LP change; make NONE in case this sound definition is invalid
		struct ambient_sound_definition *SoundDef = get_ambient_sound_definition(ambient_sound_index);
		short sound_index = (SoundDef) ? SoundDef->sound_index : NONE;
		
		if (sound_index!=NONE)
		{
			struct sound_definition *definition= get_sound_definition(sound_index);
			
			// LP change: idiot-proofing
			if (definition)
			{
			if (definition->sound_code!=NONE)
			{
				struct sound_behavior_definition *behavior= get_sound_behavior_definition(definition->behavior_index);
				// LP change: idiot-proofing
				if (!behavior) return; // Silence
	
				struct ambient_sound_data *ambient;
				short distance = 0;
				short i;
			
				if (source)
				{
					distance= distance3d(&listener->point, &source->point);
				}
			
				for (i= 0, ambient= ambient_sounds;
					i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS;
					++i, ++ambient)
				{
					if (SLOT_IS_USED(ambient))
					{
						if (ambient->sound_index==sound_index) break;
					}
					else
					{
						MARK_SLOT_AS_USED(ambient);
						
						ambient->sound_index= sound_index;
			
						ambient->variables.priority= definition->behavior_index;
						ambient->variables.volume= ambient->variables.left_volume= ambient->variables.right_volume= 0;
			
						break;
					}
				}
				
				if (i!=MAXIMUM_PROCESSED_AMBIENT_SOUNDS)
				{
					if (!source || distance<behavior->unobstructed_curve.minimum_volume_distance)
					{
						short volume, left_volume, right_volume;
						
						if (source)
						{
							// LP change: made this long-distance friendly
							long dx= long(listener->point.x) - long(source->point.x);
							long dy= long(listener->point.y) - long(source->point.y);
							
							volume= distance_to_volume(definition, distance, _sound_obstructed_proc(source));
							volume= (absolute_volume*volume)>>MAXIMUM_SOUND_VOLUME_BITS;
							
							if (dx || dy)
							{
								angle_and_volume_to_stereo_volume(arctangent(dx, dy) - listener->yaw,
									volume, &right_volume, &left_volume);
							}
							else
							{
								left_volume= right_volume= volume;
							}
						}
						else
						{
							volume= left_volume= right_volume= absolute_volume;
						}
						
						{
							short maximum_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, volume);
							short maximum_left_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, left_volume);
							short maximum_right_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, right_volume);
	
							ambient->variables.volume= CEILING(ambient->variables.volume+volume, maximum_volume);
							ambient->variables.left_volume= CEILING(ambient->variables.left_volume+left_volume, maximum_left_volume);
							ambient->variables.right_volume= CEILING(ambient->variables.right_volume+right_volume, maximum_right_volume);
						}
					}
				}
				else
				{
//					dprintf("warning: ambient sound buffer full;g;");
				}
			}
			}
		}
	}
}


#ifdef mac

// Get the sound-options record for a sound index and a sound slot (or permutation)
// Will return NULL if there is no such record;
// use the native-Marathon soundfile for the sound and use the bare MacOS Sound Manager
static SoundOptions *GetSoundOptions(short Index, short Slot)
{

	// Initialize the hash table if necessary
	if (SOHash.empty())
	{
		SOHash.resize(SOHashSize);
		objlist_set(&SOHash[0],NONE,SOHashSize);
	}
	
	// Set up a *reference* to the appropriate hashtable entry
	int16& HashVal = SOHash[SOHashFunc(Index,Slot)];
	
	// Check to see if the sound-option entry is correct;
	// if it is, then we're done.
	if (HashVal != NONE)
	{
		vector<SoundOptionsEntry>::iterator SOIter = SOList.begin() + HashVal;
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			return &SOIter->OptionsData;
		}
	}
	
	// Fallback for the case of a hashtable miss;
	// do a linear search and then update the hash entry appropriately.
	int16 Indx = 0;
	for (vector<SoundOptionsEntry>::iterator SOIter = SOList.begin(); SOIter < SOList.end(); SOIter++, Indx++)
	{
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			HashVal = Indx;
			return &SOIter->OptionsData;
		}
	}
	
	// None found
	return NULL;
}

#endif


// XML elements for parsing sound specifications;
// this is currently what ambient and random sounds are to be used

struct ambient_sound_definition *original_ambient_sound_definitions = NULL;
struct random_sound_definition *original_random_sound_definitions = NULL;

class XML_AmbientRandomAssignParser: public XML_ElementParser
{
	int Type;
	bool IsPresent[2];
	short Index, SoundIndex;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool ResetValues();

	enum {
		Ambient,
		Random
	};
	
	XML_AmbientRandomAssignParser(char *_Name, int _Type): XML_ElementParser(_Name), Type(_Type) {}
};

bool XML_AmbientRandomAssignParser::Start()
{
	// back up old values first
	if (Type == Ambient) {
		if (!original_ambient_sound_definitions) {
			original_ambient_sound_definitions = (struct ambient_sound_definition *) malloc(sizeof(struct ambient_sound_definition) * NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
			assert(original_ambient_sound_definitions);
			for (int i = 0; i < NUMBER_OF_AMBIENT_SOUND_DEFINITIONS; i++)
				original_ambient_sound_definitions[i] = ambient_sound_definitions[i];
		}
	} else if (Type == Random) {
		if (!original_random_sound_definitions) {
			original_random_sound_definitions = (struct random_sound_definition *) malloc(sizeof(struct random_sound_definition) * NUMBER_OF_RANDOM_SOUND_DEFINITIONS);
			assert(original_random_sound_definitions);
			for (int i = 0; i < NUMBER_OF_RANDOM_SOUND_DEFINITIONS; i++)
				original_random_sound_definitions[i] = random_sound_definitions[i];
		}
	}

	for (int k=0; k<2; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_AmbientRandomAssignParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		int ArrayLimit = 0;
		switch(Type)
		{
		case Ambient:
			ArrayLimit = NUMBER_OF_AMBIENT_SOUND_DEFINITIONS;
			break;
		case Random:
			ArrayLimit = NUMBER_OF_RANDOM_SOUND_DEFINITIONS;
			break;
		default:
			vassert(false,csprintf(temporary,"Unrecognized sound-parser class: %d",Type));
		}
		if (ReadBoundedInt16Value(Value,Index,0,ArrayLimit-1))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"sound"))
	{
		if (ReadBoundedInt16Value(Value,SoundIndex,NONE,SHRT_MAX))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_AmbientRandomAssignParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<2; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	switch(Type)
	{
	case Ambient:
		ambient_sound_definitions[Index].sound_index = SoundIndex;
		break;
	case Random:
		random_sound_definitions[Index].sound_index = SoundIndex;
		break;
	default:
		vassert(false,csprintf(temporary,"Unrecognized sound-parser class: %d",Type));
	}
	return true;
}

bool XML_AmbientRandomAssignParser::ResetValues()
{
	if (Type == Ambient) {
		if (original_ambient_sound_definitions) {
			for (int i = 0; i < NUMBER_OF_AMBIENT_SOUND_DEFINITIONS; i++)
				ambient_sound_definitions[i] = original_ambient_sound_definitions[i];
			free(original_ambient_sound_definitions);
			original_ambient_sound_definitions = NULL;
		}
	} else if (Type == Random) {
		if (original_random_sound_definitions) {
			for (int i = 0; i < NUMBER_OF_RANDOM_SOUND_DEFINITIONS; i++)
				random_sound_definitions[i] = original_random_sound_definitions[i];
			free(original_random_sound_definitions);
			original_random_sound_definitions = NULL;
		}
	}
	return true;
}


static XML_AmbientRandomAssignParser
	AmbientSoundAssignParser("ambient",XML_AmbientRandomAssignParser::Ambient),
	RandomSoundAssignParser("random",XML_AmbientRandomAssignParser::Random);



class XML_SO_ClearParser: public XML_ElementParser
{
public:
	bool Start();

	XML_SO_ClearParser(): XML_ElementParser("sound_clear") {}
};

bool XML_SO_ClearParser::Start()
{
	// Clear the list
	SOList.clear();
	return true;
}


static XML_SO_ClearParser SO_ClearParser;


class XML_SoundOptionsParser: public XML_ElementParser
{
	bool IndexPresent;
	short Index, Slot;
	
	SoundOptions Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool ResetValues();

	XML_SoundOptionsParser(): XML_ElementParser("sound") {}
};

bool XML_SoundOptionsParser::Start()
{
	IndexPresent = false;
	Index = NONE;
	Slot = 0;			// Default: first slot
	Data.File.clear();	// Default: no file
	
	return true;
}

bool XML_SoundOptionsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadInt16Value(Value,Index))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"slot"))
	{
		return ReadBoundedInt16Value(Value,Slot,0,MAXIMUM_PERMUTATIONS_PER_SOUND-1);
	}
	else if (StringsEqual(Tag,"file"))
	{
		size_t nchars = strlen(Value)+1;
		Data.File.resize(nchars);
		memcpy(&Data.File[0],Value,nchars);
		return true;
	}
	UnrecognizedTag();
	return false;
}

bool XML_SoundOptionsParser::AttributesDone()
{
	// Verify...
	if (!IndexPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Check to see if a frame is already accounted for
	for (vector<SoundOptionsEntry>::iterator SOIter = SOList.begin(); SOIter < SOList.end(); SOIter++)
	{
		if (SOIter->Index == Index && SOIter->Slot == Slot)
		{
			// Replace the data
			SOIter->OptionsData = Data;
			return true;
		}
	}
	
	// If not, then add a new frame entry
	SoundOptionsEntry DataEntry;
	DataEntry.Index = Index;
	DataEntry.Slot = Slot;
	DataEntry.OptionsData = Data;
	SOList.push_back(DataEntry);
	
	return true;
}

bool XML_SoundOptionsParser::ResetValues()
{
	SOList.clear();
	return true;
}

static XML_SoundOptionsParser SoundOptionsParser;



// Subclassed to set the sounds appropriately
class XML_SoundsParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);
	
	XML_SoundsParser(): XML_ElementParser("sounds") {}
};

bool XML_SoundsParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"terminal_logon"))
	{
		return ReadInt16Value(Value,_Sound_TerminalLogon);
	}
	else if (StringsEqual(Tag,"terminal_logoff"))
	{
		return ReadInt16Value(Value,_Sound_TerminalLogoff);
	}
	else if (StringsEqual(Tag,"terminal_page"))
	{
		return ReadInt16Value(Value,_Sound_TerminalPage);
	}
	else if (StringsEqual(Tag,"teleport_in"))
	{
		return ReadInt16Value(Value,_Sound_TeleportIn);
	}
	else if (StringsEqual(Tag,"teleport_out"))
	{
		return ReadInt16Value(Value,_Sound_TeleportOut);
	}
	else if (StringsEqual(Tag,"got_powerup"))
	{
		return ReadInt16Value(Value,_Sound_GotPowerup);
	}
	else if (StringsEqual(Tag,"got_item"))
	{
		return ReadInt16Value(Value,_Sound_GotItem);
	}
	else if (StringsEqual(Tag,"crunched"))
	{
		return ReadInt16Value(Value,_Sound_Crunched);
	}
	else if (StringsEqual(Tag,"exploding"))
	{
		return ReadInt16Value(Value,_Sound_Exploding);
	}
	else if (StringsEqual(Tag,"breathing"))
	{
		return ReadInt16Value(Value,_Sound_Breathing);
	}
	else if (StringsEqual(Tag,"oxygen_warning"))
	{
		return ReadInt16Value(Value,_Sound_OxygenWarning);
	}
	else if (StringsEqual(Tag,"adjust_volume"))
	{
		return ReadInt16Value(Value,_Sound_AdjustVolume);
	}
	else if (StringsEqual(Tag,"button_success"))
	{
		return ReadInt16Value(Value,_Sound_ButtonSuccess);
	}
	else if (StringsEqual(Tag,"button_failure"))
	{
		return ReadInt16Value(Value,_Sound_ButtonFailure);
	}
	else if (StringsEqual(Tag,"button_inoperative"))
	{
		return ReadInt16Value(Value,_Sound_ButtonInoperative);
	}
	else if (StringsEqual(Tag,"ogl_reset"))
	{
		return ReadInt16Value(Value,_Sound_OGL_Reset);
	}
	UnrecognizedTag();
	return false;
}


static XML_SoundsParser SoundsParser;


// LP change: added infravision-parser export
XML_ElementParser *Sounds_GetParser()
{
	SoundsParser.AddChild(&AmbientSoundAssignParser);
	SoundsParser.AddChild(&RandomSoundAssignParser);
	SoundsParser.AddChild(&SoundOptionsParser);
	SoundsParser.AddChild(&SO_ClearParser);
	
	return &SoundsParser;
}
